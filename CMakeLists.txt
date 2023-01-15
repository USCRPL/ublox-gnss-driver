add_library(ublox-gnss UBloxGen8.cpp UBloxGen9.cpp UBloxGPS.cpp UBloxMessages.cpp UBloxGPSI2C.cpp UBloxGPSSPI.cpp)
target_link_libraries(ublox-gnss mbed-os)

target_include_directories(ublox-gnss PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(ublox-gnss PRIVATE -Wno-unknown-pragmas)

# Use these CMake options to turn debug prints on or off.
option(UBLOX_GNSS_DEBUG "If true, print debug messages from the GNSS driver." FALSE)
option(UBLOX_GNSS_TRANSACTION_DEBUG "If true, print debug messages for individual SPI/I2C transactions" FALSE)

# Apply debug definitions.
# These are only used in cpp files so can be set as PRIVATE.
if(UBLOX_GNSS_DEBUG)
    target_compile_definitions(ublox-gnss PRIVATE UBLOX_GNSS_DEBUG)
endif()
if(UBLOX_GNSS_TRANSACTION_DEBUG)
    target_compile_definitions(ublox-gnss PRIVATE UBLOX_GNSS_TRANSACTION_DEBUG)
endif()