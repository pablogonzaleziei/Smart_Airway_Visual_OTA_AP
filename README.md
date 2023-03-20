# SMART_AIRWAY_VISUAL

# Smart_Airway_Visual_OTA_AP

Option 1: use Aduino IDE to upload
Option 2: use VScode with Platformio in espressif32 platform and arduino framework

## Features
- Access Point: "SMARTSIM" no password. Direct connection with ESP32 (No internet access)
- SSID and Password request for internet access (same network used on the device).
- WIFI credentials storing in flash
- OTA update (needs internet access)
- Send local IP and softAPIP via I2C and connection confirmation

## Instructions
Board: AI Thinker ESP32-CAM or ESP32-wrover Module. Partition scheme: Minimal SPIFFS (1.9 MB APP with OTA)
- Connect device to the network called "SMARTSIM" if you want to use an Access Point (The device won't have internet connection)
- Access the video stream: xxx.xxx.xxx.xxx:82/stream
- Access the OTA update web server: xxx.xxx.xxx.xxx:84/updateIndex (needs internet connection)
- Receive credentials: I2C address is 0x55, the format has to be `SSID$PASSWORD$NULL` 
- Send IP: I2C address is 0x55, will be able to send a localIP and a SoftAPIP
- Reset the credentials: set pin 2 (WiFi_rst) to HIGH, direct wire or I2C (recommended)
- I2C connection:
    - `#define I2C_SDA 14 //Changing original SDA pin from 21 to 14`
    - `#define I2C_SCL 15 //Changing original SCL pin from 22 to 15`

## Add your files

- [ ] [Create](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#create-a-file) or [upload](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#upload-a-file) files
- [ ] [Add files using the command line](https://docs.gitlab.com/ee/gitlab-basics/add-file.html#add-a-file-using-the-command-line) or push an existing Git repository with the following command:

```
cd existing_repo
git remote add origin https://gitlab.com/pablogonzaleziei/smart_airway_visual.git
git branch -M main
git push -uf origin main
```
