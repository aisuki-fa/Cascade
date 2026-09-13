#define RAYGUI_IMPLEMENTATION             // enables raygui function bodies (only in this .c file)
#include "raygui.h"                       // GuiButton, GuiSlider, GuiColorPicker etc.
#include "cascade.h"                      // SimState, UIState, SIDEBAR_W, WINDOW_H, etc.
#include "ui.h"                           // declares ui_draw_sidebar()
#include "obstacles.h"                    // obs_add_preset, obs_clear, PRESET_RAMP/BOX/FUNNEL

// ── Layout constants (px) ───────────────────────────────────────────────────
#define PAD         12                    // left/right padding inside sidebar 
#define INNER_W     196                   // content width: SIDEBAR_W - 2*PAD
#define SEC_HDR_H   18                    // section header height + gap below
#define ELEM_H      26                    // button / toggle height 
#define ELEM_GAP    6                     // gap between sibling elements in a section
#define SEC_GAP     10                    // gap between sections
#define SLIDER_H    18                    // slider track height 
#define SLIDER_GAP  4                     // gap between slider and its value box
#define LABEL_GAP   16                    // vertical space for the slider label above each slider
#define VALBOX_H    16                    // value readout box height 
#define VALBOX_W    56                    // value readout box width 
#define BTN_TRPL (INNER_W - 2*ELEM_GAP)/3 // width of each button in a 3-across row 
#define PICKER_W    170                   // color picker grid width 
#define PICKER_H    95                    // color picker height (BG_SPAWN - SEC_HDR_H - 4_offset_for_header_padding)

// ── Section background heights (px) ─────────────────────────────────────────
#define BG_SIM      80                    // SIMULATION  : Header + 3 buttons (pause/reset/theme) + Fluidity
#define BG_FLUID    256                   // FLUID       : Header + 4 sliders + value boxes 
#define BG_SPAWN    117                   // SPAWN COLOR : Header + color picker 
#define BG_VIZ      48                    // VISUALIZE   : Header + 3 buttons (solid/velocity/density) 
#define BG_OBS      208                   // OBSTACLES   : Header + 4 buttons (ramp/box/funnel/clear) + Draw Walls + 2 shape buttons

// Convert raylib color in struct format {r,g,b,a} to raygui's 0xAABBGGRR int format
// a<<24 → 0xAA__, r<<16 → __RR__, g<<8 → ____GG, b stays in byte 0
static unsigned ColorToGui(Color c) {
    return ((unsigned)c.a << 24) | ((unsigned)c.r << 16) | ((unsigned)c.g << 8) | (unsigned)c.b;
}

// Draws the value box with a numeric value inside for the sliders
static void DrawValueBox(float value, const char* fmt, int x, int y, Theme t) {  // value of slider, printf format str, position (x,y), theme colors
    Rectangle box = {(float)x, (float)y, (float)VALBOX_W, (float)VALBOX_H};      // rectangle from position(x,y) + fixed size(VALBOX_W * VALBOX_H)
    GuiDrawRectangle(box, 1, t.border, t.card);                                  // 1px border in t.border, fill in t.card
    DrawTextEx(font_ui, TextFormat(fmt, value), (Vector2){ (float)(x + 4), (float)(y + 3) }, 11, 1, t.text); // txt at (x+4,y+3),11px,spacing 1,t.text
}

// Draws the entire left sidebar: background, title, all control sections
void ui_draw_sidebar(SimState* sim, UIState* ui, ObstacleList* obs) {
    Theme t = theme_get();                                                     // determine active palette for this frame
    GuiSetFont(font_ui_small);                                                 // font for small controls: headers, labels, buttons, values

    // ── Sidebar background + border ─────────────────────────────────────────
    DrawRectangle(0, 0, SIDEBAR_W, WINDOW_H, t.panel);                         // sidebar start(x,y) ,SIDEBAR_W * WINDOW_H with t.panel fill
    DrawLine(SIDEBAR_W, 0, SIDEBAR_W, WINDOW_H, t.border);                     // divider between sidebar and sim area; coordinates w color

    // ── Custom Title ────────────────────────────────────────────────────────
    DrawTextEx(font_title, "CASCADE", (Vector2){ (float)PAD, 8.0f }, 34, 2, t.accent); // font_title, at (PAD,8), 34px, spacing 2, accent color
    DrawRectangle(PAD, 46, INNER_W, 2, t.accent);                             // accent underline below title and above simulation header

    // ── Button style ────────────────────────────────────────────────────────
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,   ColorToGui(t.card));              // normal
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED,  ColorToGui(t.accent_dim));        // hovered
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED,  ColorToGui(t.accent));            // clicked
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToGui(t.border));            // normal border
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED,ColorToGui(t.accent_dim));        // hover border
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED,ColorToGui(t.accent));            // pressed border
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,   ColorToGui(t.text));              // normal text
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED,  ColorToGui(t.text));              // hover text
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED,  ColorToGui(t.bg));                // pressed text (contrast on accent)
    GuiSetStyle(DEFAULT,TEXT_SIZE, 11);                                        // button text size (px)
    GuiSetStyle(BUTTON, TEXT_SPACING, 1);                                      // letter spacing (px)
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);                    // Centre-aligned text
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);                                      // 1px border

    // ── Slider style ────────────────────────────────────────────────────────
    GuiSetStyle(SLIDER, BASE_COLOR_NORMAL,   ColorToGui((Color){t.section.r, t.section.g, t.section.b, 255})); // forcing a-255 else (t.section)
    GuiSetStyle(SLIDER, BASE_COLOR_PRESSED,  ColorToGui(t.accent));            // idle knob color
    GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED,  ColorToGui(t.accent_dim));        // knob on hover (raygui reuses TEXT slot for the knob)
    GuiSetStyle(SLIDER, TEXT_COLOR_PRESSED,  ColorToGui(t.accent));            // knob while dragging (raygui reuses TEXT slot for the knob)
    GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, ColorToGui(t.border));            // normal border
    GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED,ColorToGui(t.border_hi));         // hover border
    GuiSetStyle(SLIDER, BORDER_COLOR_PRESSED,ColorToGui(t.accent));            // pressed border
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 8);                                      // handle width
    GuiSetStyle(SLIDER, BORDER_WIDTH, 1);                                      // 1px border

    int y = 56;                                                                // first section starts below title + underline

    // ═══ SIMULATION ═══════════════════════════════════════════════════════════════
    // background behind this section; from (PAD,y-4), INNER_W * BG_SIM, fill t.section
    DrawRectangle(PAD, y - 4, INNER_W, BG_SIM, t.section);                        
    
    // section header text at (PAD+4,y), 14px, spacing 2, t.head color
    DrawTextEx(font_ui, "SIMULATION", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head); 

    // space down to the first row of buttons, accounts for the header height + gap below it
    y += SEC_HDR_H; 

    // 3-across row of buttons as PAUSE | RESET | THEME ; draws, labels and handles clicks
    if (GuiButton((Rectangle){(float)PAD, (float)y, BTN_TRPL, ELEM_H}, sim->paused ? "RESUME" : "PAUSE"))
        sim->paused = !sim->paused;       // toggle paused state on click
    if (GuiButton((Rectangle){(float)(PAD+BTN_TRPL+ELEM_GAP), (float)y, BTN_TRPL, ELEM_H}, "RESET"))
        sim->reset_requested = true;      // main.c checks this and calls sim_reset()
    if (GuiButton((Rectangle){(float)(PAD+2*(BTN_TRPL+ELEM_GAP)), (float)y, BTN_TRPL, ELEM_H}, theme_dark ? "Dark" : "Light"))
        theme_dark = !theme_dark;         // toggle dark/light theme

    // move down past the row of buttons + gap to the next element
    y += ELEM_H + ELEM_GAP;

    // Fluidity toggle button (marching squares overlay)
    Rectangle grd = {(float)PAD, (float)y, INNER_W, ELEM_H};                   // start at (PAD,y) with size INNER_W * ELEM_H
    if (GuiButton(grd, "Fluidity")) ui->show_ms = !ui->show_ms;                // toggle overlay
    if (ui->show_ms) DrawRectangleLinesEx(grd, 2.0f, t.accent);                // accent ring when armed or active

    // move down past the button + gap to the next section
    y += ELEM_H + SEC_GAP;
    
    // ═══ FLUID ════════════════════════════════════════════════════════════════════
    // background behind this section; from (PAD,y-4), INNER_W * BG_FLUID, fill t.section
    DrawRectangle(PAD, y - 4, INNER_W, BG_FLUID, t.section);    
    
    // section header text at (PAD+4,y), 14px, spacing 2, t.head color
    DrawTextEx(font_ui, "FLUID", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head);

    // move down past the header + gap to the first slider
    y += SEC_HDR_H;

    // Gravity slider at (PAD,y), INNER_W * SLIDER_H with 12px label above it, spacing 1, t.text color
    DrawTextEx(font_ui, "Gravity", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text); 
    y += LABEL_GAP;                                                            // move down past the label to the slider track
    // slider track: from (PAD,y), INNER_W * SLIDER_H, no text on left/right, value is sim->gravity, range 0–1500
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->gravity, 0, 1500); 
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->gravity, "%.0f", PAD, y, t);                             // show current gravity as integer
    y += VALBOX_H + ELEM_GAP;                                                  // move past value box + gap

    // Viscosity slider at (PAD,y), INNER_W * SLIDER_H with 12px label above it, spacing 1, t.text color
    DrawTextEx(font_ui, "Viscosity", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text);
    y += LABEL_GAP;                                                            // move down past the label to the slider track
    // slider track: from (PAD,y), INNER_W * SLIDER_H, no text on left/right, value is sim->viscosity, range 0–5
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->viscosity, 0, 5);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->viscosity, "%.2f", PAD, y, t);                           // show current viscosity as 2 decimals
    y += VALBOX_H + ELEM_GAP;                                                  // move past value box + gap

    // Density slider at (PAD,y), INNER_W * SLIDER_H with 12px label above it, spacing 1, t.text color
    DrawTextEx(font_ui, "Density", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text);
    y += LABEL_GAP;                                                            // move down past the label to the slider track
    // slider track: from (PAD,y), INNER_W * SLIDER_H, no text on left/right, value is sim->target_density, range 50–800
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->target_density, 50, 800);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->target_density, "%.0f", PAD, y, t);                      // show current density as integer
    y += VALBOX_H + ELEM_GAP;                                                  // move past value box + gap

    // Size slider at (PAD,y), INNER_W * SLIDER_H with 12px label above it, spacing 1, t.text color
    DrawTextEx(font_ui, "Size", (Vector2){ (float)(PAD+4), (float)y }, 12, 1, t.text);
    y += LABEL_GAP;                                                            // move down past the label to the slider track
    // slider track: from (PAD,y), INNER_W * SLIDER_H, no text on left/right, value is sim->particle_radius, range 3–16
    GuiSlider((Rectangle){(float)PAD, (float)y, INNER_W, SLIDER_H}, NULL, NULL, &sim->particle_radius, 3, 16);
    y += SLIDER_H + SLIDER_GAP;                                                // move past slider track + gap
    DrawValueBox(sim->particle_radius, "%.0f", PAD, y, t);                     // show current radius as integer
    y += VALBOX_H + SEC_GAP;                                                   // move past value box + section gap

    // ═══ SPAWN COLOR ══════════════════════════════════════════════════════════════
    // background behind this section; from (PAD,y-4), INNER_W * BG_SPAWN, fill t.section
    DrawRectangle(PAD, y - 4, INNER_W, BG_SPAWN, t.section);                        

    // section header text at (PAD+4,y), 14px, spacing 2, t.head color
    DrawTextEx(font_ui, "SPAWN COLOR", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head);

    // move down past the header + gap to the color picker
    y += SEC_HDR_H;

    // color picker: from (PAD,y), 170 * 95, no label, writes to ui->spawn_color
    GuiColorPicker((Rectangle){(float)PAD, (float)y, PICKER_W, PICKER_H}, NULL, &ui->spawn_color); 

    // move down past the color picker + gap to the next section
    y += PICKER_H + SEC_GAP;                                                         

    // ═══ VISUALIZE ════════════════════════════════════════════════════════════════
    // background behind this section; from (PAD,y-4), INNER_W * BG_VIZ, fill t.section
    DrawRectangle(PAD, y - 4, INNER_W, BG_VIZ, t.section);                      

    // section header text at (PAD+4,y), 14px, spacing 2, t.head color
    DrawTextEx(font_ui, "VISUALIZE", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head);

    // move down past the header + gap to the first row of buttons
    y += SEC_HDR_H;

    // read active mode (rendering persists via ui->render_mode)
    RenderMode mode = ui->render_mode;                                   
    // select mode on click after drawing and labeling
    if (GuiButton((Rectangle){(float)PAD, (float)y, BTN_TRPL, ELEM_H}, "Solid"))   mode = RENDER_SOLID;
    if (GuiButton((Rectangle){(float)(PAD+BTN_TRPL+ELEM_GAP), (float)y, BTN_TRPL, ELEM_H}, "Velocity")) mode = RENDER_VELOCITY;
    if (GuiButton((Rectangle){(float)(PAD+2*(BTN_TRPL+ELEM_GAP)), (float)y, BTN_TRPL, ELEM_H}, "Density")) mode = RENDER_DENSITY;
    // Assign mode
    ui->render_mode = mode;
    // calc x position of the active mode button for drawing the accent ring around it
    float ax = PAD + mode*(BTN_TRPL+ELEM_GAP);            
    // Accent ring around the active mode button, in its mode color
    DrawRectangleLinesEx((Rectangle){ax, (float)y, BTN_TRPL, ELEM_H}, 2.0f, mode==RENDER_SOLID ? t.text : mode==RENDER_VELOCITY ? t.accent : t.repel);
    
    // move down past the row of buttons + gap to the next section
    y += ELEM_H + SEC_GAP;                                                           

    // ═══ OBSTACLES ════════════════════════════════════════════════════════════════
    // background behind this section; from (PAD,y-4), INNER_W * BG_OBS, fill t.section
    DrawRectangle(PAD, y - 4, INNER_W, BG_OBS, t.section);                          

    // section header text at (PAD+4,y), 14px, spacing 2, t.head color
    DrawTextEx(font_ui, "OBSTACLES", (Vector2){ (float)(PAD+4), (float)y }, 14, 2, t.head);

    // move down past the header + gap to the first row of buttons
    y += SEC_HDR_H;

    // 4-across row of buttons as RAMP | BOX | FUNNEL | CLEAR ; draws, labels and handles clicks
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Ramp"))  obs_add_preset(obs, PRESET_RAMP);      // spawn ramp
    y += ELEM_H + ELEM_GAP;                                                                                            // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Box"))   obs_add_preset(obs, PRESET_BOX);       // spawn box
    y += ELEM_H + ELEM_GAP;                                                                                            // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Funnel")) obs_add_preset(obs, PRESET_FUNNEL);   // spawn funnel
    y += ELEM_H + ELEM_GAP;                                                                                            // move past button + gap
    if (GuiButton((Rectangle){(float)PAD, (float)y, INNER_W, ELEM_H}, "Clear"))  obs_clear(obs);                       // remove all obstacles
    y += ELEM_H + ELEM_GAP;                                                                                            // move past button + gap

    Rectangle dw = {(float)PAD, (float)y, INNER_W, ELEM_H};   // Draw Walls toggle button (draw mode on/off) at (PAD,y), INNER_W * ELEM_H 
    if (GuiButton(dw, "Draw Walls")) {                        // if clicked drawed rect dw with label "Draw Walls"
        ui->draw_mode = !ui->draw_mode;                       // toggle draw mode on/off
        obs->drop_shape = DROP_NONE;                          // draw mode is 0 or DROP_NONE, so that shape-drop tool is off when draw mode is on
    }
    if (ui->draw_mode)                                        // draw mode active, will have accent ring overlay
        DrawRectangleLinesEx(dw, 2.0f, t.accent);             // accent ring around the draw walls button when draw mode is on

    // move down past the toggle + gap to the next row of buttons
    y += ELEM_H + ELEM_GAP;                                                   

    // 2-across row of buttons as CIRCLE | RECT ; draws, labels and handles clicks
    Rectangle cir = {(float)PAD, (float)y, INNER_W/2 - 3, ELEM_H};                       // circle-shape tool button rect
    Rectangle rec = {(float)(PAD + INNER_W/2 + 3), (float)y, INNER_W/2 - 3, ELEM_H};     // rect-shape tool button rect
    if (GuiButton(cir, "Circle")) {
        obs->drop_shape = (obs->drop_shape == DROP_CIRCLE) ? DROP_NONE : DROP_CIRCLE;    // click again to toggle back to spawning
        ui->draw_mode = false;                                                           // a shape tool on turns wall mode off
    }
    if (obs->drop_shape == DROP_CIRCLE)                                                  // it is circle  
        DrawRectangleLinesEx(cir, 2.0f, t.accent);                                       // accent ring overlay
    if (GuiButton(rec, "Rect")) {
        obs->drop_shape = (obs->drop_shape == DROP_RECT) ? DROP_NONE : DROP_RECT;        // click again to toggle back to spawning
        ui->draw_mode = false;                                                           // a shape tool on turns wall mode off
    }
    if (obs->drop_shape == DROP_RECT)                                                    // it is rect
        DrawRectangleLinesEx(rec, 2.0f, t.accent);                                       // accent ring overlay

    // ── Interaction mode (pinned near particle counter) ──────────────────────
    // set font to the custom UI font for the mode label
    GuiSetFont(font_ui);                  
    // For custom font, we calc the width of the text to right-align it, then draw it with DrawTextEx() 
    float w_mode = MeasureTextEx(font_ui, ui->mouse_repel ? "MODE: REPEL" : "MODE: ATTRACT", 18, 1).x; // font 18, spacing 1
    // Draw the mode label at (WINDOW_W - 16 - w_mode, WINDOW_H - 70), font size 18, spacing 1, color based on mode
    DrawTextEx(font_ui, ui->mouse_repel ? "MODE: REPEL" : "MODE: ATTRACT",
           (Vector2){ (float)(WINDOW_W - 16 - w_mode), (float)(WINDOW_H - 70) }, 18, 1,
           ui->mouse_repel ? t.repel : t.accent);

    // ── Particle counter (pinned to bottom-right) ───────────────────────────
    // For custom font, we calc the width of the text to right-align it, then draw it with DrawTextEx()
    float w_count = MeasureTextEx(font_ui, TextFormat("Particles: %d/%d", sim->count, MAX_PARTICLES), 26, 1).x;
    // Draw the particle counter at (WINDOW_W - 16 - w_count, WINDOW_H - 40), font size 26, spacing 1, t.text color
    DrawTextEx(font_ui, TextFormat("Particles: %d/%d", sim->count, MAX_PARTICLES),
           (Vector2){ (float)(WINDOW_W - 16 - w_count), (float)(WINDOW_H - 40) }, 26, 1, t.text);
}
