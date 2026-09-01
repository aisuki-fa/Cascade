#define RAYGUI_IMPLEMENTATION             // enables raygui function bodies (only in this .c file)
#include "raygui.h"                       // GuiButton, GuiSlider, GuiColorPicker, GuiDropdownBox, etc.
#include "cascade.h"                      // SimState, UIState, SIDEBAR_W, WINDOW_H, etc.
#include "ui.h"                           // declares ui_draw_sidebar()
#include "obstacles.h"                    // obs_add_preset, obs_clear, PRESET_RAMP/BOX/FUNNEL

// ── Layout constants ───────────────────────────────────────────
#define PAD         12                    // left/right padding inside sidebar (px)
#define INNER_W     196                   // content width: SIDEBAR_W - 2*PAD
#define SEC_HDR_Y   11                    // section header font size (px)
#define SEC_HDR_H   18                    // section header height + gap below
#define ELEM_H      26                    // button / toggle height (px)
#define ELEM_GAP    6                     // gap between sibling elements in a section
#define SEC_GAP     10                    // gap between sections
#define SLIDER_H    18                    // slider track height (px)
#define SLIDER_GAP  4                     // gap between slider and its value box
#define VALBOX_H    16                    // value readout box height (px)
#define VALBOX_W    56                    // value readout box width (px)

// ── Section background heights ─────────────────────────────────
#define BG_SIM      48                    // SIMULATION: 3 buttons (pause/reset/theme) + header
#define BG_FLUID    260                   // FLUID: 4 sliders + value boxes + header
#define BG_SPAWN    170                   // SPAWN COLOR: color picker + header
#define BG_VIZ      70                    // VISUALIZE: dropdown + header
#define BG_OBS      217                   // OBSTACLES: 4 buttons + toggle + header + 2 shape buttons

// Pack a raylib Color into raygui's 0xAABBGGRR int format.
static unsigned ColorToGui(Color c) {
    return ((unsigned)c.a << 24) | ((unsigned)c.r << 16) | ((unsigned)c.g << 8) | (unsigned)c.b;
}

// Draws a dark rounded box with a numeric value inside
static void DrawValueBox(float value, const char* fmt, int x, int y, Theme t) {
    Rectangle box = {(float)x, (float)y, (float)VALBOX_W, (float)VALBOX_H};      // build rectangle from position + fixed size
    GuiDrawRectangle(box, 1, t.border, t.card);                                  // 1px border, card fill
    DrawTextEx(font_ui, TextFormat(fmt, value), (Vector2){ (float)(x + 4), (float)(y + 2) }, 11, 1, t.text);
}

// Draws the entire left sidebar: background, title, all control sections
void ui_draw_sidebar(SimState* sim, UIState* ui, ObstacleList* obs) {
    Theme t = theme_get();                                                     // resolve active palette this frame
    GuiSetFont(font_ui_small);                                                 // small controls: headers, labels, buttons, values

    // ── Sidebar background + border ─────────────────────────────
    DrawRectangle(0, 0, SIDEBAR_W, WINDOW_H, t.panel);                         // sidebar fill
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H, t.border);                     // divider between sidebar and sim area

    // ── Title (Liber) ───────────────────────────────────────────
    DrawTextEx(font_title, "CASCADE", (Vector2){ (float)PAD, 8.0f }, 34, 2, t.accent);
    DrawRectangle(PAD, 46, 70, 2, t.accent);                                   // accent underline below title

    // ── Button style ────────────────────────────────────────────
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,   ColorToGui(t.card));             // normal
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED,  ColorToGui(t.card));             // hovered
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED,  ColorToGui(t.accent));           // clicked
    GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, ColorToGui(t.section));          // disabled
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToGui(t.border));           // normal border
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED,ColorToGui(t.accent_dim));       // hover border
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED,ColorToGui(t.accent));           // pressed border
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,   ColorToGui(t.text));             // normal text
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED,  ColorToGui(t.text));             // hover text
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED,  ColorToGui(t.bg));               // pressed text (contrast on accent)
    GuiSetStyle(BUTTON, TEXT_COLOR_DISABLED, ColorToGui(t.faint));            // disabled text
    GuiSetStyle(BUTTON, TEXT_SIZE, 11);                                       // button text size (px)
    GuiSetStyle(BUTTON, TEXT_SPACING, 1);                                     // letter spacing (px)
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, 0);                                   // 0 = left-aligned text
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);                                     // 1px border

    // ── Slider style ────────────────────────────────────────────
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL,   ColorToGui((Color){t.section.r, t.section.g, t.section.b, 255})); // normal track
    GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED,  ColorToGui(t.card));             // hover track
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED,  ColorToGui(t.accent));           // dragging
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, ColorToGui(t.border));           // normal border
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED,ColorToGui(t.border_hi));        // hover border
    GuiSetStyle(SLIDER, BORDER_COLOR_PRESSED,ColorToGui(t.accent));           // pressed border
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 8);                                     // handle width (px)
    GuiSetStyle(SLIDER, BORDER_WIDTH, 1);                                     // 1px border

    // ── Toggle style ────────────────────────────────────────────
    GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL,   ColorToGui(t.card));             // normal (off)
    GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED,  ColorToGui(t.card));             // hovered
    GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED,  ColorToGui(t.accent));           // checked
    GuiSetStyle(TOGGLE, TEXT_COLOR_NORMAL,   ColorToGui(t.text));             // label text
    GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED,  ColorToGui(t.bg));               // checked text (contrast on accent)

    int y = 56;                                                               // first section starts below title + underline

    // ═══ SIMULATION ═════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_SIM, t.section);                         // background behind this section
    DrawTextEx(font_ui, "SIMULATION", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head); y += SEC_HDR_H;
    int box_w = (INNER_W - 2*ELEM_GAP) / 3;              // each small box width (≈61px)
    if (GuiButton((Rectangle){(float)PAD, (float)y, box_w, ELEM_H}, sim->paused ? "RESUME" : "PAUSE"))
        sim->paused = !sim->paused;                      // flip paused state on click
    if (GuiButton((Rectangle){(float)(PAD+box_w+ELEM_GAP), (float)y, box_w, ELEM_H}, "RESET"))
        sim->reset_requested = true;                     // main.c checks this and calls sim_reset()
    if (GuiButton((Rectangle){(float)(PAD+2*(box_w+ELEM_GAP)), (float)y, box_w, ELEM_H}, theme_dark ? "Dark" : "Light"))
        theme_dark = !theme_dark;                        // toggle dark/light theme
    y += ELEM_H + SEC_GAP;

    // ═══ FLUID ══════════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_FLUID, t.section);                       // background behind this section
    DrawTextEx(font_ui, "FLUID", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head); y += SEC_HDR_H;

    DrawTextEx(font_ui, "Gravity", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->gravity, 0, 1500);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->gravity, "%.0f", PAD, y, t); y += VALBOX_H + ELEM_GAP;   // show current gravity as integer

    DrawTextEx(font_ui, "Viscosity", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->viscosity, 0, 5);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->viscosity, "%.2f", PAD, y, t); y += VALBOX_H + ELEM_GAP; // show current viscosity as 2 decimals

    DrawTextEx(font_ui, "Density", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->target_density, 50, 800);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->target_density, "%.0f", PAD, y, t); y += VALBOX_H + ELEM_GAP; // show current density as integer

    DrawTextEx(font_ui, "Size", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->particle_radius, 3, 16);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->particle_radius, "%.0f", PAD, y, t); y += VALBOX_H + SEC_GAP; // show current radius as integer

    // ═══ SPAWN COLOR ════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_SPAWN, t.section);                        // background behind this section
    DrawTextEx(font_ui, "SPAWN COLOR", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head); y += SEC_HDR_H;
    GuiColorPicker((Rectangle){(float)PAD, (float)y, 170, 95}, NULL, &ui->spawn_color); // color picker: writes to ui->spawn_color
    y += 95 + SEC_GAP;                                                         // move past picker (95px) + section gap

    // ═══ VISUALIZE ══════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_VIZ, t.section);                          // background behind this section
    DrawTextEx(font_ui, "VISUALIZE", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head); y += SEC_HDR_H;
    static int mode_active = 0;                                                // persisted: selected render mode (0=Solid, 1=Velocity, 2=Density)
    static bool mode_open = false;                                             // persisted: true when dropdown list is visible
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&                             // manual open/close: raygui dropdown takes bool by value
        CheckCollisionPointRec(GetMousePosition(), (Rectangle){(float)PAD, (float)y, INNER_W, 26}))
        mode_open = !mode_open;
    GuiDropdownBox((Rectangle){(float)PAD, (float)y, INNER_W, 26}, "Solid;Velocity;Density", &mode_active, mode_open);
    ui->render_mode = mode_active;                                             // push selected mode to renderer
    Color mode_colors[] = { t.text, t.accent, t.repel };                       // accent for dropdown by mode
    DrawRectangle(PAD - 3, y + 3, 3, 20, mode_colors[mode_active]);            // 3px-wide colored bar showing active mode
    y += 26;                                                                   // move past dropdown height
    if (mode_open) y += 80;                                                    // push content down when dropdown list is open
    y += SEC_GAP;                                                              // section gap

    // ═══ OBSTACLES ══════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_OBS, t.section);                          // background behind this section
    DrawTextEx(font_ui, "OBSTACLES", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head); y += SEC_HDR_H;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Ramp"))  obs_add_preset(obs, PRESET_RAMP);      // spawn ramp
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Box"))   obs_add_preset(obs, PRESET_BOX);       // spawn box
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Funnel")) obs_add_preset(obs, PRESET_FUNNEL);   // spawn funnel
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Clear"))  obs_clear(obs);                       // remove all obstacles
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    GuiToggle((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Draw Walls", &ui->draw_mode);                       // toggle wall-drawing mode
    y += ELEM_H + ELEM_GAP;                                                    // move past toggle + gap
    Rectangle cir = {(float)PAD, (float)y, INNER_W/2 - 3, ELEM_H};           // circle-shape tool button rect
    Rectangle rec = {(float)(PAD + INNER_W/2 + 3), (float)y, INNER_W/2 - 3, ELEM_H}; // rect-shape tool button rect
    if (GuiButton(cir, "Circle")) {
        obs->drop_shape = (obs->drop_shape == 1) ? 0 : 1;                      // click again to disarm back to spawning
        ui->draw_mode = false;                                                 // a shape tool on turns wall mode off
    }
    if (obs->drop_shape == 1)                                                  // circle armed → accent ring overlay
        DrawRectangleLinesEx(cir, 2.0f, t.accent);
    if (GuiButton(rec, "Rect")) {
        obs->drop_shape = (obs->drop_shape == 2) ? 0 : 2;                      // click again to disarm back to spawning
        ui->draw_mode = false;                                                 // a shape tool on turns wall mode off
    }
    if (obs->drop_shape == 2)                                                  // rect armed → accent ring overlay
        DrawRectangleLinesEx(rec, 2.0f, t.accent);

    // ── Interaction mode (pinned near particle counter) ──────────
    GuiSetFont(font_ui);
    float w_mode = MeasureTextEx(font_ui, ui->mouse_repel ? "MODE: REPEL" : "MODE: ATTRACT", 18, 1).x;
    DrawTextEx(font_ui, ui->mouse_repel ? "MODE: REPEL" : "MODE: ATTRACT",
           (Vector2){ (float)(WINDOW_W - 16 - w_mode), (float)(WINDOW_H - 70) }, 18, 1,
           ui->mouse_repel ? t.repel : t.accent);

    // ── Particle counter (pinned to bottom) ─────────────────────
    float w_count = MeasureTextEx(font_ui, TextFormat("Particles: %d/%d", sim->count, MAX_PARTICLES), 26, 1).x;
    DrawTextEx(font_ui, TextFormat("Particles: %d/%d", sim->count, MAX_PARTICLES),
           (Vector2){ (float)(WINDOW_W - 16 - w_count), (float)(WINDOW_H - 40) }, 26, 1, t.text);
}
