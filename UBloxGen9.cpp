//
// Author: Adhyyan Sekhsaria
// GPS Class. Almost identical to MAX8U.

#include "UBloxGen9.h"

namespace UBlox
{

bool UBloxGen9::setValue(uint32_t key, uint64_t value, uint8_t layers)
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

    data[0] = 0; // Version 0 of the message
    data[1] = layers;
	data[2] = 0; // Reserved
	data[3] = 0; // Reserved

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

bool UBloxGen9::setPlatformModel(UBloxGen9::PlatformModel model)
{
    return setValue(CFG_NAVSPG_DYNMODEL, static_cast<uint8_t>(model));
}

bool UBloxGen9::configure()
{
    // switch to UBX mode
    bool ret = true;
    ret &= setValue(CFG_SPIINPROT_NMEA, 0);
    ret &= setValue(CFG_SPIINPROT_UBX, 1);

    ret &= setValue(CFG_SPIOUTPROT_NMEA, 0);
    ret &= setValue(CFG_SPIOUTPROT_UBX, 1);
    ret &= setValue(CFG_MSGOUT_UBX_NAV_PVT + msgOutOffset_, 1);

    // Explicitly disable raw gps logging
    ret &= setValue(CFG_MSGOUT_UBX_RXM_RAWX + msgOutOffset_, 0);

    ret &= setValue(CFG_HW_ANT_CFG_VOLTCTRL, 1);
    return ret;
}

}
