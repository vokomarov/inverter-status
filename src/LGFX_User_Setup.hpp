#include <LovyanGFX.hpp> 
#pragma once
#define LGFX_USE_V1

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Bus_SPI _bus_instance; 
    lgfx::Panel_ST7789 _panel_instance;

public:
    LGFX(void)
    {
        {
            // Bus Settings (Mandatory SPI Pins AND the DC Pin) ---
            auto cfg = _bus_instance.config();
            cfg.spi_host = HSPI_HOST;      
            cfg.freq_write = 40000000;     
            cfg.pin_sclk = 14; // SCL
            cfg.pin_mosi = 13; // SDA
            cfg.pin_miso = -1;             
            cfg.pin_dc = 15;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            
            cfg.pin_cs = 5;   // CS (GPIO 5)
            cfg.pin_rst = 16; // RST (GPIO 16)

            cfg.panel_width = 128;
            cfg.panel_height = 128;
            cfg.offset_x = 2;             
            cfg.offset_y = 1;           
            cfg.invert = true;             
            
            _panel_instance.config(cfg);
        }
        
        setPanel(&_panel_instance);
    }
};
