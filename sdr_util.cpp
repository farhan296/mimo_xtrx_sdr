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

//TODO: Remove this in final code
#include "/home/AD.PATRIOT1TECH.COM/farhan.naeem/sdr_xtrx/mimo_dev/mimo_xtrx_sdr/test_signal/test.hpp"

/*******************************************************************************
 * Local MACRO
 ******************************************************************************/
#define MAX_SDR_DEVICES 8U
#define CHAN_0          0U
#define CHAN_1          1U
#define MAX_TX_CH       2U
#define MAX_RX_CH       2U
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
    SoapySDR::Stream *StreamHandle;
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

    /* Open SDR devices */
    for (int i=0; i < device_list.size(); i++)
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

    /* Specify Tx antenna (Default: TXH, options: TXH, TXW) */
    TxAnt = "TXH";

    /* Specify Tx channel (Default: 0, options: 0, 1) */ 
    TxChannel = 0;

    /* Specify Tx and Rx freq (Hz) (Default: 2.47e9) */
    CarrierFreq = 2.47e9; 

    /* Baseband waveform freq (Hz) (Default: 100e3) */
    BaseBandFreq = 100e3;

    /* Baseband waveform freq (Hz) (Default: 40e6) */
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
    for(int i=0; i < SdrDeviceList.size(); i++)
    {
        delete SdrDeviceList[i];
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
    for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
        /* Set sample rate */
        //TODO: verify if only one setting is enough to configure for all channels in TX and RX direction
        SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetSampleRateVal());
        SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_TX,CHAN_1,SdrDeviceList[i]->GetSampleRateVal());
        SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetSampleRateVal());
        SdrDeviceList[i]->DeviceHandlePtr->setSampleRate(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetSampleRateVal());

        /* Set bandwidth */
        SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetBandwidth());
        SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_TX,CHAN_1,SdrDeviceList[i]->GetBandwidth());
        SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetBandwidth());
        SdrDeviceList[i]->DeviceHandlePtr->setBandwidth(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetBandwidth());

        /* Set Tx Gain */
        SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetTxGain());
        SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_TX,CHAN_1,SdrDeviceList[i]->GetTxGain());
        SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetTxGain());
        SdrDeviceList[i]->DeviceHandlePtr->setGain(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetTxGain());

        /* Set Carrier Freq */
        //FIXME: Remove this or make configurable
        if(i == 1)
        {
            SdrDeviceList[i]->SetCarrierFreq(2.7e9);
        }
        SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_TX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());
        SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_TX,CHAN_1,SdrDeviceList[i]->GetCarrierFreq());
        SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_0,SdrDeviceList[i]->GetCarrierFreq());
        SdrDeviceList[i]->DeviceHandlePtr->setFrequency(SOAPY_SDR_RX,CHAN_1,SdrDeviceList[i]->GetCarrierFreq());
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
    double fullScale = 0.0;
    std::string format;
    size_t elemSize;
    SoapySDR::Stream *stream[MAX_SDR_DEVICES];

    /* set to {0, 1} for MIMO XTRX_CH_AB option to be selected in driver */
    std::vector<size_t> channels = {0};
    
    for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
       format = SdrDeviceList[i]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_TX,CHAN_0,fullScale);
       elemSize = SoapySDR::formatToSize(format);

        /* TODO: Currently only on Tx stream is opened per SDR device. The idea is to use same stream but switch channels as
           mentioned in setupStream API */
       SdrDeviceList[i]->StreamHandle = SdrDeviceList[i]->DeviceHandlePtr->setupStream(SOAPY_SDR_TX,format,channels);

       /* Activate stream */
       //SdrDeviceList[i]->DeviceHandlePtr->activateStream(SdrDeviceList[i]->StreamHandle);
    }

}

/*******************************************************************************
 * @brief This function is a simple sine wave gen for testing
 *
 * @param void
 * @return void
 ******************************************************************************/
void RunTest(void)
{
    std::vector<size_t> channels = {0};
    std::string format;
    double fullScale = 0.0;
    unsigned int SdrDevNum = 1;
    size_t elemSize;

    format = SdrDeviceList[SdrDevNum]->DeviceHandlePtr->getNativeStreamFormat(SOAPY_SDR_TX,CHAN_0,fullScale);

    elemSize = SoapySDR::formatToSize(format);

    runRateTestStreamLoop(SdrDeviceList[SdrDevNum]->DeviceHandlePtr,SdrDeviceList[SdrDevNum]->StreamHandle,SOAPY_SDR_TX,channels.size(), elemSize);

    for(int i=0; i < SdrDevice::InstanceCounter; i++)
    {
        SdrDeviceList[i]->DeviceHandlePtr->deactivateStream(SdrDeviceList[i]->StreamHandle);
        SdrDeviceList[i]->DeviceHandlePtr->closeStream(SdrDeviceList[i]->StreamHandle);
        SoapySDR::Device::unmake(SdrDeviceList[i]->DeviceHandlePtr);
    }

}