#ifndef THEME_H
#define THEME_H
#include "raylib.h"         // Color

extern bool theme_dark;     // global dark/light flag (theme.c); true = dark, false = light

// Active theme palette, resolved from theme_dark at call time.
typedef struct {
    Color bg;               // main simulation background
    Color panel;            // sidebar background
    Color section;          // section card fill
    Color card;             // raised control / value box
    Color border;           // 1px card border
    Color border_hi;        // hover / focused border
    Color text;             // primary text
    Color dim;              // secondary text
    Color faint;            // hints / disabled
    Color head;             // section header (accent-tinted)
    Color accent;           // active / selected / primary action
    Color accent_dim;       // accent, de-emphasized (borders/fills)
    Color obstacle;         // wall color (visible on bg)
    Color ok;               // success / good FPS
    Color warn;             // warning / mid FPS
    Color danger;           // danger / low FPS
    Color repel;            // repel-mode indicator
} Theme;

Theme theme_get(void);

// Custom fonts (defined in theme.c, loaded via theme_load_fonts()).
extern Font font_title;          // Liber (bake 36) — CASCADE logo
extern Font font_ui;             // Comfortaa (bake 26) — MODE chip, particle counter
extern Font font_ui_small;       // Comfortaa (bake 14) — headers, labels, buttons, value numbers

void theme_load_fonts(void);   // call once after InitWindow
void theme_unload_fonts(void); // call before CloseWindow

#endif