
const char updateIndex[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
  <head>
    <title>ESP32 TRUCORP</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" type="image/png" href="LOGO.png">
    <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
  </head>
  <style>
      html {
    font-family: Arial, Helvetica, sans-serif; 
    display: inline-block; 
    text-align: center;

  }
  h1 {
    font-size: 1.8rem; 
    color: rgb(205, 213, 255);
  }
  p { 
    font-size: 1.4rem;
    padding-top: 5%;
  }
  .topnav { 
    overflow: hidden; 
    background-color: #0A1128;
  }
  body {  
    margin: 0;
  }
  .content { 
    padding: 5%;
  }
  .card-grid { 
    max-width: 500px; 
    height: 200px;
    margin:  auto; 
    padding: 10px;
    display: grid; 
    grid-gap: 2rem; 
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  }
  input[type = "submit"]{
    
    flex-direction: column;
    align-items: center;
    padding: 6px 14px;
    font-family: -apple-system, BlinkMacSystemFont, 'Roboto', sans-serif;
    border-radius: 6px;
    border: none;
  
    color: #fff;
    background: linear-gradient(180deg, #4B91F7 0%, #367AF6 100%);
     background-origin: border-box;
    box-shadow: 0px 0.5px 1.5px rgba(54, 122, 246, 0.25), inset 0px 0.8px 0px -0.25px rgba(255, 255, 255, 0.2);
    user-select: none;
    -webkit-user-select: none;
    touch-action: manipulation;
  }
  input::-webkit-file-upload-button{
    
    flex-direction: column;
    align-items: center;
    padding: 6px 14px;
    font-family: -apple-system, BlinkMacSystemFont, 'Roboto', sans-serif;
    border-radius: 6px;
    border: none;
    color: #fff;
    background: linear-gradient(180deg, #4B91F7 0%, #367AF6 100%);
     background-origin: border-box;
    box-shadow: 0px 0.5px 1.5px rgba(54, 122, 246, 0.25), inset 0px 0.8px 0px -0.25px rgba(255, 255, 255, 0.2);
    user-select: none;
    -webkit-user-select: none;
    touch-action: manipulation;
  }
  .card { 
    background-color: white; 
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
  }
  .card-title { 
    font-size: 1.2rem;
    font-weight: bold;
    color: #034078
  }

  </style>
  <body>
    <div class="topnav">
      <h1>SMART AIRWAY OTA</h1>
    </div>
    <div class="content">
      <div class="card-grid">
        <div class="card">
          <p class="card-title">UPDATE</p>
          <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
            <input type='file' name='update'>
            <input type='submit' value='Update'>
             </form>
          <div id='prg'>progress: 0%</div>
          
          <script>
           $('form').submit(function(e){
           e.preventDefault();
           var form = $('#upload_form')[0];
           var data = new FormData(form);
            $.ajax({
           url: '/update',
           type: 'POST',
           data: data,
           contentType: false,
           processData:false,
           xhr: function() {
           var xhr = new window.XMLHttpRequest();
           xhr.upload.addEventListener('progress', function(evt) {
           if (evt.lengthComputable) {
           var per = evt.loaded / evt.total;
           $('#prg').html('progress: ' + Math.round(per*100) + '%');
           }
           }, false);
           return xhr;
           },
           success:function(d, s) {
           console.log('success!')
          },
          error: function (a, b, c) {
          }
          });
          });
          </script>
        </div>
      </div>
    </div>
  </body>
</html>)rawliteral";