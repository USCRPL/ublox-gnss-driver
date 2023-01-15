#include <inttypes.h>
#include <stdlib.h>

#ifndef UBLOX_MESSAGES
#define UBLOX_MESSAGES

namespace UBlox
{
extern const char* GNSSNames[7];

/**
 * @brief U-Blox GNSS identifier
 */
enum class GNSSID : uint8_t
{
    GPS = 0,
    SBAS = 1,
    Galileo = 2,
    BeiDou = 3,
    IMES = 4,
    QZSS = 5,
    GLONASS = 6
};

/**
 * @brief Structure containing info about each satellite
 */
struct SatelliteInfo
{
    /**
     * @brief GNSS that this satellite is from.
     */
    GNSSID gnss;

    /**
     * @brief ID of this satellite in its GNSS
     */
    uint8_t satelliteID;

    /**
     * @brief Carrier-noise ratio in dbHz
     */
    uint8_t signalStrength;

    /**
     * @brief Signal quality indicator (see U-Blox protocol description section 32.17.17.1)
     */
    uint8_t signalQuality;

    /**
     * @brief Get the string name of this satellite's GNSS.
     */
    const char* getGNSSName();

    /**
     * True if this satellite is being used to do navigation
     */
    bool svUsed;
};

/**
 * @brief Indicates the quality of the GPS Fix
 */
enum class GPSFix : uint8_t
{
    NONE = 0,
    DEAD_RECKONING = 1,
    FIX_2D = 2,
    FIX_3D = 3,
    FIX_GPS_DEAD_RECKONING = 4,
    TIME_ONLY = 5
};

/**
 * @brief Antenna Power Status, used in the UBX_MON_HW message
 */
enum class AntennaPowerStatus : uint8_t
{
    OFF = 0,
    ON = 1,
    DONT_KNOW = 2,
    NO_MESSAGE_RCVD = 3
};

/**
 * @brief Structure to hold a geodetic position. (https://en.wikipedia.org/wiki/Geodetic_datum)
 *
 */
struct GeodeticPosition
{
    /**
     * @brief longitude (degrees)
     */
    double longitude;

    /**
     * @brief latitude (degrees)
     */
    double latitude;

    /**
     * @brief Height above ellipsoid (mm)
     */
    int32_t height;
};

/**
 * @brief Structure to hold a geodetic position. (https://en.wikipedia.org/wiki/Geodetic_datum)
 *
 */
struct FixQuality
{
    /**
     * @brief Fix Quality (See UBloxGPS::GPSFix for details)
     */
    GPSFix fixQuality;

    /**
     * @brief Horizontal position accuracy estimate (mm). This expresses the radius around
     * the measured point that the true location may lie.
     */
    uint32_t posAccuracyHor;

    /**
     * @brief Vertical position accuracy estimate (mm). This expresses uncertainty in the height
     * measurement.
     */
    uint32_t posAccuracyVer;

    /**
     * @brief 3D Position Accuracy Estimate (cm). This expresses the 3D radius around the measured
     * point in which the true location may lie.
     */
    uint32_t posAccuracy;

    /**
     * @brief Number of SVs used in Nav Solution
     */
    uint8_t numSatellites;
};

/**
 * @brief Structure to hold velocity information.
 *
 */
struct VelocityNED
{

    /**
     * @brief North velocity component (cm/s)
     */
    int32_t northVel;

    /**
     * @brief East velocity component (cm/s)
     */
    int32_t eastVel;

    /**
     * @brief Down velocity component (cm/s)
     */
    int32_t downVel;

    /**
     * @brief Speed (3-D) (cm/s)
     */
    uint32_t speed3D;
};

/**
 * @brief Structure to hold UTC Time information.
 *
 */
struct UtcTime
{

    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// Struct to represent the GPS Time-Of-Week.
struct GPSTow
{
    uint16_t weekNumber; // number of weeks since Jan 6, 1980
    uint32_t timeOfWeek; // number of milliseconds since midnight Sunday in the current week
    uint32_t subTimeOfWeek; // Fractional part of time of week (1 LSB = 2^(-32) ms)

    bool operator==(GPSTow const & other) const
    {
        return (weekNumber == other.weekNumber) && (timeOfWeek == other.timeOfWeek) && (subTimeOfWeek == other.subTimeOfWeek);
    }
};

/**
 * @brief Structure to hold Timepulse
 *
 */
struct Timepulse
{
    /// The current time-of-week (see above)
    GPSTow tow;

    /**
     * @brief Quantization error of time pulse (ps)
     */
    int32_t timeQuantizationError;
};

/**
 * @brief parse message of type UBX-NAV-POSLLH. This function assumes that the provided
 *        buffer has the correct message type.
 *
 * @note This message outputs the geodetic position in the currently selected ellipsoid.
 *       The default is the WGS84 Ellipsoid, but can be changed. See your chip's datasheet
 *       for more info.
 *
 * @param[in] msgBuffer buffer of message bytes.
 * @return GeodeticPosition parsed from message
 */
GeodeticPosition parseNAV_POSLLH(const uint8_t* msgBuffer);

/**
 * @brief parse message of type UBX-NAV-SOL. This function assumes that the provided
 *        buffer has the correct message type.
 *
 * @note This message combines position, velocity and time solution in ECEF, including accuracy
 *       figures.This message has only been retained for backwards compatibility; users are
 *       recommended to use the UBX-NAV-PVT message in preference.
 *
 * @param[in] msgBuffer buffer of message bytes.
 * @return FixQuality parsed from message
 */
FixQuality parseNAV_SOL(const uint8_t* msgBuffer);

/**
 * @brief parse message of type UBX-NAV-VELNED. This function assumes that the provided
 *        buffer has the correct message type.
 *
 * @param[in] msgBuffer buffer of message bytes.
 * @return VelocityNED parsed from message
 */
VelocityNED parseNAV_VELNED(const uint8_t* msgBuffer);

/**
 * @brief parse message of type UBX-NAV-VELNED. This function assumes that the provided
 *        buffer has the correct message type.
 *
 * @param[in] msgBuffer buffer of message bytes.
 * @return UtcTime parsed from message
 */
UtcTime parseNAV_TIMEUTC(const uint8_t* msgBuffer);

/**
 * @brief parse message of type UBX-NAV-VELNED. This function assumes that the provided
 *        buffer has the correct message type.
 *
 * @note This message contains information on the timing of the next pulse at the TIMEPULSE0 output.
 *       The recommended configuration when using thismessage is to set both the measurement rate
 *       (UBX-CFG-RATE) and the timepulse frequency (UBX-CFG-TP5) to 1 Hz.
 *
 * @param[in] msgBuffer buffer of message bytes.
 * @return UtcTime parsed from message
 */
Timepulse parseTIM_TP(const uint8_t* msgBuffer);

/**
 * @brief parse message of type UBX-NAV-PVY. This function assumes that the provided
 *        buffer has the correct message type.
 *
 * @note This message combines position, velocity and time solution, including accuracy figures
 *
 * @param[in] msgBuffer buffer of message bytes.
 * @param[out] pos buffer to fill position data
 * @param[out] velocity buffer to fill velocity data
 * @param[out] fix buffer to fill fix data
 * @param[out] time buffer to fill time data
 * @return UtcTime parsed from message
 */
void parseNAV_PVT(const uint8_t* msgBuffer, GeodeticPosition& pos, VelocityNED& velocity,
    FixQuality& fix, UtcTime& time);

}

#endif
