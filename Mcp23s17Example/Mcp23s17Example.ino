// This example demonstrates control over SPI to the Microchip MCP23S17 16-bit IO expander
// SPI Pinouts are for Arduino Uno and Arduino Duemilanove board (will differ for Arduino MEGA)

// Download these into your Sketches/libraries/ folder...

// Load the Reference SPI library (http://arduino.cc/en/Reference/SPI)
// Including SPI.h below initializes the MOSI, MISO, and SPI_CLK pins as per ATMEGA 328P
#include <SPI.h>

// Mcp23s17 library available from https://github.com/dreamcat4/Mcp23s17
#include <Mcp23s17.h>

// Wire up the SPI Interface common lines:
// #define SPI_MOSI             11 //arduino   <->   SPI Master Out Slave In   -> SI  (Pin 13 on MCP23S17 DIP)
// #define SPI_MISO             12 //arduino   <->   SPI Master In Slave Out   -> SO  (Pin 14 on MCP23S17 DIP)
// #define SPI_CLOCK            13 //arduino   <->   SPI Slave Clock Input     -> SCK (Pin 12 on MCP23S17 DIP)

// Then choose any other free pin as the Slave Select (pin 10 if the default but doesnt have to be)
#define MCP23S17_SLAVE_SELECT_PIN  8 //arduino   <->   SPI Slave Select           -> CS  (Pin 11 on MCP23S17 DIP)

// SINGLE DEVICE
// Instantiate a single Mcp23s17 object
MCP23S17 Mcp23s17 = MCP23S17( MCP23S17_SLAVE_SELECT_PIN, true );

// MULTIPLE DEVICES
// Up to 8 MCP23S17 devices can share the same SPI bus and slave select pins.
// Assign each chip a unique 3-bit device address (by setting the A2,A1,A0 pins)
// Then below, device address is optional 2nd parameter to the constructor fn...
// MCP23S17 Mcp23s17_0 = MCP23S17(MCP23S17_SLAVE_SELECT_PIN,0x0);
// ...
// MCP23S17 Mcp23s17_7 = MCP23S17(MCP23S17_SLAVE_SELECT_PIN,0x7);

void setup()
{
  // Example usage

  // Set all pins to be outputs (by default they are all inputs)
  for(byte i= 0; i<12; i++)
    Mcp23s17.pinModeD(i, OUTPUT);
    
  Mcp23s17.applyPinMode();
}

// Cycle the output lines at a staggered rate, pin/2 milliseconds
long timeoutInterval = 500;
long previousMillis = 0;
int16_t counter1 = 0x0000;
int16_t counter2 = 0x0000;
uint16_t pinState;

void timeout()
{
  Mcp23s17.digitalWriteD(counter1, 0);
  Mcp23s17.digitalWriteD(counter1+2, 0);
  Mcp23s17.digitalWriteD(counter1+4, 0);
  Mcp23s17.digitalWriteD(counter1+6, 0);
  Mcp23s17.digitalWriteD(counter2+8, 0);
  
  counter1++;
  if(counter1>1) counter1= 0;
  counter2++;
  if(counter2>3) counter2= 0;
  
  Mcp23s17.digitalWriteD(counter1, 1);
  Mcp23s17.digitalWriteD(counter1+2, 1);
  Mcp23s17.digitalWriteD(counter1+4, 1);
  Mcp23s17.digitalWriteD(counter1+6, 1);
  Mcp23s17.digitalWriteD(counter2+8, 1);
  Mcp23s17.applyDigitalWrite();
}  

void loop()
{
  // handle timeout function, if any
  if (  millis() - previousMillis > timeoutInterval )
  {
    timeout();
    previousMillis = millis();
  }
  // Loop.
}


