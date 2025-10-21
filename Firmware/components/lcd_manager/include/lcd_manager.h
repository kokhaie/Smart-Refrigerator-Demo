#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <stdint.h>

typedef enum
{
    LCD_CONNECTIVITY_STATE_OFFLINE = 0,
    LCD_CONNECTIVITY_STATE_CONNECTING,
    LCD_CONNECTIVITY_STATE_ONLINE
} lcd_connectivity_state_t;

/**
 * @brief Initializes the display hardware, LVGL library, and starts the UI.
 *
 * This function handles all the setup required to get the screen running and
 * ready to display a user interface designed with SquareLine Studio.
 */
void lcd_manager_start(void);

/**
 * @brief Update the thermostat card's target temperature label and gauge.
 *
 * @param temperature_c Rounded temperature in °C to show on the UI.
 */
void lcd_manager_set_thermostat_target(int32_t temperature_c);

/**
 * @brief Update the thermostat card's current/room temperature label.
 *
 * @param temperature_c Rounded temperature in °C to show on the UI.
 */
void lcd_manager_set_room_temperature(int32_t temperature_c);

void lcd_manager_set_mode_display(const char *label, uint32_t accent_rgb24);

void lcd_manager_set_connectivity_state(lcd_connectivity_state_t state);

#endif
