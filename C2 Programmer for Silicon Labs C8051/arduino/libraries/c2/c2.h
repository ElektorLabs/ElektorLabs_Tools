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

#ifndef __C2_H__
#define __C2_H__

#include <arduino.h>

// Interface pins, adapt to your programmer hardware.
// C2CK pin.
#define C2_CK_BIT  _BV(PC0)
#define C2_CK_PORT PORTC
#define C2_CK_PIN  PINC
#define C2_CK_DDR  DDRC
#define C2_CK_LO  (C2_CK_PORT &= ~C2_CK_BIT)
#define C2_CK_HI  (C2_CK_PORT |= C2_CK_BIT)
#define C2_CK_OUTPUT  (C2_CK_DDR |= C2_CK_BIT)
#define C2_CK_INPUT  (C2_CK_DDR &= ~C2_CK_BIT)
// C2D pin.
#define C2_D_BIT  _BV(PC1)
#define C2_D_PORT  PORTC
#define C2_D_PIN  PINC
#define C2_D_DDR  DDRC
#define C2_D_LO  (C2_D_PORT &= ~C2_D_BIT)
#define C2_D_HI  (C2_D_PORT |= C2_D_BIT)
#define C2_D_IN  ((C2_D_PIN & C2_D_BIT)!=0)
#define C2_D_OUTPUT  (C2_D_DDR |= C2_D_BIT)
#define C2_D_INPUT  (C2_D_DDR &= ~C2_D_BIT)

// C2 Instructions
#define C2_DATA_READ  (0x00)
#define C2_ADDRESS_READ  (0x02)
#define C2_DATA_WRITE  (0x01)
#define C2_ADDRESS_WRITE  (0x03)

// C2 Flash Programming Commands
#define C2_COMMAND_GET_VERSION  (0x01)
#define C2_COMMAND_GET_DERIVATIVE  (0x02)
#define C2_COMMAND_DEVICE_ERASE  (0x03)
#define C2_COMMAND_BLOCK_READ  (0x06)
#define C2_COMMAND_BLOCK_WRITE  (0x07)
#define C2_COMMAND_PAGE_ERASE  (0x08)
#define C2_COMMAND_DIRECT_READ  (0x09)
#define C2_COMMAND_DIRECT_WRITE  (0x0a)
#define C2_COMMAND_INDIRECT_READ  (0x0b)
#define C2_COMMAND_INDIRECT_WRITE  (0x0c)

// C2 Registers
#define C2_DEVICE_ID  (0x00)
#define C2_DEVICE_REVISION  (0x01)
#define C2_FPCTL  (0x02)
//#define C2_FPDAT  (0xad) /* 'F34x, 'F38x, 'T62x/'T32x, EFM8UB2 */
#define C2_FPDAT  (0xb4) /* Most devices, check with datahseet. */
#define C2_FLKEY  (0xb7) /* 'F35x, Check with datahseet. */

#define C2_PAGE_SIZE  (512) /* Most devices, check with datahseet. */
//#define C2_PAGE_SIZE  (1024) /* 'F36x, 'F92x/'F93x, 'F96x, EFM8SB2 */

// C2 Status Bits (there are more but kind of undocumented)
#define C2_BUSY  (1<<7)
#define C2_EERROR  (1<<6)
#define C2_INBUSY  (1<<1)
#define C2_OUTREADY  (1<<0)


void c2_reset(void);
void c2_init_programming_interface(void);
void c2_write_sfr(uint8_t address, uint8_t value); // AN127's WriteSFR
uint8_t c2_read_sfr(uint8_t address); // AN127's ReadSFR
void c2_data_write_byte_with_busy(uint8_t value); // AN127's WriteCommand
uint8_t c2_data_read_byte_with_ready(void); // AN127's ReadData
uint8_t c2_write_direct(uint8_t address, uint8_t value); // AN127's WriteDirect
uint8_t c2_read_direct(uint8_t address); // AN127's ReadDirect
// void c2_unlock_flash(void); // Working but not working...
// uint8_t c2_erase_flash_page(uint8_t page); // Working but not working...
uint8_t c2_erase_device(void);
// c2_write_flash_block only seems to work right after c2_erase_device.
uint16_t c2_write_flash_block(uint16_t address, uint8_t *p_data, uint16_t data_size);
uint16_t c2_read_flash_block(uint16_t address, uint8_t *p_data, uint16_t data_size);


#endif /* __C2_H__ */
