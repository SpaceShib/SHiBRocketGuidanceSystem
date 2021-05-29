//LIBRARY   : FastWire
//PURPOSE   : Faster programs with I2C devices
//AUTHOR    : F. Ferrara  (occhiobello@gmail.com)
//EDITOR    : J. Shibley
//EDIT DATE : 2021-05-06
//MCU       : ATmega328
//STATUS    : Opperational
//REFERENCE : Copyright(C) 2012 Francesco Ferrara

#include "Arduino.h"

#define TW_START  0x08
#define TW_REP_START            0x10

#define TW_MT_SLA_ACK           0x18
#define TW_MT_SLA_NACK          0x20
#define TW_MT_DATA_ACK          0x28
#define TW_MT_DATA_NACK         0x30
#define TW_MT_ARB_LOST          0x38

/* Master Receiver */
#define TW_MR_ARB_LOST          0x38
#define TW_MR_SLA_ACK           0x40
#define TW_MR_SLA_NACK          0x48
#define TW_MR_DATA_ACK          0x50
#define TW_MR_DATA_NACK         0x58

#define TW_OK                   0
#define TW_ERROR                1

class FastWire {
  private:
    static boolean waitInt();

  public:
    static void setup(int khz, boolean pullup);
    static byte beginTransmission(byte device);
    static byte write(byte value);
    static byte writeBuf(byte device, byte address, byte *data, byte num);
    static byte readBuf(byte device, byte address, byte *data, byte num);
    static void reset();
    static byte stop();
};//End class(FastWire)

boolean FastWire::waitInt() {
  int l = 250;
  while (!(TWCR & (1 << TWINT)) && l-- > 0);
  return l > 0;
}//End waitInt()

void FastWire::setup(int khz, boolean pullup) {
  TWCR = 0;
  // activate internal pull-ups for twi (PORTC bits 4 & 5)
  // as per note from atmega8 manual pg167
  if (pullup) PORTC |= ((1 << 4) | (1 << 5));
  else        PORTC &= ~((1 << 4) | (1 << 5));

  TWSR = 0; // no prescaler => prescaler = 1
  TWBR = F_CPU / 2000 / khz - 8; // change the I2C clock rate
  TWCR = 1 << TWEN; // enable twi module, no interrupt
}//End setup()

byte FastWire::beginTransmission(byte device) {
  byte twst, retry;
  retry = 2;
  do {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWSTA);
    if (!waitInt()) return 1;
    twst = TWSR & 0xF8;
    if (twst != TW_START && twst != TW_REP_START) return 2;

    //Serial.print(device, HEX);
    //Serial.print(" ");
    TWDR = device << 1; // send device address without read bit (1)
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!waitInt()) return 3;
    twst = TWSR & 0xF8;
  } while (twst == TW_MT_SLA_NACK && retry-- > 0);
  if (twst != TW_MT_SLA_ACK) return 4;
  return 0;
}//End beginTransmission();

byte FastWire::writeBuf(byte device, byte address, byte *data, byte num) {
  byte twst, retry;

  retry = 2;
  do {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWSTA);
    if (!waitInt()) return 1;
    twst = TWSR & 0xF8;
    if (twst != TW_START && twst != TW_REP_START) return 2;

    //Serial.print(device, HEX);
    //Serial.print(" ");
    TWDR = device & 0xFE; // send device address without read bit (1)
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!waitInt()) return 3;
    twst = TWSR & 0xF8;
  } while (twst == TW_MT_SLA_NACK && retry-- > 0);
  if (twst != TW_MT_SLA_ACK) return 4;

  //Serial.print(address, HEX);
  //Serial.print(" ");
  TWDR = address; // send data to the previously addressed device
  TWCR = (1 << TWINT) | (1 << TWEN);
  if (!waitInt()) return 5;
  twst = TWSR & 0xF8;
  if (twst != TW_MT_DATA_ACK) return 6;

  for (byte i = 0; i < num; i++) {
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
    TWDR = data[i]; // send data to the previously addressed device
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!waitInt()) return 7;
    twst = TWSR & 0xF8;
    if (twst != TW_MT_DATA_ACK) return 8;
  }
  //Serial.print("\n");

  return 0;
}//End writeBuf()

byte FastWire::write(byte value) {
  byte twst;
  //Serial.println(value, HEX);
  TWDR = value; // send data
  TWCR = (1 << TWINT) | (1 << TWEN);
  if (!waitInt()) return 1;
  twst = TWSR & 0xF8;
  if (twst != TW_MT_DATA_ACK) return 2;
  return 0;
}//End write()

byte FastWire::readBuf(byte device, byte address, byte *data, byte num) {
  byte twst, retry;

  retry = 2;
  do {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWSTA);
    if (!waitInt()) return 16;
    twst = TWSR & 0xF8;
    if (twst != TW_START && twst != TW_REP_START) return 17;

    //Serial.print(device, HEX);
    //Serial.print(" ");
    TWDR = device & 0xfe; // send device address to write
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!waitInt()) return 18;
    twst = TWSR & 0xF8;
  } while (twst == TW_MT_SLA_NACK && retry-- > 0);
  if (twst != TW_MT_SLA_ACK) return 19;

  //Serial.print(address, HEX);
  //Serial.print(" ");
  TWDR = address; // send data to the previously addressed device
  TWCR = (1 << TWINT) | (1 << TWEN);
  if (!waitInt()) return 20;
  twst = TWSR & 0xF8;
  if (twst != TW_MT_DATA_ACK) return 21;

  /***/

  retry = 2;
  do {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1 << TWSTA);
    if (!waitInt()) return 22;
    twst = TWSR & 0xF8;
    if (twst != TW_START && twst != TW_REP_START) return 23;

    //Serial.print(device, HEX);
    //Serial.print(" ");
    TWDR = device | 0x01; // send device address with the read bit (1)
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!waitInt()) return 24;
    twst = TWSR & 0xF8;
  } while (twst == TW_MR_SLA_NACK && retry-- > 0);
  if (twst != TW_MR_SLA_ACK) return 25;

  for (uint8_t i = 0; i < num; i++) {
    if (i == num - 1)
      TWCR = (1 << TWINT) | (1 << TWEN);
    else
      TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    if (!waitInt()) return 26;
    twst = TWSR & 0xF8;
    if (twst != TW_MR_DATA_ACK && twst != TW_MR_DATA_NACK) return twst;
    data[i] = TWDR;
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print("\n");
  stop();

  return 0;
}//End readBuf()

void FastWire::reset() {
  TWCR = 0;
}//End reset()

byte FastWire::stop() {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  if (!waitInt()) return 1;
  return 0;
}//End stop
