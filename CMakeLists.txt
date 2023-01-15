add_library(ubloxgps MAX8U.cpp ZEDF9P.cpp UBloxGPS.cpp UBloxMessages.cpp UBloxGPSI2C.cpp UBloxGPSSPI.cpp)
target_include_directories(ubloxgps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ubloxgps ScopeGuard mbed-os)
target_compile_options(ubloxgps PUBLIC -Wno-unknown-pragmas)
