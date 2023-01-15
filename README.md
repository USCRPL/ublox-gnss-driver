# Mbed OS U-Blox GPS Driver

![U-Blox MAX-8 module](https://media.digikey.com/Photos/U-Blox%20America/MFG_MAX-8.jpg)

This driver can be used in Mbed OS projects to interact with U-Blox Gen8 and Gen9 GNSS modules.  This driver uses U-Blox's UBX protocol, and can communicate with the GNSSs over either I2C or SPI.  Basic information (position, velocity, and time) is supported, as well as timepulse for Gen8 modules and raw satellite information for Gen9 modules.  If you need more, since the driver provides common parsing infrastructure for any UBX messages, it's easy to add these!

Currently, this driver is only written for and tested on the MAX-8 (Gen8) and the ZED-F9P (Gen9) modules.  However, since the command interfaces are pretty similar within a generation, it should work with other GNSS modules from U-Blox as well.

This driver was written and maintained by several members of USC Rocket Propulsion Lab:
- Adhyyan Sekhsaria
- Jamie Smith
- Jay Sridharan
- Jasper Swallen

