/*
This library implements the Silicon Labs (formerly Cygnal) C2 programming protocol.
It is based on Silicon Labs' application note AN127 (see also AN124). Unfortunately
several versions float around on the Internet. The best one seems to be v1.1 
"FLASH PROGRAMMING VIA THE C2 INTERFACE" Rev. 1.1 12/03 AN127-DS11 by Silicon Labs.
This document contains original C listings from Cygnal. Other versions of AN127 may 
look prettier, they are full of errors and unclear flow charts & diagrams.

Requires the following external (Arduino) functions:
 - delayMicroseconds(uint16_t us)
 - delay(uint16_t ms) for delay in milliseconds
 - noInterrupts() to globally disable interrupts
 - interrupts() to globally enable interrupts
 
Don't forget to setup two io ports for the C2CK and C2D signals.

Typical usage:

 // Reset and halt CPU, enter C2 mode.
 c2_init_programming_interface();
 // Erase everything (as page-wise erasing doesn't seem to work)
 c2_erase_device();
 // Loop over HEX file to send blocks no larger than 256 bytes.
 while (...)
 {
   c2_write_flash_block(uint16_t address, uint8_t *p_data, uint16_t data_size);
 }
 // Reset when done.
 c2_reset();

Written by CPV 
(C) Elektor, 2019
*/

#include <stdint.h>
#include "c2.h"

// Local functions, not to be called directly from the application.
static void c2_strobe(void);
static void c2_write_bit(uint8_t value);
static uint8_t c2_read_bit(void);
static void c2_write_two_bits(uint8_t value);
static void c2_write_byte(uint8_t value);
static uint8_t c2_read_byte(void);
static void c2_start(uint8_t instruction);
static void c2_stop(void);
static void c2_wait(void);
static void c2_address_write(uint8_t address);
static uint8_t c2_address_read(void);
static void c2_data_write_byte(uint8_t value);
static uint8_t c2_data_read_byte(void);
static void c2_wait_for_bit(uint8_t bitmask, uint8_t value);
static uint8_t c2_ok_or_not(void);
static uint8_t c2_command_init(uint8_t command);


static void c2_strobe(void)
{
  // Strobe low time must be shorter than 5 us.
  C2_CK_LO; // CK = 0;
  __asm__("nop\n\t"); // Wait between 80 and 5000 ns.
  __asm__("nop\n\t");
  C2_CK_HI; // CK = 1
  __asm__("nop\n\t"); // Wait at least 120 ns.
}

static void c2_write_bit(uint8_t value)
{
  if (value!=0) C2_D_HI; // D = 1
  else C2_D_LO; // D = 0
  c2_strobe();
}

static uint8_t c2_read_bit(void)
{
  c2_strobe();
  return C2_D_IN;
}

static void c2_write_two_bits(uint8_t value)
{
  c2_write_bit(value&0b01);
  c2_write_bit(value&0b10);
}

static void c2_write_byte(uint8_t value)
{
  for (uint8_t mask=0x01; mask!=0; mask<<=1)
  {
    c2_write_bit(value&mask);
  }
}

static uint8_t c2_read_byte(void)
{
  uint8_t value = 0;
  for (uint8_t mask=0x01; mask!=0; mask<<=1)
  {
    if (c2_read_bit()!=0) value |= mask;
  }
  return value;
}

static void c2_start(uint8_t instruction)
{
  c2_strobe(); // Generate START.
  C2_D_OUTPUT;
  c2_write_two_bits(instruction);
}

static void c2_stop(void)
{
  C2_D_INPUT;
  c2_strobe(); // Generate STOP.
}

static void c2_wait(void)
{
  do
  {
    c2_strobe();
  }
  while (C2_D_IN==0);
}

static void c2_address_write(uint8_t address)
{
  noInterrupts();
  c2_start(C2_ADDRESS_WRITE);
  c2_write_byte(address);
  c2_stop();
  interrupts();
}

static uint8_t c2_address_read(void)
{
  noInterrupts();
  c2_start(C2_ADDRESS_READ);
  C2_D_INPUT; // Switch to read mode.
  uint8_t value = c2_read_byte();
  c2_stop(); // Stop to send last strobe.
  interrupts();
  return value;
}

static void c2_data_write_byte(uint8_t value)
{
  noInterrupts();
  c2_start(C2_DATA_WRITE);
  c2_write_two_bits(0);
  c2_write_byte(value);
  C2_D_INPUT; // Switch to read mode.
  c2_wait();
  c2_stop(); // Stop to send last strobe.
  interrupts();
}

static uint8_t c2_data_read_byte(void)
{
  noInterrupts();
  c2_start(C2_DATA_READ);
  c2_write_two_bits(0);
  C2_D_INPUT; // Switch to read mode.
  c2_wait();
  uint8_t value = c2_read_byte();
  c2_stop();
  interrupts();
  return value;
}

static void c2_wait_for_bit(uint8_t bitmask, uint8_t value)
{
  uint8_t t;
  do
  {
    t = c2_address_read();
    if (value==0 && (t&bitmask)==0) break;
    else if (value!=0 && (t&bitmask)!=0) break;
  }
  while (1);
}

static uint8_t c2_ok_or_not(void)
{
  if (c2_data_read_byte()!=0x0d) return 0; // Fail.
  return 1; // OK.
}

static uint8_t c2_command_init(uint8_t command)
{
  c2_address_write(C2_FPDAT);
  c2_data_write_byte_with_busy(command);
  c2_wait_for_bit(C2_OUTREADY,1);
  return c2_ok_or_not();
}

//
// Exported functions start here.
//

void c2_reset(void)
{
  noInterrupts();
  C2_D_INPUT;
  C2_CK_OUTPUT;
  C2_CK_LO;
  delayMicroseconds(25); // Wait at least 20 us (adjusted for imprecision).
  C2_CK_HI;
  delayMicroseconds(4); // Wait at least 2 us (adjusted for imprecision).
  interrupts();
}

void c2_init_programming_interface(void)
{
  c2_reset();
  c2_address_write(C2_FPCTL);
  c2_data_write_byte(0x02);
  c2_data_write_byte(0x04);
  c2_data_write_byte(0x01);
  delay(20); // Wait at least 20 ms.
  //c2_write_sfr(0xb6,0x10); // 'F35x Flash Timing (AN127 table 3.6)
  //c2_write_sfr(0xb2,0x83); // 'F35x Oscillator Initialisation (AN127 table 3.6)
}

void c2_write_sfr(uint8_t address, uint8_t value)
{
  // WriteSFR
  c2_address_write(address);
  c2_data_write_byte(value);
}

uint8_t c2_read_sfr(uint8_t address)
{
  // ReadSFR
  c2_address_write(address);
  return c2_data_read_byte();
}

void c2_data_write_byte_with_busy(uint8_t value)
{
  // WriteCommand
  c2_data_write_byte(value);
  c2_wait_for_bit(C2_INBUSY,0);
}

uint8_t c2_data_read_byte_with_ready(void)
{
  // ReadData
  c2_wait_for_bit(C2_OUTREADY,1);
  return c2_data_read_byte();
}

uint8_t c2_write_direct(uint8_t address, uint8_t value)
{
  // WriteDirect
  if (c2_command_init(C2_COMMAND_DIRECT_WRITE)==0) return 0;
  // Send address.
  c2_data_write_byte_with_busy(address);
  c2_data_write_byte_with_busy(0x01);
  c2_data_write_byte_with_busy(value);
  return 1;
}

uint8_t c2_read_direct(uint8_t address)
{
  // ReadDirect
  if (c2_command_init(C2_COMMAND_DIRECT_READ)==0) return 0;
  // Send address.
  c2_data_write_byte_with_busy(address);
  c2_data_write_byte_with_busy(0x01);
  c2_wait_for_bit(C2_OUTREADY,0); // as per datasheet
  return c2_data_read_byte_with_ready();
}

// Working but not working...
/*void c2_unlock_flash(void)
{
  // Unlock sequence as given in 'F35x datasheet.
  c2_write_sfr(C2_FLKEY,0xa5);
  c2_write_sfr(C2_FLKEY,0xf1);
}*/

// Working but not working...
/*uint8_t c2_erase_flash_page(uint8_t page)
{
  // This function returns OK, but no pages are erased :-(
  if (c2_command_init(C2_COMMAND_PAGE_ERASE)==0)
  {
    return 0;
  }
  // Send page number.
  c2_data_write_byte_with_busy(page);
  c2_wait_for_bit(C2_OUTREADY,1);
  if (c2_command_init(C2_COMMAND_PAGE_ERASE)==0)
  {
    return 0;
  }
  c2_data_write_byte_with_busy(0); // Dummy write to start page erase.
  c2_wait_for_bit(C2_OUTREADY,1);
  return c2_ok_or_not();
}*/

uint8_t c2_erase_device(void)
{
  if (c2_command_init(C2_COMMAND_DEVICE_ERASE)==0) return 0;
  // Send 3-byte arming sequence.
  c2_data_write_byte_with_busy(0xde);
  c2_data_write_byte_with_busy(0xad);
  c2_data_write_byte_with_busy(0xa5);
  c2_wait_for_bit(C2_OUTREADY,1);
  return c2_ok_or_not();
}

uint16_t c2_write_flash_block(uint16_t address, uint8_t *p_data, uint16_t data_size)
{
  uint16_t i;
  
  if (c2_command_init(C2_COMMAND_BLOCK_WRITE)==0) return 0;
  // Send address.
  c2_data_write_byte_with_busy(address>>8); // MSB
  c2_data_write_byte_with_busy(address&0xff); // LSB
  // Send number of bytes to write to flash.
  c2_data_write_byte_with_busy(data_size);
  c2_wait_for_bit(C2_OUTREADY,1);
  if (c2_ok_or_not()==0) return 0;
  if (data_size==0) data_size = 256; // Zero is short for 256.
  for (i=0; i<data_size; i++)
  {
    c2_data_write_byte_with_busy(p_data[i]);
  } 
  c2_wait_for_bit(C2_OUTREADY,1);
  //if (c2_ok_or_not()==0) return 0;
  //else
  {
    //digitalWrite(9,1);
    return i;
  }
}

uint16_t c2_read_flash_block(uint16_t address, uint8_t *p_data, uint16_t data_size)
{
  uint16_t i;
  if (c2_command_init(C2_COMMAND_BLOCK_READ)==0) return 0;
  // Send address.
  c2_data_write_byte_with_busy(address>>8); // MSB
  c2_data_write_byte_with_busy(address&0xff); // LSB
  // Send number of bytes to read from flash.
  c2_data_write_byte_with_busy(data_size);
  c2_wait_for_bit(C2_OUTREADY,1);
  if (c2_ok_or_not()==0) return 0;
  if (data_size==0) data_size = 256; // Zero is short for 256.
  for (i=0; i<data_size; i++)
  {
    p_data[i] = c2_data_read_byte_with_ready();
  }
  return i;
}
