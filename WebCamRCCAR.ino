#include "esp_camera.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <TokenIterator.h>
#include <UrlTokenBindings.h>
#include "soc/rtc_wdt.h"
 
const char* ssidaccess = "ESP32";
const char* passwordaccess =  "12344321";

AsyncWebServer server(8080);

#define CAMERA_MODEL_AI_THINKER

const char* ssid = "The House Home_4.1_2.4G";
const char* password = "123456789@";


#if defined(CAMERA_MODEL_WROVER_KIT)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22


#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else
#error "Camera model not selected"
#endif

// GPIO Setting
extern int gpLb =  2; // Left Wheel Back
extern int gpLf = 14; // Left Wheel Forward
extern int gpRb = 15; // Right Wheel Back
extern int gpRf = 13; // Right Wheel Forward
extern int gpLed =  4; // Light
extern String WiFiAddr ="";
extern int fanPin = 12;  

// setting PWM properties
extern int freq = 5000;
extern int ledChannel = 0;
extern int fanresolution = 8;

void startCameraServer();
int count=0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  pinMode(gpLb, OUTPUT); //Left Backward
  pinMode(gpLf, OUTPUT); //Left Forward
  pinMode(gpRb, OUTPUT); //Right Forward
  pinMode(gpRf, OUTPUT); //Right Backward
  pinMode(gpLed, OUTPUT); //Light
  ledcSetup(ledChannel, freq, fanresolution);
  ledcAttachPin(fanPin, ledChannel);

  //initialize
  digitalWrite(gpLb, LOW);
  digitalWrite(gpLf, LOW);
  digitalWrite(gpRb, LOW);
  digitalWrite(gpRf, LOW);
  digitalWrite(gpLed, LOW);

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);

  if(WiFi.status() != WL_CONNECTED){
  WiFi.softAP(ssidaccess, passwordaccess);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/wifi/*", HTTP_GET, [](AsyncWebServerRequest *request){
 
    char templatePath[] = "/wifi/:name/:pass";
 
    char urlBuffer[50];
    request->url().toCharArray(urlBuffer, 50);
 
    int urlLength = request->url().length();
 
    TokenIterator templateIterator(templatePath, strlen(templatePath), '/');
    TokenIterator pathIterator(urlBuffer, urlLength, '/');
    UrlTokenBindings bindings(templateIterator, pathIterator);
    if(bindings.hasBinding("name")){
      ssid=bindings.get("name");
    }
    if(bindings.hasBinding("pass")){
      password=bindings.get("pass");
    }
   WiFi.begin(ssid,password);
   while(WiFi.status() != WL_CONNECTED){
     delay(100);
     Serial.print(".");
   }
   if(WiFi.status() == WL_CONNECTED){
    request->send(200, "text/plain", WiFi.localIP().toString().c_str());
    Serial.println("");
    Serial.println("WiFi connected");
    startCameraServer();
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    WiFiAddr = WiFi.localIP().toString();
    Serial.println("' to connect");

   }else{
     request->send(200, "text/plain", "fail");
   } 
  });
  
  server.begin();
}

}

void loop() {

}
