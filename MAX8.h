
#ifndef UBLOX_GNSS_MAX8_H
#define UBLOX_GNSS_MAX8_H

#include "UBloxGen8.h"
#include "UBloxGPSSPI.h"
#include "UBloxGPSI2C.h"

namespace UBlox
{
class MAX8I2C : public UBloxGPSI2C, public UBloxGen8
{
public:
    /**
     * Construct a MAX8, providing pins and parameters.
     *
     * This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param i2c          I2C object connected to the correct %I2C bus.  You must set the desired bus frequency
     *                     on \c i2c before using the GPS.
     * @param user_RSTpin  Output pin connected to NRST
     * @param i2cAddress   I2C address. The MAX8 defaults to 0x42
     */
    MAX8I2C(I2C & i2c, PinName user_RSTpin, uint8_t i2cAddress = UBloxGPS_I2C_DEF_ADDRESS):
    UBloxGPS(user_RSTpin),
    UBloxGPSI2C(i2c, user_RSTpin, i2cAddress)
    {}

    const char* getName() override { return "MAX-8 via I2C"; };

private:
    /** Set I2C-specific bits in UBX-CFG-PRT payload */
    void setCFG_PRTPayload(uint8_t* data) override final
    {
        data[0] = 0; // Port Id
        data[4] = (i2cAddress_ << 1);
    }
};
};

#endif //UBLOX_GNSS_MAX8_H
