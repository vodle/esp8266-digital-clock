#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>
#include <Fonts/FreeSerifBold18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64

#define OLED_RESET 4 // for ESP8266 wemos d1 mini - 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define TIMESTRING "  МОСКОВСКОЕ ВРЕМЯ" //city string
#define UTCOFFSET 3 

// wifi config
// =======================================================================
const char* ssid     = "YOUR WIFI NAME";                      // SSID
const char* password = "YOUR WIFI PASSWORD";                    // SSID password
// =======================================================================

WiFiClient client;
IPAddress ip;  

void setup()   {                


  Serial.begin(115200); 
  Wire.pins(D3, D5);                         
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
// connect to WIFI
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);                         
  while (WiFi.status() != WL_CONNECTED) {         
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(utf8rus("CONNECTING"));
    display.display();
    Serial.print(".");
    delay(200);
    if(WiFi.status() == WL_CONNECTED){
      ip = WiFi.localIP(); 
      Serial.print("connected");
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.clearDisplay();
      display.println(utf8rus("connected"));
      display.print("local ip = ");
      display.println(utf8rus(IpAddress2String(ip)));
      display.display();
      delay(600); 
  }
  display.clearDisplay();
  }
}
//date and time variables
#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
String date;



void loop() {

if(updCnt<=0) { 
    updCnt = 10;
    Serial.println("Getting data ...");
    getTime();
    Serial.println("Data loaded");
    clkTime = millis();
  }

  
  vremya();
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }
}
//========================================================================
void vremya(void) {
  updateTime();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(utf8rus(TIMESTRING));
  display.setFont(&FreeSerifBold18pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5,45);
  display.println(String(h/10)+String(h%10)+":"+String(m/10)+String(m%10));
  display.setFont(&FreeSansBold12pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(95,45);
  display.println(String(s/10)+String(s%10));
  display.display();
  display.setFont();
}

// =======================================================================
// gettig time from google
// =======================================================================

float utcOffset = UTCOFFSET; 
long localEpoc = 0;
long localMillisAtUpdate = 0;

void getTime()
{
  WiFiClient client;
  if (!client.connect("www.google.com", 80)) {
    display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(utf8rus("FAILED TO CONNECT TO GOOGLE SERVERS"));
    return;
  }

  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    repeatCounter++;
  }

  String line;
  client.setNoDelay(false);
  while(client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    line.toUpperCase();
    if (line.startsWith("DATE: ")) {
      date = "     "+line.substring(6, 22);
      h = line.substring(23, 25).toInt();
      m = line.substring(26, 28).toInt();
      s = line.substring(29, 31).toInt();
      localMillisAtUpdate = millis();
      localEpoc = (h * 60 * 60 + m * 60 + s);
    }
  }
  client.stop();
}

// --

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = long(round(curEpoch + 3600 * utcOffset + 86400L)) % 86400L;
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

// Russion simbol output
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30-1;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70-1;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}
String IpAddress2String(IPAddress ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

