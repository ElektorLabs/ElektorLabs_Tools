#include <c2.h>

#define BAUD_RATE   (115200)
#define C2_NO_COMMAND  (0xff)
#define C2_INIT  ('i')
#define C2_RESET  ('r')
#define C2_ERASE_DEVICE  ('e')
#define C2_PAGE_WRITE  ('p')
#define C2_PAGE_READ  ('P')
#define C2_BYTE_WRITE  ('b')
#define C2_BYTE_READ  ('B')
#define C2_GET_DEVICE_ID  ('?')
#define C2_ACK  ('*')
#define C2_NACK  ('?')

/*
Some test commands
:1000008070 - read 16 bytes from 0x0000
:101234802A - read 16 bytes from 0x1234
:00000001FF - reset target
:000000817F - erase target

W :10010000214601360121470136007EFE09D2190140
R :100100806F

W :100110002146017E17C20001FF5F16002148011928
R :100110805F

W :10012000194E79234623965778239EDA3F01B2CAA7
R :100120804F

W :100130003F0156702B5E712B722B732146013421C7
R :100130803F
*/
//#define DEBUG_C2
//#define DEBUG_HEX

#define HEX_REC_TYPE_DATA  (0x00) /* write */
#define HEX_REC_TYPE_EOF  (0x01) /* reset */
#define HEX_REC_TYPE_EXTENDED_SEGMENT_ADDRESS  (0x02)
#define HEX_REC_TYPE_START_SEGMENT_ADDRESS  (0x03)
#define HEX_REC_TYPE_EXTENDED_LINEAR_ADDRESS  (0x04)
#define HEX_REC_TYPE_START_LINEAR_ADDRESS  (0x05)
#define HEX_REC_TYPE_READ  (0x80)
#define HEX_REC_TYPE_ERASE  (0x81)

const uint8_t led_pin = 13;

typedef struct
{
  uint16_t data_size;
  uint16_t address;
  uint8_t command;
  uint8_t data[256];
  uint8_t checksum;
}
c2_command_t;

c2_command_t c2_command;
static uint8_t c2_pi_state = 0;

/*uint16_t build_uint16(uint8_t msb, uint8_t lsb)
{
  return (msb<<8) + lsb;
}*/


char bin2hex[16] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',  
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'  
};

uint8_t hex2bin(uint8_t value)
{
  if (value>='0' && value<='9') return value - '0';
  else if (value>='A' && value<='F') return value - 'A' + 10;
  else if (value>='a' && value<='f') return value - 'a' + 10;
  return 0;
}

void print_hex(uint16_t value, uint8_t digits)
{
  uint16_t offset = 4*(digits-1);
  while (digits!=0)
  {
    Serial.print(bin2hex[(value>>offset)&0xf]);
    offset -= 4;
    digits -= 1;
  }
}

void dump(uint8_t *p_data, uint16_t data_size)
{
  for (int i=0; i<data_size; i+=16)
  {
    for (int j=0; j<16; j++)
    {
      print_hex(p_data[i+j],2);
    }
    Serial.println();
  }
}

uint8_t buffer[256];
void test(void)
{
  uint8_t t;
  //c2_init_programming_interface();
  execute_c2_command(C2_INIT,0);
  
  //Serial.print(c2_data_read_byte(),HEX);
  //Serial.print(c2_read_sfr(0),HEX); // Get device ID.
  //Serial.print(c2_read_sfr(1),HEX); // Get device revision.
  //Serial.println(C2_ACK); // Acknowledge command.

  digitalWrite(9,0);
  digitalWrite(10,0);
  
  /*Serial.println("Start search...");
  delay(100);

  int i = 0xff;
  while (i>0)
  {
    c2_init_programming_interface();
    c2_write_sfr(C2_FLKEY,0xa4);
    c2_write_sfr(C2_FLKEY,i&0xff);
    t = c2_read_sfr(C2_FLKEY);
    if (t==0)
    {
      Serial.print("Found: ");
      Serial.println(i);
      delay(100);
      //break;
    }
    i -= 1;
  }*/
  
  /*t = c2_read_sfr(C2_FLKEY);
  Serial.print("0. FLKEY = ");
  print_hex(t);
  Serial.println();
  delay(100);

  c2_write_sfr(C2_FLKEY,0xa5);
  t = c2_read_sfr(C2_FLKEY);
  Serial.print("1. FLKEY = ");
  print_hex(t);
  Serial.println();
  delay(100);

  c2_write_sfr(C2_FLKEY,0xf2);
  t = c2_read_sfr(C2_FLKEY);
  Serial.print("2. FLKEY = ");
  print_hex(t);
  Serial.println();
  delay(100);

  c2_unlock_flash();
  t = c2_read_sfr(C2_FLKEY);
  Serial.print("4. FLKEY = ");
  print_hex(t);
  Serial.println();
  delay(100);*/

  uint8_t s = 16;
  uint8_t p = 0;
  //uint16_t l = c2_write_flash_block(C2_PAGE_SIZE*p,(uint8_t*)bin2hex,s);
  //Serial.print("Bytes written: ");
  //Serial.println(l);
  //c2_unlock_flash();
  //delay(100);
  //c2_erase_flash_page(C2_PAGE_SIZE*p);
  //c2_erase_device();
  execute_c2_command(C2_ERASE_DEVICE,0);

  //delay(100);
  c2_command.address = C2_PAGE_SIZE*p;
  c2_command.data_size = s;
  memset(c2_command.data,0xaa,sizeof(c2_command.data));
  //c2_read_flash_block(C2_PAGE_SIZE*p,buffer,s);
  execute_c2_command(C2_PAGE_READ,0);
  dump(c2_command.data,c2_command.data_size);
  //delay(500);

  //c2_write_flash_block(C2_PAGE_SIZE*p,(uint8_t*)bin2hex,s);
  c2_command.address = C2_PAGE_SIZE*p;
  c2_command.data_size = s;
  memcpy(c2_command.data,(uint8_t*)bin2hex,c2_command.data_size);
  execute_c2_command(C2_PAGE_WRITE,0);

  //delay(100);
  //memset(buffer,0xaa,sizeof(buffer));
  //c2_read_flash_block(C2_PAGE_SIZE*p,buffer,s);
  //dump(buffer,s);
  c2_command.address = C2_PAGE_SIZE*p;
  c2_command.data_size = s;
  memset(c2_command.data,0xaa,sizeof(c2_command.data));
  execute_c2_command(C2_PAGE_READ,0);
  dump(c2_command.data,c2_command.data_size);
  //delay(500);

  /*c2_erase_flash_page(C2_PAGE_SIZE*p);

  delay(100);
  memset(buffer,0xaa,sizeof(buffer));
  c2_read_flash_block(C2_PAGE_SIZE*p,buffer,s);
  dump(buffer,s);
  delay(500);*/

  /*for (int i=0; i<8192; i+=256)
  {
    Serial.print("--- address = ");
    Serial.print(i);
    if (i%C2_PAGE_SIZE==0)
    {
      Serial.print(", page ");
      Serial.print(i/C2_PAGE_SIZE);
    }
    Serial.println();
    c2_read_flash_block(i,buffer,0);
    //if (c2_read_flash_block(i,buffer,0)!=256)
    //{
    //  Serial.println("read error");
    //}
    dump(buffer,256);
  }*/
  Serial.println("done.");
  delay(1000);

  execute_c2_command(C2_RESET,0);
  //c2_reset();
  //C2_CK_INPUT;
  //C2_D_INPUT;
}

void set_c2_pi_state(uint8_t value)
{
  c2_pi_state = value;
  digitalWrite(led_pin,c2_pi_state);
}

uint8_t get_c2_pi_state(void)
{
  return c2_pi_state;
}

void acknowledge(void)
{
  Serial.print(C2_ACK); // Acknowledge command.
  delay(2);
}

void execute_c2_command(uint8_t cmd, uint8_t ack)
{
#ifdef DEBUG_C2
  Serial.print("C2: ");
  Serial.print((char)cmd);
  Serial.print(" - ");
#endif
  
  switch (cmd)
  {
    case C2_INIT:
#ifdef DEBUG_C2
      Serial.println("init"); delay(100);
#endif
      // Initialize target's programming interface.
      c2_init_programming_interface();
      set_c2_pi_state(1);
      if (ack!=0) acknowledge();
      break;

    case C2_RESET:
#ifdef DEBUG_C2
      Serial.println("reset"); delay(100);
#endif
      // Reset the target.
      c2_reset();
      // Disconnect.
      C2_CK_INPUT;
      C2_D_INPUT;
      set_c2_pi_state(0);
      if (ack!=0) acknowledge();
      break;

    case C2_GET_DEVICE_ID:
#ifdef DEBUG_C2
      Serial.println("id"); delay(100);
#endif
      print_hex(c2_read_sfr(0),2); // Get device ID.
      print_hex(c2_read_sfr(1),2); // Get device revision.
      if (ack!=0) acknowledge();
      break;
    
    case C2_ERASE_DEVICE:
#ifdef DEBUG_C2
      Serial.println("erase"); delay(100);
#endif
      // Erase the target.
      c2_erase_device();
      if (ack!=0) acknowledge();
      break;

    case C2_PAGE_WRITE:
    {
#ifdef DEBUG_C2
      Serial.println("write"); delay(100);
#endif
      // Write page.
      c2_write_flash_block(c2_command.address,c2_command.data,c2_command.data_size);
      if (ack!=0) acknowledge();
      break;
    }

    case C2_PAGE_READ:
    {
#ifdef DEBUG_C2
      Serial.println("read"); delay(100);
#endif
      if (ack!=0) acknowledge();
      // Read page.
      c2_read_flash_block(c2_command.address,c2_command.data,c2_command.data_size);
      // Transmit data to host.
      print_intel_hex(HEX_REC_TYPE_DATA);
      break;
    }

    case C2_BYTE_WRITE:
#ifdef DEBUG_C2
      Serial.println("SFR write"); delay(100);
#endif
      // Write byte.
      c2_write_sfr(c2_command.data[0],c2_command.data[1]);
      if (ack!=0) acknowledge();
      break;

    case C2_BYTE_READ:
#ifdef DEBUG_C2
      Serial.println("SFR read"); delay(100);
#endif
      // Read byte.
      print_hex(c2_read_sfr(c2_command.data[0]),2);
      if (ack!=0) acknowledge();
      break;

    case C2_NO_COMMAND:
      break;

    default:
      Serial.println(C2_NACK); // Do not acknowledge command.
  }
}

void print_intel_hex(uint8_t record_type)
{
  uint8_t checksum = 0;
  Serial.print(':');
  checksum += c2_command.data_size;
  print_hex(c2_command.data_size,2);
  checksum += (c2_command.address>>8)&0xff;
  checksum += c2_command.address&0xff;
  print_hex(c2_command.address,4);
  checksum += record_type;
  print_hex(record_type,2);
  for (int i=0; i<c2_command.data_size; i++)
  {
    checksum += c2_command.data[i];
    print_hex(c2_command.data[i],2);
  }
  print_hex(-1*checksum,2);
  Serial.println();
}

#define HEX_IDLE  (0)
#define HEX_BYTE_COUNT  (1)
#define HEX_ADDRESS  (2)
#define HEX_RECORD_TYPE  (3)
#define HEX_DATA  (4)
#define HEX_CHECKSUM  (5)

uint8_t hex_state_machine(uint8_t value)
{
  static uint8_t state = HEX_IDLE;
  static uint8_t index = 0;
  static uint16_t digit_count = 0;
  static uint8_t byte_value = 0;
  static uint8_t checksum = 0;
  static uint8_t lsb = 0;

  if (lsb==0)
  {
    byte_value = 16*hex2bin(value); // Add MSB.
    lsb = 1;
  }
  else
  {
    byte_value += hex2bin(value); // Add LSB.
    checksum += byte_value; // Update running checksum.
    lsb = 0;
  }

  if (value==C2_NACK) state = HEX_IDLE; // Abort.
  
  switch (state)
  {
    case HEX_IDLE:
      if (value==':')
      {
        // Received start code.
        memset(&c2_command,0,sizeof(c2_command));
        lsb = 0;
        checksum = 0;
        digit_count = 2; // Next: 2 hex digits byte count (0x00 - 0xff).
        state = HEX_BYTE_COUNT;
      }
      break;
    
    case HEX_BYTE_COUNT:
      if (digit_count!=0) digit_count -= 1;
      if (digit_count==0)
      {
        c2_command.data_size = byte_value;
#ifdef DEBUG_HEX
        Serial.print("datasize = ");
        Serial.println(c2_command.data_size);
#endif
        digit_count = 4; // Next: 4 hex digits address (0x0000 - 0xffff).
        state = HEX_ADDRESS;
      }
      break;
    
    case HEX_ADDRESS:
      if (digit_count!=0)
      {
        if (digit_count==3) c2_command.address = 256*byte_value;
        if (digit_count==1) c2_command.address += byte_value;
        digit_count -= 1;
      }
      if (digit_count==0)
      {
#ifdef DEBUG_HEX
        Serial.print("address = 0x");
        print_hex(c2_command.address,4);
        Serial.println();
#endif
        digit_count = 2; // Next: 2 hex digits record type (0x00 - 0xff).
        state = HEX_RECORD_TYPE;
      }
      break;
    
    case HEX_RECORD_TYPE:
      if (digit_count!=0) digit_count -= 1;
      if (digit_count==0)
      {
        // Our own Intel HEX format extension:
        // - if MSBit of Record Type is set, then it is interpreted as a read request.
        //   In this case the Byte Count gives the number of bytes to read, 
        //   the data field is empty.
        c2_command.command = byte_value;
#ifdef DEBUG_HEX
        Serial.print("command = ");
        print_hex(c2_command.command,2);
        Serial.println();
#endif
        index = 0;
        digit_count = 2*c2_command.data_size; // Next: 2n data digits.
        if (digit_count==0 || (c2_command.command&0x80)!=0)
        {
          // No data or Read Request (no data field either), skip to checksum.
          c2_command.checksum = -checksum; // 2's complement.
          digit_count = 2; // Next: 2 hex digits checksum (0x00 - 0xff).
          state = HEX_CHECKSUM;
        }
        else
        {
#ifdef DEBUG_HEX
          Serial.print("data = ");
#endif
          state = HEX_DATA;
        }
      }
      break;
    
    case HEX_DATA:
      if (digit_count!=0)
      {
        if ((digit_count&0x1)==1)
        {
          c2_command.data[index++] = byte_value;
#ifdef DEBUG_HEX
          print_hex(byte_value,2);
          Serial.print(',');
#endif
        }
        digit_count -= 1;
      }
      if (digit_count==0)
      {
#ifdef DEBUG_HEX
        Serial.println();
#endif
        c2_command.checksum = -checksum; // 2's complement.
        digit_count = 2; // Next: 2 hex digits checksum (0x00 - 0xff).
        state = HEX_CHECKSUM;
      }
      break;
    
    case HEX_CHECKSUM:
      if (digit_count!=0) digit_count -= 1;
      if (digit_count==0)
      {
#ifdef DEBUG_HEX
        Serial.print("checksum ");
#endif
        c2_command.checksum -= byte_value; // Result should be 0.
        if (c2_command.checksum!=0)
        {
          // Invalidate command.
          c2_command.checksum = C2_NO_COMMAND;
#ifdef DEBUG_HEX
          Serial.println("fail");
#endif
        }
#ifdef DEBUG_HEX
        else Serial.println("OK");
        delay(100); // Allow any Serial.print commands to terminate.
#endif
        state = HEX_IDLE;
        return c2_command.command;
      }
      break;
  }

  return C2_NO_COMMAND;
}

void setup(void)
{
  // Initialise pins.
  //C2_D_INPUT_PULLUP;
  C2_CK_INPUT;
  C2_D_INPUT;
  //C2_CK_HI;
  //C2_CK_OUTPUT;
  pinMode(led_pin,OUTPUT);

  pinMode(8,INPUT_PULLUP); // debug
  pinMode(9,OUTPUT); // debug
  pinMode(10,OUTPUT); // debug
  digitalWrite(9,0);
  digitalWrite(10,0);
  
  Serial.begin(BAUD_RATE);
  Serial.println("C2 programmer ready...");
  set_c2_pi_state(0);
  delay(300);
}

void loop(void)
{
  if (digitalRead(8)==0)
  {
    test();
    delay(500);
  }
  
  if (Serial.available())
  {
    uint8_t ch = Serial.read();
    uint8_t hex_cmd = hex_state_machine(ch);
    switch (hex_cmd)
    {
      case HEX_REC_TYPE_DATA:
        if (get_c2_pi_state()==0)
        {
          // Target not initialized, do it now.
          execute_c2_command(C2_INIT,0);
          // Erase target.
          execute_c2_command(C2_ERASE_DEVICE,0);
        }
        // Write data.
        execute_c2_command(C2_PAGE_WRITE,1);
        break;
        
      case HEX_REC_TYPE_EOF:
        // Reset target.
        execute_c2_command(C2_RESET,1);
        break;
        
      case HEX_REC_TYPE_EXTENDED_SEGMENT_ADDRESS:
        // Segment addresses are not supported.
        break;
        
      case HEX_REC_TYPE_START_SEGMENT_ADDRESS:
        // Segment addresses are not supported.
        break;
        
      case HEX_REC_TYPE_EXTENDED_LINEAR_ADDRESS:
        // 32-bit addresses are not supported.
        break;
        
      case HEX_REC_TYPE_START_LINEAR_ADDRESS:
        // 32-bit addresses are not supported.
        break;

      case (HEX_REC_TYPE_READ): // Custom record type: read data
        if (get_c2_pi_state()==0)
        {
          // Target not initialized, do it now.
          execute_c2_command(C2_INIT,0);
        }
        // Read data.
        execute_c2_command(C2_PAGE_READ,0);
        break;

      case (HEX_REC_TYPE_ERASE): // Custom record type: erase device
        if (get_c2_pi_state()==0)
        {
          // Target not initialized, do it now.
          execute_c2_command(C2_INIT,0);
        }
        // Erase target.
        execute_c2_command(C2_ERASE_DEVICE,0);
        break;

      // Add more custom record types here.
    }
  }      
}

