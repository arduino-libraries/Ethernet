/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen

 */

#include <SPI.h>
#include <Ethernet.h>
#include "IP6Address.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

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

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)

char server[] = "ipv6.google.com";    // name address for Google (using DNS)
// Set the static IP address to use if the DHCP fails to assign
IP6Address ip(192, 168, 0, 4);
IP6Address myDns(192, 168, 0, 1);
IP6Address gateway(192, 168, 0, 1);
IP6Address subnet(255, 255, 0, 0);

IP6Address lla(ip6_lla, 16);
IP6Address gua(ip6_gua, 16);
IP6Address sn6(ip6_sn6, 16);
IP6Address gw6(ip6_gw6, 16);
IP6Address dns6(ip6_dns6, 16);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClientv6 client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement

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
    // try to congifure using IP address instead of DHCP:
    Ethernetv6.begin(mac, ip, myDns, gateway, subnet, lla, gua, sn6, gw6);
  } else {
    Serial.println("  DHCP assigned IP ");
  }

  Ethernetv6.setDnsServerIP(ip6_dns6);

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

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: ipv6.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  beginMicros = micros();
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();

    // do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
}

