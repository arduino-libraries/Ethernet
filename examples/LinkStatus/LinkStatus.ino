/*
  Link Status

  This sketch prints the Ethernet link status. When the
  Ethernet cable is connected the link status should go to "ON".
  NOTE: Only WIZnet W5200 and W5500 are capable of reporting
  the link status. W5100 will report "Unknown".
  Hardware:
   - Ethernet shield or equivalent board/shield with WIZnet W5200/W5500
  Written by Cristian Maglie
  This example is public domain.
*/

#include <SPI.h>
#include <EthernetSP.h>

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  // ====================================================================
  // W5500-Ether add-on for Spresense

  // Jumper setting is SJ1=12, use D21/EMMC_DATA3
  W5500ETH_reset(SJ1_12);

  // Jumper setting is SJ1=23, use D26/I2S_BCK
  // W5500ETH_reset(SJ1_23);

  // When Jumper setting is SJ2=23, init() is not need.

  // Jumper setting is SJ2=12, use D19/I2S_DIN
  // Ethernet.init(SJ2_12);
  // ====================================================================

  Serial.begin(9600);
}

void loop() {
  auto link = Ethernet.linkStatus();
  Serial.print("Link status: ");
  switch (link) {
    case Unknown:
      Serial.println("Unknown");
      break;
    case LinkON:
      Serial.println("ON");
      break;
    case LinkOFF:
      Serial.println("OFF");
      break;
  }
  delay(1000);
}
