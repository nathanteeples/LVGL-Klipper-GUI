# Zero G LVGL UI Notes

This workspace now includes a native LVGL implementation of the HTML prototype under `src/` and `include/`.

It also now includes an LVGL XML preview project for the online editor/viewer under `ui/`:

- `ui/project.xml`
- `ui/globals.xml`
- `ui/components/README.md`
- `ui/fonts/README.md`
- `ui/images/README.md`
- `ui/screens/home.xml`
- `ui/screens/home_idle.xml`
- `ui/screens/files.xml`
- `ui/screens/control.xml`
- `ui/screens/mesh.xml`
- `ui/screens/tuning.xml`
- `ui/screens/settings.xml`
- `ui/screens/README.md`
- `ui/widgets/README.md`

For compatibility with LVGL Online Share's current repository detection logic,
the same preview project is also mirrored at the repo root:

- `project.xml`
- `globals.xml`
- `components/README.md`
- `fonts/README.md`
- `images/README.md`
- `screens/*.xml`
- `screens/README.md`
- `widgets/README.md`

Files

- `src/main.cpp`
- `src/zerog_ui.cpp`
- `include/zerog_ui.h`
- `include/zerog_ui_assets.h`

LVGL online editor usage

- Open [viewer.lvgl.io](https://viewer.lvgl.io)
- Paste the public repo URL: `https://github.com/nathanteeples/LVGL-Klipper-GUI`
- Paste the public folder URL: `https://github.com/nathanteeples/LVGL-Klipper-GUI/tree/main/ui`
- Both URLs should now work.
- LVGL Online Share expects the target folder to contain both `project.xml` and `globals.xml`.
- The viewer currently also checks for at least one non-XML file in standard project folders such as `screens/`, `components/`, `widgets/`, `fonts/`, or `images/`, so this repo includes placeholder `README.md` files there for compatibility.
- Use the `ui/screens/` folder to switch between the static preview screens in the editor.
- These XML files are editor-friendly previews of the native runtime UI, not a one-to-one export of all runtime behavior from `src/zerog_ui.cpp`.

What is already implemented

- Fixed `480x272` landscape layout with a `56px` sidebar and `404x214` content area.
- Shared dark flat styling, white text, muted copy, red accent buttons, rounded panels, and the top bar used on every page.
- Sidebar navigation for Home, Files, Control, Bed Mesh, Tuning, and Settings.
- Home idle and printing states.
- Files two-pane browser with selected-row marquee behavior.
- Control page with jog pad, jog step buttons, temperature target buttons, extrusion controls, and motors-off button.
- Tuning page with six sliders plus a fan subpage and animated fan indicators.
- Bed Mesh page with a simple dot mesh visualization and action buttons.
- Settings home plus Network, Screen, Klipper, Audio, Console, and Power subpages.
- Full-screen keyboard and numpad overlays with apply/cancel flow.

Integration points

- `src/main.cpp`
  Replace `setup_display_and_touch()` with your JC3248W535C display and touch initialization.
- `include/zerog_ui_assets.h`
  The icon strings are intentionally abstracted there so you can swap the fallback symbols for a compiled Tabler icon font later.
- `src/zerog_ui.cpp`
  The Home top bar currently uses a styled text placeholder for the Zero G logo. Replace that label with an `lv_img` if you want the exact logo asset.

Fonts to enable in `lv_conf.h`

- `LV_FONT_MONTSERRAT_10`
- `LV_FONT_MONTSERRAT_12`
- `LV_FONT_MONTSERRAT_14`
- `LV_FONT_MONTSERRAT_16`
- `LV_FONT_MONTSERRAT_18`
- `LV_FONT_MONTSERRAT_20`
- `LV_FONT_MONTSERRAT_24`
- `LV_FONT_MONTSERRAT_28`
- `LV_FONT_MONTSERRAT_36`
- `LV_FONT_MONTSERRAT_42` if available

Icon swap guidance

- Keep the semantic names in `include/zerog_ui_assets.h`.
- If you add a real icon font, define `ZERO_G_TABLER_ICON_FONT` before including the UI headers and replace the fallback strings in the `icons` namespace with your font glyph strings.

Hardware notes

- Keep LVGL configured for `480x272` native landscape.
- Avoid large image assets unless they are necessary.
- The UI code creates all objects once and reuses them to stay friendly to ESP32-S3 class hardware.
