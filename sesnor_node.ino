#include <SPI.h>
#include <RH_RF95.h>
#include <DFRobot_DHT11.h>

#define DHT11_PIN 8
DFRobot_DHT11 DHT;

RH_RF95 rf95(10, 2); // CS=10, DIO0=2
String srcID = "1";
String destID = "3";
int sequence = 0;

void setup() {
  Serial.begin(9600);
  DHT.read(DHT11_PIN);
  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }
  rf95.setFrequency(868.0);
  rf95.setTxPower(14, false);
  rf95.setSpreadingFactor(7);
  rf95.setSignalBandwidth(125000);
  rf95.setCodingRate4(5);
  Serial.println("Sensor Node Ready");
}

void loop() {
  DHT.read(DHT11_PIN);
  if (DHT.temperature == 0 && DHT.humidity == 0) {
    Serial.println("Sensor read failed, skipping");
    delay(2000);
    return;
  }

  int temp = DHT.temperature;
  int hum = DHT.humidity;

  String message = srcID + "|" + destID + "|" + sequence++ + "|" + temp + "|" + hum + "|" + srcID;
  Serial.println("SOURCE_ID|DEST_ID|SEQUENCE_NUM|TEMP|HUM|PATH");
  Serial.println(message);
  Serial.println("Sending packet ");
  delay(200);
  Serial.println("Packet sent");
  Serial.println("");

  rf95.send((uint8_t *)message.c_str(), message.length());
  rf95.waitPacketSent();

  delay(10000);
}
