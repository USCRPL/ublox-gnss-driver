#ifndef UBLOX_ZEDF9P_H
#define UBLOX_ZEDF9P_H

#include "UBloxGen9.h"
#include "UBloxGPSSPI.h"
#include "UBloxGPSI2C.h"

namespace UBlox
{

class ZEDF9PI2C : public UBloxGPSI2C, public UBloxGen9
{
public:
    /**
     * Construct a ZEDF9P for I2C use, providing pins and parameters.
     *
     * This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param i2c          I2C object connected to the correct %I2C bus.  You must set the desired bus frequency
     *                     on \c i2c before using the GPS.
     * @param user_RSTpin  Output pin connected to NRST
     * @param i2cAddress   I2C address. The MAX8 defaults to 0x42
     */
    ZEDF9PI2C(I2C & i2c, PinName user_RSTpin, uint8_t i2cAddress = UBloxGPS_I2C_DEF_ADDRESS):
    UBloxGPS(user_RSTpin),
    UBloxGPSI2C(i2c, user_RSTpin, i2cAddress),
    UBloxGen9(MSGOUT_OFFSET_SPI)
    {}
};

class ZEDF9PSPI : public UBloxGPSSPI, public UBloxGen9
{
public:
    /**
     * Construct a ZEDF9P for SPI use, providing pins and parameters.
     *
     * This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param user_MOSIpin Hardware SPI MOSI pin connected to the ZED-F9P
     * @param user_MISOpin Hardware SPI MISO pin connected to the ZED-F9P
     * @param user_SCLKPin Hardware SPI SCLK pin connected to the ZED-F9P
     * @param user_CSPin   Hardware SPI CS pin connected to the ZED-F9P
     * @param spiClockRate SPI frequency
     */
    ZEDF9PSPI(PinName user_MOSIpin, PinName user_MISOpin, PinName user_RSTPin, PinName user_SCLKPin,
        PinName user_CSPin, int spiClockRate = 1000000)
    : UBloxGPS(user_RSTPin)
    , UBloxGPSSPI(user_MOSIpin, user_MISOpin, user_RSTPin, user_SCLKPin, user_CSPin, spiClockRate)
    , UBloxGen9(MSGOUT_OFFSET_SPI)
    {
    }
};

}
#endif //UBLOX_ZEDF9P_H
