#include <Arduino.h>
#include <lvgl.h>

#include "zerog_ui.h"

namespace {

void setup_display_and_touch() {
  // Hardware-specific hookup for the Guition JC3248W535C goes here.
  //
  // 1. Initialize the panel driver at 480x272 landscape.
  // 2. Allocate and register the LVGL draw buffers.
  // 3. Register the LVGL display flush callback.
  // 4. Register the touch read callback.
  //
  // Keep the UI native to 480x272. Do not scale a 320px-tall layout into place.
}

}  // namespace

void setup() {
  Serial.begin(115200);
  lv_init();

  setup_display_and_touch();

  zerog_ui::init();
  zerog_ui::set_time_text("11:52");
  zerog_ui::update_temperatures(200, 210, 60, 60);
  zerog_ui::update_progress(55, 10, 100, "~5m");
}

void loop() {
  lv_timer_handler();
  delay(5);
}
