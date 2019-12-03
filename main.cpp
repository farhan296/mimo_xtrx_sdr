/*
 * main.cpp
 * Copyright (c) 2020 Farhan Naeem <farhann@patriot1tech.com>
 */

/*******************************************************************************
 * Include Files
 ******************************************************************************/
#include <iostream>
#include <string>

/*******************************************************************************
 * Local Data Structure
 ******************************************************************************/


/*******************************************************************************
 * Local Function Declaration
 ******************************************************************************/
static void ShowUsage (std::string name);
static void ParseParam(std::string param);


/*******************************************************************************
 * @brief This function displays the usage of the program
 *
 * @param argc argument count from command line
 * @param argv array of pointers to arguments passed
 * @return EXIT_SUCCESS or EXIT_FAILURE
 ******************************************************************************/
int main(int argc, char* argv[])
{
    std::string param;
    if (argc > 1u)
    {
      for(int i=0; i < argc; i++)
      {
          param = argv[i];
          if(param == "--help")
          {
              ShowUsage(param);
          }
          else
          {
              ParseParam(param);
          }
          
      }  
    }
    
    return EXIT_SUCCESS;
}

/*******************************************************************************
 * @brief This function displays the usage of the program
 *
 * @param name 
 * @return void
 ******************************************************************************/
static void ShowUsage (std::string name)
{
    std::cerr << "Usage: " << name << " <option(s)>"
              << "Options:\n"
              << "\t--help\t\tShow this help message\n"
              << "\t--rate\t\tSpecify Tx and Rx sample rate (Default: 1e6)\n"
              << "\t--ampl\t\tSpecify Tx digital amplitude (Default: 0.7)\n"
              << "\t--tx-gain\tSpecify Tx gain (dB) (Default: 52 (max) Range: 0 - 52)\n"
              << "\t--tx-ant\tSpecify Tx antenna (Default: TXH, options: TXH, TXW)\n"
              << "\t--tx-chan\tSpecify Tx channel (Default: 0, options: 0, 1)\n"
              << "\t--freq\t\tSpecify Tx and Rx freq (Hz) (Default: 2.47e9)\n"
              << "\t--wave-freq\tBaseband waveform freq (Hz) (Default: 100e3)\n"
              << "\t--gen-bb\tGenerate Baseband waveform (Default:Disable, options: Enable, Disable)\n"
              << std::endl;
}

/*******************************************************************************
 * @brief This function parses the command line parameters and overrides
 * the default value
 *
 * @param param 
 * @return void
 ******************************************************************************/
static void ParseParam (std::string param)
{
   // if(param == "--rate")
   // {

    //}
}