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
#define BG_SIM      80                    // SIMULATION: 2 buttons + header
#define BG_FLUID    260                   // FLUID: 4 sliders + value boxes + header
#define BG_SPAWN    170                   // SPAWN COLOR: color picker + header
#define BG_VIZ      70                    // VISUALIZE: dropdown + header
#define BG_OBS      185                   // OBSTACLES: 4 buttons + toggle + header

// Draws a dark rounded box with a numeric value inside
static void DrawValueBox(float value, const char* fmt, int x, int y) {
    Rectangle box = {(float)x, (float)y, (float)VALBOX_W, (float)VALBOX_H};      // build rectangle from position + fixed size
    GuiDrawRectangle(box, 1, (Color){255, 255, 255, 40}, (Color){0, 0, 0, 60});  // 1px border (white 40%), dark fill (black 60%)
    DrawText(TextFormat(fmt, value), x + 4, y + 2, 11, WHITE);                   // formatted value text, offset 4px from left, 2px from top
}

// Draws the entire left sidebar: background, title, all control sections
void ui_draw_sidebar(SimState* sim, UIState* ui, ObstacleList* obs) {
    // ── Sidebar background + border ─────────────────────────────
    DrawRectangle(0, 0, SIDEBAR_W, WINDOW_H, (Color){7, 128, 227, 255});         // blue sidebar fill (0780e3)
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H, (Color){25, 40, 72, 255});       // dark border between sidebar and sim area

    // ── Title ───────────────────────────────────────────────────
    DrawText("CASCADE", PAD, 12, 28, BLACK);                             // project title at top of sidebar
    DrawRectangle(PAD, 44, 70, 2, (Color){255, 255, 255, 120});          // white underline accent below title

    // ── Button style ────────────────────────────────────────────
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,   0xFFFFFF55);                // normal
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED,  0xFFFFFF88);                // hovered
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED,  0x00000066);                // clicked
    GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, 0x9B527744);                // disabled
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000033);                // normal border
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED,0x00000066);                // hover border
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED,0x00000088);                // pressed border
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,   0x000000FF);                // normal text
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED,  0x000000FF);                // hover text
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED,  0xFFFFFFFF);                // pressed text
    GuiSetStyle(BUTTON, TEXT_COLOR_DISABLED, 0x00000088);                // disabled text
    GuiSetStyle(BUTTON, TEXT_SIZE, 10);                                  // button text size (px)
    GuiSetStyle(BUTTON, TEXT_SPACING, 1);                                // letter spacing (px)
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, 0);                              // 0 = left-aligned text
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);                                // 1px border

    // ── Slider style ────────────────────────────────────────────
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL,   0x00000044);                // normal track
    GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED,  0x00000066);                // hover track
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED,  0xFFFFFF88);                // dragging
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0x00000033);                // normal border
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED,0x00000055);                // hover border
    GuiSetStyle(SLIDER, BORDER_COLOR_PRESSED,0x00000088);                // pressed border
    GuiSetStyle(SLIDER, TEXT_COLOR_NORMAL,   0xFFFFFFFF);                // value text
    GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED,  0xFFFFFFFF);                // hover value
    GuiSetStyle(SLIDER, TEXT_COLOR_PRESSED,  0xFFFFFFFF);                // pressed value
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 8);                                // handle width (px)
    GuiSetStyle(SLIDER, BORDER_WIDTH, 1);                                // 1px border

    // ── Label style ─────────────────────────────────────────────
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, 0x000000FF);                   // label text

    int y = 52;                                                          // first section starts below title + underline

    // ═══ SIMULATION ═════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_SIM, (Color){20, 30, 60, 180});     // dark background behind this section
    DrawText("SIMULATION", PAD+4, y, SEC_HDR_Y, WHITE); y += SEC_HDR_H;    // section header text
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, sim->paused ? "RESUME" : "PAUSE"))
        sim->paused = !sim->paused;                                      // flip paused state on click
    y += ELEM_H + ELEM_GAP;                                              // move down past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "RESET"))
        sim->reset_requested = true;                                     // main.c checks this and calls sim_reset()
    y += ELEM_H + SEC_GAP;                                               // move down past button + section gap

    // ═══ FLUID ══════════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_FLUID, (Color){20, 30, 60, 180});         // dark background behind this section
    DrawText("FLUID", PAD+4, y, SEC_HDR_Y, WHITE); y += SEC_HDR_H;               // section header text

    DrawText("Gravity", PAD+4, y, 10, WHITE); y += 16;                           // slider label
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->gravity, 0, 1500);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->gravity, "%.0f", PAD, y); y += VALBOX_H + ELEM_GAP;      // show current gravity as integer

    DrawText("Viscosity", PAD+4, y, 10, WHITE); y += 16;                         // slider label
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->viscosity, 0, 5);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->viscosity, "%.2f", PAD, y); y += VALBOX_H + ELEM_GAP;    // show current viscosity as 2 decimal places

    DrawText("Density", PAD+4, y, 10, WHITE); y += 16;                           // slider label
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->target_density, 50, 800);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->target_density, "%.0f", PAD, y); y += VALBOX_H + ELEM_GAP;// show current density as integer

    DrawText("Size", PAD+4, y, 10, WHITE); y += 16;                              // slider label
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->particle_radius, 3, 16);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->particle_radius, "%.0f", PAD, y); y += VALBOX_H + SEC_GAP;// show current radius as integer

    // ═══ SPAWN COLOR ════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_SPAWN, (Color){20, 30, 60, 180});         // dark background behind this section
    DrawText("SPAWN COLOR", PAD+4, y, SEC_HDR_Y, WHITE); y += SEC_HDR_H;         // section header text
    GuiColorPicker((Rectangle){(float)PAD, (float)y, 170, 95}, NULL, &ui->spawn_color);  // color picker: writes to ui->spawn_color
    y += 95 + SEC_GAP;                                                         // move past picker (95px) + section gap

    // ═══ VISUALIZE ══════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_VIZ, (Color){20, 30, 60, 180});           // dark background behind this section
    DrawText("VISUALIZE", PAD+4, y, SEC_HDR_Y, WHITE); y += SEC_HDR_H;           // section header text
    static int mode_active = 0;                                                // persisted: selected render mode (0=Solid, 1=Velocity, 2=Density)
    static bool mode_open = false;                                             // persisted: true when dropdown list is visible
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&                             // manual open/close: raygui dropdown takes bool by value
        CheckCollisionPointRec(GetMousePosition(), (Rectangle){(float)PAD, (float)y, INNER_W, 26}))
        mode_open = !mode_open;
    GuiDropdownBox((Rectangle){(float)PAD, (float)y, INNER_W, 26}, "Solid;Velocity;Density", &mode_active, mode_open);
    ui->render_mode = mode_active;                                             // push selected mode to renderer
    Color mode_colors[] = { {255,255,255,200}, {0,200,255,200}, {255,100,50,200} }; // accent for dropdown
    DrawRectangle(PAD - 3, y + 3, 3, 20, mode_colors[mode_active]);            // 3px-wide colored bar showing active mode
    y += 26;                                                                   // move past dropdown height
    if (mode_open) y += 80;                                                    // push content down when dropdown list is open
    y += SEC_GAP;                                                              // section gap

    // ═══ OBSTACLES ══════════════════════════════════════════════
    DrawRectangle(10, y - 4, 200, BG_OBS, (Color){20, 30, 60, 180});           // dark background behind this section
    DrawText("OBSTACLES", PAD+4, y, SEC_HDR_Y, WHITE); y += SEC_HDR_H;           // section header text
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Ramp"))  obs_add_preset(obs, PRESET_RAMP);      // spawn ramp
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Box"))   obs_add_preset(obs, PRESET_BOX);       // spawn box
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Funnel")) obs_add_preset(obs, PRESET_FUNNEL);   // spawn funnel
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Clear"))  obs_clear(obs);                       // remove all obstacles
    y += ELEM_H + ELEM_GAP;                                                    // move past button + gap
    GuiToggle((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Draw Walls", &ui->draw_mode);                       // toggle wall-drawing mode

    // ── Particle counter (pinned to bottom) ─────────────────────
    DrawText(TextFormat("Particles: %d/%d", sim->count, MAX_PARTICLES), WINDOW_W - 320, WINDOW_H-40, 30, WHITE);                 // live count moved to right bottom corner 
}
