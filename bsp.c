#include "bsp.h"
#include "tft_st7789.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static uint8_t flag_led1_fix = 0;
static BSP_RelayStatus relay_status = OFF;

int Counter_ConfigRules = 0;
BSP_ConfigRule ConfigRules[32];

void bsp_splash_screen()
{
    tft_fill_screen(TFT_COLOR_WHITE);
    tft_render_image_sdcard("/l45x210.bin", 15, 2, 210, 45);
    tft_render_image_sdcard("/ts.bin", 0, 55, 240, 20);
    
    tft_set_cursor(1, 80);
    tft_set_text_size(1);
    tft_set_text_color(TFT_COLOR_RED);
    tft_printf("* ");
    tft_set_text_color(TFT_COLOR_BLACK);
    tft_printf("BSP:           Ribes THERMOSWITCH TS1\n");
    
    tft_set_text_color(TFT_COLOR_RED);
    tft_printf("* ");
    tft_set_text_color(TFT_COLOR_BLACK);
    tft_printf("System Kernel: REXEC-PIC32MX-1REL\n");
    
    tft_set_text_color(TFT_COLOR_RED);
    tft_printf("* ");
    tft_set_text_color(TFT_COLOR_BLACK);
    tft_printf("Processor:     PIC32MX250F128B\n");
}

void bsp_delay_us(unsigned int us)
{
    // Convert microseconds us into how many clock ticks it will take
	us *= _XTAL_FREQ / 1000000 / 2; // Core Timer updates every 2 ticks
       
    _CP0_SET_COUNT(0); // Set Core Timer count to 0
    
    while (us > _CP0_GET_COUNT()); // Wait until Core Timer count reaches the number we calculated earlier
}

 void bsp_delay_ms(int ms)
{
    bsp_delay_us(ms * 1000);
}

void bsp_configure_adc()
{
    AD1CON1CLR = 0x8000;    // Disable ADC before configuration
    AD1CON1 = 0x00E0;       // Internal counter ends sampling and starts conversion (auto-convert), manual sample
    AD1CON2 = 0;            // AD1CON2<15:13> set voltage reference to pins AVSS/AVDD
    AD1CON3 = 0x0f01;       // TAD = 4*TPB, acquisition time = 15*TAD 
}

void bsp_temp_sensors_initialize()
{
    ANSELBbits.ANSB2 = 1;   // RB2 = AN4 set to analog
    TRISBbits.TRISB2 = 1;   // RB2 = AN4 set to input
    
    ANSELBbits.ANSB3 = 1;   // RB3 = AN5 set to analog
    TRISBbits.TRISB3 = 1;   // RB3 = AN5 set to input

    bsp_configure_adc();
    AD1CON1SET = 0x8000;    // Enable ADC
}

int bsp_analog_read(char pin)
{
    AD1CHS = pin << 16;         // AD1CHS<16:19> controls which analog pin goes to the ADC
 
    AD1CON1bits.SAMP = 1;       // Start sampling
    while(AD1CON1bits.SAMP);    // Wait until sampling is performed
    while(!AD1CON1bits.DONE);   // wait until conversion is performed
 
    return ADC1BUF0;            // Result stored in ADC1BUF0
}

// Temperature in [°C]
float bsp_temp_read(BSP_TemperatureSensor sensor, int *adcval)
{
    float voltage;
    int adc_value = bsp_analog_read(sensor);
    *adcval = adc_value;
    voltage = adc_value * 3.3;                  // 3.3V 
    voltage /= 1024.0;
    float tempC = (voltage - 0.5) * 100;        // Converting from 10 mV per degree wit 500 mV offset
    return tempC;
}


void bsp_leds_initialize()
{
    TRISBbits.TRISB9 = 0;   // LED1 = RB9 = pin 18
    LATBbits.LATB9 = 0;
}

void bsp_signal_led1(int num_blinks)
{
    if (flag_led1_fix)
        return;
    
    for (int i = 0; i < num_blinks; i++)
    {
        LATBbits.LATB9 = 1;
        bsp_delay_ms(30);
        LATBbits.LATB9 = 0;
        bsp_delay_ms(30);
    }
}

void bsp_fix_led1()
{
    flag_led1_fix = 1;
    LATBbits.LATB9 = 1;
}

void bsp_unfix_led1()
{
    flag_led1_fix = 0;
    LATBbits.LATB9 = 0;
}

void bsp_relay_initialize()
{
    TRISBbits.TRISB7 = 0;   // Relay Control = RB7 = pin 16
    LATBbits.LATB7 = 0;
}

void bsp_relay_on()
{
    bsp_fix_led1();
    relay_status = ON;
    LATBbits.LATB7 = 1;
}

void bsp_relay_off()
{
    bsp_unfix_led1();
    relay_status = OFF;
    LATBbits.LATB7 = 0;
}

BSP_RelayStatus bsp_relay_status()
{
    return relay_status;
}

void bsp_vbat_mon_initialize()
{
    ANSELAbits.ANSA0 = 1;   // RA0 = AN0 set to analog
    TRISAbits.TRISA0 = 1;   // RA0 = AN0 set to input
}

float bsp_vbat_mon_read()
{
    float voltage;
    int adc_value = bsp_analog_read(0);
    voltage = adc_value * 3.3;  // 3.3V 
    voltage /= 1024.0;
    voltage *= 3.0;
    return voltage;
}

/* Example program:
 
check_interval		800 ms
poweron				delta > 2.0 when T1 rising
poweroff			delta < 0.5 when stable
 
 */

static uint8_t flag_poweron = 0;
static uint8_t flag_poweroff = 0;
static uint8_t flag_check_interval = 0;
static uint8_t flag_when = 0;
static uint8_t flag_delta = 0;
static uint8_t flag_cmpop = 0;
static uint8_t flag_value = 0;
static uint8_t flag_sensor = 0;
static uint8_t flag_behav = 0;

static uint8_t done_delta = 0;
static uint8_t done_when = 0;
static uint8_t done_interval = 0;

uint8_t bsp_configure(char *config_buffer, char *error_message)
{
    char *end_line;
    char *line = strtok_r(config_buffer, "\n", &end_line);
    
    while (line != NULL) {
        char *end_token;
        char copyline[1024];
        memset(copyline, 0, 1024);
        strcpy(copyline, line);
        
        if (copyline[0] == '\n' || copyline[0] == '\t' || copyline[0] == ' ' || copyline[0] == '#')
        {
            line = strtok_r(NULL, "\n", &end_line);
            continue;
        }
        
        char *tok = strtok_r(copyline, " ", &end_token);
    
        while (tok != NULL) {
            char copybuf[200];
            memset(copybuf, 0, 200);
            strcpy(copybuf, copyline);
            
            // Parse instruction
            if (!(flag_poweron || flag_poweroff || flag_check_interval)) {
                
                // Invalid instruction
                if (strcmp(tok, "poweron") && strcmp(tok, "poweroff") && strcmp(tok, "check_interval")) {
                    sprintf(error_message, "Invalid instruction '%s'. Use <poweron|poweroff|check_interval>.", tok);
                    return 0;
                }

                // Determine which instruction to activate
                if (!strcmp(tok, "poweron")) {
                    flag_poweron = 1;
                    flag_poweroff = 0;
                    flag_check_interval = 0;

                    ConfigRules[Counter_ConfigRules].RuleType = Poweron;
                } else if (!strcmp(tok, "poweroff")) {
                    flag_poweron = 0;
                    flag_poweroff = 1;
                    flag_check_interval = 0;

                    ConfigRules[Counter_ConfigRules].RuleType = Poweroff;
                } else if (!strcmp(tok, "check_interval")) {
                    flag_poweron = 0;
                    flag_poweroff = 0;
                    flag_check_interval = 1;

                    ConfigRules[Counter_ConfigRules].RuleType = CheckInterval;
                }
            }

            // Instruction parameters
            else {
                // Poweron or poweroff
                if (flag_poweron || flag_poweroff) {

                    // Delta or when already parsed
                    if (flag_delta || flag_when) {
                        // Delta
                        if (flag_delta) {
                            // Compare operator already parsed
                            if (flag_cmpop) {

                                // Expect a numeric value
                                char *ptr = tok;
                                int points = 0;
                                while (*ptr != NULL) {
                                    if (*ptr == '.') {
                                        if (points > 0) {
                                            sprintf(error_message, "Invalid numeric value. Specify a numeric value using the point as decimal separator. Example: 2.4", tok);
                                            return 0;
                                        }
                                        points++;
                                    }
                                    else if (!isdigit(*ptr)) {
                                        sprintf(error_message, "Invalid numeric value. Specify a numeric value using the point as decimal separator. Example: 2.4", tok);
                                        return 0;
                                    }                                
                                    ptr++;
                                }

                                // Number is valid
                                ConfigRules[Counter_ConfigRules].FloatValue = (float)atof(tok);
                                flag_value = 1;

                                // Exit from delta parsing
                                flag_delta = 0;
                                done_delta = 1;

                                if (done_when) {
                                    flag_poweron = 0;
                                    flag_poweroff = 0;
                                    flag_check_interval = 0;
                                    flag_when = 0;
                                    flag_delta = 0;
                                    flag_cmpop = 0;
                                    flag_value = 0;
                                    flag_sensor = 0;
                                    flag_behav = 0;

                                    Counter_ConfigRules++;
                                }
                            }
                            // Compare operator not yet parsed
                            else {                        // No compare operator parsed
                                if (strcmp(tok, ">") && strcmp(tok, "<") && strcmp(tok, "=")) {
                                    sprintf(error_message, "Invalid syntax. Use a compare operator like: 'delta < 1.0', 'delta = 1.0', or 'delta < 1.0'", tok);
                                    return 0;
                                }

                                // Compare operator parsed
                                flag_cmpop = 1;

                                if (!strcmp(tok, "<")) {
                                    ConfigRules[Counter_ConfigRules].CompareOperator = Smaller;
                                }
                                else if (!strcmp(tok, "=")) {
                                    ConfigRules[Counter_ConfigRules].CompareOperator = Equal;

                                }
                                else if (!strcmp(tok, ">")) {
                                    ConfigRules[Counter_ConfigRules].CompareOperator = Greater;
                                }
                            }
                        }
                        // When
                        else if (flag_when) {
                            if (flag_sensor) {
                                // Sensor name T1 or T2 already parsed. Expected behavior: rising or falling
                                if (!flag_behav) {
                                    // Expected behavior: rising or falling
                                    if (strcmp(tok, "stable") && strcmp(tok, "rising") && strcmp(tok, "falling")) {
                                        sprintf(error_message, "Invalid temperature curve behavior. Specify 'stable', 'rising' or 'falling'", tok);
                                        return 0;
                                    }

                                    if (!strcmp(tok, "stable")) {
                                        ConfigRules[Counter_ConfigRules].TempCurveBehavior = Stable;
                                    }
                                    else if (!strcmp(tok, "rising")) {
                                        ConfigRules[Counter_ConfigRules].TempCurveBehavior = Rising;
                                    }
                                    else if (!strcmp(tok, "falling")) {
                                        ConfigRules[Counter_ConfigRules].TempCurveBehavior = Falling;
                                    }

                                    flag_when = 0;
                                    done_when = 1;

                                    if (done_delta) {
                                        flag_poweron = 0;
                                        flag_poweroff = 0;
                                        flag_check_interval = 0;
                                        flag_when = 0;
                                        flag_delta = 0;
                                        flag_cmpop = 0;
                                        flag_value = 0;
                                        flag_sensor = 0;
                                        flag_behav = 0;

                                        Counter_ConfigRules++;
                                    }
                                }
                            }
                            else {
                                // Expected sensor name: T1 or T2
                                if (strcmp(tok, "T1") && strcmp(tok, "T2")) {
                                    sprintf(error_message, "Invalid sensor identifier. Specify 'T1' or 'T2'", tok);
                                    return 0;
                                }

                                if (!strcmp(tok, "T1")) {
                                    ConfigRules[Counter_ConfigRules].Sensor = TEMP1;
                                }
                                else if (!strcmp(tok, "T2")) {
                                    ConfigRules[Counter_ConfigRules].Sensor = TEMP2;
                                }

                                flag_sensor = 1;
                            }
                        }
                    }
                    // No delta or when parsed yet
                    else {                
                        // Error: no when and no delta directives
                        if (strcmp(tok, "when") && strcmp(tok, "delta")) {
                            sprintf(error_message, "Invalid syntax. Use <poweron|poweroff> delta <compare-operator> <value> when <T1|T2> <rising|falling>", tok);
                            return 0;
                        }

                        // Detected when or delta
                        if (!strcmp(tok, "when")) {
                            flag_when = 1;
                            flag_delta = 0;
                        }
                        else if (!strcmp(tok, "delta")) {
                            flag_when = 0;
                            flag_delta = 1;
                        }
                    }
                }

                // Check interval [ms]
                else if (flag_check_interval) {
                    ConfigRules[Counter_ConfigRules].IntegerValue = atoi(tok);

                    if (ConfigRules[Counter_ConfigRules].IntegerValue <= 0) {
                        ConfigRules[Counter_ConfigRules].IntegerValue = 1000;
                    }

                    done_interval = 1;
                    flag_check_interval = 0;
                    Counter_ConfigRules++;
                }
            }        

            tok = strtok_r(NULL, " ", &end_token);
        }
        
        line = strtok_r(NULL, "\n", &end_line);
    }    
    
    
    return 1;
}