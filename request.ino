#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <U8x8lib.h>
#include "clock.h"
#include "index.h"
#include "rom/rtc.h"

#define SLEEP_SEC 15
#define BUTTON1_PIN 37
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */


// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

ClockClass rtc(SLEEP_SEC);

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

static uint32_t state = 0;
static uint32_t count = 0;
// Pulse counter value, stored in RTC_SLOW_MEM
static size_t RTC_DATA_ATTR m_pulse_count = 0;

WebServer server(80);

void handleRoot()
{
 String s = MAIN_page;
 server.send(200, "text/html", s);
}

void handleGetDate()
{ 
  String s = rtc.toString() + " Reboot Count:" 
    + m_pulse_count;
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
}

//==============================================================
//                  SETUP
//==============================================================
void setup(void)
{
  Serial.begin(115200);
  Serial.println("");

  //m_pulse_count++;
  
  if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) {
    Serial.println("Wake up from deep sleep");
    Serial.println("Pulse count=" + m_pulse_count);
  } else {
    Serial.println("Not a deep sleep wake up");  
  }

  //pinMode(BUTTON1_PIN, INPUT);
  
  // external wakup source
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 1); //1 = High, 0 = Low

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  u8x8.drawString(0, 0, "Alarm Clock");  
    
  //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP-LED");  
  WiFi.begin();
  
  Serial.println("Connecting ...");

  //dnsServer.start(DNS_PORT, "*", apIP);
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
  switch(state) {
    case 0: {
      time_t ltime;
      time(&ltime);      
      tm * ptm = localtime(&ltime);
      char buffer[32];
      // Format: Mo, 15.06.2009 20:20:00
      strftime(buffer, 32, "%H:%M:%S", ptm);
      Serial.println(buffer);
      if(rtc.hasSecondsChanged()) {
        u8x8.drawString(0, 2, buffer);
        count++;
      }

      if(count >25) {
        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
        Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
        Serial.println("Going to sleep now");
        esp_deep_sleep_start();
      }
      break;
    }
    case 10: {
      time_t rawtime;
      struct tm timeinfo;
      if(!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");      
      } else {
        char timeStringBuff[50]; //50 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
        //print like "const char*"
        Serial.println(timeStringBuff);
        u8x8.drawString(0, 30, timeStringBuff);
      }
    }
  }
  
  //dnsServer.processNextRequest();
  server.handleClient();
}
