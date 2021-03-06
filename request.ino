/*
	TODO:	Optimize current consuption
			see https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/sleep_modes.html#

 */

#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <U8x8lib.h>
#include "rom/rtc.h"  
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_spi_flash.h>

#include "clock.h"
#include "index.h"

#define SLEEP_SEC       15
#define uS_TO_S_FACTOR  1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP   120        /* Time ESP32 will go to sleep (in seconds) */

#define BUZZER_PIN      26
#define BUTTON1_PIN     27
#define BUTTON2_PIN     22

#define STATE_DEFAULT       0
#define STATE_WIFI_INIT     1
#define STATE_WIFI_ACTIVE   2
#define STATE_DEEPSLEEP     100

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

/*
  for(int i = 0; i < 10; i++) {
    spi_flash_init();
    uint32_t fsize = spi_flash_get_chip_size();
    Serial.println(fsize);
    delay(1000);
  }
*/
  state = STATE_DEFAULT;
  
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
    case STATE_WIFI_INIT:
#if 1    
      //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP-LED");  
      WiFi.begin();
      Serial.println("Connecting ...");
#endif
      
#if 0
      WiFi.begin("ssid", "*****");
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
      }      
#endif
      
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
      
      state = STATE_WIFI_ACTIVE;
      break;
      
    case STATE_WIFI_ACTIVE:
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

  //char buf[50];
  //sprintf(buf, "B1: %d   B2: %d", b1, b2);
  //Serial.println(buf);
  
  switch(state) {
    case STATE_DEFAULT:
      if(b1 && !timeout) {
        timeout = rtc.tickMs();
      } else if(!b1){
        timeout = 0;
      } else if(b1 && (rtc.tickMs() - timeout) > 2000) {
        state = STATE_WIFI_INIT;    
      }
      break;
  }
}

void do_deepsleep(void)
{
  static uint32_t timeout = 0;
  uint32_t sleep = 0;
  
  switch(state) {    
    case STATE_DEFAULT:
      if((rtc.tickMs() - timeout) >= 1000) {
        deepsleep_timeout++;        
        timeout = rtc.tickMs();
      }
      sleep = deepsleep_timeout > 10;      
      break;
    case STATE_DEEPSLEEP:
      sleep = true;
      break;
  }

  if(sleep) {    
    server.stop();
    WiFi.mode(WIFI_OFF);

    
    esp_bluedroid_disable();
    esp_bt_controller_disable();
    esp_wifi_disconnect();
    esp_wifi_stop();
    
    /*
    NOTE:
    ======
    Only RTC IO can be used as a source for external wake
    source. They are pins: 0,2,4,12-15,25-27,32-39.
    */
    Serial.println("DeepSleep Res");
    // external wakup source
    int r = esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 0); //1 = High, 0 = Low    
    Serial.println(r);
    
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    Serial.println("Going to sleep now");
    u8x8.setPowerSave(1);

    delay(1000);
    
    esp_deep_sleep_start();
  }
}

void do_display(void)
{
  switch(state) {
    case STATE_DEFAULT:
      u8x8.drawString(0, 0, "Alarm Clock");
      break; 
    case STATE_WIFI_INIT:
      u8x8.drawString(0, 0, "Wifi Mode  ");
      break;
  }

  switch(state) {
    case STATE_DEFAULT:   
    case STATE_WIFI_INIT:
    case STATE_WIFI_ACTIVE: {      
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
