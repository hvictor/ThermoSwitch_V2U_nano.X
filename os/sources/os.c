/*******************************************************
 * RibesOs
 *
 * Main module
 *******************************************************/

#include "../include/os.h"
#include "../include/os_queue.h"

// Status Indicators
volatile uint8_t OS_FlagExecuting;

// Data structures
static uint8_t Counter_Tasks;
static uint8_t Counter_FinalizationProcedures;
static OS_TCB Tasks[OS_TASKS_MAX];
static OS_TCB FinalizationProcedures[OS_TASKS_MAX];

// Internal Prototypes
static void OS_KernelProcess();

void OS_Init()
{
    for (int i = 0; i < OS_TASKS_MAX; i++) {
        Tasks[i].Status = STOPPED;
    }
}

void OS_Start()
{
    for (int i = 0; i < OS_TASKS_MAX; i++) {
        Tasks[i].Status = READY;
    }
    
    OS_FlagExecuting = 1;
    OS_KernelProcess();
}

void OS_ScheduleTask(OS_Task t, OS_TaskSchedule schedule)
{
    Tasks[Counter_Tasks].Task = t;
    Tasks[Counter_Tasks].Status = READY;
    
    Counter_Tasks++;
}

void OS_RegisterFinalizationProcedure(OS_Task proc)
{
    FinalizationProcedures[Counter_FinalizationProcedures].Status = STOPPED;
    FinalizationProcedures[Counter_FinalizationProcedures].Task = proc;
    
    Counter_FinalizationProcedures++;
}

static void OS_KernelProcess()
{
    while (OS_FlagExecuting)
    {
        for (int i = 0; i < Counter_Tasks; i++)
        {
            if (Tasks[i].Status == READY) {
                Tasks[i].Status = EXECUTING;
                Tasks[i].Task();
                Tasks[i].Status = READY;
            }
        }
    }
}