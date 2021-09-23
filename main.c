#include <stdlib.h>
#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "spi.h"
#include "sdcard.h"
#include "tft_st7789.h"
#include "bsp.h"
#include "os/include/os.h"

//#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_1, FPLLODIV = DIV_1
//#pragma config POSCMOD = HS, FNOSC = PRI, FPBDIV = DIV_1

// WORKING:
// Use External Crystal @ 50 MHz as PRIMARY OSCILLATOR
/*
#pragma config POSCMOD = HS, FNOSC = PRI, FPBDIV = DIV_1
#pragma config FWDTEN = OFF         // Watchdog Timer Disabled
#pragma config ICESEL = ICS_PGx1    // ICE/ICD Comm Channel Select
#pragma config JTAGEN = OFF         // Disable JTAG
#pragma config FSOSCEN = OFF        // Disable Secondary Oscillator
*/

// Clock Configurator generated code:
// 12 MHz External Crystal with 18 pF
// PLL brings it to 40 MHz
/*
#pragma config DEBUG =      OFF
#pragma config JTAGEN =     OFF
#pragma config ICESEL =     ICS_PGx1
#pragma config PWP =        OFF
#pragma config BWP =        OFF
#pragma config CP =         OFF

#pragma config FNOSC =      PRIPLL
#pragma config FPBDIV =     DIV_1
#pragma config FSOSCEN =    OFF
#pragma config IESO =       OFF
#pragma config POSCMOD =    HS
#pragma config OSCIOFNC =   OFF
#pragma config FCKSM =      CSDCMD
#pragma config WDTPS =      PS1048576
#pragma config FWDTEN =     OFF
#pragma config WINDIS =     OFF
#pragma config FWDTWINSZ =  WINSZ_50

#pragma config FPLLIDIV =   DIV_3
#pragma config FPLLMUL =    MUL_20
#pragma config FPLLODIV =   DIV_2
#pragma config UPLLEN =     OFF
#pragma config UPLLIDIV =   DIV_2

#pragma config FVBUSONIO =  ON
#pragma config USERID =     0xffff
#pragma config PMDL1WAY =   ON
#pragma config IOL1WAY =    ON
#pragma config FUSBIDIO =   ON
*/

// Clock Configuration generated code:
// 25 MHz External Crystal with 18 pF
// PLL brings it to 50 MHz
#pragma config DEBUG =      OFF
#pragma config JTAGEN =     OFF
#pragma config ICESEL =     ICS_PGx1
#pragma config PWP =        OFF
#pragma config BWP =        OFF
#pragma config CP =         OFF

#pragma config FNOSC =      PRIPLL
#pragma config FPBDIV =     DIV_1
#pragma config FSOSCEN =    OFF
#pragma config IESO =       OFF
#pragma config POSCMOD =    HS
#pragma config OSCIOFNC =   OFF
#pragma config FCKSM =      CSDCMD
#pragma config WDTPS =      PS1048576
#pragma config FWDTEN =     OFF
#pragma config WINDIS =     OFF
#pragma config FWDTWINSZ =  WINSZ_50

#pragma config FPLLIDIV =   DIV_6
#pragma config FPLLMUL =    MUL_24
#pragma config FPLLODIV =   DIV_2
#pragma config UPLLEN =     OFF
#pragma config UPLLIDIV =   DIV_2

#pragma config FVBUSONIO =  ON
#pragma config USERID =     0xffff
#pragma config PMDL1WAY =   ON
#pragma config IOL1WAY =    ON
#pragma config FUSBIDIO =   ON

// Config Rules 
extern int Counter_ConfigRules;
extern BSP_ConfigRule ConfigRules[32];

// Task Data
#define TEMP_BUFSIZ 6
int counter_temp_buffer = 0;
float buffer_temp1[TEMP_BUFSIZ] = { 0.0f };
float buffer_temp2[TEMP_BUFSIZ] = { 0.0f };

float coeff_linear_regressor(float *data, int n)
{
    // y = coeff*x + q
    
    float sum_x = 0.0f;
    float sum_y = 0.0f;
    float sum_xy = 0.0f;
    float sum_x2 = 0.0f;
    float sum_y2 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        float x = (float)(i + 1);
        float y = data[i];
        float xy = x * y;
        float x2 = x * x;
        float y2 = y * y;
        sum_x += x;
        sum_y += y;
        sum_xy += xy;
        sum_x2 += x2;
        sum_y2 += y2;
    }
    
    return (n*sum_xy - sum_x*sum_y) / (n*sum_x2 - sum_x*sum_x);
}

void calc_behavior(int *behav_t1, int *behav_t2)
{
    float tol = 0.01;
    float coeff1 = coeff_linear_regressor(buffer_temp1, TEMP_BUFSIZ);
    float coeff2 = coeff_linear_regressor(buffer_temp2, TEMP_BUFSIZ);
    
    if (coeff1 > tol) *behav_t1 = 1;
    else if (coeff1 < -tol) *behav_t1 = -1;
    else *behav_t1 = 0;
    
    if (coeff2 > tol) *behav_t2 = 1;
    else if (coeff2 < -tol) *behav_t2 = -1;
    else *behav_t2 = 0;
}

// Primary Task
void Task(void)
{
    int interval = 1000;
    
    // Read check interval from configuration if any
    for (int i = 0; i < Counter_ConfigRules; i++) {
        if (ConfigRules[i].RuleType == CheckInterval) {
            BSP_ConfigRule r = ConfigRules[i];
            interval = r.IntegerValue;
        }
    }
    
    while (1)
    {
        char logbuf[80];  
        int v1, v2;
        float temp1 = bsp_temp_read(TEMP1, &v1);
        float temp2 = bsp_temp_read(TEMP2, &v2);
        float vbat = bsp_vbat_mon_read();
        
        // Shift temperature buffers
        if (counter_temp_buffer == TEMP_BUFSIZ) {
            for (int i = 0; i < TEMP_BUFSIZ - 1; i++) {
                buffer_temp1[i] = buffer_temp1[i + 1];
                buffer_temp2[i] = buffer_temp2[i + 1];
            }
        }
        
        buffer_temp1[counter_temp_buffer - 1] = temp1;
        buffer_temp2[counter_temp_buffer - 1] = temp2;
        
        if (counter_temp_buffer < TEMP_BUFSIZ) {
            
            counter_temp_buffer++;
            
            bsp_signal_led1(3);
            bsp_delay_ms(interval);
            continue;
        }
        
        // Recalculate behaviors with linear regressor
        int t1_behav, t2_behav;
        calc_behavior(&t1_behav, &t2_behav);
        
        sprintf(logbuf, "TEMP1 = %.2f C        TEMP2 = %.2f C\n", temp1, temp2);
        sdcard_putlog(LOG_OPERATION, logbuf, APPEND);
        
        // Delta
        float delta = fabsf(temp1 - temp2);
        
        // Apply Rules
        uint8_t cond_poweron = 0;
        uint8_t cond_poweroff = 0;        
        uint8_t behav_match;
        
        for (int i = 0; i < Counter_ConfigRules; i++) {
            BSP_ConfigRule rule = ConfigRules[i];

            switch (rule.RuleType) {
                case Poweron:
                    behav_match =   (rule.Sensor == TEMP1 && rule.TempCurveBehavior == Rising && t1_behav == 1) ||
                                            (rule.Sensor == TEMP1 && rule.TempCurveBehavior == Stable && t1_behav == 0) ||
                                            (rule.Sensor == TEMP1 && rule.TempCurveBehavior == Falling && t1_behav == -1) ||
                            
                                            (rule.Sensor == TEMP2 && rule.TempCurveBehavior == Rising && t2_behav == 1) ||
                                            (rule.Sensor == TEMP2 && rule.TempCurveBehavior == Stable && t2_behav == 0) ||
                                            (rule.Sensor == TEMP2 && rule.TempCurveBehavior == Falling && t2_behav == -1);
                            
                    if (behav_match) {
                        if      ((rule.CompareOperator == Smaller && delta < rule.FloatValue) ||
                                (rule.CompareOperator == Equal && fabsf(delta - rule.FloatValue) <= 0.1) ||
                                (rule.CompareOperator == Greater && delta > rule.FloatValue))
                        {
                            cond_poweron = 1;
                            cond_poweroff = 0;
                        }
                    }
                    break;
                    
                case Poweroff:
                    behav_match =   (rule.Sensor == TEMP1 && rule.TempCurveBehavior == Rising && t1_behav == 1) ||
                                            (rule.Sensor == TEMP1 && rule.TempCurveBehavior == Stable && t1_behav == 0) ||
                                            (rule.Sensor == TEMP1 && rule.TempCurveBehavior == Falling && t1_behav == -1) ||
                            
                                            (rule.Sensor == TEMP2 && rule.TempCurveBehavior == Rising && t2_behav == 1) ||
                                            (rule.Sensor == TEMP2 && rule.TempCurveBehavior == Stable && t2_behav == 0) ||
                                            (rule.Sensor == TEMP2 && rule.TempCurveBehavior == Falling && t2_behav == -1);
                            
                    if (behav_match) {
                        if      ((rule.CompareOperator == Smaller && delta < rule.FloatValue) ||
                                (rule.CompareOperator == Equal && fabsf(delta - rule.FloatValue) <= 0.1) ||
                                (rule.CompareOperator == Greater && delta > rule.FloatValue))
                        {
                            cond_poweron = 0;
                            cond_poweroff = 1;
                        }
                    }                    
                    break;
                default:
                    break;
            }
        }
        
        if (cond_poweron && bsp_relay_status() == OFF) {
            sdcard_putlog(LOG_OPERATION, "230V relay switched on\r\n", APPEND);
            bsp_relay_on();
        }
        else if (cond_poweroff && bsp_relay_status() == ON) {
            sdcard_putlog(LOG_OPERATION, "230V relay switched off\r\n", APPEND);
            bsp_relay_off();
        }
        
        if (vbat < 5.0) {
            bsp_signal_led1(6);
        } 
        
        bsp_signal_led1(1);
        bsp_delay_ms(interval);
    }
}

// Finalization Procedure
void Finalize(void)
{
    sdcard_finalize();   
}

int main(void)
{
    char config_buf[1024];
    char error_message[200];
    char log_buffer[200];
    memset(config_buf, 0, 1024);
    memset(error_message, 0, 400);
    
    bsp_leds_initialize();
    LATBbits.LATB9 = 1;

    // Initialization
    spi_configure();
    sdcard_init();
    
    // Read Configuration
    sdcard_read_file(BSP_CONFIG_FILE, config_buf, 1024);
    
    char *ptr = config_buf;
    while (*ptr != 0) {
        if (*ptr == '\r') strcpy(ptr, ptr+1);
        if (*ptr == '\t')  {
            *ptr = ' ';
        }
        if (*ptr == ' ') {
            while (*(ptr + 1) == ' ' || *(ptr + 1) == '\t')
                strcpy(ptr+1, ptr+2);
        }
        ptr++;
    }
    
    
    if (!bsp_configure(config_buf, error_message)) {
        sprintf(log_buffer, "Error: %s\r\n", error_message);
        sdcard_putlog(LOG_SYSTEM, log_buffer, APPEND);
    }
    
    sdcard_putlog(LOG_SYSTEM, "boot: REXEC picokernel (C) 2021 Ribes Microsystems\r\n", APPEND);
    
    // BSP Setup
    sdcard_putlog(LOG_SYSTEM, "boot: REXEC picokernel (C) 2021 Ribes Microsystems\r\n", APPEND);
    sdcard_putlog(LOG_SYSTEM, "boot: MCU peripherals configuration completed.\r\n", APPEND);
    sdcard_putlog(LOG_SYSTEM, "boot: Starting BSP for ThermoSwitch V2-U nano.\r\n", APPEND);
    bsp_temp_sensors_initialize();
    bsp_relay_initialize();
    bsp_vbat_mon_initialize();

    sdcard_putlog(LOG_SYSTEM, "BSP is up and running.\r\n", APPEND);
    sdcard_putlog(LOG_SYSTEM, "The system is operational.\r\n", APPEND);
    
    //bsp_splash_screen();
    //bsp_delay_ms(4000);
    
    // OS Initialization
    OS_Init();
    OS_RegisterFinalizationProcedure(Finalize);
    
    OS_Task T1 = Task;
    OS_TaskSchedule T1sched;
    T1sched.Flag_Recurrent = 0;
    T1sched.Flag_StartImmediately = 1;
    T1sched.Interval_ms = 0;
    T1sched.Interval_us = 0;
    
    sdcard_putlog(LOG_SYSTEM, "Scheduling kernel task to the operating system.\r\n", APPEND);
    
    OS_ScheduleTask(T1, T1sched);
    OS_Start();
}