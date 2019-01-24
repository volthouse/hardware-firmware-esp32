#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <U8x8lib.h>
#include "clock.h"

#define SLEEP_SEC 15

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

ClockClass rtc(SLEEP_SEC);

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

#include "index.h"

#define LED 2
#define SLEEP_SEC 15

static uint32_t count = 0;
static uint32_t rtc_time = 0;

//WiFiServer server(80);
WebServer server(80);

void handleRoot()
{
 String s = MAIN_page;
 server.send(200, "text/html", s);
}

void handleGetDate()
{ 
  String s = rtc.toString() + " Count: " 
    + String(count) + " Reboot Count:" 
    + rtc.getRebootCount() +
    + " RTC Time: " + String(rtc_time);
  server.send(200, "text/plane", s);
}

void handleSetDate()
{  
  
  int year = server.arg("year").toInt();
  int month = server.arg("month").toInt();
  int day = server.arg("day").toInt();
  int hours = server.arg("hours").toInt();
  int minutes = server.arg("minutes").toInt();
  int seconds = server.arg("seconds").toInt();
  
  rtc.setClock(year, month, day, hours, minutes, seconds);

  u8x8.drawString(0, 20, "Set Date");
}

void handleLED()
{
  /*
 String ledState = "OFF";
 String t_state = server.arg("LEDstate");
 Serial.println(t_state);
 if(t_state == "1")
 {
  //digitalWrite(LED,LOW);
  ledState = "ON";
 }
 else
 {
  //digitalWrite(LED,HIGH);
  ledState = "OFF";
 }
 
 server.send(200, "text/plane", ledState);
 */
}



//==============================================================
//                  SETUP
//==============================================================
void setup(void)
{
  Serial.begin(115200);
  //rtc_time = system_get_rtc_time();
  
  Serial.println("");

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 0, "Alarm Clock");
  
  //pinMode(LED, OUTPUT); 

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("ESP-LED");
  
  Serial.println("Connecting ...");

  dnsServer.start(DNS_PORT, "*", apIP);
  /*
  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  } 
  */
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  
 
  server.on("/", handleRoot);
  //server.on("/setLED", handleLED);
  server.on("/setDate", handleSetDate);
  server.on("/getDate", handleGetDate);

  server.begin();
  Serial.println("HTTP server started");
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void)
{   
  dnsServer.processNextRequest();
  server.handleClient();
}
