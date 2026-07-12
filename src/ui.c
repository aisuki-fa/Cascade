#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "cascade.h"
#include "ui.h"
#include "obstacles.h"

// ── Layout constants ───────────────────────────────────────────
#define PAD         12      // sidebar left/right padding
#define INNER_W     196     // content width (SIDEBAR_W - 2*PAD)
#define SEC_HDR_Y   11      // section header font size
#define SEC_HDR_H   18      // section header + gap below
#define ELEM_H      26      // button / toggle height
#define ELEM_GAP    6       // gap between sibling elements
#define SEC_GAP     10      // gap between sections
#define SLIDER_H    18      // slider track height
#define SLIDER_GAP  4       // gap between slider and value box
#define VALBOX_H    16      // value readout box height
#define VALBOX_W    56      // value readout box width

static void DrawSectionBg(int y0, int y1) {
    DrawRectangle(4, y0 - 4, INNER_W + 8, (y1 - y0) + 12, (Color){0, 0, 0, 35});
}

static void DrawSeparator(int y) {
    DrawLine(PAD, y, PAD + INNER_W, y, (Color){0, 0, 0, 50});
}

static void DrawValueBox(float value, const char* fmt, int x, int y) {
    Rectangle box = {(float)x, (float)y, (float)VALBOX_W, (float)VALBOX_H};
    GuiDrawRectangle(box, 1, (Color){255, 255, 255, 40}, (Color){0, 0, 0, 60});
    DrawText(TextFormat(fmt, value), x + 4, y + 2, 11, WHITE);
}

void ui_draw_sidebar(SimState* sim, UIState* ui, ObstacleList* obs) {
    // ── Sidebar background + border ─────────────────────────────
    DrawRectangle(0, 0, SIDEBAR_W, WINDOW_H, (Color){155, 82, 119, 255});
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H, (Color){25, 40, 72, 255});

    // ── Title ───────────────────────────────────────────────────
    DrawText("CASCADE", PAD, 12, 28, BLACK);
    DrawRectangle(PAD, 44, 70, 2, (Color){255, 255, 255, 120});

    // ── Button style ────────────────────────────────────────────
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,   0xFFFFFF55);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED,  0xFFFFFF88);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED,  0x00000066);
    GuiSetStyle(BUTTON, BASE_COLOR_DISABLED, 0x9B527744);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000033);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED,0x00000066);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED,0x00000088);
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,   0x000000FF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED,  0x000000FF);
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED,  0xFFFFFFFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_DISABLED, 0x00000088);
    GuiSetStyle(BUTTON, TEXT_SIZE, 10);
    GuiSetStyle(BUTTON, TEXT_SPACING, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, 0);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);

    // ── Slider style ────────────────────────────────────────────
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL,   0x00000044);
    GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED,  0x00000066);
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED,  0xFFFFFF88);
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0x00000033);
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED,0x00000055);
    GuiSetStyle(SLIDER, BORDER_COLOR_PRESSED,0x00000088);
    GuiSetStyle(SLIDER, TEXT_COLOR_NORMAL,   0xFFFFFFFF);
    GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED,  0xFFFFFFFF);
    GuiSetStyle(SLIDER, TEXT_COLOR_PRESSED,  0xFFFFFFFF);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 8);
    GuiSetStyle(SLIDER, BORDER_WIDTH, 1);

    // ── Label style ─────────────────────────────────────────────
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, 0x000000FF);

    int y = 52;
    int sec_start;

    // ═══ SIMULATION ═════════════════════════════════════════════
    sec_start = y;
    DrawText("SIMULATION", PAD, y, SEC_HDR_Y, BLACK); y += SEC_HDR_H;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, sim->paused ? "RESUME" : "PAUSE"))
        sim->paused = !sim->paused;
    y += ELEM_H + ELEM_GAP;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "RESET"))
        sim->reset_requested = true;
    y += ELEM_H;
    DrawSectionBg(sec_start, y);
    DrawSeparator(y + SEC_GAP / 2);
    y += SEC_GAP;

    // ═══ FLUID ══════════════════════════════════════════════════
    sec_start = y;
    DrawText("FLUID", PAD, y, SEC_HDR_Y, BLACK); y += SEC_HDR_H;

    // Gravity
    DrawText("Gravity", PAD, y, 10, BLACK); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->gravity, 0, 1500);
    y += SLIDER_H + SLIDER_GAP;
    DrawValueBox(sim->gravity, "%.0f", PAD, y); y += VALBOX_H + ELEM_GAP;

    // Viscosity
    DrawText("Viscosity", PAD, y, 10, BLACK); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->viscosity, 0, 5);
    y += SLIDER_H + SLIDER_GAP;
    DrawValueBox(sim->viscosity, "%.2f", PAD, y); y += VALBOX_H + ELEM_GAP;

    // Density
    DrawText("Density", PAD, y, 10, BLACK); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->target_density, 50, 800);
    y += SLIDER_H + SLIDER_GAP;
    DrawValueBox(sim->target_density, "%.0f", PAD, y); y += VALBOX_H + ELEM_GAP;

    // Size
    DrawText("Size", PAD, y, 10, BLACK); y += 16;
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->particle_radius, 3, 16);
    y += SLIDER_H + SLIDER_GAP;
    DrawValueBox(sim->particle_radius, "%.0f", PAD, y); y += VALBOX_H;
    DrawSectionBg(sec_start, y);
    DrawSeparator(y + SEC_GAP / 2);
    y += SEC_GAP;

    // ═══ SPAWN COLOR ════════════════════════════════════════════
    sec_start = y;
    DrawText("SPAWN COLOR", PAD, y, SEC_HDR_Y, BLACK); y += SEC_HDR_H;
    GuiColorPicker((Rectangle){(float)PAD, (float)y, 170, 95}, NULL, &ui->spawn_color);
    y += 95;
    DrawSectionBg(sec_start, y);
    DrawSeparator(y + SEC_GAP / 2);
    y += SEC_GAP;

    // ═══ VISUALIZE ══════════════════════════════════════════════
    sec_start = y;
    DrawText("VISUALIZE", PAD, y, SEC_HDR_Y, BLACK); y += SEC_HDR_H;
    static int mode_active = 0;
    static bool mode_open = false;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), (Rectangle){(float)PAD, (float)y, INNER_W, 26}))
        mode_open = !mode_open;
    GuiDropdownBox((Rectangle){(float)PAD, (float)y, INNER_W, 26}, "Solid;Velocity;Density", &mode_active, mode_open);
    ui->render_mode = mode_active;
    Color mode_colors[] = { {255,255,255,200}, {0,200,255,200}, {255,100,50,200} };
    DrawRectangle(PAD - 3, y + 3, 3, 20, mode_colors[mode_active]);
    y += 26;
    if (mode_open) y += 80;
    DrawSectionBg(sec_start, y);
    DrawSeparator(y + SEC_GAP / 2);
    y += SEC_GAP;

    // ═══ OBSTACLES ══════════════════════════════════════════════
    sec_start = y;
    DrawText("OBSTACLES", PAD, y, SEC_HDR_Y, BLACK); y += SEC_HDR_H;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Ramp"))  obs_add_preset(obs, PRESET_RAMP);
    y += ELEM_H + ELEM_GAP;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Box"))   obs_add_preset(obs, PRESET_BOX);
    y += ELEM_H + ELEM_GAP;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Funnel")) obs_add_preset(obs, PRESET_FUNNEL);
    y += ELEM_H + ELEM_GAP;
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Clear"))  obs_clear(obs);
    y += ELEM_H + ELEM_GAP;
    GuiToggle((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Draw Walls", &ui->draw_mode);
    y += ELEM_H;
    DrawSectionBg(sec_start, y);

    // ── Particle counter (pinned to bottom) ─────────────────────
    DrawText(TextFormat("Particles: %d / %d", sim->count, MAX_PARTICLES),
             PAD, WINDOW_H - 24, 10, (Color){255, 255, 255, 180});
}