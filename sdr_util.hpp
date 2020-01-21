/*
 * sdr_util.hpp
 * Copyright (c) 2020 Farhan Naeem <farhann@patriot1tech.com>
 */
#ifndef SDR_UTIL_H
#define SDR_UTIL_H

/*******************************************************************************
 * Include Files
 ******************************************************************************/
#include <SoapySDR/Types.hpp>
#include <semaphore.h>
/*******************************************************************************
 * MACRO
 ******************************************************************************/


/*******************************************************************************
 * Data Structure
 ******************************************************************************/


/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
const SoapySDR::KwargsList EnumerateSdr(void);
void OpenAndConfigureSdrDevice(SoapySDR::KwargsList &device_list);
extern sem_t DataGatherSemaphore;
extern sem_t mutex_lock;
extern sem_t DataDumpedSemaphore;
#endif /* SDR_UTIL_H */