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
  modified 23 May 2020 by KooLru

 */

#include <SPI.h>

#define MY_SERIAL Serial
//#define MY_SERIAL Serial1 // Serial 1

//SPIClass SPI_1((uint8_t)PA7, (uint8_t)PA6, (uint8_t)PA5);  // SPI1 on STM32F103xB Bluepill 
//SPIClass SPI_2(PB15, PB14, PB13);     // SPI2 on STM32F103xB Bluepill 

#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};

void setup() {

  // Open serial communications and wait for port to open:
  MY_SERIAL.begin(9600);
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // You can use Ethernet.init(pin) to configure the CS pin ans SPI
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
    
  //Ethernet.init((uint8_t)PA4);  // SPI1 on STM32 Bluepill 
  //Ethernet.init(SPI_2, PB12);  // SPI2 on STM32 Bluepill 
  
  // start the Ethernet connection:
  MY_SERIAL.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    MY_SERIAL.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      MY_SERIAL.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      MY_SERIAL.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }

  //print chip info
  if (Ethernet.hardwareStatus() == EthernetW5100) 
    MY_SERIAL.println("Ethernet shield W5100");
  else if (Ethernet.hardwareStatus() == EthernetW5200) 
    MY_SERIAL.println("Ethernet shield W5200");
  else if (Ethernet.hardwareStatus() == EthernetW5500) 
    MY_SERIAL.println("Ethernet shield W5500");

  
  // print your local IP address:
  MY_SERIAL.print("My IP address: ");
  MY_SERIAL.println(Ethernet.localIP());
}

void loop() {
  switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      MY_SERIAL.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      MY_SERIAL.println("Renewed success");
      //print your local IP address:
      MY_SERIAL.print("My IP address: ");
      MY_SERIAL.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      MY_SERIAL.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      MY_SERIAL.println("Rebind success");
      //print your local IP address:
      MY_SERIAL.print("My IP address: ");
      MY_SERIAL.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }
}
