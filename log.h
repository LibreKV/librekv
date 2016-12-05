/*
log.h
 *
 *  Created on: Oct 25, 2016
 *      Author: Hao Liu
*/
#ifndef LOG_H_
#define LOG_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stdarg.h"
#include "unistd.h"
 
#define MAXLEN (2*1024*1024)
#define MAXFILEPATH (512)
#define MAXFILENAME (50)

char log_buffer[100];

typedef enum{
    ERROR_1=-1,
    ERROR_2=-2,
    ERROR_3=-3
}ERROR0;
 
 
typedef enum{
    NONE=0,
    INF=1,
    DEBUG=2,
    ERROR=4,
    ALL=255
}LOGLEVEL;
 
typedef struct log{
    char logtime[20];
    char filepath[MAXFILEPATH];
    FILE *logfile;
}LOG;
 
typedef struct logseting{
    char filepath[MAXFILEPATH];
    unsigned int maxfilelen;
    unsigned char loglevel;
}LOGSET;
 
extern int librekv_logWrite_va(unsigned char loglevel,char *fromat,...);

extern int librekv_logWrite(unsigned char loglevel,char *buffer, unsigned buf_size);

#endif /* LOG_H_ */
