#ifndef BSP_H_
#define BSP_H_

// Use Flags
// Uncomment the following line if the board has ST7789 TFT support
//#define BSP_USE_TFT

#include <xc.h>
#include <stdint.h>

// 50 MHz external crystal
#define _XTAL_FREQ                              (50000000L)

// Pin Definition
#define BSP_PIN_VBAT_MON                        0           // 0 = AN0 = RA0 = Pin 2

// Constants
#define BSP_LOGFILE_SYSTEM                      "/system.log"
#define BSP_LOGFILE_OPERATION                   "/oper.log"
#define BSP_CONFIG_FILE                         "/config.txt"
#define BSP_VBAT_FULL                           9.0f 

// BSP Types
typedef enum {
    TEMP1 = 4,
    TEMP2 = 5
} BSP_TemperatureSensor;

typedef enum {
    OFF = 0,
    ON = 1
} BSP_RelayStatus;

typedef enum
{
    Greater,
    Smaller,
    Equal
} BSP_TemperatureDeltaCompareOperator;

typedef enum
{
    Poweron,
    Poweroff,
    CheckInterval
} BSP_ConfigRuleType;

typedef enum
{
    Stable,
    Rising,
    Falling
} BSP_TemperatureCurveBehavior;

typedef struct
{
    BSP_ConfigRuleType RuleType;
    BSP_TemperatureSensor Sensor;
    BSP_TemperatureDeltaCompareOperator CompareOperator;
    BSP_TemperatureCurveBehavior TempCurveBehavior;
    float FloatValue;
    int IntegerValue;
        
} BSP_ConfigRule;

// System Configuration
uint8_t bsp_configure(char *config_buffer, char *error_message);

// Timing functions
void bsp_delay_us(unsigned int us);
void bsp_delay_ms(int ms);

// Board-specific TFT functions
void bsp_splash_screen();

// Analog temperature sensors functions
void bsp_temp_sensors_initialize();
float bsp_temp_read(BSP_TemperatureSensor sensor, int *adcval);

// Relay switch functions
void bsp_relay_initialize();
void bsp_relay_on();
void bsp_relay_off();
BSP_RelayStatus bsp_relay_status();

// LEDs and signaling
void bsp_leds_initialize();
void bsp_signal_led1(int num_blinks);
void bsp_long_signal_led1(int num_blinks);
void bsp_fix_led1();
void bsp_unfix_led1();

// Battery voltage monitoring
void bsp_vbat_mon_initialize();
float bsp_vbat_mon_read();

#endif