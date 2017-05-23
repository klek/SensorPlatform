/*******************************************************************
   $File:    logging.h
   $Date:    Tue, 23 May 2017: 15:26
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(LOGGING_H)
#define LOGGING_H

#ifdef LOGGING
#include "uartSetup.h"
#define LOG_PRINT 1
#else
#define LOG_PRINT 0
#endif

#define LOG(f_, ...) \
    do { if (LOG_PRINT) printf((f_), ##__VA_ARGS__); } while(0)

#endif
