#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include "lvgl.h"
#include "lcd_manager.h"

extern lv_obj_t *dashboard_view_root;

void dashboard_view_init(void);
void dashboard_view_deinit(void);

void dashboard_view_set_dark_theme(bool enable);
void dashboard_view_set_room_temperature(int32_t temperature_c);
void dashboard_view_set_target_temperature(int32_t temperature_c);
void dashboard_view_set_mode_display(const char *label, uint32_t accent_rgb24);
void dashboard_view_set_connectivity_state(lcd_connectivity_state_t state);

#endif /* DASHBOARD_VIEW_H */
