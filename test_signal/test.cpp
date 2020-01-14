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
#include <cstring>
#include <semaphore.h>

#define XTRX_RSP_TEST_SIGNAL_A (2)

static void GenSignal(void);
static sig_atomic_t loopDone = false;
static void sigIntHandler(const int)
{
    loopDone = true;
}
#pragma pack(1)
double Ibuff[32768];
double Qbuff[32768];
double buffMemIQWr[32768];
double buffMemIQRd[32768];
std::complex <float> buffMem_0[16384];
std::complex <float> Databuff[16384];
int BytesRead=0;
sem_t DataSemaphore;

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
    

    //buffs[0] = &buffMem[0];
    buffs[0] = &buffMemIQWr[0];

    std::memset(buffMemIQWr,0,sizeof(buffMemIQWr));

    //GenSignal();

    //state collected in this loop
    unsigned int overflows(0);
    unsigned int underflows(0);
    unsigned long long totalSamples(0);
    const auto startTime = std::chrono::high_resolution_clock::now();
    auto timeLastPrint = std::chrono::high_resolution_clock::now();
    
    
    #if 1
    std::cout << "Starting TX stream loop, press Ctrl+C to exit..." << std::endl;
    int flagsStream = XTRX_RSP_TEST_SIGNAL_A;
    device->activateStream(stream,flagsStream);

    signal(SIGINT, sigIntHandler);
    FILE  *fileptr;
    fileptr = fopen("tx_plot.txt","w");
    
    while (not loopDone)
    {
        int ret(0);
        int flags(0);
        long long timeNs(0);
        
        //GenSignal();
        //ret = device->writeStream(stream, buffs, 1024*8, flags);
        //}
        
        //printf("CurrentTime: %f \n", CurrentTime);
        //ret = device->writeStream(stream, buffs, elemSize*numElems, flags, timeNs);
        //printf("Here\n", CurrentTime);
        
       
    }
    #endif
    fclose(fileptr);
    
}
void GenSignal(void)
{
    FILE  *fileptr;
    //fileptr = fopen("iq_plot.txt","w");
    double m_amplitude = 0.3;//0.4;
    //double sampleRate = 100e6;
    double sampleRate = 500e3;
    double m_frequency = 100e3;
    double DeltaTime = (double)((double)(1)/(double)sampleRate) ;
    double CurrentTime = 0.0;
    double DeltaPhase = (double)((2*M_PI*(double)(m_frequency/sampleRate)));
    static double CurrentPhase = 0.0;
    double Pifreq = 2*M_PI*m_frequency;
#if 0
    //for(int i=0; i< 1e6; i++)
    {
        for (size_t samples = 0; samples < 32768; samples++)
        {
            //buffMem[samples] =  (double) (m_amplitude * (double)sin(Pifreq*CurrentTime));
            //CurrentTime+=DeltaTime;
            //if(CurrentTime >= 1)
            //{ CurrentTime = 0;}
            //static unsigned int count =0;
            //buffMemIQWr[2*samples] =  (double) ((double)m_amplitude * (double)cos(CurrentPhase));
            //buffMemIQWr[2*samples+1] =  (double) ((double)m_amplitude * (double)sin(CurrentPhase));
            buffMemIQWr[2*samples] =  0.4;
            buffMemIQWr[2*samples+1] = 0.4 ;
            /*buffMemIQ[samples+2] =  -0.5;
            buffMemIQ[samples+3] = 0.5 ;
            buffMemIQ[samples+4] =  -0.5;
            buffMemIQ[samples+5] = -0.5 ;
            buffMemIQ[samples+6] =  0.5;
            buffMemIQ[samples+7] = -0.5 ;*/
            
            //fprintf(fileptr,"%f %f\n",buffMemIQ[samples],buffMemIQ[samples+1]);
            //fprintf(fileptr,"%f %f\n",buffMemIQ[samples+2],buffMemIQ[samples+3]);
            //fprintf(fileptr,"%f %f\n",buffMemIQ[samples+4],buffMemIQ[samples+5]);
            //fprintf(fileptr,"%f %f\n",buffMemIQ[samples+6],buffMemIQ[samples+7]);
            //CurrentPhase+=DeltaPhase;
            //if(CurrentPhase >= 2*M_PI)
            //{ CurrentPhase = 0;}
        }
    }
        #endif
        
    const double sample_rate = 500e3;    //sample rate to 5 MHz
    const double tone_freq = 50e3; //tone frequency
    const double f_ratio = tone_freq/sample_rate;
        const int buffer_size = 1024*8;
    for (int i = 0; i <buffer_size; i++) {      //generate TX tone
        const double pi = acos(-1);
        double w = 2*pi*i*f_ratio;
        buffMemIQWr[2*i] = cos(w);
        buffMemIQWr[2*i+1] = sin(w);
    }   
    
    //fclose(fileptr);
}

void RxLoop(
    SoapySDR::Device *device,
    SoapySDR::Stream *stream,
    const size_t numChans,
    const size_t elemSize)
{
    sem_init(&DataSemaphore, 0, 0);
    static long long unsigned int sampleNum = 0;
    //FILE  *fileptr;
    //fileptr = fopen("rx_plot.txt","w");

    //allocate buffers for the stream read/write
    const size_t numElems = device->getStreamMTU(stream);
    //std::vector<std::vector<char>> buffMem_0(numChans, std::vector<char>(elemSize*numElems));
    //std::complex <float> buffMem_0[elemSize*numElems];
    
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
    std::memset(buffMem_0,0,sizeof(buffMem_0));
    //usleep(20000000);
    device->activateStream(stream);
     
    //setReg();
    signal(SIGINT, sigIntHandler);
    
    while (not loopDone)
    {
        int ret(0);
        int flags(0);
        long long timeNs(0);
        
        BytesRead = ret = device->readStream(stream, buffs, 16384, flags, timeNs);
        if(ret>0)
        {
        memcpy(Databuff,buffMem_0,sizeof(buffMem_0));
        sem_post(&DataSemaphore);
        std::memset(buffMem_0,0,sizeof(buffMem_0));
        }
        
        //printf("ret=%d, flags=%d, timeNs=%lld\n", ret, flags, timeNs);
#if 1
        if(ret >0)
        {
            for (int samples = 0; samples < ret; samples++)
            {
            //printf("%f  ",buffMem_0[0][samples]);
            //fprintf(fileptr,"%li %f %f\n",sampleNum++,buffMem_0[samples].real(), buffMem_0[samples].imag());
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
        //usleep(15000);
    
    }
    //fclose(fileptr);
    //fclose(fileptrReal);
    //fclose(fileptrImg);
    //device->deactivateStream(stream);
}

void DataDump(void)
{
    FILE  *fileptr;
    fileptr = fopen("rx_plot.txt","w");
    signal(SIGINT, sigIntHandler);
    static long long unsigned int sampleNum = 0;

    while (not loopDone)
    {
        sem_wait(&DataSemaphore);
        for (int samples = 0; samples < BytesRead; samples++)
        {
            fprintf(fileptr,"%li %f %f\n",sampleNum++,Databuff[samples].real(), Databuff[samples].imag());
        }
        std::memset(Databuff,0,sizeof(Databuff));
    }
    fclose(fileptr);
}
