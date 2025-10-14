#include "ui_manager.h"
#include <stdio.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_spiffs.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "ui.h"

static const char *TAG = "UI_Manager";

// --- LVGL Porting ---
static _lock_t lvgl_api_lock;
static esp_lcd_panel_handle_t panel_handle = NULL;
#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2
#define LVGL_DRAW_BUF_LINES 100

#define DMA_BURST_SIZE 64 // 16, 32, 64. Higher burst size can improve the performance when the DMA buffer comes from PSRAM

#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *display = (lv_display_t *)user_ctx;
    lv_display_flush_ready(display);
    return false;
}
static void lvgl_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color_map)
{

    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(display);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // because LCD is big-endian, we need to swap the RGB bytes order
    lv_draw_sw_rgb565_swap(color_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}
static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}
static void lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t time_till_next_ms = 0;
    while (1)
    {
        _lock_acquire(&lvgl_api_lock);
        time_till_next_ms = lv_timer_handler();
        _lock_release(&lvgl_api_lock);
        // in case of triggering a task watch dog time out
        time_till_next_ms = MAX(time_till_next_ms, LVGL_TASK_MIN_DELAY_MS);
        // in case of lvgl display not ready yet
        time_till_next_ms = MIN(time_till_next_ms, LVGL_TASK_MAX_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(time_till_next_ms));
    }
}
void init_i80_bus(esp_lcd_panel_io_handle_t *io_handle)
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
        // .max_transfer_bytes = CONFIG_LCD_H_RES * 100 * sizeof(uint16_t),
        .max_transfer_bytes = CONFIG_LCD_H_RES * 120 * sizeof(uint16_t), // Changed 100 to 120
        .dma_burst_size = DMA_BURST_SIZE,

    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = CONFIG_LCD_PIN_CS,
        .pclk_hz = CONFIG_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,

    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));
}
void init_lcd_panel(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t *panel)
{
    esp_lcd_panel_handle_t panel_handle = NULL;
    ESP_LOGI(TAG, "Install LCD driver of st7789");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CONFIG_LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,

    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    // Set inversion, x/y coordinate order, x/y mirror according to your LCD module spec
    // the gap is LCD panel specific, even panels with the same driver IC, can have different gap value
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_set_gap(panel_handle, 0, 20);

    *panel = panel_handle;
}
static void lcd_color_test(lv_display_t *display)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)lv_display_get_user_data(display);
    if (!panel_handle)
    {
        ESP_LOGE(TAG, "Panel handle is NULL!");
        return;
    }

    // Allocate one strip buffer (height = 40 lines here, adjust as needed)
    int strip_lines = 40;
    size_t strip_pixels = CONFIG_LCD_H_RES * strip_lines;
    size_t buffer_size = strip_pixels * sizeof(uint16_t);

    uint16_t *line_buf = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    if (!line_buf)
    {
        ESP_LOGE(TAG, "Failed to allocate strip buffer!");
        return;
    }

    // Test colors
    uint16_t colors[] = {
        0xF800, // Red
        0x07E0, // Green
        0x001F, // Blue
        0xFFFF, // White
        0x0000  // Black
    };
    while (1)
    {

        for (int c = 0; c < sizeof(colors) / sizeof(colors[0]); c++)
        {
            ESP_LOGI(TAG, "LCD test color: 0x%04X", colors[c]);

            // Fill once (all pixels same color)
            for (size_t i = 0; i < strip_pixels; i++)
            {
                line_buf[i] = colors[c];
            }

            // Push in strips
            for (int y = 0; y < CONFIG_LCD_V_RES; y += strip_lines)
            {
                int y_end = y + strip_lines;
                if (y_end > CONFIG_LCD_V_RES)
                {
                    y_end = CONFIG_LCD_V_RES;
                }
                ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle,
                                                          0, y,
                                                          CONFIG_LCD_H_RES, y_end,
                                                          line_buf));
            }

            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
    free(line_buf);
    ESP_LOGI(TAG, "LCD color test finished");
}

void ui_manager_start(void)
{
    esp_lcd_panel_io_handle_t io_handle = NULL;
    init_i80_bus(&io_handle);

    esp_lcd_panel_handle_t panel_handle = NULL;
    init_lcd_panel(io_handle, &panel_handle);

    // Stub: user can flush pre-defined pattern to the screen before we turn on the screen or backlight

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();

    // create a lvgl display
    lv_display_t *display = lv_display_create(CONFIG_LCD_H_RES, CONFIG_LCD_V_RES);

    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    size_t draw_buffer_sz = CONFIG_LCD_H_RES * LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t);
    // alloc draw buffers used by LVGL
    uint32_t draw_buf_alloc_caps = 0;
    // #if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
    draw_buf_alloc_caps |= MALLOC_CAP_SPIRAM;
    // #endif
    void *buf1 = esp_lcd_i80_alloc_draw_buffer(io_handle, draw_buffer_sz, draw_buf_alloc_caps);
    void *buf2 = esp_lcd_i80_alloc_draw_buffer(io_handle, draw_buffer_sz, draw_buf_alloc_caps);
    assert(buf1);
    assert(buf2);
    ESP_LOGI(TAG, "buf1@%p, buf2@%p", buf1, buf2);

    // initialize LVGL draw buffers
    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    // associate the mipi panel handle to the display
    lv_display_set_user_data(display, panel_handle);
    // set color depth
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    // set the callback which can copy the rendered image to an area of the display
    lv_display_set_flush_cb(display, lvgl_flush_cb);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Register io panel event callback for LVGL flush ready notification");
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    /* Register done callback */
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, display));

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);

    ESP_LOGI(TAG, "Display LVGL animation");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    _lock_acquire(&lvgl_api_lock);
    // ui_init(display);
    lcd_color_test(display);
    _lock_release(&lvgl_api_lock);
}
