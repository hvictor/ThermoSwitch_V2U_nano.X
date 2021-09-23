/*******************************************************
 * RibesOS
 *
 * Main header
 *******************************************************/

#ifndef OS_H_
#define OS_H_

// Constants
#define __RIBES_OS__
#define OS_TASKS_MAX            8

// Includes
#include <stdint.h>
#include "../include/os_gfx.h"

// Types
typedef void (*OS_Task)(void);

typedef enum {
    READY,
    EXECUTING,
    STOPPED
} OS_TaskStatus;

typedef struct {
    OS_TaskStatus Status;
    OS_Task Task;
} OS_TCB;

typedef struct {
    uint8_t     Flag_Recurrent;
    uint8_t     Flag_StartImmediately;
    uint32_t    Interval_ms;
    uint32_t    Interval_us;
} OS_TaskSchedule;

// Functions
void OS_Init();
void OS_Start();
void OS_ScheduleTask(OS_Task t, OS_TaskSchedule schedule);
void OS_RegisterFinalizationProcedure(OS_Task proc);

#endif