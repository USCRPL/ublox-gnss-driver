//
// Constants for the UBlox GPS
//

#ifndef HAMSTER_UBLOXGPSCONSTANTS_H
#define HAMSTER_UBLOXGPSCONSTANTS_H

#define UBloxGPS_I2C_DEF_ADDRESS 0x42

// Convention we use to simplify parsing packets
#define ANY_MESSAGE_ID 0xFF

// ZED-F9P measured to need at least 675ms after reset before it can accept commands :/
#define BOOT_TIME 675ms

// Max size of command that can be sent to the chip. This value is chosen somewhat
// empirically: it doesn't seem like any of the messages in the datasheet will end up being
// longer than 500 bytes.
#define MAX_MESSAGE_LEN 500

// Characters at the start of every UBX message
#define UBX_SYNC_CHAR_1 0xB5
#define UBX_SYNC_CHAR_2 0x62

// indicies into UBX messages
#define UBX_BYTE_CLASS 2
#define UBX_BYTE_ID 3

#define UBX_DATA_OFFSET 6          // start byte of message data
#define UBX_HEADER_FOOTER_LENGTH 8 // length of message header and footer

// class ACK
#define UBX_CLASS_ACK 0x5
#define UBX_ACK_NACK 0x0
#define UBX_ACK_ACK 0x1

// class CFG
#define UBX_CLASS_CFG 0x6
#define UBX_CFG_PRT 0x0
#define UBX_CFG_MSG 0x1
#define UBX_CFG_RST 0x4
#define UBX_CFG_RATE 0x8
#define UBX_CFG_CFG 0x9
#define UBX_CFG_ANT 0x13
#define UBX_CFG_TP5 0x31
#define UBX_CFG_GNSS 0x3E
#define UBX_CFG_VALSET 0x8A

// class NAV
#define UBX_CLASS_NAV 0x1
#define UBX_NAV_POSLLH 0x2 // LLH stands for Latitude-Longitude-Height
#define UBX_NAV_SOL 0x06
#define UBX_NAV_TIMEUTC 0x21
#define UBX_NAV_SAT 0x35
#define UBX_NAV_VELNED 0x12
#define UBX_NAV_PVT 0x7

// class MON
#define UBX_CLASS_MON 0xA
#define UBX_MON_VER 0x4
#define UBX_MON_HW 0x9
#define UBX_MON_RF 0x38

// class TIM
#define UBX_CLASS_TIM 0x0D
#define UBX_TIM_TP 0x01

// class RXM
#define UBX_CLASS_RXM 0x02
#define UBX_RXM_RAWX 0x15

#define UBX_MESSAGE_START_CHAR 0xB5
#define UBX_MESSAGE_START_CHAR2 0x62
#define NMEA_MESSAGE_START_CHAR '$'

// U-Blox Gen9 configuration IDs
// ---------------------------------------------------------------------------

// section MSGOUT (output rates for various messages)

// offsets to adjust a message out CFG for a specific output port
#define MSGOUT_OFFSET_I2C 0
#define MSGOUT_OFFSET_UART1 1
#define MSGOUT_OFFSET_UART2 2
#define MSGOUT_OFFSET_USB 3
#define MSGOUT_OFFSET_SPI 4

#define CFG_MSGOUT_UBX_NAV_POSLLH 0x20910029
#define CFG_MSGOUT_UBX_NAV_PVT 0x20910006
#define CFG_MSGOUT_UBX_NAV_SAT 0x20910015
#define CFG_MSGOUT_UBX_NAV_VELNED 0x20910042

#define CFG_MSGOUT_UBX_RXM_RAWX 0x209102a4

#define CFG_I2CINPROT_NMEA 0x10710002
#define CFG_I2CINPROT_UBX 0x10710001

#define CFG_I2COUTPROT_UBX 0x10720001
#define CFG_I2COUTPROT_NMEA 0x10720002

#define CFG_SPIINPROT_UBX 0x10790001
#define CFG_SPIINPROT_NMEA 0x10790002

#define CFG_SPIOUTPROT_UBX 0x107a0001
#define CFG_SPIOUTPROT_NMEA 0x107a0002

#define CFG_HW_ANT_CFG_VOLTCTRL 0x10a3002e

#define CFG_NAVSPG_DYNMODEL 0x20110021

#endif // HAMSTER_UBLOXGPSCONSTANTS_H
