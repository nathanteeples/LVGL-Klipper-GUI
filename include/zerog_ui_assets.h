#pragma once

#include <lvgl.h>

namespace zerog_ui {

#if defined(LV_FONT_MONTSERRAT_10) && LV_FONT_MONTSERRAT_10
#define ZERO_G_FONT_10 (&lv_font_montserrat_10)
#else
#define ZERO_G_FONT_10 LV_FONT_DEFAULT
#endif

#if defined(LV_FONT_MONTSERRAT_12) && LV_FONT_MONTSERRAT_12
#define ZERO_G_FONT_12 (&lv_font_montserrat_12)
#else
#define ZERO_G_FONT_12 ZERO_G_FONT_10
#endif

#if defined(LV_FONT_MONTSERRAT_14) && LV_FONT_MONTSERRAT_14
#define ZERO_G_FONT_14 (&lv_font_montserrat_14)
#else
#define ZERO_G_FONT_14 ZERO_G_FONT_12
#endif

#if defined(LV_FONT_MONTSERRAT_16) && LV_FONT_MONTSERRAT_16
#define ZERO_G_FONT_16 (&lv_font_montserrat_16)
#else
#define ZERO_G_FONT_16 ZERO_G_FONT_14
#endif

#if defined(LV_FONT_MONTSERRAT_18) && LV_FONT_MONTSERRAT_18
#define ZERO_G_FONT_18 (&lv_font_montserrat_18)
#else
#define ZERO_G_FONT_18 ZERO_G_FONT_16
#endif

#if defined(LV_FONT_MONTSERRAT_20) && LV_FONT_MONTSERRAT_20
#define ZERO_G_FONT_20 (&lv_font_montserrat_20)
#else
#define ZERO_G_FONT_20 ZERO_G_FONT_18
#endif

#if defined(LV_FONT_MONTSERRAT_24) && LV_FONT_MONTSERRAT_24
#define ZERO_G_FONT_24 (&lv_font_montserrat_24)
#else
#define ZERO_G_FONT_24 ZERO_G_FONT_20
#endif

#if defined(LV_FONT_MONTSERRAT_28) && LV_FONT_MONTSERRAT_28
#define ZERO_G_FONT_28 (&lv_font_montserrat_28)
#else
#define ZERO_G_FONT_28 ZERO_G_FONT_24
#endif

#if defined(LV_FONT_MONTSERRAT_36) && LV_FONT_MONTSERRAT_36
#define ZERO_G_FONT_36 (&lv_font_montserrat_36)
#else
#define ZERO_G_FONT_36 ZERO_G_FONT_28
#endif

#if defined(LV_FONT_MONTSERRAT_42) && LV_FONT_MONTSERRAT_42
#define ZERO_G_FONT_READY (&lv_font_montserrat_42)
#elif defined(LV_FONT_MONTSERRAT_40) && LV_FONT_MONTSERRAT_40
#define ZERO_G_FONT_READY (&lv_font_montserrat_40)
#elif defined(LV_FONT_MONTSERRAT_38) && LV_FONT_MONTSERRAT_38
#define ZERO_G_FONT_READY (&lv_font_montserrat_38)
#else
#define ZERO_G_FONT_READY ZERO_G_FONT_36
#endif

#ifdef ZERO_G_TABLER_ICON_FONT
#define ZERO_G_ICON_FONT ZERO_G_TABLER_ICON_FONT
#else
#define ZERO_G_ICON_FONT ZERO_G_FONT_18
#endif

namespace icons {
inline constexpr const char *home = LV_SYMBOL_HOME;
inline constexpr const char *folders = LV_SYMBOL_DIRECTORY;
inline constexpr const char *hand_stop = LV_SYMBOL_STOP;
inline constexpr const char *border_corners = "[]";
inline constexpr const char *adjustments_alt = LV_SYMBOL_EDIT;
inline constexpr const char *settings = LV_SYMBOL_SETTINGS;
inline constexpr const char *nozzle = LV_SYMBOL_CHARGE;
inline constexpr const char *bed = LV_SYMBOL_DRIVE;
inline constexpr const char *power = LV_SYMBOL_POWER;
inline constexpr const char *pause = LV_SYMBOL_PAUSE;
inline constexpr const char *play = LV_SYMBOL_PLAY;
inline constexpr const char *cancel = LV_SYMBOL_CLOSE;
inline constexpr const char *delete_file = LV_SYMBOL_TRASH;
inline constexpr const char *print = LV_SYMBOL_PLAY;
inline constexpr const char *cube = "[]";
inline constexpr const char *up = LV_SYMBOL_UP;
inline constexpr const char *down = LV_SYMBOL_DOWN;
inline constexpr const char *left = LV_SYMBOL_LEFT;
inline constexpr const char *right = LV_SYMBOL_RIGHT;
inline constexpr const char *home_axis = LV_SYMBOL_HOME;
inline constexpr const char *retract = LV_SYMBOL_LEFT;
inline constexpr const char *extrude = LV_SYMBOL_RIGHT;
inline constexpr const char *motor_off = LV_SYMBOL_POWER;
inline constexpr const char *fans = LV_SYMBOL_LOOP;
inline constexpr const char *gauge = LV_SYMBOL_DRIVE;
inline constexpr const char *flow = LV_SYMBOL_UPLOAD;
inline constexpr const char *z_axis = LV_SYMBOL_UP;
inline constexpr const char *pressure = LV_SYMBOL_REFRESH;
inline constexpr const char *accel = LV_SYMBOL_CHARGE;
inline constexpr const char *corner = LV_SYMBOL_NEXT;
inline constexpr const char *back = LV_SYMBOL_LEFT;
inline constexpr const char *wifi = LV_SYMBOL_WIFI;
inline constexpr const char *screen = LV_SYMBOL_IMAGE;
inline constexpr const char *klipper = LV_SYMBOL_DRIVE;
inline constexpr const char *audio = LV_SYMBOL_VOLUME_MID;
inline constexpr const char *console = ">_";
inline constexpr const char *brightness = LV_SYMBOL_EYE_OPEN;
inline constexpr const char *sleep = LV_SYMBOL_BELL;
inline constexpr const char *theme = LV_SYMBOL_IMAGE;
inline constexpr const char *connect = LV_SYMBOL_OK;
inline constexpr const char *touch_beeps = LV_SYMBOL_BELL;
inline constexpr const char *alerts = LV_SYMBOL_WARNING;
inline constexpr const char *restart = LV_SYMBOL_REFRESH;
inline constexpr const char *restart_mcu = LV_SYMBOL_REFRESH;
inline constexpr const char *restart_host = LV_SYMBOL_WIFI;
inline constexpr const char *shutdown = LV_SYMBOL_POWER;
inline constexpr const char *calibrate = LV_SYMBOL_EDIT;
inline constexpr const char *load = LV_SYMBOL_UPLOAD;
inline constexpr const char *check = LV_SYMBOL_OK;
inline constexpr const char *keyboard = LV_SYMBOL_KEYBOARD;
inline constexpr const char *numpad_clear = "CLR";
inline constexpr const char *file_command = LV_SYMBOL_EDIT;
}

}  // namespace zerog_ui
