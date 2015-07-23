
// MCP23S17 SPI 16-bit IO expander
// http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf

// For the cmd, AAA is the 3-bit MCP23S17 device hardware address.
// Useful for letting up to 8 chips sharing same SPI Chip select
// #define MCP23S17_READ  B0100AAA1 
// #define MCP23S17_WRITE B0100AAA0 

// The default SPI Control Register - SPCR = B01010000;
// interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
// sample on leading edge of clk,system clock/4 rate (fastest).
// Enable the digital pins 11-13 for SPI (the MOSI,MISO,SPICLK)

#include <SPI.h>
#include "Mcp23s17.h"

// Note: You may need to take _RESET_ on the MCP23S17 low
// for a few hundred ms and then take (and hold) it high
// again before you begin to communicate with the chip.

//---------- constructor ----------------------------------------------------

MCP23S17::MCP23S17(uint8_t slave_select_pin, bool cacheMode_= false)
{
  SPI.begin();
  setup_ss(slave_select_pin);
  setup_device(0x00);
  write_addr(IOCON, 0);
  setup_cache(cacheMode_);
}

MCP23S17::MCP23S17(uint8_t slave_select_pin, byte aaa_hw_addr, bool cacheMode_= false)
{
  SPI.begin();
  // Set the aaa hardware address for this chip by tying the 
  // MCP23S17's pins (A0, A1, and A2) to either 5v or GND.
  setup_ss(slave_select_pin);

  // We enable HAEN on all connected devices before we can address them individually
  setup_device(0x00);
  //write_addr(IOCON, read_addr(IOCON)|HAEN);
  write_addr(IOCON, HAEN);

  // Remember the hardware address for this chip
  setup_device(aaa_hw_addr);
  setup_cache(cacheMode_);
}

//------------------ protected -----------------------------------------------

uint16_t MCP23S17::byte2uint16(byte high_byte, byte low_byte)
{
  return (uint16_t)high_byte<<8 | (uint16_t)low_byte;
}

uint8_t MCP23S17::uint16_high_byte(uint16_t uint16)
{
  return (uint16>>8);
}

byte MCP23S17::uint16_low_byte(uint16_t uint16)
{
  return (byte)(uint16 & 0x00FF);
}

void MCP23S17::setup_ss(uint8_t slave_select_pin)
{
  // Set slave select (Chip Select) pin for SPI Bus, and start high (disabled)
  ::pinMode(slave_select_pin,OUTPUT);
  ::digitalWrite(slave_select_pin,HIGH);
  this->slave_select_pin = slave_select_pin;
}

void MCP23S17::setup_device(uint8_t aaa_hw_addr)
{
  this->aaa_hw_addr = aaa_hw_addr;
  this->read_cmd  = B01000000 | aaa_hw_addr<<1 | 1<<0; // MCP23S17_READ  = B0100AAA1 
  this->write_cmd = B01000000 | aaa_hw_addr<<1 | 0<<0; // MCP23S17_WRITE = B0100AAA0
  // write_addr(IOCON, read_addr(IOCON)|SEQOP); // no need to enable SEQOP if BANK=0
}

void MCP23S17::setup_cache(bool cacheMode_)
{
  cacheMode= cacheMode_;
  if(cacheMode) {
    // cacheDIR= read_addr(IODIR);
    // cacheGPIO= read_addr(GPIO);
    // cacheGPPU= read_addr(GPPU);
    cacheDIR= 0xFFFF;
    cacheGPIO= 0x0000;
    cacheGPPU= 0x0000;
  }
}

uint16_t MCP23S17::read_addr(byte addr)
{
  byte low_byte;
  byte high_byte;
  ::digitalWrite(slave_select_pin, LOW);
  SPI.transfer(read_cmd);
  SPI.transfer(addr);
  low_byte  = SPI.transfer(0x0/*dummy data for read*/);
  high_byte = SPI.transfer(0x0/*dummy data for read*/);
  ::digitalWrite(slave_select_pin, HIGH);
  return byte2uint16(high_byte,low_byte);
}

void MCP23S17::write_addr(byte addr, uint16_t data)
{
  ::digitalWrite(slave_select_pin, LOW);
  SPI.transfer(write_cmd);
  SPI.transfer(addr);
  SPI.transfer(uint16_low_byte(data));
  SPI.transfer(uint16_high_byte(data));
  ::digitalWrite(slave_select_pin, HIGH);
}

//---------- public ----------------------------------------------------

void MCP23S17::pinMode(bool mode)
{
  if(mode)
    cacheDIR = 0x0000;
  else
    cacheDIR = 0xFFFF;

  write_addr(IODIR, cacheDIR);
}

void MCP23S17::pinMode(uint16_t mode)
{
  write_addr(IODIR, ~mode);
  cacheDIR= ~mode;
}

uint16_t MCP23S17::pinMode() {
  return  ~read_addr(IODIR);
}

void MCP23S17::pullupMode(bool mode)
{
  if(mode)
    cacheGPPU = 0xFFFF;
  else
    cacheGPPU = 0x0000;

  write_addr(GPPU, cacheGPPU);
}

void MCP23S17::pullupMode(uint8_t pin, bool mode)
{
  if(!cacheMode)
    cacheGPPU= read_addr(GPPU);
  if(mode)
    cacheGPPU|= 1<<pin;
  else
    cacheGPPU&= ~((uint16_t)1<<pin);
    
  write_addr(GPPU, cacheGPPU);
}

void MCP23S17::pullupModeD(uint8_t pin, bool mode)
{
  if(mode)
    cacheGPPU|= 1<<pin;
  else
   cacheGPPU&= ~((uint16_t)1<<pin);
 }
void MCP23S17::applyPullupMode()
{
  write_addr(GPPU, cacheGPPU);
}

void MCP23S17::pullupMode(uint16_t mode)
{
  write_addr(GPPU, mode);
  cacheGPPU= mode;
}

void MCP23S17::port(uint16_t value)
{
  write_addr(GPIO,value);
  cacheGPIO= value;
}

void MCP23S17::port(uint8_t value, uint8_t AB) {
  if(!cacheMode)
    cacheGPIO= read_addr(GPIO);
    
  if(AB!=0) {
    cacheGPIO&= 0x00FF;
    cacheGPIO|= ((uint16_t)value)<<8;
  } else {
    cacheGPIO&= 0xFF00;
    cacheGPIO|= value;
  }
  
  write_addr(GPIO, cacheGPIO);  
}

uint16_t MCP23S17::port()
{
  return read_addr(GPIO);
}

void MCP23S17::pinMode(uint8_t pin, bool mode)
{
  if(!cacheMode)
    cacheDIR= read_addr(IODIR);
    
  if(mode)
    cacheDIR&= ~((uint16_t)1<<pin);
   else
    cacheDIR|= 1<<pin;
    
  write_addr(IODIR, cacheDIR);
}

void MCP23S17::pinModeD(uint8_t pin, bool mode)
{
  if(mode)
    cacheDIR&= ~((uint16_t)1<<pin);
  else
    cacheDIR|= 1<<pin;
}

void MCP23S17::applyPinMode()
{
  write_addr(IODIR, cacheDIR);
}

void MCP23S17::digitalWrite(uint8_t pin, bool value)
{
  if(!cacheMode)
    cacheGPIO= read_addr(GPIO);
    
  if(value)
    cacheGPIO|= 1<<pin;
  else
    cacheGPIO&= ~((uint16_t)1<<pin);
    
  write_addr(GPIO, cacheGPIO);
}

void MCP23S17::digitalWriteD(uint8_t pin, bool value)
{
  if(value)
    cacheGPIO|= 1<<pin;
  else
    cacheGPIO&= ~((uint16_t)1<<pin);
}

void MCP23S17::applyDigitalWrite()
{    
  write_addr(GPIO, cacheGPIO);
}

uint8_t MCP23S17::digitalRead(uint8_t pin)
{
  return read_addr(GPIO)>>pin & 1;
}

uint16_t MCP23S17::digitalRead()
{
  return read_addr(GPIO);
}
