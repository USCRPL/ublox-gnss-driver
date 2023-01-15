#ifndef MAX8U_H
#define MAX8U_H

#include "UBloxGPS.h"
#include "UBloxGPSConstants.h"

#include "UBloxGPSI2C.h"
#include "UBloxGPSSPI.h"

#include "mbed.h"

#include <chrono>
namespace chrono = std::chrono;

namespace UBlox
{
/**
 * @brief Class holding functions common to MAX-8 GPS, independent of serial protocol
 */
class MAX8U : virtual public UBloxGPS
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
    MAX8U();

    const char* getName() override { return "MAX-8"; };

    /** Set serial protocol-specific bits in UBX-CFG-PRT payload */
    virtual void setCFG_PRTPayload(uint8_t* data) = 0;

private:
    /**
     * Tells the GPS to enable the message indicated by messageClass and messageID
     * This is done by sending a message to the GPS and then waiting for an ack message
     *
     * @return true if an ack message has been received from the gps
     */
    bool setMessageEnabled(uint8_t messageClass, uint8_t messageID, bool enabled);

    /**
     * @brief Save all the current settings to the GPS's flash memory so that they will be loaded
     * when it boots.
     *
     * @return true if the operation was successful, false otherwise.
     */
    bool saveSettings();
};

class MAX8UI2C : public UBloxGPSI2C, public MAX8U
{
public:
    /**
     * Construct a MAX8U, providing pins and parameters.
     *
     * This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param sdaPin Hardware I2C SDA pin connected to the MAX8
     * @param sclPin Hardware I2C SCL pin connected to the MAX8
     * @param rstPin Output pin connected to NRST
     * @param i2cAddress I2C address.  The MAX8 defaults to 0x42
     * @param i2cPortSpeed I2C frequency.
     */
    MAX8UI2C(PinName sdaPin, PinName sclPin, PinName rstPin,
        uint8_t i2cAddress = UBloxGPS_I2C_DEF_ADDRESS, int i2cPortSpeed = 100000);

private:
    /** Set I2C-specific bits in UBX-CFG-PRT payload */
    virtual void setCFG_PRTPayload(uint8_t* data) final;
};

class MAX8USPI : public UBloxGPSSPI, public MAX8U
{
public:
    /**
     * This driver currently does not support MAX-8 over SPI
     */
    MAX8USPI() = delete;

private:
    /** Set SPI-specific bits in UBX-CFG-PRT payload */
    virtual void setCFG_PRTPayload(uint8_t* data) final;
};
};

#endif
