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
     * @brief Construct an I2C UBloxGPS, providing pins and parameters.
     *
     * The UBloxGPSI2C class should not (and can not) be directly instantiated, since it virtually
     * inherits from UBloxGPS. Instead, either ZEDF9PI2C or MAX8UI2C should be instantiated.
     *
     * @note This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param user_SDApin  Hardware I2C SDA pin connected to the MAX8
     * @param user_SCLpin  Hardware I2C SCL pin connected to the MAX8
     * @param user_RSTpin  Output pin connected to NRST
     * @param i2cAddress   I2C address. The MAX8 defaults to 0x42
     * @param i2cPortSpeed I2C frequency.
     */
    UBloxGPSI2C(PinName user_SDApin, PinName user_SCLpin, PinName user_RSTpin,
        uint8_t i2cAddress = UBloxGPS_I2C_DEF_ADDRESS, int i2cPortSpeed = 100000);

protected:
    /*
     * @brief I2C address of the device
     */
    uint8_t i2cAddress_;

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

private:
    /**
     * @brief I2C port
     */
    I2C i2cPort_;

    /**
     * @brief I2C clock speed
     */
    const int i2cPortSpeed_;

    /** The maximum I2C frequency, in Hz (400 kHz) */
    static constexpr int I2C_MAX_SPEED = 4000000;
};
};

#endif
