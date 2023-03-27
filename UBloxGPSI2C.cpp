#include "UBloxGPSI2C.h"

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
    const size_t ubxHeaderLen = 6;
	const size_t ubxChecksumLen = 2;

	// First check how many bytes are in the buffer
	int32_t bufLen = readLen();
	if(bufLen < 0)
	{
		DEBUG("Didn't receive ack from %s reading len\r\n", getName());
		return ReadStatus::ERR;
	}

	if(static_cast<uint32_t>(bufLen) < ubxHeaderLen)
	{
		// Not a full header in the buffer
		return ReadStatus::NO_DATA;
	}

	if(i2cPort_.read((i2cAddress_ << 1) | 0x01, reinterpret_cast<char *>(rxBuffer), ubxHeaderLen) != 0)
	{
		DEBUG("Didn't receive ack from %s reading header\r\n", getName());
		return ReadStatus::ERR;
	}

	// check format
	if(rxBuffer[0] != UBX_MESSAGE_START_CHAR || rxBuffer[1] != UBX_MESSAGE_START_CHAR2)
	{
		DEBUG("Bad message header received from %s (magic bytes = %" PRIx8 " %" PRIx8"\r\n", getName(), rxBuffer[0], rxBuffer[1]);
		return ReadStatus::ERR;
	}

	size_t ubxMsgRemainingLen = (static_cast<uint16_t>(rxBuffer[5] << 8) | rxBuffer[4]) + ubxChecksumLen;
	currMessageLength_ = ubxMsgRemainingLen + ubxHeaderLen;

	if(currMessageLength_ > MAX_MESSAGE_LEN)
	{
		// can't read this!
		DEBUG("Message too long, %zu bytes.  Bailing out.\r\n", ubxMsgRemainingLen);
		return ReadStatus::ERR;
	}

	// read rest of message
	if(i2cPort_.read((i2cAddress_ << 1) | 0x01, reinterpret_cast<char *>(rxBuffer + ubxHeaderLen), ubxMsgRemainingLen) != 0)
	{
		DEBUG("Didn't receive ack from %s reading body\r\n", getName());
		return ReadStatus::ERR;
	}

	// add null terminator
	rxBuffer[currMessageLength_] = 0;

	DEBUG("Read stream of %s: ", getName());
    for (size_t j = 0; j < currMessageLength_; j++)
    {
        DEBUG("%02" PRIx8, rxBuffer[j]);
    }
    DEBUG(";\r\n");

    if (!verifyChecksum(currMessageLength_))
    {
        DEBUG("Checksums for UBX message don't match!\r\n");
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

    return ((static_cast<uint16_t>(bytesAvailableContents[0]) << 8) | bytesAvailableContents[1]);
}
};
