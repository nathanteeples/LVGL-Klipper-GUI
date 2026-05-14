#include "zerog_ui.h"

#include "zerog_ui_assets.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace zerog_ui {
namespace {

constexpr lv_coord_t kScreenWidth = 480;
constexpr lv_coord_t kScreenHeight = 272;
constexpr lv_coord_t kSidebarWidth = 56;
constexpr lv_coord_t kMainWidth = 424;
constexpr lv_coord_t kMainPaddingX = 10;
constexpr lv_coord_t kMainPaddingTop = 9;
constexpr lv_coord_t kMainPaddingBottom = 9;
constexpr lv_coord_t kTopbarHeight = 32;
constexpr lv_coord_t kContentGap = 8;
constexpr lv_coord_t kContentHeight = 214;
constexpr lv_coord_t kContentWidth = 404;

constexpr uint32_t kColorBg = 0x080808;
constexpr uint32_t kColorSidebar = 0x0b0b0b;
constexpr uint32_t kColorPanel = 0x151515;
constexpr uint32_t kColorPanelDark = 0x101010;
constexpr uint32_t kColorText = 0xf7f7f7;
constexpr uint32_t kColorMuted = 0xa8a8a8;
constexpr uint32_t kColorMutedLight = 0xd5d5d5;
constexpr uint32_t kColorRed = 0xe53935;
constexpr uint32_t kColorRedBright = 0xff524d;
constexpr uint32_t kColorGreen = 0x26d07c;
constexpr uint32_t kColorYellow = 0xffc14d;
constexpr uint32_t kColorButtonDark = 0x242424;
constexpr uint32_t kColorButtonMid = 0x202020;
constexpr uint32_t kColorTrack = 0x292929;

constexpr char kDegree[] = "\xC2\xB0";
constexpr char kSecretMask[] =
    "\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2"
    "\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2";

constexpr const char *kScreenTitles[SCREEN_COUNT] = {
    "Home", "Files", "Control", "Bed Mesh", "Tuning", "Settings"};

struct FileSpec {
  const char *name;
  const char *meta;
};

constexpr FileSpec kFiles[] = {
    {"Voron_Cube_0.2mm.gcode", "18m | PLA"},
    {"Speed_Benchy_long_filename_pressure_advance_test.gcode", "42m | PLA"},
    {"KlipperScreen_Case_Front_Panel.gcode", "2h 14m | PETG"},
    {"SHT36_Toolhead_Duct_v7_CF.gcode", "1h 03m | ASA"},
    {"Bed_Level_Test_370x370.gcode", "12m | PLA"},
    {"Cable_Clip_Tiny_4x.gcode", "9m | TPU"},
};

constexpr int kExtrusionSteps[] = {1, 5, 10, 25};
constexpr const char *kJogSteps[] = {"0.1", "1", "10", "100"};

enum SettingsPageId : uint8_t {
  SETTINGS_PAGE_HOME = 0,
  SETTINGS_PAGE_NETWORK,
  SETTINGS_PAGE_SCREEN,
  SETTINGS_PAGE_KLIPPER,
  SETTINGS_PAGE_AUDIO,
  SETTINGS_PAGE_CONSOLE,
  SETTINGS_PAGE_POWER,
  SETTINGS_PAGE_COUNT
};

enum SliderFormat : uint8_t {
  SLIDER_FMT_PERCENT = 0,
  SLIDER_FMT_MM_100,
  SLIDER_FMT_PA_1000,
  SLIDER_FMT_RAW
};

struct SliderBinding {
  lv_obj_t *slider;
  lv_obj_t *output;
  SliderFormat format;
  const char *unit;
  int8_t fan_index;
};

struct TextFieldMeta {
  bool secret;
  const char *placeholder;
};

struct FileRow {
  lv_obj_t *button;
  lv_obj_t *title;
  lv_obj_t *meta;
};

struct FanIcon {
  lv_obj_t *root;
  lv_obj_t *ring;
  lv_obj_t *hub;
  lv_obj_t *blades[3];
  uint8_t speed;
  int16_t angle;
};

struct IdleBlob {
  lv_obj_t *obj;
  lv_coord_t base_x;
  lv_coord_t base_y;
  lv_coord_t amp_x;
  lv_coord_t amp_y;
  float phase;
};

struct UiContext {
  lv_obj_t *root;
  lv_obj_t *sidebar;
  lv_obj_t *main_area;
  lv_obj_t *topbar;
  lv_obj_t *content_host;
  lv_obj_t *screen_roots[SCREEN_COUNT];
  lv_obj_t *nav_buttons[SCREEN_COUNT];
  lv_obj_t *status_pill;
  lv_obj_t *status_label;
  lv_obj_t *logo_label;
  lv_obj_t *title_label;
  lv_obj_t *time_label;
  lv_obj_t *nozzle_value;
  lv_obj_t *bed_value;

  lv_obj_t *home_idle;
  lv_obj_t *home_printing;
  lv_obj_t *home_file_name;
  lv_obj_t *pause_btn;
  lv_obj_t *pause_icon;
  lv_obj_t *pause_text;
  lv_obj_t *percent_label;
  lv_obj_t *layer_label;
  lv_obj_t *remaining_label;
  lv_obj_t *progress_arc;
  lv_obj_t *mesh_temp_label;
  lv_obj_t *mesh_profile_label;

  lv_obj_t *files_preview;
  lv_obj_t *start_print_btn;
  FileRow file_rows[sizeof(kFiles) / sizeof(kFiles[0])];
  uint8_t selected_file;
  char current_print_file[80];

  lv_obj_t *temp_nozzle_field;
  lv_obj_t *temp_bed_field;
  lv_obj_t *extrusion_amount_label;
  lv_obj_t *extrude_step_buttons[4];
  lv_obj_t *jog_step_buttons[4];

  lv_obj_t *tuning_main;
  lv_obj_t *fan_subpage;
  lv_obj_t *fan_summary_label;
  FanIcon fans[3];

  lv_obj_t *settings_pages[SETTINGS_PAGE_COUNT];

  lv_obj_t *keyboard_modal;
  lv_obj_t *keyboard_window;
  lv_obj_t *keyboard_preview;
  lv_obj_t *keyboard_target;
  TextFieldMeta keyboard_meta;
  char keyboard_buffer[72];

  lv_obj_t *numpad_modal;
  lv_obj_t *numpad_window;
  lv_obj_t *numpad_preview;
  lv_obj_t *numpad_target;
  char numpad_buffer[8];

  SliderBinding sliders[16];
  uint8_t slider_count;
  TextFieldMeta field_meta[8];
  uint8_t field_meta_count;
  IdleBlob idle_blobs[4];

  lv_timer_t *anim_timer;
  ScreenId active_screen;
  SettingsPageId active_settings_page;
  PrinterState printer_state;
  uint8_t progress;
  uint16_t layer_current;
  uint16_t layer_total;
  int nozzle_current;
  int nozzle_target;
  int bed_current;
  int bed_target;
  char remaining[24];
  char time_text[8];

  lv_style_t style_panel;
  lv_style_t style_panel_dark;
  lv_style_t style_button_red;
  lv_style_t style_button_dark;
  lv_style_t style_icon_button;
  lv_style_t style_status;
  lv_style_t style_slider_main;
  lv_style_t style_slider_indicator;
  lv_style_t style_slider_knob;
  lv_style_t style_text_field;
  lv_style_t style_tile;
  lv_style_t style_overlay;
  lv_style_t style_overlay_window;
  lv_style_t style_printer_line;
};

UiContext g;

void set_rect(lv_obj_t *obj, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
  lv_obj_set_pos(obj, x, y);
  lv_obj_set_size(obj, w, h);
}

void copy_text(char *dst, size_t len, const char *src) {
  if (!dst || len == 0U) {
    return;
  }
  if (!src) {
    dst[0] = '\0';
    return;
  }
  strncpy(dst, src, len - 1U);
  dst[len - 1U] = '\0';
}

lv_color_t color_hex(uint32_t value) {
  return lv_color_hex(value);
}

void format_temp_chip(char *buf, size_t len, int current, int target) {
  if (target > 0) {
    snprintf(buf, len, "#f7f7f7%d%s#/#a8a8a8%d%s#", current, kDegree, target, kDegree);
  } else {
    snprintf(buf, len, "#f7f7f7%d%s#/#a8a8a8Off#", current, kDegree);
  }
}

void format_temp_field(char *buf, size_t len, int target) {
  if (target > 0) {
    snprintf(buf, len, "%d%s", target, kDegree);
  } else {
    copy_text(buf, len, "Off");
  }
}

void set_button_active(lv_obj_t *btn, bool active) {
  lv_obj_set_style_bg_color(btn, active ? color_hex(kColorRed) : color_hex(kColorButtonDark), 0);
  const lv_color_t text_color = active ? color_hex(kColorText) : color_hex(kColorMuted);
  lv_obj_set_style_text_color(btn, text_color, 0);

  const uint32_t child_count = lv_obj_get_child_cnt(btn);
  for (uint32_t i = 0; i < child_count; ++i) {
    lv_obj_set_style_text_color(lv_obj_get_child(btn, i), text_color, 0);
  }
}

void set_nav_button_state(uint8_t index, bool active) {
  if (index >= SCREEN_COUNT || !g.nav_buttons[index]) {
    return;
  }

  lv_obj_t *btn = g.nav_buttons[index];
  const lv_color_t text_color = active ? color_hex(kColorText) : lv_color_hex(0xb7b7b7);
  lv_obj_set_style_bg_opa(btn, active ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  lv_obj_set_style_bg_color(btn, active ? color_hex(kColorRed) : color_hex(kColorSidebar), 0);
  lv_obj_set_style_text_color(btn, text_color, 0);
  lv_obj_set_x(btn, active ? 10 : 7);

  const uint32_t child_count = lv_obj_get_child_cnt(btn);
  for (uint32_t i = 0; i < child_count; ++i) {
    lv_obj_set_style_text_color(lv_obj_get_child(btn, i), text_color, 0);
  }
}

void slider_event_cb(lv_event_t *e);
void nav_event_cb(lv_event_t *e);
void shortcut_nav_event_cb(lv_event_t *e);
void file_select_event_cb(lv_event_t *e);
void pause_event_cb(lv_event_t *e);
void cancel_event_cb(lv_event_t *e);
void start_print_event_cb(lv_event_t *e);
void open_fans_event_cb(lv_event_t *e);
void back_fans_event_cb(lv_event_t *e);
void settings_tile_event_cb(lv_event_t *e);
void settings_back_event_cb(lv_event_t *e);
void segment_event_cb(lv_event_t *e);
void extrude_step_event_cb(lv_event_t *e);
void text_field_event_cb(lv_event_t *e);
void temp_field_event_cb(lv_event_t *e);
void temp_off_event_cb(lv_event_t *e);
void keyboard_key_event_cb(lv_event_t *e);
void keyboard_cancel_event_cb(lv_event_t *e);
void keyboard_apply_event_cb(lv_event_t *e);
void numpad_key_event_cb(lv_event_t *e);
void numpad_cancel_event_cb(lv_event_t *e);
void numpad_apply_event_cb(lv_event_t *e);
void show_settings_page(SettingsPageId page_id);

void anim_set_x(void *obj, int32_t value) {
  lv_obj_set_x(static_cast<lv_obj_t *>(obj), value);
}

void anim_set_y(void *obj, int32_t value) {
  lv_obj_set_y(static_cast<lv_obj_t *>(obj), value);
}

void anim_set_width(void *obj, int32_t value) {
  lv_obj_set_width(static_cast<lv_obj_t *>(obj), static_cast<lv_coord_t>(value));
}

void anim_set_opa(void *obj, int32_t value) {
  lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), static_cast<lv_opa_t>(value), 0);
}

void hide_anim_ready_cb(lv_anim_t *a) {
  lv_obj_t *obj = static_cast<lv_obj_t *>(a->var);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_x(obj, 0);
  lv_obj_set_style_opa(obj, LV_OPA_COVER, 0);
}

void animate_property(lv_obj_t *obj, int32_t start, int32_t end, uint16_t time,
                      lv_anim_exec_xcb_t exec_cb, lv_anim_ready_cb_t ready_cb = nullptr) {
  lv_anim_t anim;
  lv_anim_init(&anim);
  lv_anim_set_var(&anim, obj);
  lv_anim_set_values(&anim, start, end);
  lv_anim_set_time(&anim, time);
  lv_anim_set_exec_cb(&anim, exec_cb);
  if (ready_cb) {
    lv_anim_set_ready_cb(&anim, ready_cb);
  }
  lv_anim_start(&anim);
}

void show_overlay(lv_obj_t *modal, lv_obj_t *window) {
  lv_obj_clear_flag(modal, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(modal);
  lv_obj_set_style_opa(modal, LV_OPA_TRANSP, 0);
  lv_obj_set_y(window, 4);
  animate_property(modal, 0, LV_OPA_COVER, 180, anim_set_opa);
  animate_property(window, 4, 0, 180, anim_set_y);
}

void hide_overlay(lv_obj_t *modal, lv_obj_t *window) {
  LV_UNUSED(window);
  animate_property(modal, LV_OPA_COVER, 0, 160, anim_set_opa, hide_anim_ready_cb);
}

lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color, const char *text,
                     lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
  lv_obj_t *label = lv_label_create(parent);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color_hex(color), 0);
  lv_obj_set_style_text_align(label, align, 0);
  lv_label_set_text(label, text);
  return label;
}

lv_obj_t *make_panel(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                     bool dark = false) {
  lv_obj_t *panel = lv_obj_create(parent);
  lv_obj_remove_style_all(panel);
  lv_obj_add_style(panel, dark ? &g.style_panel_dark : &g.style_panel, 0);
  set_rect(panel, x, y, w, h);
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
  return panel;
}

lv_obj_t *make_button(lv_obj_t *parent, const lv_style_t *style, lv_coord_t x, lv_coord_t y,
                      lv_coord_t w, lv_coord_t h, const char *icon, const char *text,
                      lv_obj_t **icon_out = nullptr, lv_obj_t **text_out = nullptr) {
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_remove_style_all(btn);
  lv_obj_add_style(btn, style, 0);
  set_rect(btn, x, y, w, h);
  lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(btn, 0, 0);

  if (text && text[0] != '\0') {
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(btn, 7, 0);

    lv_obj_t *icon_label = make_label(btn, ZERO_G_ICON_FONT, kColorText, icon);
    lv_obj_t *text_label = make_label(btn, ZERO_G_FONT_12, kColorText, text);
    lv_obj_set_style_text_font(text_label, ZERO_G_FONT_12, 0);

    if (icon_out) {
      *icon_out = icon_label;
    }
    if (text_out) {
      *text_out = text_label;
    }
  } else {
    lv_obj_t *icon_label = make_label(btn, ZERO_G_ICON_FONT, kColorText, icon);
    lv_obj_center(icon_label);
    if (icon_out) {
      *icon_out = icon_label;
    }
  }
  return btn;
}

void init_styles() {
  lv_style_init(&g.style_panel);
  lv_style_set_bg_color(&g.style_panel, color_hex(kColorPanel));
  lv_style_set_bg_opa(&g.style_panel, LV_OPA_COVER);
  lv_style_set_radius(&g.style_panel, 18);
  lv_style_set_border_width(&g.style_panel, 0);
  lv_style_set_pad_all(&g.style_panel, 0);
  lv_style_set_shadow_width(&g.style_panel, 0);

  lv_style_init(&g.style_panel_dark);
  lv_style_set_bg_color(&g.style_panel_dark, color_hex(kColorPanelDark));
  lv_style_set_bg_opa(&g.style_panel_dark, LV_OPA_COVER);
  lv_style_set_radius(&g.style_panel_dark, 15);
  lv_style_set_border_width(&g.style_panel_dark, 0);
  lv_style_set_pad_all(&g.style_panel_dark, 0);
  lv_style_set_shadow_width(&g.style_panel_dark, 0);

  lv_style_init(&g.style_button_red);
  lv_style_set_bg_color(&g.style_button_red, color_hex(kColorRed));
  lv_style_set_bg_opa(&g.style_button_red, LV_OPA_COVER);
  lv_style_set_radius(&g.style_button_red, 14);
  lv_style_set_border_width(&g.style_button_red, 0);
  lv_style_set_text_color(&g.style_button_red, color_hex(kColorText));
  lv_style_set_text_font(&g.style_button_red, ZERO_G_FONT_12);
  lv_style_set_pad_all(&g.style_button_red, 0);
  lv_style_set_shadow_width(&g.style_button_red, 0);

  lv_style_init(&g.style_button_dark);
  lv_style_set_bg_color(&g.style_button_dark, color_hex(kColorButtonDark));
  lv_style_set_bg_opa(&g.style_button_dark, LV_OPA_COVER);
  lv_style_set_radius(&g.style_button_dark, 14);
  lv_style_set_border_width(&g.style_button_dark, 0);
  lv_style_set_text_color(&g.style_button_dark, lv_color_hex(0xe6e6e6));
  lv_style_set_text_font(&g.style_button_dark, ZERO_G_FONT_12);
  lv_style_set_pad_all(&g.style_button_dark, 0);
  lv_style_set_shadow_width(&g.style_button_dark, 0);

  lv_style_init(&g.style_icon_button);
  lv_style_set_bg_color(&g.style_icon_button, color_hex(kColorButtonDark));
  lv_style_set_bg_opa(&g.style_icon_button, LV_OPA_COVER);
  lv_style_set_radius(&g.style_icon_button, 14);
  lv_style_set_border_width(&g.style_icon_button, 0);
  lv_style_set_text_color(&g.style_icon_button, color_hex(kColorText));
  lv_style_set_text_font(&g.style_icon_button, ZERO_G_ICON_FONT);
  lv_style_set_pad_all(&g.style_icon_button, 0);
  lv_style_set_shadow_width(&g.style_icon_button, 0);

  lv_style_init(&g.style_status);
  lv_style_set_bg_color(&g.style_status, lv_color_hex(0xffffff));
  lv_style_set_bg_opa(&g.style_status, LV_OPA_20);
  lv_style_set_radius(&g.style_status, LV_RADIUS_CIRCLE);
  lv_style_set_border_width(&g.style_status, 0);
  lv_style_set_pad_left(&g.style_status, 9);
  lv_style_set_pad_right(&g.style_status, 9);
  lv_style_set_pad_top(&g.style_status, 0);
  lv_style_set_pad_bottom(&g.style_status, 0);
  lv_style_set_text_color(&g.style_status, color_hex(kColorMutedLight));
  lv_style_set_text_font(&g.style_status, ZERO_G_FONT_10);

  lv_style_init(&g.style_slider_main);
  lv_style_set_bg_color(&g.style_slider_main, color_hex(kColorTrack));
  lv_style_set_bg_opa(&g.style_slider_main, LV_OPA_COVER);
  lv_style_set_radius(&g.style_slider_main, LV_RADIUS_CIRCLE);
  lv_style_set_pad_all(&g.style_slider_main, 0);

  lv_style_init(&g.style_slider_indicator);
  lv_style_set_bg_color(&g.style_slider_indicator, color_hex(kColorRed));
  lv_style_set_bg_opa(&g.style_slider_indicator, LV_OPA_COVER);
  lv_style_set_radius(&g.style_slider_indicator, LV_RADIUS_CIRCLE);

  lv_style_init(&g.style_slider_knob);
  lv_style_set_bg_color(&g.style_slider_knob, color_hex(kColorText));
  lv_style_set_bg_opa(&g.style_slider_knob, LV_OPA_COVER);
  lv_style_set_radius(&g.style_slider_knob, LV_RADIUS_CIRCLE);
  lv_style_set_border_width(&g.style_slider_knob, 6);
  lv_style_set_border_color(&g.style_slider_knob, color_hex(kColorRed));
  lv_style_set_pad_all(&g.style_slider_knob, 0);

  lv_style_init(&g.style_text_field);
  lv_style_set_bg_color(&g.style_text_field, color_hex(kColorButtonDark));
  lv_style_set_bg_opa(&g.style_text_field, LV_OPA_COVER);
  lv_style_set_radius(&g.style_text_field, 11);
  lv_style_set_border_width(&g.style_text_field, 0);
  lv_style_set_text_color(&g.style_text_field, color_hex(kColorText));
  lv_style_set_text_font(&g.style_text_field, ZERO_G_FONT_12);
  lv_style_set_pad_all(&g.style_text_field, 0);

  lv_style_init(&g.style_tile);
  lv_style_set_bg_color(&g.style_tile, color_hex(kColorPanel));
  lv_style_set_bg_opa(&g.style_tile, LV_OPA_COVER);
  lv_style_set_radius(&g.style_tile, 17);
  lv_style_set_border_width(&g.style_tile, 0);
  lv_style_set_pad_all(&g.style_tile, 0);

  lv_style_init(&g.style_overlay);
  lv_style_set_bg_color(&g.style_overlay, lv_color_hex(0x000000));
  lv_style_set_bg_opa(&g.style_overlay, LV_OPA_70);
  lv_style_set_border_width(&g.style_overlay, 0);
  lv_style_set_radius(&g.style_overlay, 0);

  lv_style_init(&g.style_overlay_window);
  lv_style_set_bg_color(&g.style_overlay_window, lv_color_hex(0x090909));
  lv_style_set_bg_opa(&g.style_overlay_window, LV_OPA_COVER);
  lv_style_set_border_width(&g.style_overlay_window, 0);
  lv_style_set_radius(&g.style_overlay_window, 0);
  lv_style_set_pad_all(&g.style_overlay_window, 0);

  lv_style_init(&g.style_printer_line);
  lv_style_set_line_color(&g.style_printer_line, lv_color_hex(0xededed));
  lv_style_set_line_opa(&g.style_printer_line, LV_OPA_90);
  lv_style_set_line_width(&g.style_printer_line, 4);
  lv_style_set_line_rounded(&g.style_printer_line, true);
}

lv_obj_t *make_top_chip(lv_obj_t *parent, const char *icon, uint32_t icon_color, lv_obj_t **value_out) {
  lv_obj_t *chip = lv_obj_create(parent);
  lv_obj_remove_style_all(chip);
  lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(chip, 0, 0);
  lv_obj_set_style_pad_column(chip, 4, 0);
  lv_obj_set_style_bg_opa(chip, LV_OPA_TRANSP, 0);
  lv_obj_set_height(chip, LV_SIZE_CONTENT);

  make_label(chip, ZERO_G_FONT_12, icon_color, icon);
  lv_obj_t *value = make_label(chip, ZERO_G_FONT_12, kColorMuted, "");
  lv_obj_set_style_text_font(value, ZERO_G_FONT_12, 0);
  lv_obj_set_style_text_color(value, color_hex(kColorMuted), 0);
  lv_label_set_recolor(value, true);

  if (value_out) {
    *value_out = value;
  }
  return chip;
}

void update_topbar_layout() {
  const bool on_home = (g.active_screen == SCREEN_HOME);
  lv_obj_update_layout(g.logo_label);
  lv_obj_update_layout(g.title_label);

  if (on_home) {
    lv_obj_clear_flag(g.logo_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.title_label, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(g.logo_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g.title_label, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(g.title_label, kScreenTitles[g.active_screen]);
  }

  lv_obj_t *anchor = on_home ? g.logo_label : g.title_label;
  lv_coord_t anchor_w = lv_obj_get_width(anchor);
  lv_obj_set_pos(g.status_pill, anchor_w + 7, 5);
}

void update_fan_summary() {
  int active = 0;
  for (const FanIcon &fan : g.fans) {
    if (fan.speed > 0U) {
      active++;
    }
  }
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d active", active);
  lv_label_set_text(g.fan_summary_label, buffer);
}

SliderBinding *bind_slider(lv_obj_t *slider, lv_obj_t *output, SliderFormat format,
                           const char *unit, int8_t fan_index = -1) {
  if (g.slider_count >= sizeof(g.sliders) / sizeof(g.sliders[0])) {
    return nullptr;
  }
  SliderBinding *binding = &g.sliders[g.slider_count++];
  binding->slider = slider;
  binding->output = output;
  binding->format = format;
  binding->unit = unit;
  binding->fan_index = fan_index;
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, binding);
  return binding;
}

TextFieldMeta *make_field_meta(bool secret, const char *placeholder) {
  if (g.field_meta_count >= sizeof(g.field_meta) / sizeof(g.field_meta[0])) {
    return nullptr;
  }
  TextFieldMeta *meta = &g.field_meta[g.field_meta_count++];
  meta->secret = secret;
  meta->placeholder = placeholder;
  return meta;
}

void refresh_slider_output(const SliderBinding *binding) {
  if (!binding || !binding->slider || !binding->output) {
    return;
  }

  char buffer[24];
  const int32_t value = lv_slider_get_value(binding->slider);

  switch (binding->format) {
    case SLIDER_FMT_PERCENT:
      snprintf(buffer, sizeof(buffer), "%ld%%", static_cast<long>(value));
      break;
    case SLIDER_FMT_MM_100:
      snprintf(buffer, sizeof(buffer), "%.2f", value / 100.0f);
      break;
    case SLIDER_FMT_PA_1000:
      snprintf(buffer, sizeof(buffer), "%.3f", value / 1000.0f);
      break;
    case SLIDER_FMT_RAW:
    default:
      if (binding->unit && binding->unit[0] != '\0') {
        snprintf(buffer, sizeof(buffer), "%ld%s", static_cast<long>(value), binding->unit);
      } else {
        snprintf(buffer, sizeof(buffer), "%ld", static_cast<long>(value));
      }
      break;
  }

  lv_label_set_text(binding->output, buffer);

  if (binding->fan_index >= 0 && binding->fan_index < 3) {
    g.fans[binding->fan_index].speed = static_cast<uint8_t>(value);
    update_fan_summary();
  }
}

lv_obj_t *make_slider_row(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w,
                          const char *icon, const char *title, int32_t min_value,
                          int32_t max_value, int32_t value, SliderFormat format,
                          const char *unit, int8_t fan_index = -1) {
  const lv_coord_t row_height = (fan_index >= 0) ? 47 : 43;
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_remove_style_all(row);
  set_rect(row, x, y, w, row_height);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);

  char title_buffer[48];
  snprintf(title_buffer, sizeof(title_buffer), "%s %s", icon, title);
  lv_obj_t *label = make_label(row, ZERO_G_FONT_10, kColorMuted, title_buffer);
  set_rect(label, 0, 0, w - 48, 13);

  lv_obj_t *output = make_label(row, ZERO_G_FONT_10, kColorText, "");
  set_rect(output, w - 52, 0, 52, 13);
  lv_obj_set_style_text_align(output, LV_TEXT_ALIGN_RIGHT, 0);

  lv_obj_t *slider = lv_slider_create(row);
  set_rect(slider, 0, 13, w, 30);
  lv_obj_add_style(slider, &g.style_slider_main, LV_PART_MAIN);
  lv_obj_add_style(slider, &g.style_slider_indicator, LV_PART_INDICATOR);
  lv_obj_add_style(slider, &g.style_slider_knob, LV_PART_KNOB);
  lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
  lv_obj_set_style_width(slider, 16, LV_PART_MAIN);
  lv_obj_set_style_width(slider, 16, LV_PART_INDICATOR);
  lv_obj_set_style_width(slider, 28, LV_PART_KNOB);
  lv_obj_set_style_height(slider, 28, LV_PART_KNOB);
  lv_obj_set_style_pad_all(slider, 0, LV_PART_MAIN);
  lv_slider_set_range(slider, min_value, max_value);
  lv_slider_set_value(slider, value, LV_ANIM_OFF);

  SliderBinding *binding = bind_slider(slider, output, format, unit, fan_index);
  refresh_slider_output(binding);
  return row;
}

void create_cube_faces(lv_obj_t *parent, lv_coord_t size, bool large) {
  lv_obj_t *cube = lv_obj_create(parent);
  lv_obj_remove_style_all(cube);
  set_rect(cube, 0, 0, size, size);
  lv_obj_center(cube);
  lv_obj_set_style_bg_opa(cube, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(cube, LV_OBJ_FLAG_SCROLLABLE);

  const lv_coord_t face = large ? 34 : 26;
  const lv_coord_t offset = large ? 18 : 14;

  lv_obj_t *top = lv_obj_create(cube);
  lv_obj_remove_style_all(top);
  set_rect(top, offset / 2, 0, face, face);
  lv_obj_set_style_bg_color(top, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_bg_opa(top, LV_OPA_40, 0);
  lv_obj_set_style_radius(top, 4, 0);

  lv_obj_t *front = lv_obj_create(cube);
  lv_obj_remove_style_all(front);
  set_rect(front, 0, offset, face, face);
  lv_obj_set_style_bg_color(front, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_bg_opa(front, LV_OPA_18, 0);
  lv_obj_set_style_radius(front, 4, 0);

  lv_obj_t *side = lv_obj_create(cube);
  lv_obj_remove_style_all(side);
  set_rect(side, offset, offset, face, face);
  lv_obj_set_style_bg_color(side, color_hex(kColorRed), 0);
  lv_obj_set_style_bg_opa(side, LV_OPA_55, 0);
  lv_obj_set_style_radius(side, 4, 0);
}

void create_glow(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t size, uint32_t color,
                 lv_opa_t opa) {
  lv_obj_t *glow = lv_obj_create(parent);
  lv_obj_remove_style_all(glow);
  set_rect(glow, x, y, size, size);
  lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(glow, color_hex(color), 0);
  lv_obj_set_style_bg_opa(glow, opa, 0);
  lv_obj_set_style_border_width(glow, 0, 0);
  lv_obj_set_style_shadow_width(glow, 0, 0);
  lv_obj_clear_flag(glow, LV_OBJ_FLAG_SCROLLABLE);
}

void update_home_visibility() {
  const bool idle = (g.printer_state == PRINTER_IDLE);
  if (idle) {
    lv_obj_clear_flag(g.home_idle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g.home_printing, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(g.home_idle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g.home_printing, LV_OBJ_FLAG_HIDDEN);
  }
}

void refresh_file_selection() {
  for (uint8_t i = 0; i < static_cast<uint8_t>(sizeof(kFiles) / sizeof(kFiles[0])); ++i) {
    const bool selected = (i == g.selected_file);
    lv_obj_set_style_bg_color(g.file_rows[i].button,
                              selected ? color_hex(kColorRed) : color_hex(kColorButtonMid), 0);
    lv_obj_set_style_text_color(g.file_rows[i].button,
                                selected ? color_hex(kColorText) : color_hex(kColorText), 0);
    lv_label_set_long_mode(g.file_rows[i].title,
                           selected ? LV_LABEL_LONG_SCROLL_CIRCULAR : LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_color(g.file_rows[i].meta,
                                selected ? lv_color_hex(0xffffff) : lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_opa(g.file_rows[i].meta,
                              selected ? LV_OPA_70 : LV_OPA_60, 0);
  }
}

void refresh_pause_button() {
  const bool paused = (g.printer_state == PRINTER_PAUSED);
  lv_label_set_text(g.pause_icon, paused ? icons::play : icons::pause);
  lv_label_set_text(g.pause_text, paused ? "Resume" : "Pause");
}

void refresh_status_visuals(const char *text, uint32_t bg, lv_opa_t bg_opa, uint32_t fg,
                            lv_coord_t width) {
  lv_label_set_text(g.status_label, text);
  animate_property(g.status_pill, lv_obj_get_width(g.status_pill), width, 260, anim_set_width);
  lv_obj_set_style_bg_color(g.status_pill, color_hex(bg), 0);
  lv_obj_set_style_bg_opa(g.status_pill, bg_opa, 0);
  lv_obj_set_style_text_color(g.status_label, color_hex(fg), 0);
}

void animate_idle_scene(uint32_t tick) {
  for (uint8_t i = 0; i < 4; ++i) {
    IdleBlob &blob = g.idle_blobs[i];
    if (!blob.obj) {
      continue;
    }
    const float t = (tick / 1000.0f) + blob.phase;
    const float dx = sinf(t * 0.32f) * static_cast<float>(blob.amp_x);
    const float dy = cosf(t * 0.28f) * static_cast<float>(blob.amp_y);
    lv_obj_set_pos(blob.obj, blob.base_x + static_cast<lv_coord_t>(dx),
                   blob.base_y + static_cast<lv_coord_t>(dy));
  }
}

void position_fan_blades(FanIcon &fan) {
  if (!fan.root) {
    return;
  }

  const int center_x = 13;
  const int center_y = 13;
  const int radius = 7;

  for (uint8_t i = 0; i < 3; ++i) {
    const float angle = (fan.angle + (i * 120)) * 3.1415926f / 180.0f;
    const int bx = center_x + static_cast<int>(cosf(angle) * radius) - 3;
    const int by = center_y + static_cast<int>(sinf(angle) * radius) - 3;
    set_rect(fan.blades[i], bx, by, 6, 6);
  }

  lv_obj_set_style_opa(fan.root, fan.speed == 0U ? LV_OPA_35 : LV_OPA_COVER, 0);
}

void animate_fans() {
  for (FanIcon &fan : g.fans) {
    if (!fan.root) {
      continue;
    }
    if (fan.speed > 0U) {
      fan.angle = static_cast<int16_t>((fan.angle + (2 + fan.speed / 10U)) % 360);
    }
    position_fan_blades(fan);
  }
}

void anim_timer_cb(lv_timer_t *timer) {
  const uint32_t tick = lv_tick_get();
  if (g.printer_state == PRINTER_IDLE) {
    animate_idle_scene(tick);
  }
  animate_fans();
  LV_UNUSED(timer);
}

void create_home_idle_card(lv_obj_t *page) {
  g.home_idle = lv_obj_create(page);
  lv_obj_remove_style_all(g.home_idle);
  set_rect(g.home_idle, 0, 0, kContentWidth, kContentHeight);
  lv_obj_set_style_bg_color(g.home_idle, lv_color_hex(0x090909), 0);
  lv_obj_set_style_bg_opa(g.home_idle, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(g.home_idle, 20, 0);
  lv_obj_set_style_border_width(g.home_idle, 0, 0);
  lv_obj_set_style_pad_all(g.home_idle, 0, 0);
  lv_obj_clear_flag(g.home_idle, LV_OBJ_FLAG_SCROLLABLE);

  const struct {
    lv_coord_t x;
    lv_coord_t y;
    lv_coord_t size;
    lv_coord_t amp_x;
    lv_coord_t amp_y;
    float phase;
    uint32_t color;
    lv_opa_t opa;
  } blob_specs[4] = {
      {-40, -20, 150, 10, 6, 0.0f, kColorRedBright, LV_OPA_45},
      {170, -18, 130, 8, 10, 1.2f, kColorRed, LV_OPA_28},
      {225, 110, 140, 14, 10, 2.3f, kColorRedBright, LV_OPA_22},
      {10, 120, 150, 8, 14, 3.0f, 0x781212, LV_OPA_42},
  };

  for (uint8_t i = 0; i < 4; ++i) {
    create_glow(g.home_idle, blob_specs[i].x, blob_specs[i].y, blob_specs[i].size,
                blob_specs[i].color, blob_specs[i].opa);
    g.idle_blobs[i].obj = lv_obj_get_child(g.home_idle, i);
    g.idle_blobs[i].base_x = blob_specs[i].x;
    g.idle_blobs[i].base_y = blob_specs[i].y;
    g.idle_blobs[i].amp_x = blob_specs[i].amp_x;
    g.idle_blobs[i].amp_y = blob_specs[i].amp_y;
    g.idle_blobs[i].phase = blob_specs[i].phase;
  }

  lv_obj_t *ready = make_label(g.home_idle, ZERO_G_FONT_READY, kColorText, "Ready");
  set_rect(ready, 18, 18, 180, 50);

  const ScreenId shortcuts[4] = {SCREEN_FILES, SCREEN_CONTROL, SCREEN_TUNING, SCREEN_SETTINGS};
  const char *shortcut_icons[4] = {icons::folders, icons::hand_stop, icons::adjustments_alt,
                                   icons::power};
  for (uint8_t i = 0; i < 4; ++i) {
    lv_obj_t *btn = make_button(g.home_idle, (i == 3) ? &g.style_button_red : &g.style_icon_button,
                                218 + (44 * i), 160, 36, 36, shortcut_icons[i], "");
    lv_obj_set_style_radius(btn, 14, 0);
    lv_obj_add_event_cb(btn, shortcut_nav_event_cb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(shortcuts[i])));
  }
}

void create_progress_card(lv_obj_t *page) {
  lv_obj_t *card = make_panel(page, 247, 0, 157, 214);
  g.layer_label = make_label(card, ZERO_G_FONT_14, kColorText, "Layer 10 / 100");
  set_rect(g.layer_label, 10, 10, 137, 20);

  g.progress_arc = lv_arc_create(card);
  set_rect(g.progress_arc, 24, 44, 108, 108);
  lv_arc_set_range(g.progress_arc, 0, 100);
  lv_arc_set_value(g.progress_arc, 55);
  lv_arc_set_bg_angles(g.progress_arc, 0, 360);
  lv_arc_set_rotation(g.progress_arc, 270);
  lv_obj_set_style_arc_width(g.progress_arc, 12, LV_PART_MAIN);
  lv_obj_set_style_arc_color(g.progress_arc, lv_color_hex(0x272727), LV_PART_MAIN);
  lv_obj_set_style_arc_width(g.progress_arc, 12, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(g.progress_arc, color_hex(kColorRed), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(g.progress_arc, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(g.progress_arc, 0, 0);
  lv_obj_set_style_pad_all(g.progress_arc, 0, 0);
  lv_obj_set_style_opa(g.progress_arc, LV_OPA_TRANSP, LV_PART_KNOB);
  lv_obj_clear_flag(g.progress_arc, LV_OBJ_FLAG_CLICKABLE);

  g.percent_label = make_label(card, ZERO_G_FONT_24, kColorText, "55%");
  set_rect(g.percent_label, 44, 82, 70, 28);
  lv_obj_set_style_text_align(g.percent_label, LV_TEXT_ALIGN_CENTER, 0);

  g.remaining_label = make_label(card, ZERO_G_FONT_10, kColorMuted, "~5m remaining",
                                 LV_TEXT_ALIGN_CENTER);
  set_rect(g.remaining_label, 16, 181, 125, 18);
}

void create_file_row(lv_obj_t *parent, uint8_t index, lv_coord_t y) {
  FileRow &row = g.file_rows[index];
  row.button = lv_btn_create(parent);
  lv_obj_remove_style_all(row.button);
  set_rect(row.button, 0, y, 173, 45);
  lv_obj_set_style_bg_color(row.button, color_hex(kColorButtonMid), 0);
  lv_obj_set_style_bg_opa(row.button, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(row.button, 13, 0);
  lv_obj_set_style_border_width(row.button, 0, 0);
  lv_obj_set_style_pad_all(row.button, 0, 0);
  lv_obj_clear_flag(row.button, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(row.button, file_select_event_cb, LV_EVENT_CLICKED,
                      reinterpret_cast<void *>(static_cast<uintptr_t>(index)));

  row.title = make_label(row.button, ZERO_G_FONT_10, kColorText, kFiles[index].name);
  set_rect(row.title, 9, 7, 155, 17);
  lv_label_set_long_mode(row.title, LV_LABEL_LONG_DOT);

  row.meta = make_label(row.button, ZERO_G_FONT_10, 0xffffff, kFiles[index].meta);
  set_rect(row.meta, 9, 25, 120, 13);
  lv_obj_set_style_text_opa(row.meta, LV_OPA_60, 0);
}

lv_obj_t *make_temp_row(lv_obj_t *parent, lv_coord_t y, const char *icon, const char *title,
                        lv_obj_t **field_out) {
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_remove_style_all(row);
  set_rect(row, 9, y, 179, 40);
  lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  char label_text[24];
  snprintf(label_text, sizeof(label_text), "%s %s", icon, title);
  lv_obj_t *title_label = make_label(row, ZERO_G_FONT_10, kColorMuted, label_text);
  set_rect(title_label, 0, 11, 63, 16);

  lv_obj_t *off_btn = make_button(row, &g.style_button_dark, 70, 3, 44, 34, "", "Off");
  lv_obj_set_style_radius(off_btn, 12, 0);
  lv_obj_add_event_cb(off_btn, temp_off_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *field_btn =
      make_button(row, &g.style_button_red, 121, 3, 58, 34, "", "Off", nullptr, nullptr);
  lv_obj_set_style_radius(field_btn, 12, 0);
  lv_obj_add_style(field_btn, &g.style_text_field, 0);
  lv_obj_set_style_bg_color(field_btn, color_hex(kColorRed), 0);
  lv_obj_add_event_cb(field_btn, temp_field_event_cb, LV_EVENT_CLICKED, nullptr);

  if (field_out) {
    *field_out = field_btn;
  }
  return row;
}

lv_obj_t *make_segment_button(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w,
                              lv_coord_t h, const char *text, bool active) {
  lv_obj_t *btn = make_button(parent, &g.style_button_dark, x, y, w, h, "", text);
  lv_obj_set_style_radius(btn, 10, 0);
  set_button_active(btn, active);
  lv_obj_add_event_cb(btn, segment_event_cb, LV_EVENT_CLICKED, nullptr);
  return btn;
}

void create_sidebar_background() {
  g.sidebar = lv_obj_create(g.root);
  lv_obj_remove_style_all(g.sidebar);
  set_rect(g.sidebar, 0, 0, kSidebarWidth, kScreenHeight);
  lv_obj_set_style_bg_color(g.sidebar, color_hex(kColorSidebar), 0);
  lv_obj_set_style_bg_opa(g.sidebar, LV_OPA_COVER, 0);
  lv_obj_clear_flag(g.sidebar, LV_OBJ_FLAG_SCROLLABLE);
}

void create_page_root(ScreenId screen_id) {
  lv_obj_t *page = lv_obj_create(g.content_host);
  lv_obj_remove_style_all(page);
  set_rect(page, 0, 0, kContentWidth, kContentHeight);
  lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
  if (screen_id != SCREEN_HOME) {
    lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);
  }
  g.screen_roots[screen_id] = page;
}

void create_keyboard_overlay();
void create_numpad_overlay();
void create_printer_diagram(lv_obj_t *parent);

}  // namespace

void create_sidebar() {
  create_sidebar_background();
  const char *icons_by_screen[SCREEN_COUNT] = {icons::home,        icons::folders,
                                               icons::hand_stop,   icons::border_corners,
                                               icons::adjustments_alt, icons::settings};

  for (uint8_t i = 0; i < SCREEN_COUNT; ++i) {
    g.nav_buttons[i] =
        make_button(g.sidebar, &g.style_icon_button, 7, 7 + (i * 42), 42, 37, icons_by_screen[i], "");
    lv_obj_set_style_bg_opa(g.nav_buttons[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(g.nav_buttons[i], lv_color_hex(0xb7b7b7), 0);
    lv_obj_add_event_cb(g.nav_buttons[i], nav_event_cb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(i)));
  }
  set_nav_button_state(SCREEN_HOME, true);
}

void create_topbar() {
  g.topbar = lv_obj_create(g.main_area);
  lv_obj_remove_style_all(g.topbar);
  set_rect(g.topbar, kMainPaddingX, kMainPaddingTop, kContentWidth, kTopbarHeight);
  lv_obj_set_style_bg_opa(g.topbar, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(g.topbar, LV_OBJ_FLAG_SCROLLABLE);

  g.logo_label = make_label(g.topbar, ZERO_G_FONT_20, kColorText, "#f7f7f7 Zero# #ff524d G#");
  lv_label_set_recolor(g.logo_label, true);
  set_rect(g.logo_label, 0, 4, 120, 24);

  g.title_label = make_label(g.topbar, ZERO_G_FONT_16, kColorText, "Home");
  set_rect(g.title_label, 0, 7, 130, 20);
  lv_obj_add_flag(g.title_label, LV_OBJ_FLAG_HIDDEN);

  g.status_pill = lv_obj_create(g.topbar);
  lv_obj_remove_style_all(g.status_pill);
  lv_obj_add_style(g.status_pill, &g.style_status, 0);
  set_rect(g.status_pill, 127, 5, 62, 22);
  lv_obj_clear_flag(g.status_pill, LV_OBJ_FLAG_SCROLLABLE);

  g.status_label = make_label(g.status_pill, ZERO_G_FONT_10, kColorMutedLight, "Printing");
  lv_obj_center(g.status_label);

  lv_obj_t *telemetry = lv_obj_create(g.topbar);
  lv_obj_remove_style_all(telemetry);
  lv_obj_set_flex_flow(telemetry, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(telemetry, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(telemetry, 0, 0);
  lv_obj_set_style_pad_column(telemetry, 7, 0);
  lv_obj_set_style_bg_opa(telemetry, LV_OPA_TRANSP, 0);
  lv_obj_set_size(telemetry, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_align(telemetry, LV_ALIGN_RIGHT_MID, 0, 0);

  g.time_label = make_label(telemetry, ZERO_G_FONT_12, 0xdddddd, "11:52");
  lv_obj_t *nozzle_chip = make_top_chip(telemetry, icons::nozzle, kColorRedBright, &g.nozzle_value);
  lv_obj_t *bed_chip = make_top_chip(telemetry, icons::bed, kColorMuted, &g.bed_value);
  LV_UNUSED(nozzle_chip);
  LV_UNUSED(bed_chip);

  update_topbar_layout();
}

void create_home_screen() {
  lv_obj_t *page = g.screen_roots[SCREEN_HOME];
  create_home_idle_card(page);

  g.home_printing = lv_obj_create(page);
  lv_obj_remove_style_all(g.home_printing);
  set_rect(g.home_printing, 0, 0, kContentWidth, kContentHeight);
  lv_obj_set_style_bg_opa(g.home_printing, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(g.home_printing, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *print_card = make_panel(g.home_printing, 0, 0, 237, 214);
  g.home_file_name = make_label(print_card, ZERO_G_FONT_16, kColorText, "Voron_Cube_0.2mm.gcode");
  set_rect(g.home_file_name, 10, 10, 217, 25);
  lv_label_set_long_mode(g.home_file_name, LV_LABEL_LONG_SCROLL_CIRCULAR);

  lv_obj_t *model_box = make_panel(print_card, 10, 48, 217, 106, true);
  create_glow(model_box, 130, -12, 96, kColorRed, LV_OPA_22);
  create_cube_faces(model_box, 54, false);

  g.pause_btn = make_button(print_card, &g.style_button_red, 10, 162, 104, 42, icons::pause,
                            "Pause", &g.pause_icon, &g.pause_text);
  lv_obj_add_event_cb(g.pause_btn, pause_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *cancel_btn =
      make_button(print_card, &g.style_button_dark, 122, 162, 105, 42, icons::cancel, "Cancel");
  lv_obj_add_event_cb(cancel_btn, cancel_event_cb, LV_EVENT_CLICKED, nullptr);

  create_progress_card(g.home_printing);
}

void create_files_screen() {
  lv_obj_t *page = g.screen_roots[SCREEN_FILES];

  lv_obj_t *file_list = make_panel(page, 0, 0, 189, 214);
  lv_obj_set_style_pad_all(file_list, 8, 0);
  lv_obj_set_scrollbar_mode(file_list, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(file_list, LV_DIR_VER);

  lv_obj_t *list_inner = lv_obj_create(file_list);
  lv_obj_remove_style_all(list_inner);
  set_rect(list_inner, 8, 8, 173, 316);
  lv_obj_set_style_bg_opa(list_inner, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(list_inner, LV_OBJ_FLAG_SCROLLABLE);

  for (uint8_t i = 0; i < static_cast<uint8_t>(sizeof(kFiles) / sizeof(kFiles[0])); ++i) {
    create_file_row(list_inner, i, i * 51);
  }

  lv_obj_t *preview_card = make_panel(page, 199, 0, 205, 214);
  g.files_preview = make_panel(preview_card, 9, 9, 187, 146, true);
  create_glow(g.files_preview, 110, -16, 110, kColorRed, LV_OPA_20);
  create_cube_faces(g.files_preview, 72, true);

  g.start_print_btn = make_button(preview_card, &g.style_button_red, 9, 163, 89, 42, icons::print,
                                  "Print");
  lv_obj_add_event_cb(g.start_print_btn, start_print_event_cb, LV_EVENT_CLICKED, nullptr);
  lv_obj_set_style_bg_color(g.start_print_btn, lv_color_hex(0x343434), LV_STATE_DISABLED);
  lv_obj_set_style_text_color(g.start_print_btn, lv_color_hex(0x8a8a8a), LV_STATE_DISABLED);

  lv_obj_t *delete_btn = make_button(preview_card, &g.style_button_dark, 106, 163, 90, 42,
                                     icons::delete_file, "Delete");
  LV_UNUSED(delete_btn);

  refresh_file_selection();
}

void create_mesh_screen() {
  lv_obj_t *page = g.screen_roots[SCREEN_MESH];

  lv_obj_t *left = make_panel(page, 0, 0, 192, 214);
  make_label(left, ZERO_G_FONT_10, kColorMuted, "Deviation");
  lv_obj_t *value = make_label(left, ZERO_G_FONT_20, kColorText, "0.18 mm");
  set_rect(value, 12, 26, 100, 22);

  lv_obj_t *visual = make_panel(left, 12, 60, 168, 142, true);
  create_glow(visual, -14, -16, 90, kColorRed, LV_OPA_24);
  create_glow(visual, 104, 84, 78, 0xffffff, LV_OPA_12);

  const uint8_t hot_indices[] = {4, 8, 12, 18, 22};
  const uint8_t warm_indices[] = {3, 7, 13, 16, 21};
  for (uint8_t i = 0; i < 25; ++i) {
    lv_obj_t *dot = lv_obj_create(visual);
    lv_obj_remove_style_all(dot);
    const lv_coord_t col = i % 5;
    const lv_coord_t row = i / 5;
    set_rect(dot, 26 + (col * 25), 16 + (row * 25), 16, 16);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    uint32_t color = 0x555555;
    for (uint8_t hot : hot_indices) {
      if (hot == i) {
        color = kColorRed;
      }
    }
    for (uint8_t warm : warm_indices) {
      if (warm == i) {
        color = kColorYellow;
      }
    }
    lv_obj_set_style_bg_color(dot, color_hex(color), 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
  }

  lv_obj_t *right = make_panel(page, 202, 0, 202, 214);
  make_label(right, ZERO_G_FONT_10, kColorMuted, "Bed temperature");
  g.mesh_temp_label = make_label(right, ZERO_G_FONT_READY, kColorText, "60C");
  set_rect(g.mesh_temp_label, 12, 34, 110, 48);
  g.mesh_profile_label = make_label(right, ZERO_G_FONT_10, kColorMuted, "Profile: textured_pc");
  set_rect(g.mesh_profile_label, 12, 90, 150, 14);

  make_button(right, &g.style_button_red, 12, 160, 84, 42, icons::calibrate, "Calibrate");
  make_button(right, &g.style_button_dark, 106, 160, 84, 42, icons::load, "Load");
}

void create_control_screen() {
  lv_obj_t *page = g.screen_roots[SCREEN_CONTROL];

  lv_obj_t *left = make_panel(page, 0, 0, 197, 214);
  make_temp_row(left, 9, icons::nozzle, "Nozzle", &g.temp_nozzle_field);
  make_temp_row(left, 57, icons::bed, "Bed", &g.temp_bed_field);

  lv_obj_t *extrusion = lv_obj_create(left);
  lv_obj_remove_style_all(extrusion);
  set_rect(extrusion, 9, 105, 179, 44);
  lv_obj_set_style_bg_opa(extrusion, LV_OPA_TRANSP, 0);
  make_label(extrusion, ZERO_G_FONT_10, kColorMuted, "Extrusion");
  make_button(extrusion, &g.style_button_dark, 0, 14, 64, 30, icons::retract, "");
  g.extrusion_amount_label = make_label(extrusion, ZERO_G_FONT_10, kColorMuted, "5 mm",
                                        LV_TEXT_ALIGN_CENTER);
  set_rect(g.extrusion_amount_label, 70, 20, 38, 12);
  make_button(extrusion, &g.style_button_red, 115, 14, 64, 30, icons::extrude, "");

  lv_obj_t *bottom = lv_obj_create(left);
  lv_obj_remove_style_all(bottom);
  set_rect(bottom, 9, 157, 179, 36);
  lv_obj_set_style_bg_opa(bottom, LV_OPA_TRANSP, 0);

  for (uint8_t i = 0; i < 4; ++i) {
    g.extrude_step_buttons[i] = make_segment_button(bottom, i * 33, 0, 30, 36,
                                                    (i == 0) ? "1" : (i == 1) ? "5" : (i == 2) ? "10" : "25",
                                                    i == 1);
    lv_obj_add_event_cb(g.extrude_step_buttons[i], extrude_step_event_cb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(kExtrusionSteps[i])));
  }
  make_button(bottom, &g.style_icon_button, 135, 0, 44, 36, icons::motor_off, "");

  lv_obj_t *right = make_panel(page, 207, 0, 197, 214);
  for (uint8_t i = 0; i < 4; ++i) {
    g.jog_step_buttons[i] = make_segment_button(right, 9 + (46 * i), 9, 41, 30, kJogSteps[i], i == 0);
  }

  const lv_coord_t start_x = 21;
  const lv_coord_t start_y = 58;
  const lv_coord_t w = 48;
  const lv_coord_t h = 38;
  const lv_coord_t gap = 6;

  make_button(right, &g.style_icon_button, start_x + w + gap, start_y, w, h, icons::up, "");
  make_button(right, &g.style_icon_button, start_x, start_y + h + gap, w, h, icons::left, "");
  lv_obj_t *home_axis = make_button(right, &g.style_button_red, start_x + w + gap,
                                    start_y + h + gap, w, h, icons::home_axis, "");
  LV_UNUSED(home_axis);
  make_button(right, &g.style_icon_button, start_x + ((w + gap) * 2), start_y + h + gap, w, h,
              icons::right, "");
  make_button(right, &g.style_icon_button, start_x, start_y + ((h + gap) * 2), w, h, "", "Z-");
  make_button(right, &g.style_icon_button, start_x + w + gap, start_y + ((h + gap) * 2), w, h,
              icons::down, "");
  make_button(right, &g.style_icon_button, start_x + ((w + gap) * 2),
              start_y + ((h + gap) * 2), w, h, "", "Z+");
}

void create_tuning_screen() {
  lv_obj_t *page = g.screen_roots[SCREEN_TUNING];

  g.tuning_main = lv_obj_create(page);
  lv_obj_remove_style_all(g.tuning_main);
  set_rect(g.tuning_main, 0, 0, kContentWidth, kContentHeight);
  lv_obj_set_style_bg_opa(g.tuning_main, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(g.tuning_main, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *left = make_panel(g.tuning_main, 0, 0, 197, 214);
  make_slider_row(left, 9, 9, 179, icons::gauge, "Speed", 30, 180, 100, SLIDER_FMT_PERCENT, "%");
  make_slider_row(left, 9, 58, 179, icons::flow, "Flow", 80, 120, 98, SLIDER_FMT_PERCENT, "%");
  make_slider_row(left, 9, 107, 179, icons::z_axis, "Z Offset", -40, 40, 0, SLIDER_FMT_MM_100, "");

  lv_obj_t *fans_btn = lv_btn_create(left);
  lv_obj_remove_style_all(fans_btn);
  set_rect(fans_btn, 9, 156, 179, 40);
  lv_obj_set_style_bg_color(fans_btn, color_hex(kColorButtonMid), 0);
  lv_obj_set_style_bg_opa(fans_btn, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(fans_btn, 14, 0);
  lv_obj_set_style_border_width(fans_btn, 0, 0);
  lv_obj_set_style_pad_all(fans_btn, 0, 0);
  lv_obj_clear_flag(fans_btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(fans_btn, open_fans_event_cb, LV_EVENT_CLICKED, nullptr);

  make_label(fans_btn, ZERO_G_FONT_12, kColorText, "");
  lv_obj_t *fans_text = make_label(fans_btn, ZERO_G_FONT_12, kColorText, "");
  lv_label_set_text_fmt(fans_text, "%s Fans", icons::fans);
  set_rect(fans_text, 12, 12, 80, 16);
  g.fan_summary_label = make_label(fans_btn, ZERO_G_FONT_10, kColorMuted, "2 active");
  set_rect(g.fan_summary_label, 112, 14, 48, 12);
  lv_obj_t *chevron = make_label(fans_btn, ZERO_G_FONT_16, kColorRedBright, ">");
  set_rect(chevron, 160, 10, 12, 18);

  lv_obj_t *right = make_panel(g.tuning_main, 207, 0, 197, 214);
  make_slider_row(right, 9, 9, 179, icons::pressure, "Pressure Adv.", 0, 120, 45,
                  SLIDER_FMT_PA_1000, "");
  make_slider_row(right, 9, 58, 179, icons::accel, "Acceleration", 500, 12000, 5000,
                  SLIDER_FMT_RAW, "");
  make_slider_row(right, 9, 107, 179, icons::corner, "Corner Vel.", 1, 20, 5, SLIDER_FMT_RAW, "");

  g.fan_subpage = lv_obj_create(page);
  lv_obj_remove_style_all(g.fan_subpage);
  set_rect(g.fan_subpage, 0, 0, kContentWidth, kContentHeight);
  lv_obj_set_style_bg_opa(g.fan_subpage, LV_OPA_TRANSP, 0);
  lv_obj_add_flag(g.fan_subpage, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(g.fan_subpage, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *fan_list = make_panel(g.fan_subpage, 0, 0, 197, 214);
  lv_obj_t *back_btn = make_button(fan_list, &g.style_button_red, 9, 9, 31, 31, icons::back, "");
  lv_obj_set_style_radius(back_btn, 12, 0);
  lv_obj_add_event_cb(back_btn, back_fans_event_cb, LV_EVENT_CLICKED, nullptr);
  lv_obj_t *fan_title = make_label(fan_list, ZERO_G_FONT_14, kColorText, "Fans");
  set_rect(fan_title, 48, 13, 80, 18);

  make_slider_row(fan_list, 9, 44, 179, icons::fans, "Hotend", 0, 100, 70, SLIDER_FMT_PERCENT,
                  "%", 0);
  make_slider_row(fan_list, 9, 98, 179, icons::fans, "Part Cooling", 0, 100, 45,
                  SLIDER_FMT_PERCENT, "%", 1);
  make_slider_row(fan_list, 9, 152, 179, icons::fans, "Chamber Filter", 0, 100, 0,
                  SLIDER_FMT_PERCENT, "%", 2);

  lv_obj_t *diagram_panel = make_panel(g.fan_subpage, 207, 0, 197, 214);
  create_printer_diagram(diagram_panel);
  update_fan_summary();
}

static lv_obj_t *make_settings_header(lv_obj_t *page, const char *title) {
  lv_obj_t *header = lv_obj_create(page);
  lv_obj_remove_style_all(header);
  set_rect(header, 0, 0, kContentWidth, 31);
  lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *back = make_button(header, &g.style_button_red, 0, 0, 31, 31, icons::back, "");
  lv_obj_set_style_radius(back, 12, 0);
  lv_obj_add_event_cb(back, settings_back_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *label = make_label(header, ZERO_G_FONT_16, kColorText, title);
  set_rect(label, 40, 7, 120, 18);
  return header;
}

lv_obj_t *make_field_card(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, const char *label,
                          const char *value, const char *placeholder, bool secret) {
  lv_obj_t *card = make_panel(parent, x, y, w, 51);
  lv_obj_set_style_radius(card, 15, 0);
  lv_obj_t *caption = make_label(card, ZERO_G_FONT_10, kColorMuted, label);
  set_rect(caption, 9, 8, w - 18, 14);

  lv_obj_t *field = make_button(card, &g.style_text_field, 9, 23, w - 18, 19, "", value);
  lv_obj_set_style_text_align(field, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_style_pad_left(field, 10, 0);
  lv_obj_set_style_radius(field, 11, 0);
  TextFieldMeta *meta = make_field_meta(secret, placeholder);
  lv_obj_add_event_cb(field, text_field_event_cb, LV_EVENT_CLICKED, meta);
  return card;
}

lv_obj_t *make_wide_row(lv_obj_t *parent, lv_coord_t y) {
  lv_obj_t *row = make_panel(parent, 0, y, 404, 44);
  lv_obj_set_style_radius(row, 15, 0);
  return row;
}

void create_settings_home_page(lv_obj_t *page) {
  struct TileSpec {
    SettingsPageId page_id;
    const char *icon;
    const char *label;
    bool danger;
  };

  const TileSpec tiles[6] = {
      {SETTINGS_PAGE_NETWORK, icons::wifi, "Network", false},
      {SETTINGS_PAGE_SCREEN, icons::screen, "Screen", false},
      {SETTINGS_PAGE_KLIPPER, icons::klipper, "Klipper", false},
      {SETTINGS_PAGE_AUDIO, icons::audio, "Audio", false},
      {SETTINGS_PAGE_CONSOLE, icons::console, "Console", false},
      {SETTINGS_PAGE_POWER, icons::power, "Power", true},
  };

  for (uint8_t i = 0; i < 6; ++i) {
    const lv_coord_t col = i % 3;
    const lv_coord_t row = i / 3;
    lv_obj_t *tile = lv_btn_create(page);
    lv_obj_remove_style_all(tile);
    set_rect(tile, col * 137, row * 111, 129, 103);
    lv_obj_add_style(tile, &g.style_tile, 0);
    if (tiles[i].danger) {
      lv_obj_set_style_bg_color(tile, lv_color_hex(0xe53935), 0);
      lv_obj_set_style_bg_opa(tile, LV_OPA_90, 0);
    }
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(tile, settings_tile_event_cb, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(tiles[i].page_id)));

    lv_obj_t *icon = make_label(tile, ZERO_G_FONT_24, tiles[i].danger ? kColorText : kColorRedBright,
                                tiles[i].icon);
    set_rect(icon, 12, 16, 40, 28);
    lv_obj_t *label = make_label(tile, ZERO_G_FONT_14, kColorText, tiles[i].label);
    set_rect(label, 12, 72, 90, 18);
  }
}

void create_settings_screen_page(lv_obj_t *page) {
  make_settings_header(page, "Screen");

  lv_obj_t *brightness = make_wide_row(page, 39);
  lv_obj_t *brightness_title = make_label(brightness, ZERO_G_FONT_10, kColorMuted, "");
  lv_label_set_text_fmt(brightness_title, "%s Brightness", icons::brightness);
  set_rect(brightness_title, 10, 14, 90, 14);
  lv_obj_t *brightness_out = make_label(brightness, ZERO_G_FONT_12, kColorText, "82%");
  set_rect(brightness_out, 356, 13, 34, 16);
  lv_obj_t *brightness_slider = lv_slider_create(brightness);
  set_rect(brightness_slider, 112, 7, 236, 30);
  lv_obj_add_style(brightness_slider, &g.style_slider_main, LV_PART_MAIN);
  lv_obj_add_style(brightness_slider, &g.style_slider_indicator, LV_PART_INDICATOR);
  lv_obj_add_style(brightness_slider, &g.style_slider_knob, LV_PART_KNOB);
  lv_obj_set_style_width(brightness_slider, 16, LV_PART_MAIN);
  lv_obj_set_style_width(brightness_slider, 16, LV_PART_INDICATOR);
  lv_obj_set_style_width(brightness_slider, 28, LV_PART_KNOB);
  lv_obj_set_style_height(brightness_slider, 28, LV_PART_KNOB);
  lv_slider_set_range(brightness_slider, 10, 100);
  lv_slider_set_value(brightness_slider, 82, LV_ANIM_OFF);
  refresh_slider_output(bind_slider(brightness_slider, brightness_out, SLIDER_FMT_PERCENT, "%"));

  lv_obj_t *sleep = make_wide_row(page, 91);
  lv_obj_t *sleep_title = make_label(sleep, ZERO_G_FONT_10, kColorMuted, "");
  lv_label_set_text_fmt(sleep_title, "%s Sleep", icons::sleep);
  set_rect(sleep_title, 10, 14, 90, 14);
  make_segment_button(sleep, 112, 7, 66, 30, "5m", true);
  make_segment_button(sleep, 186, 7, 66, 30, "15m", false);
  make_segment_button(sleep, 260, 7, 76, 30, "Never", false);

  lv_obj_t *theme = make_wide_row(page, 143);
  lv_obj_t *theme_title = make_label(theme, ZERO_G_FONT_10, kColorMuted, "");
  lv_label_set_text_fmt(theme_title, "%s Theme", icons::theme);
  set_rect(theme_title, 10, 14, 90, 14);
  make_segment_button(theme, 112, 7, 62, 30, "Dark", true);
  make_segment_button(theme, 182, 7, 58, 30, "Dim", false);
  make_segment_button(theme, 248, 7, 58, 30, "Red", false);
}

void create_settings_audio_page(lv_obj_t *page) {
  make_settings_header(page, "Audio");

  lv_obj_t *volume = make_wide_row(page, 39);
  lv_obj_t *volume_title = make_label(volume, ZERO_G_FONT_10, kColorMuted, "");
  lv_label_set_text_fmt(volume_title, "%s Volume", icons::audio);
  set_rect(volume_title, 10, 14, 90, 14);
  lv_obj_t *volume_out = make_label(volume, ZERO_G_FONT_12, kColorText, "55%");
  set_rect(volume_out, 356, 13, 34, 16);
  lv_obj_t *volume_slider = lv_slider_create(volume);
  set_rect(volume_slider, 112, 7, 236, 30);
  lv_obj_add_style(volume_slider, &g.style_slider_main, LV_PART_MAIN);
  lv_obj_add_style(volume_slider, &g.style_slider_indicator, LV_PART_INDICATOR);
  lv_obj_add_style(volume_slider, &g.style_slider_knob, LV_PART_KNOB);
  lv_obj_set_style_width(volume_slider, 16, LV_PART_MAIN);
  lv_obj_set_style_width(volume_slider, 16, LV_PART_INDICATOR);
  lv_obj_set_style_width(volume_slider, 28, LV_PART_KNOB);
  lv_obj_set_style_height(volume_slider, 28, LV_PART_KNOB);
  lv_slider_set_range(volume_slider, 0, 100);
  lv_slider_set_value(volume_slider, 55, LV_ANIM_OFF);
  refresh_slider_output(bind_slider(volume_slider, volume_out, SLIDER_FMT_PERCENT, "%"));

  lv_obj_t *beeps = make_wide_row(page, 91);
  lv_obj_t *beeps_title = make_label(beeps, ZERO_G_FONT_10, kColorMuted, "");
  lv_label_set_text_fmt(beeps_title, "%s Touch Beeps", icons::touch_beeps);
  set_rect(beeps_title, 10, 14, 110, 14);
  make_segment_button(beeps, 200, 7, 56, 30, "On", true);
  make_segment_button(beeps, 264, 7, 56, 30, "Off", false);

  lv_obj_t *alerts = make_wide_row(page, 143);
  lv_obj_t *alerts_title = make_label(alerts, ZERO_G_FONT_10, kColorMuted, "");
  lv_label_set_text_fmt(alerts_title, "%s Alerts", icons::alerts);
  set_rect(alerts_title, 10, 14, 100, 14);
  make_segment_button(alerts, 170, 7, 58, 30, "Chime", true);
  make_segment_button(alerts, 236, 7, 50, 30, "Tone", false);
  make_segment_button(alerts, 294, 7, 62, 30, "Silent", false);
}

void create_settings_screen() {
  lv_obj_t *page = g.screen_roots[SCREEN_SETTINGS];

  for (uint8_t i = 0; i < SETTINGS_PAGE_COUNT; ++i) {
    g.settings_pages[i] = lv_obj_create(page);
    lv_obj_remove_style_all(g.settings_pages[i]);
    set_rect(g.settings_pages[i], 0, 0, kContentWidth, kContentHeight);
    lv_obj_set_style_bg_opa(g.settings_pages[i], LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g.settings_pages[i], LV_OBJ_FLAG_SCROLLABLE);
    if (i != SETTINGS_PAGE_HOME) {
      lv_obj_add_flag(g.settings_pages[i], LV_OBJ_FLAG_HIDDEN);
    }
  }

  create_settings_home_page(g.settings_pages[SETTINGS_PAGE_HOME]);

  lv_obj_t *network = g.settings_pages[SETTINGS_PAGE_NETWORK];
  make_settings_header(network, "Network");
  make_field_card(network, 0, 39, 223, "Wi-Fi SSID", "TarmacNet", "Wi-Fi SSID", false);
  make_field_card(network, 231, 39, 173, "Password", kSecretMask, "Optional", true);
  make_field_card(network, 0, 98, 223, "Hostname", "mercury-one", "Hostname", false);

  create_settings_screen_page(g.settings_pages[SETTINGS_PAGE_SCREEN]);

  lv_obj_t *klipper = g.settings_pages[SETTINGS_PAGE_KLIPPER];
  make_settings_header(klipper, "Klipper");
  make_field_card(klipper, 0, 39, 223, "Moonraker IP", "192.168.1.42", "Moonraker IP", false);
  make_field_card(klipper, 231, 39, 173, "Port", "7125", "7125", false);
  make_field_card(klipper, 0, 98, 223, "API Key", "Optional", "Optional", true);
  lv_obj_t *connect = lv_btn_create(klipper);
  lv_obj_remove_style_all(connect);
  set_rect(connect, 0, 157, 404, 43);
  lv_obj_set_style_bg_color(connect, color_hex(kColorButtonMid), 0);
  lv_obj_set_style_bg_opa(connect, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(connect, 15, 0);
  lv_obj_set_style_border_width(connect, 0, 0);
  lv_obj_set_style_pad_all(connect, 0, 0);
  make_label(connect, ZERO_G_FONT_20, kColorGreen, icons::connect);
  lv_obj_set_pos(lv_obj_get_child(connect, 0), 12, 10);
  lv_obj_t *connect_text = make_label(connect, ZERO_G_FONT_12, kColorText, "Test Connection");
  set_rect(connect_text, 48, 13, 120, 16);
  lv_obj_t *connect_state = make_label(connect, ZERO_G_FONT_10, kColorMuted, "Ready");
  set_rect(connect_state, 350, 15, 40, 12);

  create_settings_audio_page(g.settings_pages[SETTINGS_PAGE_AUDIO]);

  lv_obj_t *console = g.settings_pages[SETTINGS_PAGE_CONSOLE];
  make_settings_header(console, "Console");
  lv_obj_t *console_box = make_panel(console, 0, 39, 404, 126, true);
  lv_obj_t *console_text = make_label(console_box, ZERO_G_FONT_12, 0xd8d8d8,
                                      "status: connected\nmcu: ready\nmoonraker: reachable");
  set_rect(console_text, 12, 12, 220, 60);
  lv_obj_t *command_btn =
      make_button(console, &g.style_text_field, 0, 173, 404, 42, icons::file_command, "Enter command");
  lv_obj_set_style_radius(command_btn, 15, 0);
  TextFieldMeta *console_meta = make_field_meta(false, "Enter command");
  lv_obj_add_event_cb(command_btn, text_field_event_cb, LV_EVENT_CLICKED, console_meta);

  lv_obj_t *power = g.settings_pages[SETTINGS_PAGE_POWER];
  make_settings_header(power, "Power");

  const struct {
    lv_coord_t x;
    lv_coord_t y;
    const char *icon;
    const char *label;
    bool danger;
  } power_buttons[4] = {
      {0, 39, icons::restart, "Restart Klipper", false},
      {206, 39, icons::restart_mcu, "Restart MCU", false},
      {0, 127, icons::restart_host, "Restart Host", false},
      {206, 127, icons::shutdown, "Shutdown", true},
  };

  for (uint8_t i = 0; i < 4; ++i) {
    lv_obj_t *btn = lv_btn_create(power);
    lv_obj_remove_style_all(btn);
    set_rect(btn, power_buttons[i].x, power_buttons[i].y, 198, 80);
    lv_obj_add_style(btn, &g.style_tile, 0);
    if (power_buttons[i].danger) {
      lv_obj_set_style_bg_color(btn, lv_color_hex(0xe53935), 0);
      lv_obj_set_style_bg_opa(btn, LV_OPA_90, 0);
    }
    lv_obj_set_style_radius(btn, 17, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    make_label(btn, ZERO_G_FONT_24, power_buttons[i].danger ? kColorText : kColorRedBright,
               power_buttons[i].icon);
    lv_obj_set_pos(lv_obj_get_child(btn, 0), 84, 14);
    lv_obj_t *label = make_label(btn, ZERO_G_FONT_12, kColorText, power_buttons[i].label,
                                 LV_TEXT_ALIGN_CENTER);
    set_rect(label, 18, 50, 162, 16);
  }
}

static void create_keyboard_keys(lv_obj_t *parent) {
  const char *rows[] = {"1234567890", "qwertyuiop", "asdfghjkl.", "zxcvbnm:-_"};
  const lv_coord_t row_y[] = {0, 41, 82, 123};
  const lv_coord_t button_h = 35;
  const lv_coord_t button_gap = 6;
  const lv_coord_t button_w = 40;

  for (uint8_t row = 0; row < 4; ++row) {
    const size_t count = strlen(rows[row]);
    for (size_t i = 0; i < count; ++i) {
      char text[2] = {rows[row][i], '\0'};
      lv_obj_t *key = make_button(parent, &g.style_button_dark,
                                  static_cast<lv_coord_t>(i * (button_w + button_gap)),
                                  row_y[row], button_w, button_h, "", text);
      lv_obj_set_style_radius(key, 10, 0);
      lv_obj_add_event_cb(key, keyboard_key_event_cb, LV_EVENT_CLICKED, nullptr);
    }
  }

  lv_obj_t *space = make_button(parent, &g.style_button_dark, 0, 164, 318, button_h, "", "SPACE");
  lv_obj_set_style_radius(space, 10, 0);
  lv_obj_add_event_cb(space, keyboard_key_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *back = make_button(parent, &g.style_button_red, 324, 164, 64, button_h, "", "Back");
  lv_obj_set_style_radius(back, 10, 0);
  lv_obj_add_event_cb(back, keyboard_key_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *slash = make_button(parent, &g.style_button_dark, 394, 164, 66, button_h, "", "/");
  lv_obj_set_style_radius(slash, 10, 0);
  lv_obj_add_event_cb(slash, keyboard_key_event_cb, LV_EVENT_CLICKED, nullptr);
}

void create_keyboard_overlay() {
  g.keyboard_modal = lv_obj_create(g.root);
  lv_obj_remove_style_all(g.keyboard_modal);
  lv_obj_add_style(g.keyboard_modal, &g.style_overlay, 0);
  set_rect(g.keyboard_modal, 0, 0, kScreenWidth, kScreenHeight);
  lv_obj_add_flag(g.keyboard_modal, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(g.keyboard_modal, LV_OBJ_FLAG_SCROLLABLE);

  g.keyboard_window = lv_obj_create(g.keyboard_modal);
  lv_obj_remove_style_all(g.keyboard_window);
  lv_obj_add_style(g.keyboard_window, &g.style_overlay_window, 0);
  set_rect(g.keyboard_window, 0, 0, kScreenWidth, kScreenHeight);
  lv_obj_clear_flag(g.keyboard_window, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *cancel = make_button(g.keyboard_window, &g.style_button_dark, 10, 10, 40, 40,
                                 icons::cancel, "");
  lv_obj_add_event_cb(cancel, keyboard_cancel_event_cb, LV_EVENT_CLICKED, nullptr);

  g.keyboard_preview = make_panel(g.keyboard_window, 58, 10, 364, 40, false);
  lv_obj_set_style_bg_color(g.keyboard_preview, lv_color_hex(0x171717), 0);
  lv_obj_set_style_radius(g.keyboard_preview, 14, 0);
  lv_obj_t *preview_text = make_label(g.keyboard_preview, ZERO_G_FONT_16, kColorText, "");
  set_rect(preview_text, 12, 10, 340, 20);
  g.keyboard_preview = preview_text;

  lv_obj_t *apply =
      make_button(g.keyboard_window, &g.style_button_red, 430, 10, 40, 40, icons::check, "");
  lv_obj_add_event_cb(apply, keyboard_apply_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *keys = lv_obj_create(g.keyboard_window);
  lv_obj_remove_style_all(keys);
  set_rect(keys, 10, 59, 460, 199);
  lv_obj_set_style_bg_opa(keys, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(keys, LV_OBJ_FLAG_SCROLLABLE);
  create_keyboard_keys(keys);
}

static void create_numpad_keys(lv_obj_t *parent) {
  const char *keys[] = {"1",  "2", "3", "4", "5", "6",
                        "7",  "8", "9", "CLR", "0", "Back"};
  for (uint8_t i = 0; i < 12; ++i) {
    const lv_coord_t col = i % 3;
    const lv_coord_t row = i / 3;
    const lv_coord_t x = col * 156;
    const lv_coord_t y = row * 57;
    lv_obj_t *btn = make_button(parent, (strcmp(keys[i], "Back") == 0) ? &g.style_button_red
                                                                        : &g.style_button_dark,
                                x, y, 148, 49, "", keys[i]);
    if (strcmp(keys[i], "CLR") == 0) {
      lv_obj_set_style_bg_color(btn, lv_color_hex(0x2a2a2a), 0);
      lv_obj_set_style_text_color(btn, lv_color_hex(0xcfcfcf), 0);
    }
    lv_obj_set_style_radius(btn, 14, 0);
    lv_obj_set_style_text_font(btn, ZERO_G_FONT_20, 0);
    lv_obj_add_event_cb(btn, numpad_key_event_cb, LV_EVENT_CLICKED, nullptr);
  }
}

void create_numpad_overlay() {
  g.numpad_modal = lv_obj_create(g.root);
  lv_obj_remove_style_all(g.numpad_modal);
  lv_obj_add_style(g.numpad_modal, &g.style_overlay, 0);
  set_rect(g.numpad_modal, 0, 0, kScreenWidth, kScreenHeight);
  lv_obj_add_flag(g.numpad_modal, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(g.numpad_modal, LV_OBJ_FLAG_SCROLLABLE);

  g.numpad_window = lv_obj_create(g.numpad_modal);
  lv_obj_remove_style_all(g.numpad_window);
  lv_obj_add_style(g.numpad_window, &g.style_overlay_window, 0);
  set_rect(g.numpad_window, 0, 0, kScreenWidth, kScreenHeight);
  lv_obj_clear_flag(g.numpad_window, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *cancel = make_button(g.numpad_window, &g.style_button_dark, 10, 10, 40, 40,
                                 icons::cancel, "");
  lv_obj_add_event_cb(cancel, numpad_cancel_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *preview_box = make_panel(g.numpad_window, 58, 10, 364, 40, false);
  lv_obj_set_style_bg_color(preview_box, lv_color_hex(0x171717), 0);
  lv_obj_set_style_radius(preview_box, 14, 0);
  g.numpad_preview = make_label(preview_box, ZERO_G_FONT_16, kColorText, "Off");
  set_rect(g.numpad_preview, 12, 10, 340, 20);

  lv_obj_t *apply =
      make_button(g.numpad_window, &g.style_button_red, 430, 10, 40, 40, icons::check, "");
  lv_obj_add_event_cb(apply, numpad_apply_event_cb, LV_EVENT_CLICKED, nullptr);

  lv_obj_t *keys = lv_obj_create(g.numpad_window);
  lv_obj_remove_style_all(keys);
  set_rect(keys, 10, 59, 460, 203);
  lv_obj_set_style_bg_opa(keys, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(keys, LV_OBJ_FLAG_SCROLLABLE);
  create_numpad_keys(keys);
}

static lv_obj_t *make_line(lv_obj_t *parent, const lv_point_t *points, uint16_t count,
                           uint8_t width, lv_opa_t opa) {
  lv_obj_t *line = lv_line_create(parent);
  lv_line_set_points(line, points, count);
  lv_obj_add_style(line, &g.style_printer_line, 0);
  lv_obj_set_style_line_width(line, width, 0);
  lv_obj_set_style_line_opa(line, opa, 0);
  return line;
}

static void create_single_fan(lv_obj_t *parent, FanIcon &fan, lv_coord_t x, lv_coord_t y, uint8_t speed) {
  fan.root = lv_obj_create(parent);
  lv_obj_remove_style_all(fan.root);
  set_rect(fan.root, x, y, 26, 26);
  lv_obj_set_style_bg_opa(fan.root, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(fan.root, LV_OBJ_FLAG_SCROLLABLE);

  fan.ring = lv_obj_create(fan.root);
  lv_obj_remove_style_all(fan.ring);
  set_rect(fan.ring, 0, 0, 26, 26);
  lv_obj_set_style_radius(fan.ring, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(fan.ring, color_hex(kColorPanel), 0);
  lv_obj_set_style_bg_opa(fan.ring, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(fan.ring, 3, 0);
  lv_obj_set_style_border_color(fan.ring, color_hex(kColorRedBright), 0);

  fan.hub = lv_obj_create(fan.root);
  lv_obj_remove_style_all(fan.hub);
  set_rect(fan.hub, 10, 10, 6, 6);
  lv_obj_set_style_radius(fan.hub, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(fan.hub, color_hex(kColorRedBright), 0);
  lv_obj_set_style_bg_opa(fan.hub, LV_OPA_COVER, 0);

  for (uint8_t i = 0; i < 3; ++i) {
    fan.blades[i] = lv_obj_create(fan.root);
    lv_obj_remove_style_all(fan.blades[i]);
    set_rect(fan.blades[i], 10, 3, 6, 6);
    lv_obj_set_style_radius(fan.blades[i], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(fan.blades[i], color_hex(kColorRedBright), 0);
    lv_obj_set_style_bg_opa(fan.blades[i], LV_OPA_COVER, 0);
  }

  fan.speed = speed;
  fan.angle = 0;
  position_fan_blades(fan);
}

void create_printer_diagram(lv_obj_t *parent) {
  lv_obj_t *diagram = make_panel(parent, 9, 9, 179, 196, true);
  create_glow(diagram, 38, 16, 100, kColorRed, LV_OPA_16);

  static const lv_point_t frame_left[] = {{31, 128}, {40, 128}, {40, 30}, {40, 30}};
  static const lv_point_t frame_right[] = {{130, 30}, {130, 128}, {139, 128}};
  static const lv_point_t frame_top[] = {{40, 30}, {130, 30}};
  static const lv_point_t gantry[] = {{52, 54}, {118, 54}, {85, 54}, {85, 78}};
  static const lv_point_t bed[] = {{55, 113}, {115, 113}};
  static const lv_point_t part[] = {{73, 113}, {73, 92}, {98, 92}, {98, 113}};
  static const lv_point_t tool[] = {{73, 78}, {97, 78}, {91, 98}, {79, 98}, {73, 78}};
  static const lv_point_t spool[] = {{30, 42}, {24, 34}, {30, 24}, {36, 34}, {30, 42}, {30, 57}};
  static const lv_point_t filter[] = {{116, 36}, {136, 36}, {136, 56}, {116, 56}, {116, 36}};
  static const lv_point_t filter_lines1[] = {{121, 42}, {131, 42}};
  static const lv_point_t filter_lines2[] = {{121, 50}, {131, 50}};

  make_line(diagram, frame_left, 4, 5, LV_OPA_90);
  make_line(diagram, frame_right, 3, 5, LV_OPA_90);
  make_line(diagram, frame_top, 2, 5, LV_OPA_90);
  make_line(diagram, gantry, 4, 4, LV_OPA_75);
  make_line(diagram, bed, 2, 6, LV_OPA_72);
  make_line(diagram, part, 4, 4, LV_OPA_75);
  make_line(diagram, tool, 5, 5, LV_OPA_90);
  make_line(diagram, spool, 6, 4, LV_OPA_68);
  make_line(diagram, filter, 5, 4, LV_OPA_68);
  make_line(diagram, filter_lines1, 2, 3, LV_OPA_68);
  make_line(diagram, filter_lines2, 2, 3, LV_OPA_68);

  create_single_fan(diagram, g.fans[0], 72, 75, 70);
  create_single_fan(diagram, g.fans[1], 99, 81, 45);
  create_single_fan(diagram, g.fans[2], 114, 33, 0);
}

void init(lv_obj_t *parent) {
  memset(&g, 0, sizeof(g));
  init_styles();

  g.root = parent ? parent : lv_scr_act();
  g.active_screen = SCREEN_HOME;
  g.active_settings_page = SETTINGS_PAGE_HOME;
  g.printer_state = PRINTER_PRINTING;
  g.selected_file = 0;
  g.progress = 55;
  g.layer_current = 10;
  g.layer_total = 100;
  g.nozzle_current = 200;
  g.nozzle_target = 210;
  g.bed_current = 60;
  g.bed_target = 60;
  copy_text(g.remaining, sizeof(g.remaining), "~5m");
  copy_text(g.time_text, sizeof(g.time_text), "11:52");
  copy_text(g.current_print_file, sizeof(g.current_print_file), kFiles[0].name);

  lv_obj_set_style_bg_color(g.root, color_hex(kColorBg), 0);
  lv_obj_set_style_bg_opa(g.root, LV_OPA_COVER, 0);
  lv_obj_clear_flag(g.root, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(g.root, kScreenWidth, kScreenHeight);

  g.main_area = lv_obj_create(g.root);
  lv_obj_remove_style_all(g.main_area);
  set_rect(g.main_area, kSidebarWidth, 0, kMainWidth, kScreenHeight);
  lv_obj_set_style_bg_opa(g.main_area, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(g.main_area, LV_OBJ_FLAG_SCROLLABLE);

  g.content_host = lv_obj_create(g.main_area);
  lv_obj_remove_style_all(g.content_host);
  set_rect(g.content_host, kMainPaddingX, kMainPaddingTop + kTopbarHeight + kContentGap, kContentWidth,
           kContentHeight);
  lv_obj_set_style_bg_opa(g.content_host, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(g.content_host, LV_OBJ_FLAG_SCROLLABLE);

  create_sidebar();
  create_topbar();

  for (uint8_t i = 0; i < SCREEN_COUNT; ++i) {
    create_page_root(static_cast<ScreenId>(i));
  }

  create_home_screen();
  create_files_screen();
  create_control_screen();
  create_mesh_screen();
  create_tuning_screen();
  create_settings_screen();
  create_keyboard_overlay();
  create_numpad_overlay();

  update_home_visibility();
  refresh_file_selection();
  refresh_pause_button();
  update_status();
  update_temperatures(g.nozzle_current, g.nozzle_target, g.bed_current, g.bed_target);
  update_progress(g.progress, g.layer_current, g.layer_total, g.remaining);

  g.anim_timer = lv_timer_create(anim_timer_cb, 60, nullptr);
}

void show_screen(ScreenId screen_id) {
  if (screen_id >= SCREEN_COUNT || g.screen_roots[screen_id] == nullptr) {
    return;
  }

  if (screen_id == g.active_screen) {
    return;
  }

  lv_obj_t *old_page = g.screen_roots[g.active_screen];
  lv_obj_t *new_page = g.screen_roots[screen_id];

  lv_obj_clear_flag(new_page, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(new_page);
  lv_obj_set_style_opa(new_page, LV_OPA_TRANSP, 0);
  lv_obj_set_x(new_page, 8);

  animate_property(new_page, 8, 0, 230, anim_set_x);
  animate_property(new_page, 0, LV_OPA_COVER, 210, anim_set_opa);

  animate_property(old_page, 0, -6, 230, anim_set_x);
  animate_property(old_page, LV_OPA_COVER, 0, 180, anim_set_opa, hide_anim_ready_cb);

  for (uint8_t i = 0; i < SCREEN_COUNT; ++i) {
    set_nav_button_state(i, i == screen_id);
  }

  g.active_screen = screen_id;
  if (screen_id == SCREEN_SETTINGS) {
    show_settings_page(SETTINGS_PAGE_HOME);
  }
  update_topbar_layout();
  update_status();
}

void show_keyboard(lv_obj_t *target_field) {
  if (!target_field) {
    return;
  }
  g.keyboard_target = target_field;
  g.keyboard_meta.secret = false;
  g.keyboard_meta.placeholder = "";

  const uint32_t text_index = (lv_obj_get_child_cnt(target_field) > 1U) ? 1U : 0U;
  const char *current = lv_label_get_text(lv_obj_get_child(target_field, text_index));
  if (current && g.keyboard_meta.secret && strcmp(current, kSecretMask) == 0) {
    copy_text(g.keyboard_buffer, sizeof(g.keyboard_buffer), "");
  } else if (current && g.keyboard_meta.placeholder &&
             strcmp(current, g.keyboard_meta.placeholder) == 0) {
    copy_text(g.keyboard_buffer, sizeof(g.keyboard_buffer), "");
  } else {
    copy_text(g.keyboard_buffer, sizeof(g.keyboard_buffer), current);
  }
  lv_label_set_text(g.keyboard_preview, g.keyboard_buffer);
  show_overlay(g.keyboard_modal, g.keyboard_window);
}

void show_numpad(lv_obj_t *target_temp_field) {
  if (!target_temp_field) {
    return;
  }
  g.numpad_target = target_temp_field;
  const uint32_t text_index = (lv_obj_get_child_cnt(target_temp_field) > 1U) ? 1U : 0U;
  const char *current = lv_label_get_text(lv_obj_get_child(target_temp_field, text_index));
  if (current && strcmp(current, "Off") != 0) {
    uint8_t pos = 0;
    for (size_t i = 0; current[i] != '\0' && pos < sizeof(g.numpad_buffer) - 1U; ++i) {
      if (current[i] >= '0' && current[i] <= '9') {
        g.numpad_buffer[pos++] = current[i];
      }
    }
    g.numpad_buffer[pos] = '\0';
  } else {
    g.numpad_buffer[0] = '\0';
  }

  if (g.numpad_buffer[0] == '\0') {
    lv_label_set_text(g.numpad_preview, "Off");
  } else {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%s%s", g.numpad_buffer, kDegree);
    lv_label_set_text(g.numpad_preview, buffer);
  }
  show_overlay(g.numpad_modal, g.numpad_window);
}

void update_status() {
  bool is_paused = (g.printer_state == PRINTER_PAUSED);

  if (g.printer_state == PRINTER_IDLE) {
    refresh_status_visuals("Idle", 0xffffff, LV_OPA_28, kColorMutedLight, 42);
  } else if (is_paused) {
    refresh_status_visuals("Paused", kColorYellow, LV_OPA_22, kColorYellow, 58);
  } else if (g.active_screen == SCREEN_HOME) {
    refresh_status_visuals("Printing", kColorGreen, LV_OPA_16, kColorGreen, 62);
  } else {
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "Printing %u%%", g.progress);
    refresh_status_visuals(buffer, kColorGreen, LV_OPA_16, kColorGreen, 86);
  }

  update_home_visibility();
  refresh_pause_button();

  if (g.start_print_btn) {
    if (g.printer_state == PRINTER_IDLE) {
      lv_obj_clear_state(g.start_print_btn, LV_STATE_DISABLED);
      lv_obj_set_style_bg_color(g.start_print_btn, color_hex(kColorRed), 0);
      const uint32_t child_count = lv_obj_get_child_cnt(g.start_print_btn);
      for (uint32_t i = 0; i < child_count; ++i) {
        lv_obj_set_style_text_color(lv_obj_get_child(g.start_print_btn, i), color_hex(kColorText), 0);
      }
    } else {
      lv_obj_add_state(g.start_print_btn, LV_STATE_DISABLED);
      lv_obj_set_style_bg_color(g.start_print_btn, lv_color_hex(0x343434), 0);
      const uint32_t child_count = lv_obj_get_child_cnt(g.start_print_btn);
      for (uint32_t i = 0; i < child_count; ++i) {
        lv_obj_set_style_text_color(lv_obj_get_child(g.start_print_btn, i), lv_color_hex(0x8a8a8a), 0);
      }
    }
  }
}

void update_temperatures(int nozzle_current, int nozzle_target, int bed_current, int bed_target) {
  g.nozzle_current = nozzle_current;
  g.nozzle_target = nozzle_target;
  g.bed_current = bed_current;
  g.bed_target = bed_target;

  char buffer[24];
  format_temp_chip(buffer, sizeof(buffer), nozzle_current, nozzle_target);
  lv_label_set_text(g.nozzle_value, buffer);
  format_temp_chip(buffer, sizeof(buffer), bed_current, bed_target);
  lv_label_set_text(g.bed_value, buffer);

  format_temp_field(buffer, sizeof(buffer), nozzle_target);
  lv_label_set_text(lv_obj_get_child(g.temp_nozzle_field, 1), buffer);
  format_temp_field(buffer, sizeof(buffer), bed_target);
  lv_label_set_text(lv_obj_get_child(g.temp_bed_field, 1), buffer);

  snprintf(buffer, sizeof(buffer), "%dC", bed_current);
  lv_label_set_text(g.mesh_temp_label, buffer);
}

void update_progress(uint8_t percent, uint16_t layer_current, uint16_t layer_total,
                     const char *remaining) {
  g.progress = percent;
  g.layer_current = layer_current;
  g.layer_total = layer_total;
  copy_text(g.remaining, sizeof(g.remaining), remaining);

  char buffer[24];
  snprintf(buffer, sizeof(buffer), "%u%%", g.progress);
  lv_label_set_text(g.percent_label, buffer);
  lv_arc_set_value(g.progress_arc, g.progress);

  snprintf(buffer, sizeof(buffer), "Layer %u / %u", g.layer_current, g.layer_total);
  lv_label_set_text(g.layer_label, buffer);

  snprintf(buffer, sizeof(buffer), "%s remaining", g.remaining);
  lv_label_set_text(g.remaining_label, buffer);

  update_status();
}

void set_printer_state(PrinterState state) {
  g.printer_state = state;
  update_status();
}

void set_time_text(const char *time_text) {
  copy_text(g.time_text, sizeof(g.time_text), time_text);
  lv_label_set_text(g.time_label, g.time_text);
}

void set_selected_file(uint8_t index) {
  if (index >= sizeof(kFiles) / sizeof(kFiles[0])) {
    return;
  }
  g.selected_file = index;
  refresh_file_selection();
}

namespace {

void show_settings_page(SettingsPageId page_id) {
  if (page_id >= SETTINGS_PAGE_COUNT) {
    return;
  }

  for (uint8_t i = 0; i < SETTINGS_PAGE_COUNT; ++i) {
    if (i == page_id) {
      lv_obj_clear_flag(g.settings_pages[i], LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(g.settings_pages[i], LV_OBJ_FLAG_HIDDEN);
    }
  }
  g.active_settings_page = page_id;
}

void slider_event_cb(lv_event_t *e) {
  SliderBinding *binding = static_cast<SliderBinding *>(lv_event_get_user_data(e));
  refresh_slider_output(binding);
}

void nav_event_cb(lv_event_t *e) {
  ScreenId screen_id = static_cast<ScreenId>(
      reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
  show_screen(screen_id);
}

void shortcut_nav_event_cb(lv_event_t *e) {
  ScreenId screen_id = static_cast<ScreenId>(
      reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
  if (screen_id == SCREEN_SETTINGS) {
    show_screen(SCREEN_SETTINGS);
  } else {
    show_screen(screen_id);
  }
}

void file_select_event_cb(lv_event_t *e) {
  uint8_t index = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
  set_selected_file(index);
}

void pause_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  g.printer_state = (g.printer_state == PRINTER_PAUSED) ? PRINTER_PRINTING : PRINTER_PAUSED;
  update_status();
}

void cancel_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  g.printer_state = PRINTER_IDLE;
  update_status();
}

void start_print_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  g.printer_state = PRINTER_PRINTING;
  copy_text(g.current_print_file, sizeof(g.current_print_file), kFiles[g.selected_file].name);
  lv_label_set_text(g.home_file_name, g.current_print_file);
  update_status();
}

void open_fans_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  lv_obj_add_flag(g.tuning_main, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(g.fan_subpage, LV_OBJ_FLAG_HIDDEN);
}

void back_fans_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  lv_obj_add_flag(g.fan_subpage, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(g.tuning_main, LV_OBJ_FLAG_HIDDEN);
}

void settings_tile_event_cb(lv_event_t *e) {
  SettingsPageId page_id = static_cast<SettingsPageId>(
      reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
  show_settings_page(page_id);
}

void settings_back_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  show_settings_page(SETTINGS_PAGE_HOME);
}

void segment_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  lv_obj_t *parent = lv_obj_get_parent(btn);
  const uint32_t count = lv_obj_get_child_cnt(parent);
  for (uint32_t i = 0; i < count; ++i) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    if (lv_obj_check_type(child, &lv_btn_class)) {
      set_button_active(child, child == btn);
    }
  }
}

void extrude_step_event_cb(lv_event_t *e) {
  segment_event_cb(e);
  const int amount = static_cast<int>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d mm", amount);
  lv_label_set_text(g.extrusion_amount_label, buffer);
}

void text_field_event_cb(lv_event_t *e) {
  g.keyboard_target = lv_event_get_target(e);
  TextFieldMeta *meta = static_cast<TextFieldMeta *>(lv_event_get_user_data(e));
  if (meta) {
    g.keyboard_meta = *meta;
  } else {
    g.keyboard_meta.secret = false;
    g.keyboard_meta.placeholder = "";
  }

  const char *current = lv_label_get_text(lv_obj_get_child(g.keyboard_target, 1));
  if (g.keyboard_meta.secret && strcmp(current, kSecretMask) == 0) {
    g.keyboard_buffer[0] = '\0';
  } else if (g.keyboard_meta.placeholder && strcmp(current, g.keyboard_meta.placeholder) == 0) {
    g.keyboard_buffer[0] = '\0';
  } else {
    copy_text(g.keyboard_buffer, sizeof(g.keyboard_buffer), current);
  }
  lv_label_set_text(g.keyboard_preview, g.keyboard_buffer);
  show_overlay(g.keyboard_modal, g.keyboard_window);
}

void temp_field_event_cb(lv_event_t *e) {
  g.numpad_target = lv_event_get_target(e);
  const char *current = lv_label_get_text(lv_obj_get_child(g.numpad_target, 1));
  if (strcmp(current, "Off") == 0) {
    g.numpad_buffer[0] = '\0';
  } else {
    uint8_t pos = 0;
    for (size_t i = 0; current[i] != '\0' && pos < sizeof(g.numpad_buffer) - 1U; ++i) {
      if (current[i] >= '0' && current[i] <= '9') {
        g.numpad_buffer[pos++] = current[i];
      }
    }
    g.numpad_buffer[pos] = '\0';
  }

  if (g.numpad_buffer[0] == '\0') {
    lv_label_set_text(g.numpad_preview, "Off");
  } else {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%s%s", g.numpad_buffer, kDegree);
    lv_label_set_text(g.numpad_preview, buffer);
  }
  show_overlay(g.numpad_modal, g.numpad_window);
}

void temp_off_event_cb(lv_event_t *e) {
  lv_obj_t *off_btn = lv_event_get_target(e);
  lv_obj_t *row = lv_obj_get_parent(off_btn);
  const uint32_t count = lv_obj_get_child_cnt(row);
  for (uint32_t i = 0; i < count; ++i) {
    lv_obj_t *child = lv_obj_get_child(row, i);
    if (lv_obj_check_type(child, &lv_btn_class) && child != off_btn) {
      lv_label_set_text(lv_obj_get_child(child, 1), "Off");
      if (child == g.temp_nozzle_field) {
        update_temperatures(g.nozzle_current, 0, g.bed_current, g.bed_target);
      } else if (child == g.temp_bed_field) {
        update_temperatures(g.nozzle_current, g.nozzle_target, g.bed_current, 0);
      }
      break;
    }
  }
}

void keyboard_key_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  const char *text = lv_label_get_text(lv_obj_get_child(btn, 1));
  if (strcmp(text, "Back") == 0) {
    size_t len = strlen(g.keyboard_buffer);
    if (len > 0U) {
      g.keyboard_buffer[len - 1U] = '\0';
    }
  } else if (strcmp(text, "SPACE") == 0) {
    size_t len = strlen(g.keyboard_buffer);
    if (len + 1U < sizeof(g.keyboard_buffer)) {
      g.keyboard_buffer[len] = ' ';
      g.keyboard_buffer[len + 1U] = '\0';
    }
  } else {
    const size_t len = strlen(g.keyboard_buffer);
    if (len + strlen(text) < sizeof(g.keyboard_buffer)) {
      strcat(g.keyboard_buffer, text);
    }
  }
  lv_label_set_text(g.keyboard_preview, g.keyboard_buffer);
}

void keyboard_cancel_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  hide_overlay(g.keyboard_modal, g.keyboard_window);
}

void keyboard_apply_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  if (g.keyboard_target) {
    lv_obj_t *text_label = lv_obj_get_child(g.keyboard_target, 1);
    if (g.keyboard_meta.secret && g.keyboard_buffer[0] != '\0') {
      lv_label_set_text(text_label, kSecretMask);
    } else if (g.keyboard_buffer[0] != '\0') {
      lv_label_set_text(text_label, g.keyboard_buffer);
    } else if (g.keyboard_meta.placeholder) {
      lv_label_set_text(text_label, g.keyboard_meta.placeholder);
    } else {
      lv_label_set_text(text_label, "");
    }
  }
  hide_overlay(g.keyboard_modal, g.keyboard_window);
}

void numpad_key_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  const char *text = lv_label_get_text(lv_obj_get_child(btn, 1));
  if (strcmp(text, "Back") == 0) {
    size_t len = strlen(g.numpad_buffer);
    if (len > 0U) {
      g.numpad_buffer[len - 1U] = '\0';
    }
  } else if (strcmp(text, "CLR") == 0) {
    g.numpad_buffer[0] = '\0';
  } else if (strlen(g.numpad_buffer) < 3U) {
    strcat(g.numpad_buffer, text);
  }

  if (g.numpad_buffer[0] == '\0') {
    lv_label_set_text(g.numpad_preview, "Off");
  } else {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%s%s", g.numpad_buffer, kDegree);
    lv_label_set_text(g.numpad_preview, buffer);
  }
}

void numpad_cancel_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  hide_overlay(g.numpad_modal, g.numpad_window);
}

void numpad_apply_event_cb(lv_event_t *e) {
  LV_UNUSED(e);
  if (g.numpad_target) {
    int value = (g.numpad_buffer[0] != '\0') ? atoi(g.numpad_buffer) : 0;
    if (g.numpad_target == g.temp_nozzle_field) {
      update_temperatures(g.nozzle_current, value, g.bed_current, g.bed_target);
    } else if (g.numpad_target == g.temp_bed_field) {
      update_temperatures(g.nozzle_current, g.nozzle_target, g.bed_current, value);
    }
  }
  hide_overlay(g.numpad_modal, g.numpad_window);
}

}  // namespace

}  // namespace zerog_ui
