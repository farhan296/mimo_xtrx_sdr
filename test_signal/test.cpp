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
#include <fstream>

unsigned char trig =0;
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
    FILE  *fileptr;
    fileptr = fopen("tx_plot.txt","w");

    //allocate buffers for the stream read/write
    const size_t numElems = device->getStreamMTU(stream);
    std::vector<std::vector<double>> buffMem(numChans, std::vector<double>(elemSize*numElems));

    std::vector<void *> buffs(numChans);
    double m_amplitude = 0.7;
     double sampleRate = 1e6;
    double m_frequency = sampleRate/100;
    float phaseIncrement = (float)((2*M_PI*m_frequency)/(float)sampleRate) ;
    float currentPhase = 0.0;

    for (size_t i = 0; i < numChans; i++)
    {    for (size_t samples = 0; samples < elemSize*numElems; samples++)
        {
            buffMem[i][samples] =  (double) (m_amplitude * (double)sin(currentPhase));
            fprintf(fileptr,"%lu %f\n",samples, buffMem[i][samples]);
           currentPhase+=phaseIncrement; 
        }

    }

    fclose(fileptr);
    buffs[0] = buffMem[0].data();

    //state collected in this loop
    unsigned int overflows(0);
    unsigned int underflows(0);
    unsigned long long totalSamples(0);
    const auto startTime = std::chrono::high_resolution_clock::now();
    auto timeLastPrint = std::chrono::high_resolution_clock::now();

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
           
            ret = device->writeStream(stream, buffs.data(), elemSize*numElems, flags, timeNs);

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
       
        if (timeLastPrint + std::chrono::seconds(5) < now)
        {
            timeLastPrint = now;
            const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
            const auto sampleRate = double(totalSamples)/timePassed.count();
            printf("\b--TX--%g Msps\t%g MBps", sampleRate, sampleRate*numChans*elemSize);
            if (overflows != 0) printf("\t--TX--Overflows %u", overflows);
            if (underflows != 0) printf("\t--TX--Underflows %u", underflows);
            printf("\n ");
        }
    }
    
}

void RxLoop(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const size_t numChans,
    const size_t elemSize)
{
    FILE  *fileptr;
    fileptr = fopen("rx_plot.txt","w");

    //allocate buffers for the stream read/write
    const size_t numElems = device->getStreamMTU(stream);
    //std::vector<std::vector<char>> buffMem_0(numChans, std::vector<char>(elemSize*numElems));
    std::complex <float> buffMem_0[elemSize*numElems];
    
    std::vector<void *> buffs(numChans);
    
    int loop =0 ;
    //buffs[0] = buffMem_0[0].data();

    buffs[0] = &buffMem_0[0];

    //state collected in this loop
    unsigned int overflows(0);
    unsigned int underflows(0);
    unsigned long long totalSamples(0);
    const auto startTime = std::chrono::high_resolution_clock::now();
    auto timeLastPrint = std::chrono::high_resolution_clock::now();

    std::cout << "Starting RX stream loop, press Ctrl+C to exit..." << std::endl;
    device->activateStream(stream);
    //setReg();
    signal(SIGINT, sigIntHandler);
    
    while (not loopDone)
    {
        int ret(0);
        int flags(0);
        long long timeNs(0);
       if((loop < 1) &&(trig == 1))
       { loop++;
        ret = device->readStream(stream, buffs.data(), elemSize*numElems, flags, timeNs);
        //printf("ret=%d, flags=%d, timeNs=%lld\n", ret, flags, timeNs);
#if 1
//for(size_t samples1 = 0; samples1 < 500; samples1++)
{
        for (int samples = 0; samples < ret; samples++)
        {
            //printf("%f  ",buffMem_0[0][samples]);
            fprintf(fileptr,"%f %f\n",buffMem_0[samples].real(), buffMem_0[samples].imag());
            //fprintf(fileptrReal,"%f\n",buffMem_0[samples].real());
            //fprintf(fileptrImg,"%f\n",buffMem_0[samples].imag());
        }
       }
#endif
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
       
        if (timeLastPrint + std::chrono::seconds(5) < now)
        {
            timeLastPrint = now;
            const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
            const auto sampleRate = double(totalSamples)/timePassed.count();
            printf("\b--RX--%g Msps\t%g MBps", sampleRate, sampleRate*numChans*elemSize);
            if (overflows != 0) printf("\t--RX--Overflows %u", overflows);
            if (underflows != 0) printf("\t--RX--Underflows %u", underflows);
            printf("\n ");
        }
        //usleep(100000);
    
    }
    fclose(fileptr);
    //fclose(fileptrReal);
    //fclose(fileptrImg);
    //device->deactivateStream(stream);
}