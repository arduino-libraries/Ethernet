/*
  Repeating Web client

 This sketch connects to a a web server and makes a request
 using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board.

 This example uses DNS, by assigning the Ethernet client with a MAC address,
 IP address, and DNS address.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 19 Apr 2012
 by Tom Igoe
 modified 21 Jan 2014
 by Federico Vanzati

 http://www.arduino.cc/en/Tutorial/WebClientRepeating
 This code is in the public domain.

 */

#include <SPI.h>
#include <Ethernet.h>
#include "IP6Address.h"

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

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

// Set the static IP address to use if the DHCP fails to assign
IP6Address ip(192, 168, 0, 177);
IP6Address myDns(192, 168, 0, 1);
IP6Address gateway(192, 168, 0, 1);
IP6Address subnet(255, 255, 0, 0);

IP6Address lla(ip6_lla, 16);
IP6Address gua(ip6_gua, 16);
IP6Address sn6(ip6_sn6, 16);
IP6Address gw6(ip6_gw6, 16);
IP6Address dns6(ip6_dns6, 16);

// initialize the library instance:
EthernetClientv6 client;

#if 1
char server[] = "ipv6.google.com";
#else
char server[] = "www.arduino.cc";  // also change the Host line in httpRequest()
#endif
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 30*1000;  // delay between updates, in milliseconds

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start serial port:
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
  
  httpRequest();
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:

    #if 1
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: ipv6.google.com");
    client.println("Connection: close");
    #else
    client.println("GET /latest.txt HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    #endif
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}




