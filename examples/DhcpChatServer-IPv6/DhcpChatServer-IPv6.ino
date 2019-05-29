/*
 DHCP Chat  Server

 A simple server that distributes any incoming messages to all
 connected clients.  To use, telnet to your device's IP address and type.
 You can see the client's input in the serial monitor as well.
 Using an Arduino Wiznet Ethernet shield.

 THis version attempts to get an IP address using DHCP

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 21 May 2011
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 Based on ChatServer example by David A. Mellis

 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:

byte mac[] = {
0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

byte ip6_lla[] = {
0xfe, 0x80, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x02, 0x00, 0xdc, 0xff,
0xfe, 0x57, 0x57, 0x61
};

byte ip6_gua[] = {
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};

byte ip6_sn6[] = {
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};

byte ip6_gw6[] = {
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00
};

// https://developers.google.com/speed/public-dns/docs/using
// 2001:4860:4860::8888
// 2001:4860:4860::8844

byte ip6_dns6[] = {
0x20, 0x01, 0x48, 0x60,
0x48, 0x60, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x88, 0x88
};

IP6Address ip(192, 168, 0, 4);
IP6Address myDns(192, 168, 0, 1);
IP6Address gateway(192, 168, 0, 1);
IP6Address subnet(255, 255, 0, 0);

IP6Address lla(ip6_lla, 16);
IP6Address gua(ip6_gua, 16);
IP6Address sn6(ip6_sn6, 16);
IP6Address gw6(ip6_gw6, 16);

// telnet defaults to port 23
EthernetServerv6 server(23);
bool gotAMessage = false; // whether or not you got a message from the client yet

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
  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernetv6.begin(mac) == 0) {
    Serial.println("AddressAutoConfig Failed");
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernetv6.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernetv6.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }

    // initialize the Ethernet device not using DHCP:
    Ethernetv6.begin(mac, ip, myDns, gateway, subnet, lla, gua, sn6, gw6);
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

  // start listening for clients
  server.begin();

  Serial.println("Chat server address:");
  Serial.print("IPv4 ADR: "); Serial.println(Ethernetv6.localIP());
  Serial.print("IPv6 LLA: "); Serial.println(Ethernetv6.linklocalAddress());
  Serial.print("IPv6 GUA: "); Serial.println(Ethernetv6.globalunicastAddress());
}

void loop() {
  // wait for a new client:
  EthernetClientv6 client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!gotAMessage) {
      Serial.println("We have a new client");
      client.println("Hello, client!");
      gotAMessage = true;
    }

    // read the bytes incoming from the client:
    char thisChar = client.read();
    // echo the bytes back to the client:
    server.write(thisChar);
    // echo the bytes to the server as well:
    Serial.print(thisChar);
  }
  
  Ethernetv6.maintain();
}

