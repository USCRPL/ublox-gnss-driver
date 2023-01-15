#include "internal/ScopeGuard.h"
#include "UBloxGPSSPI.h"

namespace UBlox
{
UBloxGPSSPI::UBloxGPSSPI(PinName user_MOSIpin, PinName user_MISOpin, PinName user_RSTpin,
    PinName user_SCLKpin, PinName user_CSPin, int spiClockRate)
    : UBloxGPS(user_RSTpin)
    , spiPort_(user_MOSIpin, user_MISOpin, user_SCLKpin, user_CSPin, use_gpio_ssel)
    , spiClockRate_(std::min(spiClockRate, UBLOX_SPI_MAX_SPEED))
{
    spiPort_.format(8, 0); // Setup SPI for 8 bit data, SPI Mode 0. UBLox8 default is SPI Mode 0
    spiPort_.frequency(spiClockRate_);
    spiPort_.lock();
    spiPort_.deselect();
}

UBloxGPS::ReadStatus UBloxGPSSPI::performSPITransaction(uint8_t* packet, uint16_t packetLen)
{

    DEBUG_TR("Beginning SPI transaction ----------------------------------\r\n");

    auto init_spi = [this]() { spiPort_.select(); };

    auto cleanup_spi = [this]() { spiPort_.deselect(); };

    ScopeGuard<decltype(init_spi), decltype(cleanup_spi)> spiManager(init_spi, cleanup_spi);

    // If we are receiving a UBX message, this variable gets filled with the expected length.
    uint32_t ubxMsgLen = 0;

    // If we are receiving a message in this transaction, this index is nonzero and indicates where
    // in the RX buffer to save the current byte of the message
    uint32_t rxIndex = 0;

    // True if this is the first loop of an RX-only transaction.
    bool isRXOnly = packetLen == 0;

    /* CONTINUE WHILE:
     * we still have data to send OR
     * we are in the middle of receiving a packet OR
     * we are trying to start an RX only transaction.
     *
     * QUIT IF:
     * we have been going for 10000 cycles OR
     * checksums do not match OR
     * no data is received with an RX only transaction.
     */
    for (int i = 0; (i < packetLen || rxIndex > 0 || isRXOnly) && i < 10000; i++)
    {
        uint8_t dataToSend = (i < packetLen) ? packet[i] : 0xFF;
        uint8_t incoming = spiPort_.write(dataToSend);

        DEBUG_TR(
            "SPI 0x%" PRIx8 " <--> 0x%" PRIx8 " (rxIndex = %d)\r\n", incoming, dataToSend, rxIndex);

        // last byte of original packet?
        if (i == packetLen - 1)
        {
            DEBUG_TR("Sent packet (% " PRIu16 " bytes): ");
            for (uint16_t j = 0; j < packetLen; j++)
            {
                DEBUG_TR(" %02" PRIx8, packet[j]);
            }
            DEBUG_TR("\r\n");
        }

        if (rxIndex < MAX_MESSAGE_LEN)
        {
            rxBuffer[rxIndex] = incoming;
        }

        // check for the start of a packet
        if (rxIndex == 0)
        {
            switch (incoming)
            {
                case NMEA_MESSAGE_START_CHAR:
                    isNMEASentence = true;
                    break;

                case UBX_MESSAGE_START_CHAR:
                    isNMEASentence = false;
                    break;

                case 0xFF:
                    // 0xFF is sent to indicate no data
                    if (isRXOnly)
                    {
                        return ReadStatus::NO_DATA;
                    }
                    else
                    {
                        continue;
                    }
                default:
                    printf("Received unknown byte 0x%" PRIx8
                           ", not the start of a UBX or NMEA message.\r\n",
                        incoming);
                    rxIndex = 0;
                    continue;
            }
        }
        else if (rxIndex == 5 && !isNMEASentence)
        {
            // Populate ubxMsgLen with the size of the incoming UBX message
            // Add 8 to account for the sync(2) bytes, class, id, length(2) and checksum(2) bytes
            ubxMsgLen = (static_cast<uint16_t>(rxBuffer[rxIndex] << 8) | rxBuffer[rxIndex - 1]) + 8;
        }

        // if it's an NMEA sentence, there is a CRLF at the end
        // if it's an UBX  sentence, there is a length passed before the payload
        if (isNMEASentence && incoming == '\n')
        {
            currMessageLength_ = rxIndex + 1;
            rxIndex = 0;
            if (i >= packetLen)
            {
                return ReadStatus::DONE;
            }
        }
        else if (!isNMEASentence && ubxMsgLen != 0 && rxIndex == ubxMsgLen - 1)
        {
            DEBUG("Received packet (% " PRIu16 " bytes): ", ubxMsgLen);
            for (uint16_t j = 0; j < ubxMsgLen; j++)
            {
                DEBUG(" %02" PRIx8, rxBuffer[j]);
            }
            DEBUG("\r\n");

            if (rxIndex < MAX_MESSAGE_LEN)
            {
                rxBuffer[rxIndex + 1] = 0;
            }

            if (!verifyChecksum(ubxMsgLen))
            {
                printf("Checksums for UBX message don't match!\r\n");
                if (i >= packetLen)
                {
                    return ReadStatus::ERR;
                }
            }

            processMessage();
            currMessageLength_ = rxIndex + 1;
            if (i >= packetLen)
            {
                return ReadStatus::DONE;
            }
        }
        rxIndex++;
    }

    DEBUG_TR("\r\n\r\n");
    return ReadStatus::DONE;
}

bool UBloxGPSSPI::sendMessage(uint8_t* packet, uint16_t packetLen)
{
    return performSPITransaction(packet, packetLen) == ReadStatus::DONE;
}

UBloxGPS::ReadStatus UBloxGPSSPI::readMessage()
{
    return performSPITransaction(nullptr, 0);
}
};
