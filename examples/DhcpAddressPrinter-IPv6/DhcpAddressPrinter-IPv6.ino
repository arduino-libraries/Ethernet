/*
  DHCP-based IP printer

  This sketch uses the DHCP extensions to the Ethernet library
  to get an IP address via DHCP and print the address obtained.
  using an Arduino Wiznet Ethernet shield.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 12 April 2011
  modified 9 Apr 2012
  by Tom Igoe
  modified 02 Sept 2015
  by Arturo Guadalupi

 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

IP6Address ip(192, 168, 0, 4);
IP6Address myDns(192, 168, 0, 1);
IP6Address gateway(192, 168, 0, 1);
IP6Address subnet(255, 255, 0, 0);

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernetv6.begin(mac) == 0) {
    Serial.println("AddressAutoConfig Failed");
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernetv6.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernetv6.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }

  Serial.println("==================================================================");
  Serial.println("Network Information");
  Serial.println("==================================================================");
  Serial.print("IPv4 ADR: "); Serial.println(Ethernetv6.localIP());
  Serial.print("IPv6 LLA: "); Serial.println(Ethernetv6.linklocalAddress());
  Serial.print("IPv6 GUA: "); Serial.println(Ethernetv6.globalunicastAddress());
  Serial.print("IPv6 GAW: "); Serial.println(Ethernetv6.gateway6());
  Serial.print("IPv6 SUB: "); Serial.println(Ethernetv6.subnetmask6());
  Serial.print("IPv6 DNS: "); Serial.println(Ethernetv6.dnsServerIP());
  Serial.println("==================================================================");

  // print your local IP address:
  Serial.print("IPv6 GUA: "); Serial.println(Ethernetv6.globalunicastAddress());
}

void loop() {
  switch (Ethernetv6.maintain()) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.println("My IP address: ");
      Serial.print("LLA:");
      Serial.println(Ethernetv6.linklocalAddress());
      
      Serial.print("GUA:");
      Serial.println(Ethernetv6.globalunicastAddress());

      Serial.print("IPv4:");
      Serial.println(Ethernetv6.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.println("My IP address: ");
      Serial.print("LLA:");
      Serial.println(Ethernetv6.linklocalAddress());
      
      Serial.print("GUA:");
      Serial.println(Ethernetv6.globalunicastAddress());

      Serial.print("IPv4:");
      Serial.println(Ethernetv6.localIP());
      break;

    default:
      //nothing happened
      break;
  }
}

