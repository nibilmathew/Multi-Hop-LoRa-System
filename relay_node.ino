#include <SPI.h>
#include <RH_RF95.h>

RH_RF95 rf95(10, 2);
String myID = "2";
String lastSeenPacket = "";

void setup() {
  Serial.begin(9600);
  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }
  rf95.setFrequency(868.0);
  rf95.setTxPower(14, false);
  rf95.setSpreadingFactor(7);
  rf95.setSignalBandwidth(125000);
  rf95.setCodingRate4(5);
  Serial.println("Relay Node Ready");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      String raw = "";
      for (uint8_t i = 0; i < len; i++) raw += (char)buf[i];

      Serial.print("Received: ");
      Serial.println(raw);

      int indices[5], idx = 0, pos = -1;
      while ((pos = raw.indexOf('|', pos + 1)) != -1 && idx < 5)
        indices[idx++] = pos;

      if (idx < 5) {
        Serial.println("Malformed, skip");
        return;
      }

      String destID = raw.substring(indices[0] + 1, indices[1]);
      String seqID = raw.substring(indices[1] + 1, indices[2]);
      String srcID = raw.substring(0, indices[0]);
      String path = raw.substring(indices[4] + 1);

      String uniqueKey = srcID + "-" + seqID;
      if (lastSeenPacket == uniqueKey) {
        Serial.println("Duplicate packet, skipping");
        return;
      }

      lastSeenPacket = uniqueKey;

      if (destID == myID) {
        Serial.println("Packet meant for me, skipping");
        return;
      }

      if (path.indexOf(myID) != -1) {
        Serial.println("Already forwarded, skipping");
        return;
      }

      String newMessage = raw + "-" + myID;
      Serial.println("SOURCE_ID|DEST_ID|SEQ_NUM|TEMP|HUM|PATH");
      Serial.println(newMessage);
      Serial.println("Forwarding...");
      Serial.println("Done");
      Serial.println("");

      rf95.send((uint8_t *)newMessage.c_str(), newMessage.length());
      rf95.waitPacketSent();
    }
  }
}
