#include "LightDimmer.h"
#include "Switch.h"
#include <SPI.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <PubSubClient.h>
#include <avr/wdt.h>

LightDimmer light3000(3, 100);
LightDimmer light6000(5, 255);
Switch pump(4, true);
Switch heat150(8, true);
Switch heat10(6, true);
Switch air(7, true);

int LIGHT_3000_ON_HOUR = 7;
int LIGHT_3000_ON_MINUTE = 0;
int LIGHT_3000_ON_SECOND = 0;
int LIGHT_3000_OFF_HOUR = 15;
int LIGHT_3000_OFF_MINUTE = 30;
int LIGHT_3000_OFF_SECOND = 0;
int LIGHT_6000_ON_HOUR = 7;
int LIGHT_6000_ON_MINUTE = 0;
int LIGHT_6000_ON_SECOND = 0;
int LIGHT_6000_OFF_HOUR = 15;
int LIGHT_6000_OFF_MINUTE = 30;
int LIGHT_6000_OFF_SECOND = 0;

byte mac[] = {
  0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC
};

IPAddress MQTTBroker(192, 168, 1, 5);
char* pumpControlTopic="aquariot/pump";
char* airControlTopic="aquariot/air";
char* heat150ControlTopic="aquariot/heat150";
char* heat10ControlTopic="aquariot/heat10";
char* light3000ControlTopic="aquariot/light/red";
char* light6000ControlTopic="aquariot/light/white";

const int timeZone = 1;
const time_t syncInterval = 3600 * 24;

EthernetUDP Udp;
EthernetClient ethClient;
PubSubClient MQTTClient(ethClient);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  setupInitialSwitchesStatus();

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP - Rebooting");
    delay(100);
    reboot();
  }

  MQTTClient.setServer(MQTTBroker, 1883);
  MQTTClient.setCallback(MQTTCallback);
    
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(Ethernet.localIP());
  Udp.begin(1337);
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  if (timeStatus() != timeSet) {
    Serial.println("Failed to configure Time sync - Rebooting");
    delay(100);
    reboot();
  }
  setSyncInterval(syncInterval);

  setupInitialLightingConditions();
  setupLightingDimmingTimes();
  //Alarm.timerRepeat(1, debugStatus);
}

void setupInitialLightingConditions() {
  time_t LIGHT_3000_ON = computeUnixTime(LIGHT_3000_ON_HOUR, LIGHT_3000_ON_MINUTE, LIGHT_3000_ON_SECOND);
  time_t LIGHT_3000_OFF = computeUnixTime(LIGHT_3000_OFF_HOUR, LIGHT_3000_OFF_MINUTE, LIGHT_3000_OFF_SECOND);
  time_t LIGHT_6000_ON = computeUnixTime(LIGHT_6000_ON_HOUR, LIGHT_6000_ON_MINUTE, LIGHT_6000_ON_SECOND);
  time_t LIGHT_6000_OFF = computeUnixTime(LIGHT_6000_OFF_HOUR, LIGHT_6000_OFF_MINUTE, LIGHT_6000_OFF_SECOND);

  if (now() >= LIGHT_3000_ON && now() <= LIGHT_3000_OFF) {
    light3000.turnOn();
  }
  if (now() >= LIGHT_6000_ON && now() <= LIGHT_6000_OFF) {
    light6000.turnOn();
  }
}

time_t computeUnixTime (int hours, int minutes, int seconds) {
  tmElements_t TIME;
  TIME.Second = seconds;
  TIME.Minute = minutes;
  TIME.Hour = hours;
  TIME.Day = day();
  TIME.Month = month();
  TIME.Year = year() - 2000 + 30;
  return makeTime(TIME);
}

void setupInitialSwitchesStatus() {
  pump.turnOn();
  heat150.turnOn();
  heat10.turnOn();
  air.turnOn();
}

void setupLightingDimmingTimes() {
  Alarm.alarmRepeat(LIGHT_3000_ON_HOUR, LIGHT_3000_ON_MINUTE, LIGHT_3000_ON_SECOND, dimLight3000On);
  Alarm.alarmRepeat(LIGHT_3000_OFF_HOUR, LIGHT_3000_OFF_MINUTE, LIGHT_3000_OFF_SECOND, dimLight3000Off);
  Alarm.alarmRepeat(LIGHT_6000_ON_HOUR, LIGHT_6000_ON_MINUTE, LIGHT_6000_ON_SECOND, dimLight6000On);
  Alarm.alarmRepeat(LIGHT_6000_OFF_HOUR, LIGHT_6000_OFF_MINUTE, LIGHT_6000_OFF_SECOND, dimLight6000Off);
  Alarm.timerRepeat(1, updateLighting);
}

void dimLight3000On() {
  light3000.dimOn();
}

void dimLight3000Off() {
  light3000.dimOff();
}

void dimLight6000On() {
  light6000.dimOn();
}

void dimLight6000Off() {
  light6000.dimOff();
}

void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("MQTTCallback has been called");
  Serial.println(topic);
  Serial.write(payload, length);
  Serial.println();
  if (strcmp(topic,pumpControlTopic) == 0) {
    if ((char) payload[0] == '1') {
      pump.turnOn();
      MQTTClient.publish("aquariot/info", "Pump on");
    }
    if ((char) payload[0] == '0') {
      pump.turnOff();
      MQTTClient.publish("aquariot/info", "Pump off");
    }
  } else if (strcmp(topic,airControlTopic) == 0) {
    if ((char) payload[0] == '1') {
      air.turnOn();
      MQTTClient.publish("aquariot/info", "Air on");
    }
    if ((char) payload[0] == '0') {
      air.turnOff();
      MQTTClient.publish("aquariot/info", "Air off");
    }
  } else if (strcmp(topic,heat150ControlTopic) == 0) {
    if ((char) payload[0] == '1') {
      heat150.turnOn();
      MQTTClient.publish("aquariot/info", "Heat 150 on");
    }
    if ((char) payload[0] == '0') {
      heat150.turnOff();
      MQTTClient.publish("aquariot/info", "Heat 150 off");
    }
  } else if (strcmp(topic,heat10ControlTopic) == 0) {
    if ((char) payload[0] == '1') {
      heat10.turnOn();
      MQTTClient.publish("aquariot/info", "Heat 10 on");
    }
    if ((char) payload[0] == '0') {
      heat10.turnOff();
      MQTTClient.publish("aquariot/info", "Heat 10 off");
    }
  } else if (strcmp(topic,light3000ControlTopic) == 0) {
    String brightnessValue = "";
    for (int i=0;i<length;i++) {
      brightnessValue += (char)payload[i];
    }
    int brightness = brightnessValue.toInt();
    light3000.setBrightness(brightness);
  }  else if (strcmp(topic,light6000ControlTopic) == 0) {
    String brightnessValue = "";
    for (int i=0;i<length;i++) {
      brightnessValue += (char)payload[i];
    }
    int brightness = brightnessValue.toInt();
    light6000.setBrightness(brightness);
  }
}

void updateLighting() {
  light3000.updateDimming();
  light6000.updateDimming();
}

void debugStatus() {
  digitalClockDisplay();
  Serial.print("Current light 3000 brightness: ");
  Serial.println(light3000.getBrightness());
  Serial.print("Current light 6000 brightness: ");
  Serial.println(light6000.getBrightness());
  Serial.print("Current pump status: ");
  Serial.println(pump.getStatus());
  Serial.print("Current heat 150 status: ");
  Serial.println(heat150.getStatus());
  Serial.print("Current heat 10 status: ");
  Serial.println(heat10.getStatus());
  Serial.print("Current air status: ");
  Serial.println(air.getStatus());
}

void loop() {
  if (!MQTTClient.connected()) {
    reconnect();
  }
  MQTTClient.loop();
  Alarm.delay(1);
}

void reconnect() {
  // Loop until we're reconnected
  while (!MQTTClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (MQTTClient.connect("aquariotClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      MQTTClient.publish("aquariot","MQTT client ready");
      // ... and resubscribe
      MQTTClient.subscribe("aquariot/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTTClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      Alarm.delay(5000);
    }
  }
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket("fr.pool.ntp.org");
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

void sendNTPpacket(char* poolServerName)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(poolServerName, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void reboot() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (1) {}
}
