#include <SPI.h>
#include <RH_RF95.h>
#include <Console.h>
#include <Bridge.h>
#include <Process.h>

#define BAUDRATE 115200

RH_RF95 rf95(10, 2);
String myID = "3";

String thingSpeakKey = "YOUR_API_KEY";

// Track last received sequence per node
String lastSourceID = "";
int lastSeqNum = -1;

void setup() {
  Bridge.begin(BAUDRATE);
  Console.begin();
  while (!Console);

  if (!rf95.init()) {
    Console.println("LoRa init failed");
    while (1);
  }
  rf95.setFrequency(868.0);
  rf95.setTxPower(14, false);
  rf95.setSpreadingFactor(7);
  rf95.setSignalBandwidth(125000);
  rf95.setCodingRate4(5);
  Console.println("Gateway Ready");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      String raw = "";
      for (uint8_t i = 0; i < len; i++) raw += (char)buf[i];

      Console.println("");
      Console.print("Received: ");
      Console.println(raw);

      //Format: srcID|destID|seq|temp|hum|path
      int i1 = raw.indexOf('|');
      int i2 = raw.indexOf('|', i1 + 1);
      int i3 = raw.indexOf('|', i2 + 1);
      int i4 = raw.indexOf('|', i3 + 1);
      int i5 = raw.indexOf('|', i4 + 1);

      if (i1 == -1 || i2 == -1 || i3 == -1 || i4 == -1 || i5 == -1) {
        Console.println("Malformed packet");
        return;
      }

      String srcID = raw.substring(0, i1);
      String destID = raw.substring(i1 + 1, i2);
      int seqNum = raw.substring(i2 + 1, i3).toInt();
      String temp = raw.substring(i3 + 1, i4);
      String hum = raw.substring(i4 + 1, i5);
      String path = raw.substring(i5 + 1);

      // Drop if not meant for this node
      if (destID != myID) {
        Console.println("Not for me, ignoring.");
        return;
      }

      // Check for duplicate
      if (srcID == lastSourceID && seqNum == lastSeqNum) {
        Console.println("Duplicate packet detected, ignored.");
        return;
      }

      // Update last seen values
      lastSourceID = srcID;
      lastSeqNum = seqNum;

      Console.println("=== SENSOR DATA RECEIVED ===");
      Console.println("Source: " + srcID);
      Console.println("Sequence: " + String(seqNum));
      Console.println("Temperature: " + temp + "°C");
      Console.println("Humidity: " + hum + "%");
      Console.println("Path: " + path);
      Console.println("Uploading to ThingSpeak...");

      Process p;
      String url = "http://api.thingspeak.com/update?api_key=" + thingSpeakKey +
                   "&field1=" + temp + "&field2=" + hum;

      p.begin("wget");
      p.addParameter("-qO-");
      p.addParameter(url);
      p.run();

      if (p.exitValue() == 0) {
        Console.println("Upload successful");
      } else {
        Console.println("Upload failed");
      }
    }
  }
}
