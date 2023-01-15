#include "UBloxGPSI2C.h"
#include "internal/ScopeGuard.h"

namespace UBlox
{
UBloxGPSI2C::UBloxGPSI2C(I2C & i2c, PinName user_RSTpin, uint8_t i2cAddress)
    : UBloxGPS(user_RSTpin)
    , i2cAddress_(i2cAddress)
    , i2cPort_(i2c)
{
}

bool UBloxGPSI2C::sendMessage(uint8_t* packet, uint16_t packetLen)
{
    // to indicate an i2c write, shift the 7 bit address up 1 bit and keep bit 0 as a 0
    I2C::Result result = i2cPort_.write(i2cAddress_ << 1, reinterpret_cast<const char *>(packet), packetLen);

    if(result == I2C::ACK)
    {
        DEBUG("%s I2C write acked!\r\n", getName());
        return true;
    }
    else
    {
        printf("%s I2C write failed!\r\n", getName());
        return false;
    }
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

    // Set up transaction via scope guard
    auto initI2C = [this]()
    {
        i2cPort_.lock();
        i2cPort_.start();
    };

    auto cleanupI2C = [this]()
    {
        i2cPort_.stop();
        i2cPort_.unlock();
    };

    {
        // This will start the I2C transaction now, and end it whenever it goes out of scope (e.g. by returning
        // from the function)
        ScopeGuard<decltype(initI2C), decltype(cleanupI2C)> spiManager(initI2C, cleanupI2C);

        auto result = i2cPort_.write_byte((i2cAddress_ << 1) | 0x01);

        if (result != I2C::ACK)
        {
            printf("Didn't receive ack from %s\r\n", getName());
            return ReadStatus::ERR;
        }

        currMessageLength_ = 0;
        int ubxMsgLen = 100000; // large value to stop loop exit condition, will read real value later

        // for loop in case there's a data error and we don't detect the last byte
        for (int rxIndex = 0; rxIndex < i2cOutputSize; rxIndex++)
        {
            int readResult = i2cPort_.read_byte(true);
            uint8_t incoming = static_cast<uint8_t>(readResult);

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
    }

    DEBUG_TR("Read stream of %s: ", getName());
    for (size_t j = 0; j < currMessageLength_; j++)
    {
        DEBUG_TR("%02" PRIx8, rxBuffer[j]);
    }
    DEBUG_TR(";\r\n");

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
    // Do a one-byte write to set the register read pointer
    char setReadPointerCmd[] = {0xFD}; // Bytes Available register
    auto result = i2cPort_.write((i2cAddress_ << 1) | 0x00, setReadPointerCmd, 1, true);

    if(result != I2C::ACK)
    {
        return -1;
    }

    // Now read the bytes available register
    char bytesAvailableContents[2];
    result = i2cPort_.read((i2cAddress_ << 1) | 0x01, bytesAvailableContents, 2);

    if(result != I2C::ACK)
    {
        return -1;
    }

    return (static_cast<uint16_t>(bytesAvailableContents[0] << 8) | bytesAvailableContents[0]);
}
};
