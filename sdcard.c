#include <string.h>

#include "sdcard.h"
#include "bsp.h"

DIR dir;            // Directory information for the current directory
FATFS fso;          // File System Object for the file system we are reading from
FILINFO fileInfo;   // Information for the file we have opened (not really necessary to have this)

void sdcard_init()
{
    // Wait for the disk to initialise
    while(disk_initialize(0));
    
    // Mount the disk
    f_mount(&fso, "", 0);
    
    // Change dir to the root directory
    f_chdir("/");
    
    // Open the directory
    f_opendir(&dir, ".");
}

void sdcard_finalize()
{
    f_unmount("");
}

int sdcard_read_file(const char *filename, char *buffer, int buffer_size)
{
    FIL f;
    int tot_read_bytes = 0;
    char *ptr = buffer;
    unsigned int bytes_to_read = buffer_size;

    f_open(&f, filename, FA_READ | FA_OPEN_EXISTING);
    
    while (tot_read_bytes < buffer_size) {
        unsigned int rbytes;
        f_read(&f, buffer, bytes_to_read, &rbytes);
        
        if (rbytes <= 0)
            break;
        
        tot_read_bytes += rbytes;
    }
    
    return tot_read_bytes;
}

uint8_t sdcard_putlog(SDCARD_LogType type, char *logbuf, SDCARD_LogAccessMode mode)
{
    FIL f;
    int wbytes = 0;
    int tot_wbytes = 0;
    int bytes_to_write = strlen(logbuf);
    const char *logfile;
    
    switch (type) {
        case LOG_SYSTEM:
            logfile = BSP_LOGFILE_SYSTEM;
            break;
        case LOG_OPERATION:
            logfile = BSP_LOGFILE_OPERATION;
            break;
        default:
            return SDCARD_ERROR_LOG_INVALID_TYPE;
            break;
    }
    
    switch (mode)
    {
        case OVERWRITE:
            f_open(&f, logfile, FA_WRITE | FA_OPEN_ALWAYS);
            break;
            
        case APPEND:
            f_open(&f, logfile, FA_WRITE | FA_OPEN_ALWAYS | FA_OPEN_APPEND);
            break;
            
        default:
            return SDCARD_ERROR_LOG_INVALID_MODE;
            break;
    }
    
    while (tot_wbytes < bytes_to_write)
    {
        f_write(&f, logbuf, strlen(logbuf), &wbytes);
        tot_wbytes += wbytes;
    }
    
    f_sync(&f);
    f_close(&f);
    
    return SDCARD_OK;
}