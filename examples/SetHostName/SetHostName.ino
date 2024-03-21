/*
  Set Host Name

  This example shows you how to set the host name with the Ethernet library.

  Circuit:
  * Ethernet shield attached to pins 10, 11, 12, 13

  created 28 May 2023
  by Attila Herczog
*/

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Host name to use
char hostName[] = "ExampleHostName";

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Host Name Example");

  // Set the Host Name
  // Call this function before Ethernet.begin() to set your host name.
  Ethernet.setHostName(hostName);

  // Start the Ethernet connection and the server:
  Ethernet.begin(mac);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }

  Serial.print("My IP is: ");
  Serial.println(Ethernet.localIP());
  Serial.print("My host name is: ");
  Serial.println(Ethernet.getHostName());
  Serial.println("You can now check your router's DHCP table to see the assigned host name.");
}

void loop() {}
