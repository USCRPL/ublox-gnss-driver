//
// Author: Adhyyan Sekhsaria
//
//

#ifndef UBLOX_UBLOX_GEN_9_H
#define UBLOX_UBLOX_GEN_9_H

#include "UBloxGPS.h"
#include "UBloxGPSConstants.h"

#include "mbed.h"

namespace UBlox
{
/**
 * @brief Class holding functions common to ZED-F9P GPS, independent of serial protocol
 */
class UBloxGen9 : virtual public UBloxGPS
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

    /**
     * @brief see UBloxGPS::configure
     */
    bool configure() override;

    int getGPSGeneration() override
    {
        return 9;
    }

protected:

    uint8_t msgOutOffset_;

    /**
     * The ZEDF9P class should not (and can not) be directly instantiated, since it virtually
     * inherits from UBloxGPS. Instead, either ZEDF9PSPI or ZEDF9PI2C should be instantiated.
     *
     * @param msgOutOffset Offset added to message IDs in the configuration (e.g. CFG_MSGOUT_UBX_NAV_PVT) to select the port.
     */
    UBloxGen9(uint8_t msgOutOffset):
    msgOutOffset_(msgOutOffset)
    {

    }

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

};

#endif // UBLOX_UBLOX_GEN_9_H
