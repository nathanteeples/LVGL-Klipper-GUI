#pragma once

#include <lvgl.h>
#include <stdint.h>

namespace zerog_ui {

enum ScreenId : uint8_t {
  SCREEN_HOME = 0,
  SCREEN_FILES,
  SCREEN_CONTROL,
  SCREEN_MESH,
  SCREEN_TUNING,
  SCREEN_SETTINGS,
  SCREEN_COUNT
};

enum PrinterState : uint8_t {
  PRINTER_IDLE = 0,
  PRINTER_PRINTING,
  PRINTER_PAUSED
};

void init(lv_obj_t *parent = nullptr);

void create_sidebar();
void create_topbar();
void create_home_screen();
void create_files_screen();
void create_control_screen();
void create_mesh_screen();
void create_tuning_screen();
void create_settings_screen();

void show_screen(ScreenId screen_id);
void show_keyboard(lv_obj_t *target_field);
void show_numpad(lv_obj_t *target_temp_field);

void update_status();
void update_temperatures(int nozzle_current, int nozzle_target, int bed_current, int bed_target);
void update_progress(uint8_t percent, uint16_t layer_current, uint16_t layer_total, const char *remaining);

void set_printer_state(PrinterState state);
void set_time_text(const char *time_text);
void set_selected_file(uint8_t index);

}  // namespace zerog_ui
