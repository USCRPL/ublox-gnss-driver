# Mbed OS U-Blox GPS Driver

This driver can be used in Mbed OS projects to interact with U-Blox's Gen8 and Gen9 GNSS modules.  This driver uses U-Blox's UBX protocol, and can communicate with the GNSSs over either I2C or SPI.  Basic information (position, velocity, and time) is reported in the same format for all modules, as well as timepulse for Gen8 modules and raw satellite information for Gen9 modules.  If you need more, since the driver provides common parsing infrastructure for any UBX messages, it's easy to add these!

Currently, this driver is only written for and tested on the MAX-8 (Gen8) and the ZED-F9P (Gen9) modules.  However, since the command interfaces are pretty similar within a generation, it should work with other GNSS modules from U-Blox as well.

This driver was written and maintained by several members of USC Rocket Propulsion Lab:
- Adhyyan Sekhsaria
- Jamie Smith
- Jay Sridharan
- Jasper Swallen

# About the GNSS Modules

There are a huge range of U-Blox GNSS modules, from small, cheap QFN chips designed for consumer products, all the way up to high-end multi-GNSS receivers that can provide extremely accurate time and position information.  A summary of their product line is available [here](https://www.u-blox.com/sites/default/files/GNSS_LineCard_UBX-13004717.pdf).

Compared to most GNSSs from other manufacturers, an advantage of U-Blox GNSSs is that most support SPI and/or I2C, meaning that data can be received synchronously with no interrupt activity in the background, and the GNSSs can share a bus with other chips.  

Note: It should also be possible to receive from these chips using a background thread on Mbed devices which support asynchronous SPI and I2C, but this is not currently implemented by the driver.

## MAX-8

![U-Blox MAX-8 module](https://content.u-blox.com/sites/default/files/products/MAX-8-top-bottom.png)

The MAX-8 is a smaller and cheaper GNSS module from U-Blox.  It can only receive from one GNSS at a time and lacks an SPI interface, but is a simple, proven, and cost-effective option for many systems.

Note: The MAX-8 lacks flash storage, so any saved settings will be lost if the battery backed RAM loses power.

In this driver, it can be used through the `MAX8I2C` class in `MAX8.h`.

Note: In theory, the MAX-M8 should be able to be used as a drop-in replacement for this module and will appear the same to software but receive from three GNSS systems concurrently.  This has not been tested however. 

## ZED-F9P

![U-Blox ZED-F9P module](https://content.u-blox.com/sites/default/files/products/ZED-F9P-top-bottom.png)

The ZED-F9P is U-Blox's top-of-the-line GNSS module, offering incredible accuracy thanks to its ability to receive all four GNSS systems (GPS, GLONASS, Galileo, BeiDou) at the same time.  It also offers the advanced "High Precision GNSS" feature set, enabling raw outputs of tracking data needed for advanced GPS hacking.

In this driver, it can be used through the `ZEDF9PSPI` and `ZEDF9PI2C` classes in `ZEDF9P.h`.

### Background: Gen8 vs Gen9
In the firmware version jump from Gen 8 to Gen 9, U-Blox threw away and rewrote the entire configuration system: the part of the protocol used to enable different messages and set other various settings.  They also deprecated or removed a number of old messages, such as UBX-NAV-SOL.

For this reason, the driver uses `UBloxGen8` and `UBloxGen9` subclasses, which encapsulate the differences between the two GNSSs.  Each GNSS instance inherits from one of these two classes.

### WARNING: Reset Weirdness
U-Blox, as a company, uses an extremely weird definition of the term "reset pin" which is different from any other hardware vendor I've ever heard of.

On all U-Blox modules I've seen, the pin labeled "RESET" is actually a *factory reset* pin.  It causes all configured data on the GNSS module to be deleted, returning it to factory settings.  I believe this is mainly intended as a recovery tool, in case you get the GNSS into a state where it can't communicate with other devices.

Where you get into trouble is, if you assume that the RESET pin works like a standard reset and toggle it whenever your board boots up.  First of all, this makes you waste time by forcing you to reconfigure all settings every time the GNSS boots.  But also, cached GNSS data such as the ephemeris (which takes 30+ seconds to receive when a GNSS does a "cold boot") is deleted too, meaning the GNSS must reacquire it from scratch. So, if you toggle the reset pin on every boot, any time power is lost, you're going to sit around for 30+ seconds waiting for lock, instead of doing a few-second-long warm boot as intended.  

The only way to do a "normal" reset (reset the digital logic, but keep the data in battery-backed RAM and flash) is via a UBX command, UBX-CFG-RESET.  The `begin()` and `softwareReset()` functions can be used to send this command.

If you do not need factory reset functionality, you're free to tie the module's RESET pin to logic high, and pass NC to the driver for its reset pin.