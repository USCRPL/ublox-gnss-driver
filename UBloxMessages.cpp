#include "UBloxMessages.h"
#include "UBloxGPSConstants.h"
#include <cmath>
#include <cstring>

namespace
{

/**
 * Helper function to read form the data buffer.
 * Numerical values can only be read from aligned memory addresses, but sometimes we need to not do
 * that. This function takes an address and an optional offset, and reads a value of the template
 * type.
 */
template <typename T> T readUnalignedValue(const uint8_t* data, size_t offset = 0)
{
    T value;
    memcpy(&value, data + offset, sizeof(value));
    return value;
}

}

namespace UBlox
{

const char* GNSSNames[7] = { "GPS", "SBAS", "Galileo", "BeiDou", "IMES", "QZSS", "GLONASS" };

const char* SatelliteInfo::getGNSSName() { return GNSSNames[static_cast<uint8_t>(gnss)]; }

GeodeticPosition parseNAV_POSLLH(const uint8_t* msgBuffer)
{
    GeodeticPosition pos;

    pos.longitude = (double)readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 4) * 1e-7;
    pos.latitude = (double)readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 8) * 1e-7;
    pos.height = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 12);

#if UBloxGPS_DEBUG
    printf(
        "Got NAV_POSLLH message.  Longitude=%.06f deg, Latitude=%.06f deg, Height=%.02f mm\r\n",
        longitude,
        latitude,
        height);
#endif

    return pos;
}

FixQuality parseNAV_SOL(const uint8_t* msgBuffer)
{
    FixQuality fix;

    fix.fixQuality = static_cast<GPSFix>(msgBuffer[UBX_DATA_OFFSET + 10]);
    fix.posAccuracy = readUnalignedValue<uint32_t>(msgBuffer, UBX_DATA_OFFSET + 24);
    fix.posAccuracyHor = fix.posAccuracy;
    fix.posAccuracyVer = fix.posAccuracy;
    fix.numSatellites = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 47]);

#if UBloxGPS_DEBUG
    printf("Got NAV_SOL message.  Fix quality=%" PRIu8
                       ", Pos accuracy=%.02f m, Num satellites=%" PRIu8 "\r\n",
        static_cast<uint8_t>(fixQuality),
        posAccuracy,
        numSatellites);
#endif

    return fix;
}

VelocityNED parseNAV_VELNED(const uint8_t* msgBuffer)
{

    VelocityNED velocity;

    velocity.northVel = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 4);
    velocity.eastVel = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 8);
    velocity.downVel = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 12);
    velocity.speed3D = readUnalignedValue<uint32_t>(msgBuffer, UBX_DATA_OFFSET + 16);

#if UBloxGPS_DEBUG
    printf("Got NAV_VELNED message.  North Vel=%" PRIi32 ", East Vel=%" PRIi32
                       ", Down Vel=%" PRIi32 ", 3D Speed=%" PRIi32 "\r\n",
        northVel,
        eastVel,
        downVel,
        speed3D);
#endif

    return velocity;
}

UtcTime parseNAV_TIMEUTC(const uint8_t* msgBuffer)
{
    UtcTime time;

    time.year = readUnalignedValue<uint16_t>(msgBuffer, UBX_DATA_OFFSET + 12);
    time.month = msgBuffer[UBX_DATA_OFFSET + 14];
    time.day = msgBuffer[UBX_DATA_OFFSET + 15];
    time.hour = msgBuffer[UBX_DATA_OFFSET + 16];
    time.minute = msgBuffer[UBX_DATA_OFFSET + 17];
    time.second = msgBuffer[UBX_DATA_OFFSET + 18];

#if UBloxGPS_DEBUG
    printf("Got NAV_TIMEUTC message.  year=%" PRIu16 ", month =%" PRIu8 ", day=%" PRIu8
                       ", hour = %" PRIu8 ", min = %" PRIu8 ", sec = %" PRIu8 "\r\n",
        year,
        month,
        day,
        hour,
        minute,
        second);
#endif

    return time;
}

Timepulse parseTIM_TP(const uint8_t* msgBuffer)
{

    Timepulse pulse;

    pulse.tow.timeOfWeek = readUnalignedValue<uint32_t>(msgBuffer, UBX_DATA_OFFSET + 0);
    pulse.tow.subTimeOfWeek = readUnalignedValue<uint32_t>(msgBuffer, UBX_DATA_OFFSET + 4);
    pulse.timeQuantizationError = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 8);
    pulse.tow.weekNumber = readUnalignedValue<uint16_t>(msgBuffer, UBX_DATA_OFFSET + 12);

#if UBloxGPS_DEBUG
    uint8_t flag = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 14]);
    uint8_t refInfo = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 15]);
    printf("Got TIM-TP message. time of week=%" PRIu32 ", sub ms time=%" PRIu32
                       ", week number= %d"
                       "Quantization error=%" PRIi32 ", flags=%d %d",
        timeOfWeek,
        subTimeOfWeek,
        weekNumber,
        timeQuantizationError,
        flag,
        refInfo);
#endif

    return pulse;
}

void parseNAV_PVT(const uint8_t* msgBuffer, GeodeticPosition& pos, VelocityNED& velocity,
    FixQuality& fix, UtcTime& time)
{

    pos.longitude = (double)readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 24) * 1e-7;
    pos.latitude = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 28) * 1e-7;
    pos.height = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 32);

    velocity.northVel = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 48);
    velocity.eastVel = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 52);
    velocity.downVel = readUnalignedValue<int32_t>(msgBuffer, UBX_DATA_OFFSET + 56);
    velocity.speed3D
        = sqrt(pow(velocity.northVel, 2) + pow(velocity.eastVel, 2) + pow(velocity.downVel, 2));

    fix.fixQuality = static_cast<GPSFix>(msgBuffer[UBX_DATA_OFFSET + 20]);
    fix.numSatellites = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 23]);
    fix.posAccuracyHor = readUnalignedValue<uint32_t>(msgBuffer, UBX_DATA_OFFSET + 40);
    fix.posAccuracyVer = readUnalignedValue<uint32_t>(msgBuffer, UBX_DATA_OFFSET + 44);
    fix.posAccuracy = fmaxf(fix.posAccuracyHor / 10.0f, fix.posAccuracyVer / 10.0f);

    time.year = readUnalignedValue<uint16_t>(msgBuffer, UBX_DATA_OFFSET + 4);
    time.month = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 6]);
    time.day = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 7]);
    time.hour = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 8]);
    time.minute = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 9]);
    time.second = static_cast<uint8_t>(msgBuffer[UBX_DATA_OFFSET + 10]);

#if UBloxGPS_DEBUG
    printf("GOT NAV_PVT\r\n");
    printf("NAV PVT: Longitude=%.06f deg, Latitude=%.06f deg, Height=%.02f mm\r\n",
        longitude,
        latitude,
        height);
#endif
}

}
