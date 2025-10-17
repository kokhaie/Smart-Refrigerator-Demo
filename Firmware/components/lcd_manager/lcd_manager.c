#include "lcd_manager.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "esp_heap_caps.h"
#include "ui.h"

static const char *TAG = "LCD_MANAGER";

#define LCD_H_RES 240
#define LCD_V_RES 280
#define DMA_BURST_SIZE 64
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

// --- Rotation define (0, 90, 180, 270) ---
#define LCD_ROTATION 270

static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_display_t *lvgl_disp = NULL;

// Allocate framebuffers in PSRAM
static lv_color_t *lvgl_buf1 = NULL;
static lv_color_t *lvgl_buf2 = NULL;

//------------------------------------------------------------------
// Flush callback (SPIRAM → LCD)
//------------------------------------------------------------------
static void psram_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(
        panel,
        area->x1, area->y1,
        area->x2 + 1, area->y2 + 1, // exclusive end
        px_map));

    lv_display_flush_ready(disp);
}

//------------------------------------------------------------------
// i80 bus init
//------------------------------------------------------------------
static void init_i80_bus(esp_lcd_panel_io_handle_t *io_handle)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = CONFIG_LCD_PIN_DC,
        .wr_gpio_num = CONFIG_LCD_PIN_WR,
        .data_gpio_nums = {
            CONFIG_LCD_PIN_DATA0,
            CONFIG_LCD_PIN_DATA1,
            CONFIG_LCD_PIN_DATA2,
            CONFIG_LCD_PIN_DATA3,
            CONFIG_LCD_PIN_DATA4,
            CONFIG_LCD_PIN_DATA5,
            CONFIG_LCD_PIN_DATA6,
            CONFIG_LCD_PIN_DATA7,
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_H_RES * LCD_V_RES * sizeof(uint16_t),
        .dma_burst_size = DMA_BURST_SIZE,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = CONFIG_LCD_PIN_CS,
        .pclk_hz = CONFIG_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {0, 0, 0, 1},
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .flags = {.swap_color_bytes = 1},
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));
}

//------------------------------------------------------------------
// ST7789 rotation helper (MADCTL register 0x36)
//------------------------------------------------------------------
static void st7789_set_rotation(esp_lcd_panel_io_handle_t io, uint16_t rotation)
{
    uint8_t madctl = 0x00;

    switch (rotation)
    {
    case 0:
        madctl = 0x00; // Normal
        break;
    case 90:
        madctl = 0x60; // MV + MX
        break;
    case 180:
        madctl = 0xC0; // MX + MY
        break;
    case 270:
        madctl = 0xA0; // MV + MY
        break;
    default:
        madctl = 0x00;
        break;
    }

    ESP_LOGI(TAG, "Apply MADCTL rotation = %d deg (0x%02X)", rotation, madctl);
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io, 0x36, &madctl, 1));
}

//------------------------------------------------------------------
// LCD panel init (ST7789 240x280 with gap_y=20)
//------------------------------------------------------------------
static void init_lcd_panel(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t *panel)
{
    ESP_LOGI(TAG, "Install LCD driver of st7789");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CONFIG_LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // Use BGR if colors wrong
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, panel));

    esp_lcd_panel_reset(*panel);
    esp_lcd_panel_init(*panel);

    // --- Hardware rotation ---
    st7789_set_rotation(io_handle, LCD_ROTATION);

    esp_lcd_panel_set_gap(*panel, 20, 0);

    esp_lcd_panel_invert_color(*panel, true);
    esp_lcd_panel_disp_on_off(*panel, true);
}

//------------------------------------------------------------------
// Main entry
//------------------------------------------------------------------
void lcd_manager_start(void)
{
    esp_lcd_panel_io_handle_t io_handle = NULL;
    init_i80_bus(&io_handle);
    init_lcd_panel(io_handle, &panel_handle);

    ESP_LOGI(TAG, "Initialize LVGL task & tick");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    // --- Allocate LVGL framebuffers in PSRAM ---
    size_t fb_size = LCD_H_RES * LCD_V_RES * sizeof(lv_color_t);

    lvgl_buf1 = (lv_color_t *)heap_caps_aligned_alloc(
        128, fb_size,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    lvgl_buf2 = (lv_color_t *)heap_caps_aligned_alloc(
        128, fb_size,
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    assert(lvgl_buf1 && lvgl_buf2);

    // --- Register LVGL display ---
#if (LCD_ROTATION == 90) || (LCD_ROTATION == 270)
    lvgl_disp = lv_display_create(LCD_V_RES, LCD_H_RES); // swap res
#else
    lvgl_disp = lv_display_create(LCD_H_RES, LCD_V_RES);
#endif

    lv_display_set_flush_cb(lvgl_disp, psram_flush_cb);
    lv_display_set_buffers(
        lvgl_disp,
        lvgl_buf1,
        lvgl_buf2,
        fb_size,
        LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(lvgl_disp, panel_handle);

    ESP_LOGI(TAG, "Load SquareLine UI");
    lvgl_port_lock(0);

    // Call SquareLine Studio generated UI
    ui_init(lvgl_disp);

    lvgl_port_unlock();
}
