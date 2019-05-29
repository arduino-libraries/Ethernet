/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include "IP6Address.h"

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

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServerv6 server(80);
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
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernetv6.begin(mac);
  
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

  // start the server dual stack
  server.begin(1);
  Serial.println("Web Server address:");
  Serial.print("IPv4 ADR: "); Serial.println(Ethernetv6.localIP());
  Serial.print("IPv6 LLA: "); Serial.println(Ethernetv6.linklocalAddress());
  Serial.print("IPv6 GUA: "); Serial.println(Ethernetv6.globalunicastAddress());
}

void loop() {
  // listen for incoming clients
  EthernetClientv6 client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

