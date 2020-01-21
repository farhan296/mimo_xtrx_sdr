/*
 * sdr_util.cpp
 * Copyright (c) 2020 Farhan Naeem <farhann@patriot1tech.com>
 */

/*******************************************************************************
 * Include Files
 ******************************************************************************/
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

//TODO: Remove this in final code
#include "/home/AD.PATRIOT1TECH.COM/farhan.naeem/sdr_xtrx/mimo_dev/mimo_xtrx_sdr/test_signal/test.hpp"
#include "sdr_util.hpp"
/*******************************************************************************
 * Local MACRO
 ******************************************************************************/
#define MAX_SDR_DEVICES 8U
#define CHAN_0          0U
#define CHAN_1          1U
#define MAX_TX_CH       2U
#define MAX_RX_CH       2U

sem_t DataGatherSemaphore;
sem_t DataDumpedSemaphore;
sem_t mutex_lock;
/*******************************************************************************
 * Local Data Structure
 ******************************************************************************/
class SdrDevice
{
    unsigned char DeviceId;
    
     /* Specify Tx and Rx sample rate (Default: 1e6) */
    double SampleRateBB;

    /* Specify Tx digital amplitude (Default: 0.7) */
    double Amplitude;

    /* Specify Tx gain (dB) (Default: 52 (max) Range: 0 - 52) */
    double TxGain;

    /* Specify Tx gain (dB) (Default: 52 (max) Range: 0 - 30) */
    double RxGain;

    /* Specify Tx antenna (Default: TXH, options: TXH, TXW) */
    std::string TxAnt;

    /* Specify Tx channel (Default: 0, options: 0, 1) */ 
    size_t TxChannel;

    /* Specify Tx and Rx freq (Hz) (Default: 2.47e9) */
    double CarrierFreq;

    /* Baseband waveform freq (Hz) (Default: 100e3) */
    double BaseBandFreq;

    /* Tx and Rx Bandwidth (Default: 40e6) */
    double Bandwidth;

    
    public:
    
    /* Destructor */
    ~SdrDevice();

    /* Constructor */
    SdrDevice();

    void SetSampleRateVal(double vlaue);
    double GetSampleRateVal(void);
    void SetAmpl(double vlaue);
    double GetAmpl(void);
    void SetTxGain(double vlaue);
    double GetTxGain(void);
    void SetRxGain(double vlaue);
    double GetRxGain(void);
    void SetTxAnt(std::string ant);
    std::string GetTxAnt(void);
    void SetCarrierFreq(double vlaue);
    double GetCarrierFreq(void);
    void SetBaseBandFreq(double vlaue);
    double GetBaseBandFreq(void);
    void SetBandwidth(double vlaue);
    double GetBandwidth(void);

    SoapySDR::Device *DeviceHandlePtr;
    static unsigned char InstanceCounter;
    
    /* Pointer to stream */
    SoapySDR::Stream *TxStreamHandle;
    SoapySDR::Stream *RxStreamHandle;
};

/* Initialize static mmber of class */
unsigned char SdrDevice::InstanceCounter = 0;

/* List to store SDR device handle */
std::vector <SdrDevice *> SdrDeviceList (MAX_SDR_DEVICES);
/*******************************************************************************
 * Local Function Declaration
 ******************************************************************************/
static void ShowUsage (std::string name);
static void ParseParam(std::string param);
static void ConfigureSdrDevice(void);
static void SetupTxStream(void);
static void RunTest(void);
static void SetupRxStream(void);
static void *TxThreadTest(void *);
static void *RxThreadTest(void *);
static void *DataGather(void *);

/*******************************************************************************
 * @brief This function enumerates connected SDR devices on PCIe bus
 *
 * @param void
 * @return void
 ******************************************************************************/
const SoapySDR::KwargsList EnumerateSdr(void)
{
    /* Just a tag to make things work*/
    std::string tag = "mimo=sdr";
    const auto results = SoapySDR::Device::enumerate(tag);
   
    for (size_t i = 0; i < results.size(); i++)
    {
        std::cout << "Found device " << i << std::endl;
        for (const auto &it : results[i])
        {
            std::cout << "  " << it.first << " = " << it.second << std::endl;
        }
        std::cout << std::endl;
    }
    if (results.empty()) std::cerr << "No devices found!" << std::endl;
    else std::cout << std::endl;

    return results;
}

/*******************************************************************************
 * @brief This function opens and configures the SDR devices
 *
 * @param device_list List of devices obtained after enumeration
 * @return void
 ******************************************************************************/
void OpenAndConfigureSdrDevice(SoapySDR::KwargsList &device_list)
{
    SoapySDR::Device *device(nullptr);
//int i=1;
    /* Open SDR devices */
    for (unsigned int i=0; i < device_list.size(); i++)
    {
        device = SoapySDR::Device::make_after_enum(device_list[i]);

        if(device != nullptr)
        {
            SdrDeviceList[i] = new SdrDevice;
            SdrDeviceList[i]->DeviceHandlePtr = device;
        }
    }

    /* Configure SDR Devices */
    ConfigureSdrDevice();

    /* Setup Tx Stream */
    SetupTxStream();

    /* Setup Rx Stream */
    SetupRxStream();

    printf("Instance Created: %d\n\r",SdrDevice::InstanceCounter);

    /* FIXME: Remove this in final code */
    RunTest();
}
/*******************************************************************************
 * @brief This function is constructor for SdrDevice class
 *
 * @param void
 * @return void
 ******************************************************************************/
SdrDevice::SdrDevice()
{
    
    DeviceId = SdrDevice::InstanceCounter++;

    SoapySDR::Device *DeviceHandlePtr = nullptr;
    
     /* Specify Tx and Rx sample rate (Default: 1e6) */
    SampleRateBB = 1e6;

    /* Specify Tx digital amplitude (Default: 0.7) */
    Amplitude = 0.7;

    /* Specify Tx gain (dB) (Default: 52 (max) Range: 0 - 52) */
    TxGain = 52;

    /* Specify Tx gain (dB) (Default: 30 (max) Range: 0 - 30) */
    RxGain = 30;

    /* Specify Tx antenna (Default: TXH, options: TXH, TXW) */
    TxAnt = "TXH";

    /* Specify Tx channel (Default: 0, options: 0, 1) */ 
    TxChannel = 0;

    /* Specify Tx and Rx freq (Hz) (Default: 2.47e9) */
    CarrierFreq = 2.47e9; 

    /* Baseband waveform freq (Hz) (Default: 100e3) */
    BaseBandFreq = 100e3;

    /* Bandwidth (Default: 40e6) */
    Bandwidth = 40e6;
}

/*******************************************************************************
 * @brief This function is destructor for SdrDevice class
 *
 * @param void
 * @return void
 ******************************************************************************/
SdrDevice::~SdrDevice()
{
    //TODO: Make sure to unmake/close all open SDR devices as well
    //FIXME: Not being called check the cause
    for(unsigned int i=0; i < SdrDeviceList.size(); i++)
    {
        delete SdrDeviceList[i];
        printf("DelSdrDev=%i",i);
    }
}

/*******************************************************************************
 * @brief This function sets the sample rate for both Tx and Rx (DAC / ADC)
 *
 * @param value Value to set to
 * @return void
 ******************************************************************************/
inline void SdrDevice::SetSampleRateVal(double vlaue)
{
    this->SampleRateBB = vlaue;
}

/*******************************************************************************
 * @brief This function gets the sample rate for both Tx and Rx (DAC / ADC)
 *
 * @param void
 * @return SampleRateBB
 ******************************************************************************/
inline double SdrDevice::GetSampleRateVal(void)
{
    return (this->SampleRateBB);
}

/*******************************************************************************
 * @brief This function sets the bandwidth freq for Tx and Rx
 *
 * @param value Value to set to
 * @return void
 ******************************************************************************/
inline void SdrDevice::SetBandwidth(double vlaue)
{
    this->Bandwidth = vlaue;
}

/*******************************************************************************
 * @brief This function sets the bandwidth freq for Tx and Rx
 *
 * @param void
 * @return Bandwidth
 ******************************************************************************/
inline double SdrDevice::GetBandwidth(void)
{
    return (this->Bandwidth);
}

/*******************************************************************************
 * @brief This function sets the Tx gain
 *
 * @param value Value to set to
 * @return void
 ******************************************************************************/
inline void SdrDevice::SetTxGain(double vlaue)
{
    this->TxGain = vlaue;
}

/*******************************************************************************
 * @brief This function returns the Tx gain
 *
 * @param void
 * @return Tx gain
 ******************************************************************************/
inline double SdrDevice::GetTxGain(void)
{
    return (this->TxGain);
}

/*******************************************************************************
 * @brief This function sets the Rx gain
 *
 * @param value Value to set to
 * @return void
 ******************************************************************************/
inline void SdrDevice::SetRxGain(double vlaue)
{
    this->RxGain = vlaue;
}

/*******************************************************************************
 * @brief This function returns the Rx gain
 *
 * @param void
 * @return Tx gain
 ******************************************************************************/
inline double SdrDevice::GetRxGain(void)
{
    return (this->RxGain);
}

/*******************************************************************************
 * @brief This function sets the carrier frequency
 *
 * @param value Value to set to
 * @return void
 ******************************************************************************/
inline void SdrDevice::SetCarrierFreq(double vlaue)
{
    this->CarrierFreq = vlaue;
}

/*******************************************************************************
 * @brief This function returns the carrier frequency
 *
 * @param void
 * @return Carrier frequency
 ******************************************************************************/
inline double SdrDevice::GetCarrierFreq(void)
{
    return (this->CarrierFreq);
}


/*******************************************************************************
 * @brief This function configures the SDR device parameters
 *
 * @param void
 * @return void
 ******************************************************************************/
static void ConfigureSdrDevice(void)
{
    //int i=1;
    for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
        SdrDeviceList[i]->DeviceHandlePtr->setAntenna(SOAPY_SDR_TX,CHAN_0,"TXH");
        SdrDeviceList[i]->DeviceHandlePtr->setAntenna(SOAPY_SDR_RX,CHAN_0,"LNAH");
        
        
        /* Set Tx sample rate (ADC/DAC rate) */
        //TODO: verify if only one setting is enough to configure for all channels in TX and RX direction
        //SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetSampleRateVal());
        SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_TX,CHAN_0,2e6);
         
        //SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_TX,CHAN_1,SdrDeviceList[i]->GetSampleRateVal());

        /* Set Rx Sample Rate (ADC/DAC rate) should be same as BB (signal gen )sample rate */
        //SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetSampleRateVal());
        SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_RX,CHAN_0,2e6);
        
        //SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetSampleRateVal());

        /* Set Tx bandwidth (Filters) */
        SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_TX,CHAN_0,40e6);
        
        //SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_TX,CHAN_1,SdrDeviceList[i]->GetBandwidth());

        /*  Set Rx Bandwidth (Filters) */
        SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_RX,CHAN_0,40e6);
        
        //SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetBandwidth());
        //SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetBandwidth());

        /* Set Tx Gain */
        SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_TX,CHAN_0,40/*SdrDeviceList[i]->GetTxGain()*/);
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_TX,CHAN_0,"PAD",0);
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetTxGain());
        

        /* Set Rx Gain */
        SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetRxGain());
        
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_0,15);
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_0,"LNA",31);
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_0,"TIA",0);
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_0,"PGA",0);
        //SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetRxGain());


        /* Set Carrier Freq */
        //FIXME: Remove this or make configurable
        if(i == 1)
        {
            //SdrDeviceList[i]->SetCarrierFreq(2.7e9);
        }
        SoapySDR::Kwargs args = {{"RF","2470000000"},{"BB","500000"}};
        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_TX,CHAN_0,0,args);
        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_0,0,args);

        SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());
        SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());

        printf("Tx ANT CHAN_0: %s\n\r",SdrDeviceList[i]->DeviceHandlePtr->getAntenna(SOAPY_SDR_TX,CHAN_0).c_str());
        printf("Rx ANT CHAN_0: %s\n\r",SdrDeviceList[i]->DeviceHandlePtr->getAntenna(SOAPY_SDR_RX,CHAN_0).c_str());

        printf("Tx Sample Rate CHAN_0: %f MHz\n\r",SdrDeviceList[i]->DeviceHandlePtr->getSampleRate(SOAPY_SDR_TX,CHAN_0)/1e6);
        printf("Rx Sample Rate CHAN_0: %f MHz\n\r",SdrDeviceList[i]->DeviceHandlePtr->getSampleRate(SOAPY_SDR_RX,CHAN_0)/1e6);

        printf("Tx Bandwidth CHAN_0: %f MHz\n\r",SdrDeviceList[i]->DeviceHandlePtr->getBandwidth(SOAPY_SDR_TX,CHAN_0)/1e6);
        printf("Rx Bandwidth CHAN_0: %f MHz\n\r",SdrDeviceList[i]->DeviceHandlePtr->getBandwidth(SOAPY_SDR_RX,CHAN_0)/1e6);

        printf("Tx GAIN CHAN_0 PAD: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getGain(SOAPY_SDR_TX,CHAN_0,"PAD"));

        printf("Rx GAIN CHAN_0 LNA: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getGain(SOAPY_SDR_RX,CHAN_0,"LNA"));
        printf("Rx GAIN CHAN_0 TIA: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getGain(SOAPY_SDR_RX,CHAN_0,"TIA"));
        printf("Rx GAIN CHAN_0 PGA: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getGain(SOAPY_SDR_RX,CHAN_0,"PGA"));

        printf("Tx CHAN_0 RF Freq: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getFrequency(SOAPY_SDR_TX,CHAN_0,"RF"));
        printf("Tx CHAN_0 BB Freq: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getFrequency(SOAPY_SDR_TX,CHAN_0,"BB"));

        printf("Rx CHAN_0 RF Freq: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getFrequency(SOAPY_SDR_RX,CHAN_0,"RF"));
        printf("Rx CHAN_0 BB Freq: %f\n\r",SdrDeviceList[i]->DeviceHandlePtr->getFrequency(SOAPY_SDR_RX,CHAN_0,"BB"));

        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());
        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());
        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_0,0,args);
        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());
        //SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetCarrierFreq());
    }
}

/*******************************************************************************
 * @brief This function setup Tx streams for all channels
 *
 * @param void
 * @return void
 ******************************************************************************/
static void SetupTxStream(void)
{
    //double fullScale = 0.0;
    //std::string format;

    /* set to {0, 1} for MIMO XTRX_CH_AB option to be selected in driver */
    std::vector<size_t> channels = {0};
    int i=0;
    //for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
       //format = SdrDeviceList[i]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_TX,CHAN_0,fullScale);
SoapySDR::Kwargs args = {{"syncRxTxStreamsAct","false"}};
        /* TODO: Currently only on Tx stream is opened per SDR device. The idea is to use same stream but switch channels as
           mentioned in setupStream API */
       SdrDeviceList[i]->TxStreamHandle = SdrDeviceList[i]->DeviceHandlePtr->setupStream(SOAPY_SDR_TX,SOAPY_SDR_CF32,channels/*,args*/);

    }

}

/*******************************************************************************
 * @brief This function setup Rx streams for all channels
 *
 * @param void
 * @return void
 ******************************************************************************/
static void SetupRxStream(void)
{
    //double fullScale = 0.0;
    SoapySDR::Kwargs args = {{"syncRxTxStreamsAct","false"}};
    //std::string format;

    /* set to {0, 1} for MIMO XTRX_CH_AB option to be selected in driver */
    std::vector<size_t> channels = {0};
    int i=1;
    //for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
       //format = SdrDeviceList[i]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_RX,CHAN_0,fullScale);

        /* TODO: Currently only one Rx stream is opened per SDR device. Look for a method to open stream for both channels*/
       SdrDeviceList[i]->RxStreamHandle = SdrDeviceList[i]->DeviceHandlePtr->setupStream(SOAPY_SDR_RX,SOAPY_SDR_CF32,channels/*,args*/);

    }

}

/*******************************************************************************
 * @brief This function is a simple sine wave gen for testing
 *
 * @param void
 * @return void
 ******************************************************************************/
static void RunTest(void)
{
    #if 0
    std::vector<size_t> channels = {0};
    std::string format;
    double fullScale = 0.0;
    unsigned int SdrDevNum = 0;
    size_t elemSize;

    format = SdrDeviceList[SdrDevNum]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_TX,CHAN_0,fullScale);

    elemSize = SoapySDR::formatToSize(format);

    runRateTestStreamLoop(SdrDeviceList[SdrDevNum]->DeviceHandlePtr,SdrDeviceList[SdrDevNum]->TxStreamHandle,SOAPY_SDR_TX,channels.size(), elemSize);
#endif
    pthread_t TxThread, RxThread, DataThread;
    sem_init(&mutex_lock, 0, 1);
    sem_init(&DataGatherSemaphore, 0, 0);
    sem_init(&DataDumpedSemaphore, 0, 1);
    pthread_create(&TxThread,NULL,TxThreadTest,NULL);
    pthread_create(&RxThread,NULL,RxThreadTest,NULL);
    pthread_create(&DataThread,NULL,DataGather,NULL);
    //RxThreadTest(NULL);
    //TxThreadTest(NULL);
    pthread_join(TxThread, NULL);
    pthread_join(RxThread, NULL);
    pthread_join(DataThread, NULL);    

    for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
        SdrDeviceList[i]->DeviceHandlePtr->deactivateStream(SdrDeviceList[i]->TxStreamHandle);
        SdrDeviceList[i]->DeviceHandlePtr->closeStream(SdrDeviceList[i]->TxStreamHandle);

        SdrDeviceList[i]->DeviceHandlePtr->deactivateStream(SdrDeviceList[i]->RxStreamHandle);
        SdrDeviceList[i]->DeviceHandlePtr->closeStream(SdrDeviceList[i]->RxStreamHandle);

        SoapySDR::Device::unmake(SdrDeviceList[i]->DeviceHandlePtr);
    }

}

/*******************************************************************************
 * @brief This function is a simple thread to control Tx
 *
 * @param void* pointer
 * @return void pointer
 ******************************************************************************/
static void *TxThreadTest(void *)
{
    std::vector<size_t> channels = {0};
    //std::string format;
    double fullScale = 0.0;
    unsigned int SdrDevNum = 0;
    size_t elemSize;

    //format = SdrDeviceList[SdrDevNum]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_TX,CHAN_0,fullScale);

    elemSize = SoapySDR::formatToSize(SOAPY_SDR_CF32);

    //TODO: Make runRateTestStreamLoop configurable and send &SdrDeviceList[SdrDevNum] as first argument

    runRateTestStreamLoop(SdrDeviceList[SdrDevNum]->DeviceHandlePtr,SdrDeviceList[SdrDevNum]->TxStreamHandle,SOAPY_SDR_TX,channels.size(), elemSize);
}

/*******************************************************************************
 * @brief This function is a simple thread to control Rx
 *
 * @param void* pointer
 * @return void pointer
 ******************************************************************************/
static void *RxThreadTest(void *)
{
    std::vector<size_t> channels = {0};
    //std::string format;
    double fullScale = 0.0;
    unsigned int SdrDevNum = 1;
    size_t elemSize;

    //format = SdrDeviceList[SdrDevNum]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_RX,CHAN_0,fullScale);

    elemSize = SoapySDR::formatToSize(SOAPY_SDR_CF32);

    RxLoop(SdrDeviceList[SdrDevNum]->DeviceHandlePtr,SdrDeviceList[SdrDevNum]->RxStreamHandle,channels.size(), elemSize);
}

/*******************************************************************************
 * @brief This function is a simple thread to control Data Gather
 *
 * @param void* pointer
 * @return void pointer
 ******************************************************************************/
static void *DataGather(void *)
{
    DataDump();
}