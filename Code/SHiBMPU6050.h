//PROJECT   : SHiB MPU6050 Library
//PURPOSE   : Simplified R/W Library for MPU6050
//COURSE    : ICS4U
//AUTHOR    : J. Shibley
//DATE      : 2021-05-06
//MCU       : ATmega328 (Nano)
//STATUS    : Opperational
//REFERENCE :

#include "SHiBI2C.h"

#define mpuI2CAddress   0x68

#define regGyroConfig   0x1B
#define regAccelConfig  0x1C
#define regAccelXoutH   0x3B
#define regPwrMgmt1     0x6B
#define regWhoAmI       0x75

class MPU6050 {
  public:
    void initialize();
    bool testConnection();

    uint8_t getDeviceID();
    void getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);

  private:
    uint8_t buffer[14];
};

void MPU6050::initialize() {
  I2C::writeBitsSHiB(mpuI2CAddress, regPwrMgmt1, 2, 3, 0x01);
  //Sets the clock source to use the X Gyro for reference

  I2C::writeBitsSHiB(mpuI2CAddress, regGyroConfig, 4, 2, 0x00);
  //Sets the gyroscope to its most sensitive settings (+/- 250 degrees/sec)

  I2C::writeBitsSHiB(mpuI2CAddress, regAccelConfig, 4, 2, 0x00);
  //Sets the accelerometer to its most sensitive settings (+/- 2g)

  I2C::writeBitSHiB(mpuI2CAddress, regPwrMgmt1, 6, false);
  //Dissable sleep mode
}//End initialize()

bool MPU6050::testConnection() {
    return getDeviceID() == 0x34;
}//End testConnection()

uint8_t MPU6050::getDeviceID() {
    I2C::readBitsSHiB(mpuI2CAddress, regWhoAmI, 6, 6, buffer);
    return buffer[0];
}//End getDeviceID()

void MPU6050::getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
    I2C::readBytesSHiB(mpuI2CAddress, regAccelXoutH, 14, buffer);
    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];
    *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    *gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}//End getMotion6()
