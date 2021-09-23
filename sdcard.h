#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "ff.h"
#include "diskio.h"
#include "ffconf.h"

#define SDCARD_OK                       0
#define SDCARD_ERROR_LOG_INVALID_MODE   -1
#define SDCARD_ERROR_LOG_OPEN           -2
#define SDCARD_ERROR_LOG_WRITE          -3
#define SDCARD_ERROR_LOG_INVALID_TYPE   -99

typedef enum
{
    APPEND = 0,
    OVERWRITE
} SDCARD_LogAccessMode;

typedef enum {
    LOG_SYSTEM = 0,
    LOG_OPERATION = 1
} SDCARD_LogType;


void sdcard_init();
void sdcard_finalize();
uint8_t sdcard_putlog(SDCARD_LogType type, char *logbuf, SDCARD_LogAccessMode mode);
int sdcard_read_file(const char *filename, char *buffer, int buffer_size);

#endif