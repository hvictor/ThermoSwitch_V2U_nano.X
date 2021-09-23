#ifndef OS_MODULE_GFX_H_
#define OS_MODULE_GFX_H_

// Constants
#define __OS_GFX__

#define GFX_SCRL_TXB_BUFSIZ         240
#define GFX_SCRL_TXB_NUMBUF         6
#define GFX_NUM_TEXTBOXES           16

#define GFX_NUM_HISTOGRAMS          2
#define GFX_HISOTGRAM_BOX_BUFSIZ    40

// Types
typedef struct
{
    uint8_t ID;
    uint8_t Count;
    char TextBuffers[GFX_SCRL_TXB_NUMBUF][GFX_SCRL_TXB_BUFSIZ];
    
    uint8_t PositionX;
    uint8_t PositionY;
    uint8_t Width;
    uint8_t Height;
    
} OS_GFX_ScrollingTextBox;

typedef struct
{
    uint8_t ID;
    uint8_t Samples;
    uint32_t Max;
    uint32_t Min;
    uint32_t Buffer[GFX_HISOTGRAM_BOX_BUFSIZ];
    
    uint16_t PositionX;
    uint16_t PositionY;
    uint16_t Width;
    uint16_t Height;
} OS_GFX_HistogramBox;

void OS_GFX_RenderBatteryIndicator(float current_voltage, float full_voltage);
void OS_GFX_RenderProgressIndicator(uint8_t x, uint8_t y, uint8_t width, uint8_t height, float percent);
OS_GFX_ScrollingTextBox *OS_GFX_CreateScrollingTextBox(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void OS_GFX_DestroyScrollingTextBox(uint8_t id);
OS_GFX_HistogramBox *OS_GFX_CreateHistogramBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t samples);
void OS_GFX_UpdateHistogramBox(OS_GFX_HistogramBox *HistogramBox, uint32_t value);
void OS_GFX_DestroyHistogramBox(uint8_t id);
void OS_GFX_Printf(OS_GFX_ScrollingTextBox *TextBox, char *format, ...);

#endif

