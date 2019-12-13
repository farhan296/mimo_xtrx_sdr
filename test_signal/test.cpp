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

    //allocate buffers for the stream read/write
    const size_t numElems = device->getStreamMTU(stream);
    double buffMem[elemSize*numElems];
    void * buffs[]={buffMem};
    double m_amplitude = 0.4;
    double sampleRate = 1e6;
    double m_frequency = sampleRate/10;
    double DeltaTime = (double)((double)(1)/(double)sampleRate) ;
    double CurrentTime = 0.0;


    buffs[0] = &buffMem[0];

    //state collected in this loop
    unsigned int overflows(0);
    unsigned int underflows(0);
    unsigned long long totalSamples(0);
    const auto startTime = std::chrono::high_resolution_clock::now();
    auto timeLastPrint = std::chrono::high_resolution_clock::now();

    std::cout << "Starting TX stream loop, press Ctrl+C to exit..." << std::endl;
    device->activateStream(stream);

    signal(SIGINT, sigIntHandler);

    while (not loopDone)
    {
        int ret(0);
        int flags(0);
        long long timeNs(0);
        
           
        for (size_t samples = 0; samples < elemSize*numElems; samples++)
        {
            buffMem[samples] =  (double) (m_amplitude * (double)sin(2*M_PI*m_frequency*CurrentTime));
            CurrentTime+=DeltaTime; 
        }
        
        //printf("CurrentTime: %f \n", CurrentTime);
        ret = device->writeStream(stream, buffs, elemSize*numElems, flags, timeNs);
        //printf("Here\n", CurrentTime);
        
       
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
    
    void* buffs[]={buffMem_0};
    
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
        
        ret = device->readStream(stream, buffs, elemSize*numElems, flags, timeNs);
        //printf("ret=%d, flags=%d, timeNs=%lld\n", ret, flags, timeNs);
#if 1
        if(ret >0)
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