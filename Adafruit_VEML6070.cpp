/*!
 * @file Adafruit_VEML6070.cpp
 *
 * @mainpage Adafruit VEML6070 UV sensor
 *
 * @section intro_sec Introduction
 *
 * Designed specifically to work with the VEML6070 sensor from Adafruit
 * ----> https://www.adafruit.com/products/2899
 *
 * These sensors use I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface with the breakout.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "Wire.h"
#include "SoftWire.h"     // SoftWire is available from Arduino Library Manager

#include "Adafruit_VEML6070.h"

/**************************************************************************/
/*! 
    @brief constructor initializes default configuration value
*/
/**************************************************************************/
template <class I2C>
Adafruit_VEML6070<I2C>::Adafruit_VEML6070(I2C *i2c) {
    //default setting
    _commandRegister.reg = 0x02;
    _i2c = i2c;          // Type matching of function makes this safe
}

/**************************************************************************/
/*! 
    @brief  setup and initialize communication with the hardware
    @param itime the integration time to use for the data
    @param twoWire Optional pointer to the desired TwoWire I2C object. Defaults to &Wire
*/
/**************************************************************************/
template <class I2C>
void Adafruit_VEML6070<I2C>::begin(veml6070_integrationtime_t itime) {
  _commandRegister.bit.IT = itime;

  clearAck();
  writeCommand();
}

/**************************************************************************/
/*! 
    @brief  Set the threshold-based interrupt feature
    @param  state true to enable, false to disable
    @param  level 1 for threshold value of 145, 0 for 102 (default)
*/
/**************************************************************************/
template <class I2C>
void Adafruit_VEML6070<I2C>::setInterrupt(bool state, bool level) {
  _commandRegister.bit.ACK = state;
  _commandRegister.bit.ACK_THD = level;

  clearAck();
  writeCommand();
}

/**************************************************************************/
/*! 
    @brief  Clear possible interrupt state (ACK active) by reading register
            If set, MUST be cleared before device will respond at other
            I2C addresses.
            See datasheet rev 1.7, p.7, and app note p. 13 (example code)
    @return True if ACK was active (interrupt triggered)
*/
/**************************************************************************/
template <class I2C>
bool Adafruit_VEML6070<I2C>::clearAck() {
  _i2c->begin();
  bool ret = _i2c->requestFrom(VEML6070_ADDR_ARA, 1);

  #ifdef _DEBUG_
    if (ret) { Serial.println("\nACK cleared\n"); }
  #endif

  return ret;
}

/**************************************************************************/
/*! 
    @brief  read the chips UV sensor
    @return the UV reading as a 16 bit integer
*/
/**************************************************************************/
template <class I2C>
uint16_t Adafruit_VEML6070<I2C>::readUV() {
  waitForNext();

  if (_i2c->requestFrom(VEML6070_ADDR_H, 1) != 1) return -1;
  uint16_t uvi = _i2c->read();
  uvi <<= 8;
  if (_i2c->requestFrom(VEML6070_ADDR_L, 1) != 1) return -1;
  uvi |= _i2c->read();

  return uvi;  
}

/**************************************************************************/
/*! 
    @brief  wait for one integration period (with ~10% clock error margin)
*/
/**************************************************************************/
template <class I2C>
void Adafruit_VEML6070<I2C>::waitForNext() {
  // Map the integration time code to the correct multiple (datasheet p. 8)
  // {0 -> 1, 1 -> 2; 2 -> 4; 3 -> 8}
  uint8_t itCount = 1;
  for (uint8_t i = _commandRegister.bit.IT; i > 0; i--) { itCount *= 2; }

  for (uint8_t i = 0; i < itCount; i++) {
    delay(63);  // Depends on RSET = 270K, note actual time is shorter
                // than 62.5ms for RSET = 300K in datasheet table
  }
}

/**************************************************************************/
/*! 
    @brief  enter or exit sleep (shutdown) mode. While in sleep mode
      the chip draws ~1uA
    @param state true to enter sleep mode, false to exit
*/
/**************************************************************************/
template <class I2C>
void Adafruit_VEML6070<I2C>::sleep(bool state) {
  _commandRegister.bit.SD = state;

  writeCommand();
}


/**************************************************************************/
/*! 
    @brief write current internal _commandRegister value to device
*/
/**************************************************************************/
template <class I2C>
void Adafruit_VEML6070<I2C>::writeCommand() {
  _i2c->begin();
  _i2c->beginTransmission(VEML6070_ADDR_L);
  _i2c->write(_commandRegister.reg);
  _i2c->endTransmission();
}

template class Adafruit_VEML6070<TwoWire>;
template class Adafruit_VEML6070<SoftWire>;
