/**
 * @file examples.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "ui/generated/examples.h"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void examples_init(const char * asset_path)
{
    examples_init_gen(asset_path);

    /* Hook for additional customization if needed */
}
