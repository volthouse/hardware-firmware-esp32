/*
	TODO:	Optimize current consuption
			see https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/sleep_modes.html#

 +/

#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <U8x8lib.h>
#include "rom/rtc.h"  

#include "clock.h"
#include "index.h"

#define SLEEP_SEC 15
#define BUTTON1_PIN 38
#define BUTTON2_PIN 37
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */


// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

ClockClass rtc(SLEEP_SEC);

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;

static uint32_t state = 0;
static uint32_t deepsleep_timeout = 0;

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

void handleSetData(void)
{
  Serial.println("setData");  
  state = server.arg("data").toInt();
}

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

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
   

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  
    
  

  //dnsServer.start(DNS_PORT, "*", apIP);
  /*
  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  } 
  */
  
  
}

void do_wifi(void)
{
  switch(state) {
    case 1:
      //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP-LED");  
      WiFi.begin();

      Serial.println("Connecting ...");
      
      server.on("/", handleRoot);  
      server.on("/setDate", handleSetDate);
      server.on("/getDate", handleGetDate);
      server.on("/setData", handleSetData);
    
      server.begin();
      
      Serial.println("");
      Serial.print("Connected to ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());  
      Serial.println("HTTP server started");
      
      state = 2;
      break;
      
    case 2:
      //dnsServer.processNextRequest();
      server.handleClient();
      break;
  }  
}

void do_buttons(void)
{
  static uint32_t timeout = 0;
  
  uint8_t b1 = digitalRead(BUTTON1_PIN) == 0 ? 1 : 0;
  uint8_t b2 = digitalRead(BUTTON2_PIN) == 0 ? 1 : 0;
  
  switch(state) {
    case 0:
      if(b1 && !timeout) {
        timeout = rtc.tickMs();
      } else if(!b1){
        timeout = 0;
      } else if(b1 && (rtc.tickMs() - timeout) > 2000) {
        state = 1;    
      }
      break;
  }
}

void do_deepsleep(void)
{
  static uint32_t timeout = 0;
  uint32_t sleep = 0;
  
  switch(state) {    
    case 0:
      if((rtc.tickMs() - timeout) >= 1000) {
        deepsleep_timeout++;        
        timeout = rtc.tickMs();
      }
      sleep = deepsleep_timeout > 10;      
      break;
    case 100:
      sleep = true;
      break;
  }

  if(sleep) {
    // external wakup source
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_38, 0); //1 = High, 0 = Low
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    Serial.println("Going to sleep now");
    u8x8.setPowerSave(1);
    esp_deep_sleep_start();
  }
}

void do_display(void)
{
  switch(state) {
    case 0:
      u8x8.drawString(0, 0, "Alarm Clock");
      break; 
    case 1:
      u8x8.drawString(0, 0, "Wifi Mode  ");
      break;
  }

  switch(state) {
    case 0:   
    case 1:
    case 2: {      
      if(rtc.tick()) {
        u8x8.drawString(0, 2, rtc.timeToCStr());
        char buf[20];
        sprintf(buf, "Deepsleep: %d", deepsleep_timeout);
        u8x8.drawString(0, 3, buf);
        sprintf(buf, "State: %d", state);
        u8x8.drawString(0, 4, buf);
      }   
      break;
    }    
  }
}

void loop(void)
{
  do_buttons();
  do_display();
  do_display();
  do_wifi();
  do_deepsleep();
}
