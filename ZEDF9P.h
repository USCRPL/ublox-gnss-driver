//
// Author: Adhyyan Sekhsaria
//
//

#ifndef HAMSTER_ZEDF9P_H
#define HAMSTER_ZEDF9P_H

#include "UBloxGPS.h"
#include "UBloxGPSConstants.h"

#include "UBloxGPSI2C.h"
#include "UBloxGPSSPI.h"

#include "mbed.h"

namespace UBlox
{
/**
 * @brief Class holding functions common to ZED-F9P GPS, independent of serial protocol
 */
class ZEDF9P : virtual public UBloxGPS
{
public:
    /**
     * Platform model selection. Allows one to choose the environment that the GPS is in.
     * Provides a tradeoff between accuracy and tolerance against motion.
     *
     * See ZED-F9P integration manual section 3.1.7.1 for more info.
     */
    enum class PlatformModel : uint8_t
    {
        PORTABLE = 0,
        STATIONARY = 2,
        PEDESTRIAN = 3,
        AUTOMOT = 4,
        SEA = 5,
        AIR_1G = 6,
        AIR_2G = 7,
        AIR_4G = 8,
        WRIST = 9
    };

    /**
     * Set the platform model in use. Default is PORTABLE on new units.
     * @param model
     * @return
     */
    bool setPlatformModel(PlatformModel model);

protected:
    /**
     * The ZEDF9P class should not (and can not) be directly instantiated, since it virtually
     * inherits from UBloxGPS. Instead, either ZEDF9PSPI or ZEDF9PI2C should be instantiated.
     */
    ZEDF9P();

    /**
     * Implementation of UBX-SET-VAL
     * Used to configure the sensor
     * @param key config key
     * @param value The value associated with the key. It takes care of the size of the int
     * @param layers bitmask which indicates the layer to save the config on the GPS. Flash, BBR,
     *               and RAM
     * @return true if setting was successful and ACK is received.
     */
    bool setValue(uint32_t key, uint64_t value, uint8_t layers = 0x7);

private:
    const char* getName() override { return "ZED-F9P"; };
};

class ZEDF9PI2C : public UBloxGPSI2C, public ZEDF9P
{
public:
    /**
     * Construct a ZEDF9P for I2C use, providing pins and parameters.
     *
     * This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param user_SDApin Hardware I2C SDA pin connected to the ZED-F9P
     * @param user_SCLpin Hardware I2C SCL pin connected to the ZED-F9P
     * @param user_RSTPin Output pin connected to NRST
     * @param i2cAddress I2C address. The ZED-F9P defaults to 0x42
     * @param i2cPortSpeed I2C frequency.
     */
    ZEDF9PI2C(PinName user_SDApin, PinName user_SCLpin, PinName user_RSTPin,
        uint8_t i2cAddress = UBloxGPS_I2C_DEF_ADDRESS, int i2cPortSpeed = 100000);

    /**
     * @brief see UBloxGPS::configure
     */
    bool configure() override;
};

class ZEDF9PSPI : public UBloxGPSSPI, public ZEDF9P
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
        PinName user_CSPin, int spiClockRate = 1000000);

    /**
     * @brief see UBloxGPS::configure
     */
    bool configure() override;
};
};

#endif // HAMSTER_ZEDF9P_H
