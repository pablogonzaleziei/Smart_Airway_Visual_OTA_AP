#include <Arduino.h>
#include <Wire.h>
#include "esp_camera.h"
#include "EEPROM.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include <Update.h>
#include "updateIndex.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#define LENGTH(x) (strlen(x) + 1)  // length of char string
#define EEPROM_SIZE 200            // EEPROM size
#define WiFi_rst 2                 //WiFi credential reset pin (Boot button on ESP32)
#define I2C_SDA 14                 //Changing original SDA pin from 21 to 14
#define I2C_SCL 15                 //Changing original SCL pin from 22 to 15
/*
  xxx.xxx.xxx.xxx:84/updateIndex for updating the code over-the-air
  xxx.xxx.xxx.xxx:82/stream for video stream if using the original code of app_httpd.cpp
  xxx.xxx.xxx.xxx for video stream if using the reduced code of app_httpd.cpp
*/

int wifi_connected = 0;

WebServer server(84); 

String ssid;
String pass;
char *credentials[2];  //credentials[2] = {ssid, password}

//const char* ssid = "TRUCORP";
//const char* password = "Snickers";

const char *ssid_ap = "SMARTSIM";
const char *pass_ap = "";

void startCameraServer();
void cameraConfig();
void receiveEvent(int messageLength);
void sendIP();
void wifiConnectAP();
void ota();
void reset();

void setup(void) {

  Serial.begin(115200);
  Wire.begin((uint8_t)55, I2C_SDA, I2C_SCL, 400000);  // I2C with address 0x55, SDA in pin 14, SCL in pin 15, frequency(NULL)
  Wire.onReceive(receiveEvent);                       // Register event. Receive data (I2C)
  Wire.onRequest(sendIP);                             // Register event. Send data (I2C)

  pinMode(WiFi_rst, INPUT);  // Signal received from nRF52 (wire) erase EEPROM
  cameraConfig();            // Camera settings
  //wifiConnect();
  wifiConnectAP();
  ota();
  server.begin();
}

void loop(void) {
  
  while (WiFi.status() != WL_CONNECTED) {
    wifi_connected = 0;
    //wifiConnect();  // Connection to WIFI
    if (digitalRead(WiFi_rst) == HIGH) {

      reset();
    }
  }
  
  server.handleClient();
  delay(100);
}

void ota() {

  /*return index page which is stored in updateIndex */

  server.on("/updateIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send_P(200, "text/html", updateIndex);
  });

  /*handling uploading firmware file */
  server.on(
    "/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },
    []() {
      HTTPUpload &upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {  //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });
}

void cameraConfig() {

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition

  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    Serial.print("--------------------------------------------------------------------------PSRAM--------------------------------------------------------------------------------------------");
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 8;
    config.fb_count = 2;

  } else {
    Serial.print("----------------------------------------------------------------------------NO PSRAM------------------------------------------------------------------------------------------");
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 5;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;  //frame buffer go to DRAM instead of PSRAM
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void writeStringToFlash(const char *toStore, int startAddr) {
  int i = 0;
  for (; i < LENGTH(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}

String readStringFromFlash(int startAddr) {
  char in[128];  // char array of size 128 for reading the stored data
  int i = 0;
  for (; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}

void wifiConnect() {

  if (!EEPROM.begin(EEPROM_SIZE)) {  //Init EEPROM
    Serial.println("failed to init EEPROM");
    delay(1000);
  } else {
    ssid = readStringFromFlash(0);  // Read SSID stored at address 0
    Serial.print("1.SSID = ");
    Serial.println(ssid);
    pass = readStringFromFlash(40);  // Read Password stored at address 40
    Serial.print("1.PSS = ");
    Serial.println(pass);
  }
  Serial.println("+++++++++++++++++++++++++++ Credentials read from EEPROM ++++++++++++++++++++++++++++++++++++++++++");
  Serial.print(ssid);
  Serial.print("/");
  Serial.print(pass);

  WiFi.begin(ssid.c_str(), pass.c_str());
  delay(3600);

  Serial.print("_");
  Serial.println(WiFi.status());
  Serial.println("########################################################################################");

  if (WiFi.status() != WL_CONNECTED)  // if WiFi is not connected
  {
    Wire.onReceive(receiveEvent);
    Serial.print("2.SSID:");
    Serial.println(ssid);
    Serial.print("2.PSS:");
    Serial.println(pass);
    Serial.println("Store SSID & PSS in Flash");
    writeStringToFlash(ssid.c_str(), 0);   // storing ssid at address 0
    writeStringToFlash(pass.c_str(), 40);  // storing pss at address 40

  } else {
    Serial.println("WiFi Connected");
    startCameraServer();
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
    startCameraServer();
  }
}

void wifiConnectAP() {

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_ap, pass_ap);
  delay(100);
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("' to connect");
  startCameraServer();
}
void receiveEvent(int messageLength) {

  char msg[messageLength];
  for (int i = 0; i < messageLength; i++) {
    msg[i] = Wire.read();  //msg = credentials received from the main board (Format: SSID$PASSWORD$NULL)
  }

  char *token = strtok(msg, "$");  //split msg in parts(tokens) with strtok
  int i = 0;                       //For position of tokens
  while (token != NULL) {          //until null character is detected
    credentials[i++] = token;      //credentials[0] = first token
    token = strtok(NULL, "$");     //split if it detects "#"
  }

  ssid = credentials[0];
  pass = credentials[1];
}

void reset() {

  Serial.println("Reseting the WiFi credentials");
  writeStringToFlash("", 0);   // Reset the SSID
  writeStringToFlash("", 40);  // Reset the Password
  // Serial.println("Wifi credentials erased");
  Serial.println("Restarting the ESP_____________________________________________________");
  ESP.restart();  // Restart ESP
  delay(200);
}

void sendIP() {

  //if (WiFi.status() == WL_CONNECTED) {
  IPAddress localIP = WiFi.localIP();
  wifi_connected = 1;
  uint8_t Buffer[5] = { localIP[0], localIP[1], localIP[2], localIP[3], wifi_connected};
  //Serial.println(wifi_connected);
  Wire.write(Buffer, 5);  //data bytes are queued in local buffer
  delay(500);
  //}
}