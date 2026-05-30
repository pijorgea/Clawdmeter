/* clang-format off */
#if 1  /* Set it to "1" to enable the content */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   Color settings
 *====================*/
#define LV_COLOR_DEPTH 16

/*=========================
   Memory settings
 *=========================*/
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC   malloc
#define LV_MEM_CUSTOM_FREE    free
#define LV_MEM_CUSTOM_REALLOC realloc

/*===================
   HAL settings
 *===================*/
#define LV_USE_OS LV_OS_NONE

/*========================
   Rendering config
 *========================*/
#define LV_DRAW_BUF_STRIDE_ALIGN 1
#define LV_DRAW_BUF_ALIGN        4

/*========================
   Logging
 *========================*/
#define LV_USE_LOG 0

/*========================
   Assertions
 *========================*/
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

/*========================
   Tick
 *========================*/
#define LV_TICK_CUSTOM 0

/*========================
   Fonts
 *========================*/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT       &lv_font_montserrat_14

/*===================
   Widget enable
 *===================*/
#define LV_USE_ARC       1
#define LV_USE_BAR       1
#define LV_USE_BTN       1
#define LV_USE_BTNMATRIX 0
#define LV_USE_CALENDAR  0
#define LV_USE_CANVAS    1
#define LV_USE_CHART     0
#define LV_USE_CHECKBOX  0
#define LV_USE_DROPDOWN  0
#define LV_USE_IMAGE     1
#define LV_USE_IMG       1
#define LV_USE_IMGBTN    0
#define LV_USE_KEYBOARD  1
#define LV_USE_LABEL     1
#define LV_USE_LED       0
#define LV_USE_LINE      1
#define LV_USE_LIST      0
#define LV_USE_MENU      0
#define LV_USE_MSGBOX    0
#define LV_USE_ROLLER    0
#define LV_USE_SCALE     0
#define LV_USE_SLIDER    0
#define LV_USE_SPAN      0
#define LV_USE_SPINBOX   0
#define LV_USE_SPINNER   0
#define LV_USE_SWITCH    0
#define LV_USE_TABLE     0
#define LV_USE_TABVIEW   0
#define LV_USE_TEXTAREA  1
#define LV_USE_TILEVIEW  0
#define LV_USE_WIN       0

/*===================
   Extra features
 *===================*/
#define LV_USE_SNAPSHOT 1
#define LV_USE_FLEX     1
#define LV_USE_GRID     1
#define LV_USE_ANIMIMG  0

/*===================
   Drivers (none)
 *===================*/
#define LV_USE_SDL 0

#endif /* LV_CONF_H */
#endif /* End of "Enable the content" */
