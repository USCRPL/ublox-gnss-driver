#ifndef UBLOXGPS_H
#define UBLOXGPS_H

#include "UBloxGPSConstants.h"
#include "UBloxMessages.h"
#include "mbed.h"
#include <cinttypes>

#define UBloxGPS_DEBUG 0

// Enable to print bytes sent and received in each SPI transaction
#define UBloxGPS_TRANSACTION_DEBUG 0

namespace UBlox
{
using us_time = std::chrono::microseconds;

class UBloxGPS
{
public:
    /**
     * Types of reset.
     * See ZED-F9P integration manual section 3.14.
     */
    enum class SWResetType : uint16_t
    {
        /**
         * @brief Simulates the receiver being powered down for a short time (<4 hrs)
         */
        HOT_START = 0x0,

        /**
         * @brief Simulates the receiver being powered down for a long time (>4 hrs)
         */
        WARM_START = 0x1,

        /**
         * @brief Clears ALL learned data and simulates a factory-new signal acquisition.
         */
        COLD_START = 0xFFFF
    };

    /**
     * @brief Construct a generic UBloxGPS
     *
     * @param user_RSTPin Output pin connected to NRST
     */
    UBloxGPS(PinName user_RST);

    /**
     * @brief Start a software reset of the above type. Call UBloxGPS::begin after this to
     * re-initialize the chip.
     * @note see details on timing these function calls in the documentation for UBloxGPS::begin
     *
     * @param type type of reset to request. See UBlox::UBloxGPS::SWResetType
     */
    void softwareReset(SWResetType type);

    /**
     * @brief Initialization procedure.
     * @details This method starts a software reset (if one is not already in progress.)
     * If a reset was in progress but is not finished, begin() will wait the remaining time.
     * Additionally, proper communcation with the chip is checked, and settings are written to the
     * chip, if requested.
     *
     * @param[in] shouldConfigure whether or not to configure the chip. Some UBlox GPS units have NVM to
     * store the configuration, so if the unit has previously been configured, you can save time by
     * passing false to this parameter.
     *
     * @return whether or not initialization was successful
     */
    bool begin(bool shouldConfigure);

    /**
     * @brief Attempt to read messages before the timeout. If a message is received before the
     * timeout period ends, continue reading until there is no data left. Otherwise, give up.
     *
     * @param timeout amount of time to wait for packets before giving up. If zero is passed to this
     * argument, the function will try once and quit. Otherwise it will continue to read until the
     * timeout is complete.
     *
     * @return the total number of packets read.
     */
    int update(us_time timeout);

    /**
     * @brief Reads and prints the current enabled GNSS Constellations and prints out the IDs for
     * them
     */
    void printGNSSConfig();

    /**
     * Reads information from the GPS about all the satellites it can see and
     * populates the given buffer.
     *
     * @param satelliteInfos array of SatelliteInfos that the caller allocates.
     * @param infoLen length of the satelliteInfos array.
     *
     * @return The number of satellites the GPS returned info about. If negative, there was an
     * error. If <= the size of the array, then it's the number of valid array entries. If greater
     * than the size of the array, then the max number of array elements were filled in.
     */
    ssize_t getSatelliteInfo(SatelliteInfo satelliteInfos[], size_t infoLen);

    /**
     * @brief Wait to receive a single MON_RF message and returns the power status
     *
     * @return the status of the power of the antenna, or AntennaPowerStatus::NO_MESSAGE_RCVD if we
     * timed out while waiting for a message.
     */
    AntennaPowerStatus getAntennaPowerStatus();

    /**
     * @brief Check the software version.
     * @details If printInfo is true or in debug mode, this will print all version info to the
     * console. Otherwise, it will just check if the info could be read successfully.
     *
     * @param printVersion prints the version info.
     * @param printExtra Info print additional information read from the chip.
     *
     * @return true if the check was successful, false otherwise.
     */
    bool checkVersion(bool printVersion, bool printExtraInfo);

    /**
     * @brief Start a hardware reset of the GPS using the reset pin.
     * @details This is equivalent to a cold start and will cause all GNSS data to be deleted.
     * You will be able to communicate with it once you call begin()
     */
    void hardwareReset();

    /**
     * @brief Request that the GPS send us a timepulse update.
     * @details The timepulse data packet contains nanosecond-accurate data about the next
     * timepulse. Timepulse data is not updated by the standard update cycle since it's only needed
     * in specific situations.
     * @return
     */
    void requestTimepulseUpdate();

    /**
     * @brief State Variable for position.
     * @details This variable is populated when a new message is received.
     * To update this variable explicitly, call waitForMessage() with a message type that contains
     * positional information (NAV_POSLLH or NAV_PVT). Otherwise, call update periodically so that
     * this variable is updated as new information is received. See UBloxMessages.cpp for options.
     *
     * Holds latitude, longitude, and height
     */
    GeodeticPosition position;

    /**
     * @brief State Variable for fix quality.
     * @details This variable is populated when a new message is received.
     * To update this variable, call waitForMessage() with a message type that contains velocity
     * information (NAV_SOL or NAV_PVT). Otherwise, call update periodically so that this variable
     * is updated as new information is received. See UBloxMessages.cpp for options.
     */
    FixQuality fixQuality;

    /**
     * @brief State Variable for velocity
     * @details This variable is populated when a new message is received.
     * To update this variable, call waitForMessage() with a message type that contains velocity
     * information (NAV_VELNED or NAV_PVT). Otherwise, call update() periodically so that this
     * variable is updated as new information is received. See UBloxMessages.cpp for options.
     *
     * Holds north, east, and south velocity, and 3-D speed
     */
    VelocityNED velocity;

    /**
     * @brief State Variable for time.
     * @details This variable is populated when a new message is received.
     * To update this variable, call waitForMessage() with a message type that contains time
     * information (NAV_TIMEUTC or NAV_PVT). Otherwise, call update periodically so that this
     * variable is updated as new information is received. See UBloxMessages.cpp for options.
     */
    UtcTime time;

    /**
     * @brief State Variable for time pulse.
     * @details This variable is populated when a new message is received.
     * To update this variable, call waitForMessage() with a message type that contains timepulse
     * information (TIM_TP). Otherwise, call update periodically so that this variable is updated as
     * new information is received. See UBloxMessages.cpp for options.
     */
    Timepulse timePulse;

    /**
     * @brief State Variable for antenna power status.
     * @details This variable is populated when a new message is received.
     * To update this variable, call getAntennaPowerStatus()
     */
    AntennaPowerStatus antennaPowerStatus;

protected:
    /**
     * Get the name of this GPS module for debug messages
     * @return a c-string with the name of the GPS module
     */
    virtual const char* getName() = 0;

    /**
     * @brief Configure the GPS with the appropriate communications and message settings for this
     * driver. and save the configuration to NVM on the chip. After initial configuration, the
     * settings should be reloaded automatically on power-up
     * @return true if the configuration was successful, false otherwise
     */
    virtual bool configure() = 0;

    /**
     * @brief Assemble a packet with the given payload with the preable, length and checksum, and
     * send it to the chip.
     *
     * @param messageClass class of message being sent
     * @param messageID id of the message being sent
     * @param data buffer containing data payload for the packet
     * @param dataLen length of the data buffer
     * @param waitForACK wait until an acknowledgement message is received.
     * @param waitForResponse wait until a response for the message is received.
     * @param timeout how long to wait before timing out while waiting for ACK or response
     * @return true if the command was sent successfully, false otherwise.
     */
    bool sendCommand(uint8_t messageClass, uint8_t messageID, const uint8_t* data, uint16_t dataLen,
        bool waitForACK, bool waitForResponse, us_time timeout);

    /**
     * @brief RX Buffer to hold an incoming message
     */
    uint8_t rxBuffer[MAX_MESSAGE_LEN + 1];

    /**
     * @brief Flag to indicate the type of message currently in UBloxGPS::spiRxBuffer.
     * @note If true, the message is NMEA, otherwise UBX.
     */
    bool isNMEASentence = false;

    int DEBUG(const char* format, ...);
    int DEBUG_TR(const char* format, ...);

protected:
    /**
     * @brief enum representing possible read outcomes.
     */
    enum class ReadStatus : uint8_t
    {
        DONE = 0,
        NO_DATA,
        ERR
    };

    /**
     * @brief Perform an I2C or SPI write
     *
     * @note waitForACK assumes that the ACK message has not been read yet. To preserve this
     * assumption, when checking for ACK, make sure to call waitForACK immediately after the initial
     * message has been sent. If two messages are sent consecutively, the ACK message for the first
     * message may be received during the TX of the second message, and it will be ignored. Note
     * that not all message types report an ACK, so waitForACK does not always need to follow a call
     * to sendMessage.
     *
     * @param packet buffer of bytes to send out to the chip
     * @param packetLen number of bytes in packet.
     *
     * @return true if the write was successful, false otherwise.
     */
    virtual bool sendMessage(uint8_t* packet, uint16_t packetLen) = 0;

    /**
     * @brief Read EXACTLY zero or one messages from the chip.
     *
     * @return ReadStatus::DONE if a message was successfully read.
     *         ReadStatus::NO_DATA if there was not a valid byte available
     *         ReadStatus::ERR if an invalid byte or checksum was detected.
     */
    virtual ReadStatus readMessage() = 0;

    /**
     * @brief Update state variable from information contained in the message in rxBuffer
     */
    void processMessage();

    /**
     * @brief Verify the validity of the packet in rxBuffer
     * @returns true if the packet is valid, false otherwise.
     */
    bool verifyChecksum(uint32_t messageLength);

    /**
     * @brief Length of message currently in currMessageLength
     */
    size_t currMessageLength_ = 0;

private:
    /**
     * @brief Wait for an ACK for the given message class and ID.
     *
     * @details This function assumes that the ACK message has not been read yet. To preserve this
     * assumption, make sure to always call waitForACK immediately after the initial message has
     * been sent. If two messages are sent consecutively, the ACK message for the first message may
     * be received during the TX of the second message, and it will be ignored.
     *
     * @param sentMessageClass Class of the message for which we are expecting an ACK.
     * @param sentMessageID ID of the message which which we are expecting an ACK
     * @param timeout How long to wait for the message an ACK, before quitting and returning false.
     * @return true if a correct ACK was received, false otherwise.
     */
    bool waitForACK(uint8_t sentMessageClass, uint8_t sentMessageID, us_time timeout = 1500ms);

    /**
     * Wait for a specific message to be received.
     * If we get another message that is not the one we're looking for during this time, then we
     * process that message using processMessage()
     * If the message is not received before the timeout, returns false.
     *
     * @param messageClass Class of the message to wait for
     * @param messageID ID of the of the message to wait for. If msgID doesn't matter, put 0xFF
     * @param timeout How long to wait for the message.
     * @return
     */
    bool waitForMessage(uint8_t messageClass, uint8_t messageID = 0xFF, us_time timeout = 1500ms);

    /**
     * @brief Calculate the checksum for the given packet. The packet should include the sync bytes
     * and rest of header.
     *
     * @param[in] packet pointer to packet
     * @param[in] length of packet, including header and checksum bytes. I.e. data length + 8
     * @param[out] The chka portion of the checksum (first byte)
     * @param[out] The chkb portion of the checksum (second byte)
     * @returns true if the calculation was successful, otherwise false.
     */
    bool calcChecksum(
        const uint8_t* packet, uint32_t packetLen, uint8_t& chka, uint8_t& chkb) const;

    /**
     * @brief Hardware Reset pin
     */
    DigitalOut reset_;

    /**
     * @brief Timer to keep track of time since reset.
     */
    Timer resetTimer_;

    /**
     * @brief Flag to indicate that a reset had been initiated.
     */
    bool resetInProgress_ = false;
};

}

#endif // HAMSTER_UBLOXGPS_H
