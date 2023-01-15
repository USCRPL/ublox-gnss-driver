#ifndef UBLOXGPS_SPI_H
#define UBLOXGPS_SPI_H

#include "UBloxGPS.h"

namespace UBlox
{
/**
 * @brief Specialization of UBloxGPS for communication over SPI
 */
class UBloxGPSSPI : virtual public UBloxGPS
{
public:
    /**
     * @brief Construct an SPI UBloxGPS, providing pins and parameters.
     *
     * The UBloxGPSSPI class should not (and can not) be directly instantiated, since it virtually
     * inherits from UBloxGPS. Instead, either ZEDF9PSPI or MAX8USPI should be instantiated.
     *
     * @note This doesn't actually initialize the chip, you will need to call begin() for that.
     *
     * @param user_MOSIpin Hardware SPI MOSI pin connected to the MAX8
     * @param user_MISOpin Hardware SPI MISO pin connected to the MAX8
     * @param user_SCLKpin Hardware SPI SCLK pin connected to the MAX8
     * @param user_CSpin   Hardware SPI CS (chip select) connected to the MAX8
     * @param user_RSTpin  Output pin connected to NRST
     * @param spiClockRate The MAX8 can go up to to 5.5 MHz (5500000)
     */
    UBloxGPSSPI(PinName user_MOSIpin, PinName user_MISOpin, PinName user_RSTpin,
        PinName user_SCLKpin, PinName user_CSPin, int spiClockRate = 1000000);

private:
    /**
     * @brief Perform an SPI Transaction, and attempt to exit as quickly as possible
     *
     * @details If packetLen is 0, performSPITransaction will attempt a read-only operation. It
     * will read exactly zero or one packets (depending on if data is immediately available)
     * If packetLen is greater than 0, performSPITransaction will transmit all the data in #packet,
     * while processing any packets that are received. If an RX operation is in progress when all of
     * the TX bytes have been sent out, performSPITransaction will complete the read of the current
     * packet, process it, and exit. If any RX errors occur during the TX of the packet, those
     * errors are ignored until the packet has been completely sent out.
     *
     * @param packet buffer of bytes to send out to the chip
     * @param packetLen number of bytes in packet.
     *
     * @return ReadStatus::DONE if an RX-only operation was initiated, and a packet was read; or if
     * a TX operation was initiated, and the message was sent successfully
     *         ReadStatus::NO_DATA if an RX-only operation was initiated and there was not a valid
     * byte available
     *         ReadStatus::ERR if an invalid byte or checksum was detected.
     */
    ReadStatus performSPITransaction(uint8_t* packet, uint16_t packetLen);

    /**
     * @brief Perform an SPI write
     *
     * @param packet buffer of bytes to send out to the chip
     * @param packetLen number of bytes in packet.
     *
     * @return true if the write was successful, false otherwise.
     */
    virtual bool sendMessage(uint8_t* packet, uint16_t packetLen) final;

    /**
     * @brief Perform an SPI read
     *
     * @return ReadStatus::DONE if the read was successful
     *         ReadStatus::NO_DATA if there were no valid bytes available
     *         ReadStatus::ERR if an invalid byte or checksum was detected.
     */
    virtual ReadStatus readMessage() final;

private:
    /**
     * @brief SPI port
     */
    SPI spiPort_;

    /**
     * @brief SPI clock speed
     */
    const int spiClockRate_;

    /** The maximum SPI frequency, in Hz (5.5 MHz) */
#define UBLOX_SPI_MAX_SPEED 5500000
};
};

#endif
