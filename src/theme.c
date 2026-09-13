#include "theme.h"          // include headers for theme
#include <stddef.h>         // NULL
bool theme_dark = true;     // default = dark

static const Theme DARK = { // dark theme palette
    {  7,   9,  26, 255 },  // background
    { 10,  15,  30, 255 },  // sidebar panel backdrop
    { 18,  26,  48, 255 },  // section; sim, fluid, spawn-col, visualize, obs backdrop
    { 24,  34,  60, 255 },  // card; sim 3s, obs ones and numbers
    { 44,  58,  96, 255 },  // border; border on sliders, cards and sidebar separation
    { 70,  92, 140, 255 },  // border_hi; border of sliders when mouse hovers
    {226, 233, 245, 255 },  // text
    {150, 165, 195, 255 },  // dim; helper text on the bottom
    { 92, 106, 138, 255 },  // faint
    {160, 240, 255, 255 },  // section header; sim, fluid, spawn-col, visualize, obs backdrop
    { 34, 211, 238, 255 },  // accent color 
    { 15, 118, 140, 255 },  // accent_dim
    {148, 163, 184, 255 },  // obstacle
    { 52, 211, 153, 255 },  // ok
    {251, 191,  36, 255 },  // warn
    {248, 113, 113, 255 },  // danger
    {251, 146,  60, 255 },  // repel
};

static const Theme LIGHT = {// light theme palette
    {246, 248, 255, 255 },  // background
    {235, 239, 250, 255 },  // sidebar panel backdrop
    {157, 204, 235, 255 },  // section; sim, fluid, spawn-col, visualize, obs backdrop
    {255, 255, 255, 255 },  // card; sim 3s, obs ones and numbers
    {120, 160, 220, 255 },  // border; border on sliders, cards and sidebar separation
    {  0, 140, 255, 255 },  // border_hi; border of sliders when mouse hovers
    {  0,  80, 255, 255 },  // text; 
    { 40,  90, 200, 255 },  // dim; helper text on the bottom
    { 90, 130, 210, 255 },  // faint
    { 20,  50, 255, 255 },  // section header; sim, fluid, spawn-col, visualize, obs backdrop
    {  0, 160, 240, 255 },  // accent color
    { 80, 180, 250, 255 },  // accent_dim
    {255, 140,   0, 255 },  // obstacle
    {  0, 200, 120, 255 },  // ok
    {255, 196,   0, 255 },  // warn
    {255,  60,  60, 255 },  // danger
    {255, 150,   0, 255 },  // repel
};

// Select active theme palette
Theme theme_get(void) {
    return theme_dark ? DARK : LIGHT;
}

Font font_title     = {0};  // Title font not loaded yet
Font font_ui        = {0};  // UI font not loaded yet 
Font font_ui_small  = {0};  // Small UI font not loaded yet

// Baked at/near each font's render size so nothing magnifies and downscale
// is minimal; bilinear and mipmaps so that subtle blur keeps glyphs legible
void theme_load_fonts(void) {
    font_title     = LoadFontEx("assets/fonts/Liber.ttf",     36, NULL, 0);
    font_ui        = LoadFontEx("assets/fonts/Comfortaa.ttf", 26, NULL, 0);
    font_ui_small  = LoadFontEx("assets/fonts/Comfortaa.ttf", 14, NULL, 0);
    GenTextureMipmaps(&font_title.texture);
    GenTextureMipmaps(&font_ui.texture);
    GenTextureMipmaps(&font_ui_small.texture);
    SetTextureFilter(font_title.texture,    TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(font_ui.texture,       TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(font_ui_small.texture, TEXTURE_FILTER_BILINEAR);
}

// Unload fonts 
void theme_unload_fonts(void) {
    UnloadFont(font_title);
    UnloadFont(font_ui);
    UnloadFont(font_ui_small);
}
