#include <stddef.h>

#include "../../tft_st7789.h"
#include "../include/os_gfx.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>

// Data structures
static uint8_t Counter_TextBoxes = 0;
static uint8_t Counter_HistogramBoxes = 0;

static OS_GFX_ScrollingTextBox TextBoxes[GFX_NUM_TEXTBOXES];
static OS_GFX_HistogramBox HistogramBoxes[GFX_NUM_HISTOGRAMS];

static uint16_t background_color = TFT_COLOR_BLACK; 
static uint16_t indicator_color = TFT_COLOR_WHITE;

void OS_GFX_RenderBatteryIndicator(float current_voltage, float full_voltage)
{
    uint8_t x = 224, y = 0;
    uint8_t w = 15, h = 15;
    uint8_t connector_width = 7;
    uint8_t connector_height = 2;
    uint8_t connector_x_offset = 5;
    
    float percent_full = current_voltage / full_voltage;
    int h_empty = (1.0 - percent_full) * (h - 1);
    
    tft_fill_rect(x + connector_x_offset, y, connector_width, connector_height, TFT_COLOR_WHITE);
    
    // Zero-out 
    tft_fill_rect(x, y + connector_height, w, h, background_color);
    tft_draw_rect(x, y + connector_height, w, h, indicator_color);
    tft_fill_rect(x + 1, y + connector_height + 1 + h_empty, h - 1, h - h_empty, TFT_COLOR_GREEN);
}

void OS_GFX_RenderProgressIndicator(uint8_t x, uint8_t y, uint8_t width, uint8_t height, float percent)
{
    uint8_t r = 255, g = 255, b = 0;
    
    tft_draw_rect(x, y, width, height, TFT_COLOR_WHITE);
    uint8_t wx = 2;
    uint8_t px = x + 1;
    for (int i = 0; i < width / 2; i++)
    {
        if ((i + 1) * wx > percent * width)
            break;
        
        if (px + wx >= x + width)
            wx--;
        
        tft_fill_rect(px, y+1, wx, height - 1, tft_color_u16(r, g, b));
        
        if (g - 5 < 0)
            g = 0;
        else
            g -= 5;
         
        px += 2;
    }
    
    tft_fill_rect(px, y + 1, x + width - px, height - 1, TFT_COLOR_BLACK);
}

OS_GFX_HistogramBox *OS_GFX_CreateHistogramBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t samples)
{

    OS_GFX_HistogramBox *box = &HistogramBoxes[Counter_HistogramBoxes];
    box->PositionX = x;
    box->PositionY = y;
    box->Width = width;
    box->Height = height;
    box->Max = 0;
    box->Min = 0;
    
    memset(box->Buffer, 0, sizeof(box->Buffer));
    box->Samples = 0;
    
    Counter_HistogramBoxes++;
    
    tft_draw_rect(box->PositionX, box->PositionY, box->Width, box->Height, TFT_COLOR_LUNAR_BLUE_DARK);
     
    return box;
}

void OS_GFX_UpdateHistogramBox(OS_GFX_HistogramBox *Box, uint32_t value)
{
    float mean = 0;
    
    if (Box->Samples == 0)
    {
        Box->Max = value;
        Box->Min = value;
    }
    else
    {
        for (int k = 0; k < Box->Samples; k++) 
        {
            mean += (float)Box->Buffer[k];
            
            if (Box->Buffer[k] > Box->Max)
                Box->Max = Box->Buffer[k];

            if (Box->Buffer[k] < Box->Min)
                Box->Min = Box->Buffer[k];
        }
        
        mean /= Box->Samples;
    }
    
    if (Box->Samples < GFX_HISOTGRAM_BOX_BUFSIZ) {
        Box->Buffer[Box->Samples] = value;
        Box->Samples++;
    }
    
    else {
        
        for (int i = 0; i < GFX_HISOTGRAM_BOX_BUFSIZ-1; i++) {
            Box->Buffer[i] = Box->Buffer[i + 1];
        }
        Box->Buffer[GFX_HISOTGRAM_BOX_BUFSIZ - 1] = value; 
    }

    for (int k = 0; k < Box->Samples; k++) {
        if (Box->Max > Box->Min) {
            float v = Box->Buffer[k];
            int min = Box->Min;
            int max = Box->Max;
            int h = Box->Height;
            tft_fill_rect(k * 4 + k + 1, Box->PositionY + 1, 4, Box->Height - 1, TFT_COLOR_BLACK);
            float renormalized_value = (float)Box->Buffer[k] / (float)Box->Max;            
            float renormalized_mean = mean / (float)Box->Max;
            float rescaled_value = renormalized_value * Box->Height;
            float rescaled_mean = renormalized_mean * Box->Height;
            
            float diff = rescaled_value - rescaled_mean;
            float represented_value = rescaled_value + 10*diff;
            
            if (represented_value > Box->Height)
                represented_value = Box->Height;
            
            if (represented_value < -Box->Height)
                represented_value = 0;
                        
            tft_fill_rect(k * 4 + k + 1, Box->PositionY + h + 1 - represented_value, 4, represented_value, TFT_COLOR_LUNAR_BLUE_DARK);
        } else {
            tft_fill_rect(k * 4 + k + 1, Box->PositionY + Box->Height - Box->Height / 2, 4, Box->Height / 2, TFT_COLOR_LUNAR_BLUE_DARK);
        }
    }
}

void OS_GFX_DestroyHistogramBox(uint8_t id)
{
    
}

OS_GFX_ScrollingTextBox *OS_GFX_CreateScrollingTextBox(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    if (Counter_TextBoxes >= GFX_NUM_TEXTBOXES)
        return NULL;
    
    int i = 0;
    for ( ; i < GFX_NUM_TEXTBOXES; i++) {
        if (!TextBoxes[i].ID) {
            break;
        }
    }
    
    OS_GFX_ScrollingTextBox *TextBox = &(TextBoxes[i]);
    TextBox->ID = i + 1;
    TextBox->Count = 0;
    TextBox->PositionX = x;
    TextBox->PositionY = y;
    TextBox->Width = width;
    TextBox->Height = height;
    
    for (i = 0; i < GFX_SCRL_TXB_NUMBUF; i++) {
        memset(TextBox->TextBuffers[i], 0, GFX_SCRL_TXB_BUFSIZ);
    }
    
    Counter_TextBoxes++;
    
    return TextBox;
}

void OS_GFX_DestroyScrollingTextBox(uint8_t id)
{
    TextBoxes[id - 1].ID = 0;
}

void OS_GFX_Printf(OS_GFX_ScrollingTextBox *TextBox, char *format, ...)
{
    va_list argp;
    va_start(argp, format);
    
    // Black-out existing lines
    /*
    tft_set_cursor(TextBox->PositionX, TextBox->PositionY);
    tft_set_text_color(TFT_COLOR_BLACK);
    for (int i = 0; i < GFX_SCRL_TXB_NUMBUF; i++) {
        if (strlen(TextBox->TextBuffers[i])) {
            tft_printf(TextBox->TextBuffers[i]);
        }
    }
    */
    
    // Copy up and free the last buffer
    tft_set_cursor(TextBox->PositionX, TextBox->PositionY);

    for (int i = 1; i < GFX_SCRL_TXB_NUMBUF; i++) {
        // Black-out
        if (strlen(TextBox->TextBuffers[i - 1])) {
            tft_set_text_color(TFT_COLOR_BLACK);
            tft_printf(TextBox->TextBuffers[i - 1]);
            tft_set_cursor(TextBox->PositionX, tft_get_cursor_y() - tft_get_char_pixels_y());
        }
        
        strcpy(TextBox->TextBuffers[i - 1], TextBox->TextBuffers[i]);
        
        if (strlen(TextBox->TextBuffers[i - 1])) {
            tft_set_text_color(TFT_COLOR_WHITE);
            tft_printf(TextBox->TextBuffers[i - 1]);
        }
        else {
            tft_printf("\n");
        }
    }
    
    // Clear and write the last buffer
    tft_set_text_color(TFT_COLOR_BLACK);
    tft_printf(TextBox->TextBuffers[GFX_SCRL_TXB_NUMBUF - 1]);
    memset(TextBox->TextBuffers[GFX_SCRL_TXB_NUMBUF - 1], 0, GFX_SCRL_TXB_BUFSIZ);
    char *ptr = &(TextBox->TextBuffers[GFX_SCRL_TXB_NUMBUF - 1][0]);
    
    while (*format != '\0')
    {
        if (*format == '%')
        {
            format++;
            if (*format == '%')
            {
                *ptr++ = '%';
            } else if (*format == 'c')
            {
                char char_to_print = va_arg(argp, int);
                *ptr++ = char_to_print;
            }
        } else
        {
            *ptr++ = *format;
        }
        
        format++;
    }
    
    // Re-write updated buffers
    tft_set_cursor(TextBox->PositionX, TextBox->PositionY + tft_get_char_pixels_y() * (GFX_SCRL_TXB_NUMBUF - 1));
    tft_set_text_color(TFT_COLOR_RED);
    if (strlen(TextBox->TextBuffers[GFX_SCRL_TXB_NUMBUF - 1])) {
        tft_printf(TextBox->TextBuffers[GFX_SCRL_TXB_NUMBUF - 1]);
    }
    
    va_end(argp);
    
    
}