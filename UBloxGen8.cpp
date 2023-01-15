#include <UBloxGen8.h>

namespace UBlox
{
UBloxGen8::UBloxGen8()
{
}

bool UBloxGen8::configure()
{
    // Configure DDC(I2C) by writing

    // Configures the MAX8 to output in UBX format instead of NMEA format.

    /*
    UBX-CFG-PRT Payload
    1 PortId  = 0
    1 reserved1
    2 txReady
    4 mode - 7 address and 0 for write
    4 reserved - all 0s
    2 inProtoMask - keep 0th bit on, rest off
    2 outProtoMask - keep 0th bit on, rest off
    2 flags - all 0
    2 reserved - all 0
    */

    static constexpr uint16_t DATA_LEN = 20;
    uint8_t data[DATA_LEN];

    setCFG_PRTPayload(data);

    data[1] = 0; // Reserved

    // disable TX ready
    data[2] = 0;
    data[3] = 0;

    data[5] = 0;
    data[6] = 0;
    data[7] = 0;

    data[8] = 0;
    data[9] = 0;
    data[10] = 0;
    data[11] = 0;

    data[12] = 0x01; // enabling UBX mode for input
    data[13] = 0;

    data[14] = 0x01; // enabling UBX mode for output
    data[15] = 0;

    data[16] = 0;
    data[17] = 0;
    data[18] = 0;
    data[19] = 0;

    if (!sendCommand(UBX_CLASS_CFG, UBX_CFG_PRT, data, DATA_LEN, true, false, 500ms))
    {
        return false;
    }

    // enable NAV messages
    if (!setMessageEnabled(UBX_CLASS_NAV, UBX_NAV_PVT, true))
    {
        return false;
    }

    return saveSettings();
}

bool UBloxGen8::configureTimepulse(uint32_t frequency, float onPercentage, chrono::nanoseconds delayTime)
{
    /**
     * @brief Parameters to configure timepulse
     *
     * Used to pack data, internal to this function
     */
    struct TimepulseParameters
    {
        uint8_t tpIdx = 0;
        uint8_t version = 0x01;
        uint16_t reserved1;           // For padding purposes
        int16_t antCableDelay = 0;    // ns
        int16_t rfGroupDelay = 0;     // ns
        uint32_t freqPeriod = 1;      // Frequency of Period depending on flag. Hz or us
        uint32_t freqPeriodLock = 1;  // Freq or Period when gets lock. Hz or us
        uint32_t pulseLenRatio; // Length or Ratio. us or 2^-32 LSB
        uint32_t pulseLenRatioLock = 0;
        int32_t userConfigDelay = 0; // time pulse delay. ns
        uint32_t flag =
            (1 << 0) | // Activate timepulse
            (1 << 1) | // If set, synchronize time pulse to GNSS as soon as GNSS time is valid.
            (0 << 2) | // Locked Set. Enable the freqPeriodLock and pulseLenRatioLock fields
            (1 << 3) | // when set freqPeriod is Frequency, otherwise its a period
            (0 << 4) | // when set pulseLenRatio is Length, otherwise its a ratio
            (1 << 5) | // Aligning to top of second
            (1 << 6) | // when set "falling edge at top of second", else "rising edge at top of second"
            (1 << 7) | // Time grid to use (0: UTC, 1: GPS, 2: GLONASS, 3: BeiDou, 4: Galileo)
            (0 << 11); // Sync mode. Not relevant if Locked Set is not set.
    } __attribute__((packed));
    static constexpr uint16_t PARAMS_SIZE = sizeof(struct TimepulseParameters);
    static_assert(PARAMS_SIZE == 32); /* Should be 32 bytes (see Interface Description 3.10.22.1) */

    TimepulseParameters params;

    params.freqPeriod = frequency;
    params.pulseLenRatio = static_cast<uint32_t>(onPercentage * (static_cast<float>(std::numeric_limits<uint32_t>::max()) + 1));
    params.userConfigDelay = static_cast<int32_t>(delayTime.count());

    if (!sendCommand(UBX_CLASS_CFG,
            UBX_CFG_TP5,
            reinterpret_cast<uint8_t*>(&params),
            PARAMS_SIZE,
            true,
            false,
            500ms))
    {
        printf("Did not receive ACK for Timepulse Config\r\n");
        return false;
    }
    else
    {
        printf("Timepulse Configuration Successful!");
        return true;
    }
}

bool UBloxGen8::setMessageEnabled(uint8_t messageClass, uint8_t messageID, bool enabled)
{
    static constexpr size_t DATA_LEN = 3;
    uint8_t data[DATA_LEN];

    data[0] = messageClass;                          // byte 0: class
    data[1] = messageID;                             // byte 1: ID
    data[2] = static_cast<uint8_t>(enabled ? 1 : 0); // byte 2: rate

    if (!sendCommand(UBX_CLASS_CFG, UBX_CFG_MSG, data, DATA_LEN, true, false, 500ms))
    {
        printf(
            "Message NOT enabled : 0x%" PRIx8 " , 0x%" PRIx8 " \r\n", messageClass, messageID);
        return false;
    }
    else
    {
        printf(
            "Message Enabled : 0x%" PRIx8 " , 0x%" PRIx8 " \r\n", messageClass, messageID);
        return true;
    }
}


bool UBloxGen8::saveSettings()
{
    static constexpr size_t DATA_LEN = 13;
    uint8_t data[DATA_LEN];

    // don't clear any settings
    memset(data, 0, 4);

    // save all settings
    data[4] = 0b00011111;
    data[5] = 0b00011111;
    data[6] = 0;
    data[7] = 0;

    // Don't load any settings
    memset(data + 8, 0, 4);

    // Save in battery-backed RAM and flash.
    data[12] = 0b11;

    return sendCommand(UBX_CLASS_CFG, UBX_CFG_CFG, data, DATA_LEN, true, false, 1s);
}
}
