//
// Author: Adhyyan Sekhsaria
// GPS Class. Almost identical to MAX8U.

#include "ZEDF9P.h"

namespace UBlox
{
ZEDF9P::ZEDF9P()
{
}

ZEDF9PI2C::ZEDF9PI2C(PinName user_SDApin, PinName user_SCLpin, PinName user_RSTPin,
    uint8_t i2cAddress, int i2cPortSpeed)
    : UBloxGPS(user_RSTPin)
    , UBloxGPSI2C(user_SDApin, user_SCLpin, user_RSTPin, i2cAddress, i2cPortSpeed)
    , ZEDF9P()
{
}

ZEDF9PSPI::ZEDF9PSPI(PinName user_MOSIpin, PinName user_MISOpin, PinName user_RSTPin,
    PinName user_SCLKPin, PinName user_CSPin, int spiClockRate)
    : UBloxGPS(user_RSTPin)
    , UBloxGPSSPI(user_MOSIpin, user_MISOpin, user_RSTPin, user_SCLKPin, user_CSPin, spiClockRate)
    , ZEDF9P()
{
}

bool ZEDF9P::setValue(uint32_t key, uint64_t value, uint8_t layers)
{
    static constexpr int SETUP_BYTES = 4;
    static constexpr int KEY_SIZE = sizeof(key);
    static_assert(KEY_SIZE == 4); // a uint32_t should always be exactly 4 bytes
    static constexpr int MAX_VALUE_SIZE = 8;
    static constexpr int MAX_DATA_LEN = SETUP_BYTES + KEY_SIZE + MAX_VALUE_SIZE;

    /* Get the number of bytes from the key. sizeBits holds bits 30:28. A value of 0x1 indicates 1
     * bit (but must use a full byte), a value of 0x2 represents 1 byte, 0x3 represents 2 bytes, 0x4
     * represents 4 bytes, and 0x5 represents 8 bytes. See Interface Description section 6.2 for
     * more information about the other bits. */
    int sizeBits = (key >> 28) & 0x7;
    int valueLen = 1 << (sizeBits == 1 ? 0 : (sizeBits - 2));

    int totalLen = SETUP_BYTES + KEY_SIZE + valueLen;
    uint8_t data[MAX_DATA_LEN];

    data[0] = 0;
    data[1] = layers;

    memcpy(data + SETUP_BYTES, &key, KEY_SIZE);
    memcpy(data + SETUP_BYTES + KEY_SIZE, &value, valueLen); // Assuming little endinaness

    if (!sendCommand(UBX_CLASS_CFG, UBX_CFG_VALSET, data, totalLen, true, false, 1s))
    {
        printf("Ublox GPS: Failed to set value!\r\n");
        return false;
    }
    DEBUG("UBX GPS: Set value successfully\r\n");
    return true;
}

bool ZEDF9PI2C::configure()
{
    // switch to UBX mode
    bool ret = true;
    ret &= setValue(CFG_I2CINPROT_NMEA, 0);
    ret &= setValue(CFG_I2CINPROT_UBX, 1);

    ret &= setValue(CFG_I2COUTPROT_NMEA, 0);
    ret &= setValue(CFG_I2CINPROT_UBX, 1);
    ret &= setValue(CFG_MSGOUT_UBX_NAV_PVT + MSGOUT_OFFSET_I2C, 1);

    // Explicitly disable raw gps logging
    ret &= setValue(CFG_MSGOUT_UBX_RXM_RAWX + MSGOUT_OFFSET_I2C, 1);

    ret &= setValue(CFG_HW_ANT_CFG_VOLTCTRL, 1);
    return ret;
}

bool ZEDF9PSPI::configure()
{
    // switch to UBX mode
    bool ret = true;
    ret &= setValue(CFG_SPIINPROT_NMEA, 0);
    ret &= setValue(CFG_SPIINPROT_UBX, 1);

    ret &= setValue(CFG_SPIOUTPROT_NMEA, 0);
    ret &= setValue(CFG_SPIOUTPROT_UBX, 1);
    ret &= setValue(CFG_MSGOUT_UBX_NAV_PVT + MSGOUT_OFFSET_SPI, 1);

    // Explicitly disable raw gps logging
    ret &= setValue(CFG_MSGOUT_UBX_RXM_RAWX + MSGOUT_OFFSET_SPI, 0);

    ret &= setValue(CFG_HW_ANT_CFG_VOLTCTRL, 1);
    return ret;
}

bool ZEDF9P::setPlatformModel(ZEDF9P::PlatformModel model)
{
    return setValue(CFG_NAVSPG_DYNMODEL, static_cast<uint8_t>(model));
}

}
