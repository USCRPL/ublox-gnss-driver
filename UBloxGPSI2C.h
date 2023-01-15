#ifndef UBLOXGPS_I2C_H
#define UBLOXGPS_I2C_H

#include "UBloxGPS.h"

namespace UBlox
{
/**
 * @brief Specialization of UBloxGPS for communication over I2C
 */
class UBloxGPSI2C : virtual public UBloxGPS
{
public:
    /**
     * @brief Construct an I2C UBloxGPS, using a preexisting I2C bus object.
     *
     * The UBloxGPSI2C class should not (and can not) be directly instantiated, since it virtually
     * inherits from UBloxGPS. Instead, either ZEDF9PI2C or MAX8UI2C should be instantiated.
     *
     * @note This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param i2c          I2C object connected to the correct %I2C bus.  You must set the desired bus frequency
     *                     on \c i2c before using the GPS.
     * @param user_RSTpin  Output pin connected to NRST
     * @param i2cAddress   I2C address. The MAX8 defaults to 0x42
     */
    UBloxGPSI2C(I2C & i2c, PinName user_RSTpin, uint8_t i2cAddress = UBloxGPS_I2C_DEF_ADDRESS);

protected:
    /**
     * @brief I2C address of the device
     */
    uint8_t i2cAddress_;

    /**
     * @brief I2C port
     */
    I2C & i2cPort_;

private:
    /**
     * @brief Perform an I2C write
     *
     * Send the header then the data to the MAX8 on the configured I2C address
     *
     * @param packet buffer of bytes to send out to the chip
     * @param packetLen number of bytes in packet.
     *
     * @return true if the write was successful, false otherwise (if the sensor does not ACK)
     */
    virtual bool sendMessage(uint8_t* packet, uint16_t packetLen) final;

    /**
     * @brief Perform an I2C read
     *
     * @return ReadStatus::DONE if the read was successful
     *         ReadStatus::NO_DATA if there were no valid bytes available
     *         ReadStatus::ERR if an invalid byte or checksum was detected.
     */
    virtual ReadStatus readMessage() final;

    /**
     * @brief Returns length of buffer in the GPS module's I2C output buffer.
     *
     * @returns Length of buffer, or -1 if unsuccessful.
     */
    int32_t readLen();
};
};

#endif
