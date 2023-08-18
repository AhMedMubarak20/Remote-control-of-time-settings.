#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

char auth[] = "your_blynk_auth_token";
char ssid[] = "your_wifi_ssid";
char pass[] = "your_wifi_password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

WiFiUDP Udp;
unsigned int localPort = 8888;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  setSyncProvider(getNtpTime);
  setSyncInterval(300); // Sync time every 5 minutes
}

void loop() {
  Blynk.run();
}

time_t getNtpTime() {
  IPAddress ntpServerIP;
  WiFi.hostByName(ntpServer, ntpServerIP);

  if (Udp.begin(localPort)) {
    sendNTPpacket(ntpServerIP);
    delay(1000);
    if (Udp.parsePacket()) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + gmtOffset_sec;
    }
  }
  return 0;
}

void sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

BLYNK_WRITE(V1) {
  int newHour = param.asInt();       // Get updated hour value from app
  int newMinute = param[1].asInt();  // Get updated minute value from app
  
  setTime(newHour, newMinute, second(), day(), month(), year());
  setSyncInterval(0);  // Disable automatic NTP sync
}