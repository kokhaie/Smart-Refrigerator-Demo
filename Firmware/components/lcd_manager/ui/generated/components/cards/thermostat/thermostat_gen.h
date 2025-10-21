/**
 * @file thermostat_gen.h
 */

#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

lv_obj_t * thermostat_create(lv_obj_t * parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*THERMOSTAT_H*/
