########################################################################
# Project setup -- only needed if device support is a stand-alone build
# We recommend that the support module be built in-tree with the driver.
########################################################################
cmake_minimum_required(VERSION 2.6)
project(xtrx_frontend CXX)

enable_testing()

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# Header and library resources needed to communicate with the device.
# These may be found within the build tree or in an external project.
########################################################################

########################################################################
# build the module
########################################################################
find_package(SoapySDR CONFIG)
find_package(Threads)

set(MY_DEVICE_INCLUDE_DIRS ${SoapySDR_INCLUDE_DIRS})
set(MY_DEVICE_LIBRARIES ${SoapySDR_LIBRARIES})

if (NOT SoapySDR_FOUND)
    message(WARNING "SoapySDR development files not found - skipping support")
    return()
endif ()

include_directories(${MY_DEVICE_INCLUDE_DIRS})
#SOAPY_SDR_MODULE_UTIL(
#    TARGET xtrx_frontend
#    SOURCES test.cpp
#    LIBRARIES ${MY_DEVICE_LIBRARIES}
#)

add_executable(xtrx_frontend main.cpp sdr_util.cpp ${PROJECT_SOURCE_DIR}/test_signal/test.cpp)
target_link_libraries(xtrx_frontend PUBLIC ${MY_DEVICE_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})