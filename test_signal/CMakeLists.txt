########################################################################
# Project setup -- only needed if device support is a stand-alone build
# We recommend that the support module be built in-tree with the driver.
########################################################################
cmake_minimum_required(VERSION 2.6)
project(test_sig CXX)

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

set(MY_DEVICE_INCLUDE_DIRS ${SoapySDR_INCLUDE_DIRS})
set(MY_DEVICE_LIBRARIES ${SoapySDR_LIBRARIES})

if (NOT SoapySDR_FOUND)
    message(WARNING "SoapySDR development files not found - skipping support")
    return()
endif ()


add_executable(test_sig test.cpp)
target_link_libraries(test_sig PUBLIC ${MY_DEVICE_LIBRARIES})