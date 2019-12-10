// Copyright (c) 2016-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <string>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <csignal>
#include <chrono>
#include <cstdio>
#include "/home/AD.PATRIOT1TECH.COM/farhan.naeem/sdr_xtrx/images/sources/liblms7002m/liblms7002m.h"
#include <liblms7002m.h>
#include "/home/AD.PATRIOT1TECH.COM/farhan.naeem/sdr_xtrx/images/sources/build/liblms7002m/lms7002m_defs.h"
#include "/home/AD.PATRIOT1TECH.COM/farhan.naeem/sdr_xtrx/images/sources/libxtrx/xtrx_fe.h"
#include <math.h>

static sig_atomic_t loopDone = false;
static void sigIntHandler(const int)
{
    loopDone = true;
}

void runRateTestStreamLoop(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const int direction,
    const size_t numChans,
    const size_t elemSize)
{
    
    unsigned char toggleVal = 0;
    //allocate buffers for the stream read/write
    const size_t numElems = device->getStreamMTU(stream);
    std::vector<std::vector<double>> buffMem_1(numChans, std::vector<double>(elemSize*numElems));
    std::vector<std::vector<char>> buffMem_0(numChans, std::vector<char>(elemSize*numElems));
    std::vector<void *> buffs(numChans);
    std::vector<void *> buffs1(numChans);
    std::vector<void *> buffs0(numChans);
    double values[elemSize*numElems];
    double m_amplitude = 0.7;
     double sampleRate = 1e6;
    double m_frequency = sampleRate/10;
    double m_phase = 0.0;
    double m_time = 0.0;
    
    double m_deltaTime = (double)((double)1 / (double)sampleRate); 
    for (size_t i = 0; i < numChans; i++)
    {    for (size_t samples = 0; samples < elemSize*numElems; samples++)
        {
            //buffMem_1[i][j] = 1U;
            //buffMem_0[i][j] = 0U;
            double rads = M_PI/180;
            //buffMem_1[i][m_time] = (double)((double)m_amplitude*(double)sin(2*M_PI*(m_frequency /sampleRate )*m_time + M_PI/2));
            buffMem_1[i][samples] = (double)((double)m_amplitude*(double)sin(2*M_PI* m_frequency* m_time + m_phase));
            m_time += m_deltaTime;
        }

    }
    buffs1[0] = buffMem_1[0].data();
     buffs0[0] = buffMem_0[0].data();
    //state collected in this loop
    unsigned int overflows(0);
    unsigned int underflows(0);
    unsigned long long totalSamples(0);
    const auto startTime = std::chrono::high_resolution_clock::now();
    auto timeLastPrint = std::chrono::high_resolution_clock::now();
    auto timeLastSpin = std::chrono::high_resolution_clock::now();
    auto timeLastStatus = std::chrono::high_resolution_clock::now();
    int spinIndex(0);

    std::cout << "Starting TX stream loop, press Ctrl+C to exit..." << std::endl;
    device->activateStream(stream);
    //setReg();
    signal(SIGINT, sigIntHandler);
    while (not loopDone)
    {
        int ret(0);
        int flags(0);
        long long timeNs(0);
        switch(direction)
        {
        case SOAPY_SDR_RX:
            ret = device->readStream(stream, buffs.data(), numElems, flags, timeNs);
            break;
        case SOAPY_SDR_TX:
           // if(toggleVal == 0)
            //{
              //  toggleVal = 1;
              //  ret = device->writeStream(stream, buffs0.data(), numElems, flags, timeNs);
            //}
            //else
            //{
              //  toggleVal = 0;
                ret = device->writeStream(stream, buffs1.data(), elemSize*numElems, flags, timeNs);
            //}
            break;
        }

        if (ret == SOAPY_SDR_TIMEOUT) continue;
        if (ret == SOAPY_SDR_OVERFLOW)
        {
            overflows++;
            continue;
        }
        if (ret == SOAPY_SDR_UNDERFLOW)
        {
            underflows++;
            continue;
        }
        if (ret < 0)
        {
            std::cerr << "Unexpected stream error " << SoapySDR::errToStr(ret) << std::endl;
            break;
        }
        totalSamples += ret;

        const auto now = std::chrono::high_resolution_clock::now();
        /*if (timeLastSpin + std::chrono::milliseconds(300) < now)
        {
            timeLastSpin = now;
            static const char spin[] = {"|/-\\"};
            printf("\b%c", spin[(spinIndex++)%4]);
            fflush(stdout);
        }*/
        //occasionally read out the stream status (non blocking)
        /*if (timeLastStatus + std::chrono::seconds(1) < now)
        {
            timeLastStatus = now;
            while (true)
            {
                size_t chanMask; int flags; long long timeNs;
                ret = device->readStreamStatus(stream, chanMask, flags, timeNs, 0);
                if (ret == SOAPY_SDR_OVERFLOW) overflows++;
                else if (ret == SOAPY_SDR_UNDERFLOW) underflows++;
                else if (ret == SOAPY_SDR_TIME_ERROR) {}
                else break;
            }
        }*/
        if (timeLastPrint + std::chrono::seconds(5) < now)
        {
            timeLastPrint = now;
            const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
            const auto sampleRate = double(totalSamples)/timePassed.count();
            printf("\b%g Msps\t%g MBps", sampleRate, sampleRate*numChans*elemSize);
            if (overflows != 0) printf("\tOverflows %u", overflows);
            if (underflows != 0) printf("\tUnderflows %u", underflows);
            printf("\n ");
        }
        //usleep(1000);
    }
    //device->deactivateStream(stream);
}

void RxLoop(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const int direction,
    const size_t numChans,
    const size_t elemSize)
{
    //allocate buffers for the stream read/write
    const size_t numElems = device->getStreamMTU(stream);
    std::vector<std::vector<char>> buffMem_0(numChans, std::vector<char>(elemSize*numElems));
    std::vector<void *> buffs(numChans);
    
    
    buffs[0] = buffMem_0[0].data();

    //state collected in this loop
    unsigned int overflows(0);
    unsigned int underflows(0);
    unsigned long long totalSamples(0);
    const auto startTime = std::chrono::high_resolution_clock::now();
    auto timeLastPrint = std::chrono::high_resolution_clock::now();
    auto timeLastSpin = std::chrono::high_resolution_clock::now();
    auto timeLastStatus = std::chrono::high_resolution_clock::now();
    int spinIndex(0);

    std::cout << "Starting RX stream loop, press Ctrl+C to exit..." << std::endl;
    device->activateStream(stream);
    //setReg();
    signal(SIGINT, sigIntHandler);
    while (not loopDone)
    {
        int ret(0);
        int flags(0);
        long long timeNs(0);
        
        ret = device->readStream(stream, buffs.data(), elemSize*numElems, flags, timeNs);
        //printf("ret=%d, flags=%d, timeNs=%lld\n", ret, flags, timeNs);
#if 1
        for (size_t samples = 0; samples < elemSize*numElems; samples++)
        {
            printf("%f  ",buffMem_0[0][samples]);
        }
#endif
        if (ret == SOAPY_SDR_TIMEOUT) continue;
        if (ret == SOAPY_SDR_OVERFLOW)
        {
            overflows++;
            continue;
        }
        if (ret == SOAPY_SDR_UNDERFLOW)
        {
            underflows++;
            continue;
        }
        if (ret < 0)
        {
            std::cerr << "Unexpected stream error " << SoapySDR::errToStr(ret) << std::endl;
            break;
        }
        totalSamples += ret;

        const auto now = std::chrono::high_resolution_clock::now();
       
        if (timeLastPrint + std::chrono::seconds(5) < now)
        {
            timeLastPrint = now;
            const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
            const auto sampleRate = double(totalSamples)/timePassed.count();
            printf("\b%g Msps\t%g MBps", sampleRate, sampleRate*numChans*elemSize);
            if (overflows != 0) printf("\tOverflows %u", overflows);
            if (underflows != 0) printf("\tUnderflows %u", underflows);
            printf("\n ");
        }
        //usleep(100000);
    }
    //device->deactivateStream(stream);
}