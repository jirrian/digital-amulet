/*
 *  Adapted from ESP8266 Simple HTTP get webclient test example
 *  Using FastLED to handle neopixels and ArduinoJSON to parse request result
 */
#include <FastLED.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#define NUM_LEDS 10
#define DATA_PIN 14

#define SPEED 15   // How fast the colors move.  Higher numbers = faster motion
#define STEPS 300   // How wide the bands of color are.  1 = more like a gradient, 10 = more like stripes

CRGB leds[NUM_LEDS];

int brightness;

//String likesValue = "161";
//String gotNewComment = "new comment!";

const char* ssid     = "jillian";
const char* password = "Xiaorbz01";
 
const char* host = "blog.jzhong.today";
 
void setup() {
  Serial.begin(115200);
  delay(100);
 
  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}
 
int value = 0;
 
void loop() {
  delay(5000);
  ++value;
 
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/digitalamulet/data.html#access_token=10172653.65b3c0f.a3d5dceb01624d4a9085e64d78e7480a";
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
  
  // Read all the lines of the reply from server and same them
  String values[4];
  int i = 0;

  String frontContent = client.readStringUntil('^');
  frontContent = client.readStringUntil('^');
  frontContent = client.readStringUntil('^');
  frontContent = client.readStringUntil('^');
  Serial.println(frontContent);
  String likesValue = client.readStringUntil('^');
  Serial.println(likesValue);
  String gotNewComment = client.readStringUntil('^');
  Serial.println(gotNewComment);
  /*
   * handle neopixels
  int likesValueInt = likesValue.toInt();
  brightness = map(likesValueInt, 0, 500, 0, 255);

  if(gotNewComment == "new comment!"){
    colorWipe(7,6,3);
  }
  
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; 

  FillLEDsFromPaletteColors(startIndex, 5, 0, false);
  FillLEDsFromPaletteColors(startIndex, 6, 9, true);
  FastLED.setBrightness(brightness);
  FastLED.delay(1000 / SPEED);
  */
  Serial.println();
  Serial.println("closing connection");
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
    delay(30);
    while(len1 > 0 || len2 > 0){
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
      delay(30);
    }     
}


