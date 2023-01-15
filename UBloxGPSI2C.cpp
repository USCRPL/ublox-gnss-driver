#include "UBloxGPSI2C.h"

namespace UBlox
{
UBloxGPSI2C::UBloxGPSI2C(PinName user_SDApin, PinName user_SCLpin, PinName user_RSTPin,
    uint8_t i2cAddress, int i2cPortSpeed)
    : UBloxGPS(user_RSTPin)
    , i2cAddress_(i2cAddress)
    , i2cPort_(user_SDApin, user_SCLpin)
    , i2cPortSpeed_(std::min(i2cPortSpeed, I2C_MAX_SPEED))
{
    // Set I2C port frequency
    i2cPort_.frequency(i2cPortSpeed_);
}

bool UBloxGPSI2C::sendMessage(uint8_t* packet, uint16_t packetLen)
{
    // start the transaction and contact the GPS
    i2cPort_.start();

    // to indicate an i2c read, shift the 7 bit address up 1 bit and keep bit 0 as a 0
    int writeResult = i2cPort_.write(i2cAddress_ << 1);

    if (writeResult != 1)
    {
        printf("%s I2C write failed!\r\n", getName());
        i2cPort_.stop();
        return false;
    }

    DEBUG("%s I2C write acked!\r\n", getName());

    for (uint16_t i = 0; i < packetLen; i++)
    {
        i2cPort_.write(packet[i]);
    }

    i2cPort_.stop();

    return true;
}

UBloxGPS::ReadStatus UBloxGPSI2C::readMessage()
{
    int i2cOutputSize = readLen();
    if (i2cOutputSize == -1)
    {
        printf("Didn't rcv ack from %s when reading length\r\n", getName());
        return ReadStatus::ERR;
    }

    if (i2cOutputSize == 0)
    {
        // nothing to do
        return ReadStatus::NO_DATA;
    }

    i2cPort_.start();
    int readResult = i2cPort_.write((i2cAddress_ << 1) | 0x01);

    if (readResult != 1)
    {
        printf("Didn't receive ack from %s\r\n", getName());
        i2cPort_.stop();
        return ReadStatus::ERR;
    }

    bool isLastByte = false;
    currMessageLength_ = 0;
    int ubxMsgLen = 100000; // large value to stop loop exit condition, will read real value later

    // for loop in case there's a data error and we don't detect the last byte
    for (int rxIndex = 0; rxIndex < i2cOutputSize; rxIndex++)
    {
        int value = i2cPort_.read(!isLastByte);
        if (value == -1)
        {
            i2cPort_.stop();
            return ReadStatus::ERR;
        }
        uint8_t incoming = static_cast<uint8_t>(value);

        if (rxIndex == 0)
        {
            if (incoming == NMEA_MESSAGE_START_CHAR)
            {
                // NMEA sentences start with a dollars sign
                isNMEASentence = true;
            }
            else if (incoming == UBX_MESSAGE_START_CHAR)
            {
                // UBX sentences start with a 0xB5
                isNMEASentence = false;
            }
            else if (incoming == 0xFF)
            {
                DEBUG("Received 0xFF despite output buffer length > 0\r\n");
            }
            else
            {
                printf("Unknown first character \r\n");
                i2cPort_.stop();
                return ReadStatus::ERR;
            }
        }
        else if (rxIndex == 5 && !isNMEASentence)
        {
            // read length and change that to msg length
            ubxMsgLen = (static_cast<uint16_t>(rxBuffer[rxIndex] << 8) | rxBuffer[rxIndex - 1]) + 8;
            // non-payload body of a ubx message is 8
        }

        if (rxIndex <= MAX_MESSAGE_LEN)
        {
            rxBuffer[rxIndex] = incoming;
            currMessageLength_++;
        }

        // if it's an NMEA sentence, there is a CRLF at the end
        // if it's an UBX  sentence, there is a length passed before the payload
        if ((isNMEASentence && incoming == '\n') || (!isNMEASentence && rxIndex == ubxMsgLen - 1))
        {
            break;
        }
    }

    if (currMessageLength_ <= MAX_MESSAGE_LEN)
    {
        // add null terminator
        rxBuffer[currMessageLength_] = 0;
    }

    i2cPort_.stop();

    DEBUG("Read stream of %s: ", getName());
    for (size_t j = 0; j < currMessageLength_; j++)
    {
        DEBUG("%02" PRIx8, rxBuffer[j]);
    }
    DEBUG(";\r\n");

    if (!verifyChecksum(currMessageLength_))
    {
        printf("Checksums for UBX message don't match!\r\n");
        return ReadStatus::ERR;
    }

    processMessage();

    return ReadStatus::DONE;
}

int32_t UBloxGPSI2C::readLen()
{

    i2cPort_.start();
    int i2cStatus = i2cPort_.write((i2cAddress_ << 1) | 0x00);
    if (i2cStatus != 1)
    {
        i2cPort_.stop();
        return -1;
    }
    i2cPort_.write(0xFD);

    i2cPort_.start();
    i2cStatus = i2cPort_.write((i2cAddress_ << 1) | 0x01);
    if (i2cStatus != 1)
    {
        i2cPort_.stop();
        return -1;
    }

    uint8_t highByte = static_cast<uint8_t>(i2cPort_.read(true));
    uint8_t lowByte = static_cast<uint8_t>(i2cPort_.read(false));

    i2cPort_.stop();

    return (static_cast<uint16_t>(highByte << 8) | lowByte);
}
};
