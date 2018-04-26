/*
 *  Adapted from ESP8266 HTTP over TLS (HTTPS) example sketch
 *  Using FastLED to handle neopixels and ArduinoJSON to parse request result
 */
#include <FastLED.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define NUM_LEDS 10
#define DATA_PIN 14

#define SPEED 50   // How fast the colors move.  Higher numbers = faster motion
#define STEPS 300   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes

CRGB leds[NUM_LEDS];

int brightness;
int savedCommentsNum = 0;
bool gotNewComment = 1;
int likesValue = 300;

const char* ssid     = "jillian";
const char* password = "Xiaorbz01";
 
const char* host = "api.instagram.com";
const int httpsPort = 443;
// Use web browser to view and copy
// SHA1 fingerprint of the certificate
//const char* fingerprint = "22 DF E5 5A BF 3C 99 CF A2 45 DD C9 A6 4C 36 04 6B EC F0 8F";

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long HTTPReqPeriod = 7000; // wait 7 seconds before making http request again

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  startMillis = millis();  //initial start time
 
  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
     leds[6] = CRGB::Blue;
      FastLED.show();
      delay(500);
      // Now turn the LED off, then pause
      leds[6] = CRGB::Black;
      FastLED.show();
      delay(500);
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  doRequest();

}
 
void loop() {

  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  
  if (currentMillis - startMillis >= HTTPReqPeriod ){  //test whether 30 seconds have passed
    doRequest();  
    startMillis = millis(); // reset start time so https request is only done once
  }  
  
  /*
   * handle neopixels
   */
  brightness = map((likesValue / 3), 0, 800, 0, 255);

  if(gotNewComment){
    colorWipe(7,6,3);
    delay(600);
    gotNewComment = false;
  }
  
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; 

  FillLEDsFromPaletteColors(startIndex, 5, 0, false);
  FillLEDsFromPaletteColors(startIndex, 6, 9, true);
  FastLED.setBrightness(brightness);
  FastLED.delay(1000 / SPEED);
}

/* adapted from
 * https://codebender.cc/sketch:381684
 */
void FillLEDsFromPaletteColors( uint8_t colorIndex, int startLED, int endLED, bool notReverse) {
  if(notReverse){
    for( int i = startLED; i <= endLED; i++) {
      leds[i] = ColorFromPalette(OceanColors_p, colorIndex, brightness, LINEARBLEND);
      colorIndex += (STEPS / (endLED - startLED + 1));              
    }
  }
  else{
    for( int i = startLED; i >= endLED; i--) {
      leds[i] = ColorFromPalette(OceanColors_p, colorIndex, brightness, LINEARBLEND);
      colorIndex += (STEPS / (startLED - endLED + 1));              
    }
  }
  FastLED.show();
}

void colorWipe(int startLED, int lenLED1, int lenLED2){
    int len1 = lenLED1;
    int len2 = lenLED2;
    int count = 1;
    
    leds[startLED].r = 255;
    FastLED.show();
    delay(50);
    while(len1 >= 0 || len2 >= 0){
      if(len1 >= 0){
        leds[startLED-count].r = 255-(count*30);
        len1--;
      }
      if(len2 >= 0){
        leds[startLED+count].r = 255-(count*30);
        len2--;
      }
      count++;
      FastLED.show();
      delay(50);
    }     
}

/*
 * function handles https request and parses json response
 * edits likesValue and gotNewComment
 */
void doRequest(){ 
 
      Serial.print("connecting to ");
      Serial.println(host);
      
      // Use WiFiClientSecure class to create TLS connection
      WiFiClientSecure client;
      client.setTimeout(10000);
      if (!client.connect(host, httpsPort)) {
        Serial.println("connection failed");
        return;
      }
    
    //  if (client.verify(fingerprint, host)) {
    //    Serial.println("certificate matches");
    //  } else {
    //    Serial.println("certificate doesn't match");
    //  }
      
      // We now create a URI for the request
      String url = "/v1/users/self/media/recent/?count=3&access_token=10172653.65b3c0f.a3d5dceb01624d4a9085e64d78e7480a";
      Serial.print("Requesting URL: ");
      Serial.println(url);
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" + 
                   "Connection: close\r\n\r\n");

      /*
       * adapted from ArduinoJSON example: https://arduinojson.org/example/http-client/
       */
      // Check HTTP status
      char status[32] = {0};
      client.readBytesUntil('\r', status, sizeof(status));
      if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        return;
      }
    
      // Skip HTTP headers
      char endOfHeaders[] = "\r\n\r\n";
      if (!client.find(endOfHeaders)) {
        Serial.println(F("Invalid response"));
        return;
      }
    
      // Allocate JsonBuffer
      // Used arduinojson.org/assistant to compute the capacity.
      const size_t bufferSize = 5*JSON_ARRAY_SIZE(0) + JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + 8*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 14*JSON_OBJECT_SIZE(3) + 11*JSON_OBJECT_SIZE(4) + 2*JSON_OBJECT_SIZE(15) + JSON_OBJECT_SIZE(16) + 7572;
      DynamicJsonBuffer jsonBuffer(bufferSize);
    
      // Parse JSON object
      Serial.println("Parsing JSON");
    
      JsonObject& root = jsonBuffer.parseObject(client);
      if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        return;
      }
    
      JsonArray& data = root["data"];
      
      // calculate likes value and check for new comments for all posts returned
      // likes value is (# of likes) / (number of posts)
      int post0_likes = data[0]["likes"]["count"];
      int post1_likes = data[1]["likes"]["count"];
      int post2_likes = data[2]["likes"]["count"];
      likesValue = post0_likes + post1_likes + post2_likes;
      
      int post0_comments = data[0]["comments"]["count"];
      int post1_comments = data[1]["comments"]["count"];
      int post2_comments = data[2]["comments"]["count"];
      int curCommentsNum = post0_comments + post1_comments + post2_comments;
    
      Serial.println(likesValue);
      Serial.println(curCommentsNum);
     
      //compare old value of number of comments to new one
      gotNewComment = false;
      if (curCommentsNum > savedCommentsNum){
        savedCommentsNum = curCommentsNum;
        gotNewComment = true;
      }

      Serial.println(gotNewComment);
      
      Serial.println();
      Serial.println("closing connection");
}


