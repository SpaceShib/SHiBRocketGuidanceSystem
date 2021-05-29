#include "FastWire.h"

class I2C {
  public:
    I2C();

    static int8_t readBitSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data);
    static int8_t readBitsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
    static int8_t readByteSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t *data);
    static int8_t readWordSHiB(uint8_t devAddr, uint8_t regAddr, uint16_t *data);
    static int8_t readBytesSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
    static int8_t readWordsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);

    static bool writeBitSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data);
    static bool writeBitsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
    static bool writeByteSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t data);
    static bool writeWordSHiB(uint8_t devAddr, uint8_t regAddr, uint16_t data);
    static bool writeBytesSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
    static bool writeWordsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data);

    static uint16_t readTimeout;
};

int8_t I2C::readBitSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data) {
  uint8_t b;
  uint8_t count = readByteSHiB(devAddr, regAddr, &b);
  *data = b & (1 << bitNum);
  return count;
}//End readBit()

int8_t I2C::readBitsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data) {
  uint8_t count, b;
  if ((count = readByteSHiB(devAddr, regAddr, &b)) != 0) {
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    b &= mask;
    b >>= (bitStart - length + 1);
    *data = b;
  }//End if
  return count;
}//End readBits()

int8_t I2C::readByteSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t *data) {
  return readBytesSHiB(devAddr, regAddr, 1, data);
}//End readByte()

int8_t I2C::readWordSHiB(uint8_t devAddr, uint8_t regAddr, uint16_t *data) {
  return readWordsSHiB(devAddr, regAddr, 1, data);
}//End readWord()

int8_t I2C::readBytesSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data) {
  int8_t count = 0;
  uint32_t t1 = millis();
  // FastWire library
  // no loop required for fastwire
  uint8_t status = FastWire::readBuf(devAddr << 1, regAddr, data, length);
  if (status == 0) {
    count = length; // success
  } else {
    count = -1; // error
  }

  return count;
}//End readBytes

int8_t I2C::readWordsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data) {
  int8_t count = 0;
  uint32_t t1 = millis();

  // FastWire library
  // no loop required for fastwire
  uint8_t intermediate[(uint8_t)length * 2];
  uint8_t status = FastWire::readBuf(devAddr << 1, regAddr, intermediate, (uint8_t)(length * 2));
  if (status == 0) {
    count = length; // success
    for (uint8_t i = 0; i < length; i++) {
      data[i] = (intermediate[2 * i] << 8) | intermediate[2 * i + 1];
    }//End for
  } else {
    count = -1; // error
  }//End else
  return count;
}//End readWords

bool I2C::writeBitSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
  uint8_t b;
  readByteSHiB(devAddr, regAddr, &b);
  b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
  return writeByteSHiB(devAddr, regAddr, b);
}

bool I2C::writeBitsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
  uint8_t b;
  if (readByteSHiB(devAddr, regAddr, &b) != 0) {
    uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
    data <<= (bitStart - length + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    b &= ~(mask); // zero all important bits in existing byte
    b |= data; // combine data with existing byte
    return writeByteSHiB(devAddr, regAddr, b);
  } else {
    return false;
  }//End else
}//End writeBits()

bool I2C::writeByteSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
  return writeBytesSHiB(devAddr, regAddr, 1, &data);
}//End writeByte()

bool I2C::writeWordSHiB(uint8_t devAddr, uint8_t regAddr, uint16_t data) {
  return writeWordsSHiB(devAddr, regAddr, 1, &data);
}//End writeWord()

bool I2C::writeBytesSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data) {
  uint8_t status = 0;
  FastWire::beginTransmission(devAddr);
  FastWire::write(regAddr);
  for (uint8_t i = 0; i < length; i++) {
    FastWire::write((uint8_t) data[i]);
  }//End for()
  FastWire::stop();
  //status = FastWire::endTransmission();
  return status == 0;
}//End writeBytes

bool I2C::writeWordsSHiB(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t* data) {
  uint8_t status = 0;
  FastWire::beginTransmission(devAddr);
  FastWire::write(regAddr);
  for (uint8_t i = 0; i < length; i++) {
    FastWire::write((uint8_t)(data[i] >> 8));       // send MSB
    status = FastWire::write((uint8_t)data[i]);   // send LSB
    if (status != 0) break;
  }//End for()
  FastWire::stop();
  //status = FastWire::endTransmission();
  return status == 0;
}//End writeWords
