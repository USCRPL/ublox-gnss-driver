#ifndef UBLOX_UBLOX_GEN_8_H
#define UBLOX_UBLOX_GEN_8_H

#include "UBloxGPS.h"
#include "UBloxGPSConstants.h"

#include "mbed.h"

#include <chrono>
namespace chrono = std::chrono;

namespace UBlox
{
/**
 * @brief Class holding functions common to MAX-8 GPS, independent of serial protocol
 */
class UBloxGen8 : virtual public UBloxGPS
{
public:
    /**
     * @brief see UBloxGPS::configure
     */
    virtual bool configure() override;

    /**
     * Enables timepulse functionality for the sensor
     */
    bool configureTimepulse(uint32_t frequency, float onPercentage, chrono::nanoseconds delayTime);

protected:
    /**
     * The MAX8U class should not (and can not) be directly instantiated, since it virtually
     * inherits from UBloxGPS. Instead, either MAX8USPI or MAX8UI2C should be instantiated.
     */
    UBloxGen8();

    /** Set serial protocol-specific bits in UBX-CFG-PRT payload */
    virtual void setCFG_PRTPayload(uint8_t *data) = 0;

private:
    /**
     * Tells the GPS to enable the message indicated by messageClass and messageID
     * This is done by sending a message to the GPS and then waiting for an ack message
     *
     * @return true if an ack message has been received from the gps
     */
    bool setMessageEnabled(uint8_t messageClass, uint8_t messageID, bool enabled);

    /**
     * @brief Save all the current settings so that they will be loaded
     * when it boots.  Always saves to battery-backed RAM, which will keep the settings saved unless
     * battery power is removed from the module.  Also, on modules with flash memory (does not include the MAX-8),
     * the settings are saved permanently in flash.
     *
     * @return true if the operation was successful, false otherwise.
     */
    bool saveSettings();
};

}

#endif
