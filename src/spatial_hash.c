#include "spatial_hash.h"

// ─────────────────────────────────────────────────────────────
// sh_build
//
// Rebuilds the spatial hash from the current particle positions.
// Must be called once per frame, BEFORE sh_query is used, since
// particle positions change every frame.
//
// Two passes over the particle array:
//   1. Cell assignment  — count how many particles fall in each cell.
//   2. Counting sort    — use a prefix sum to pack particle indices
//                          into sorted_ids[], grouped by cell.
// ─────────────────────────────────────────────────────────────
void sh_build(SpatialHash* sh, Particle* p, int count) {
    int sim_width  = WINDOW_W;  // playable area excludes the sidebar
    int sim_height = WINDOW_H;

    sh->grid_w    = (sim_width  / CELL_SIZE) + 1;
    sh->grid_h    = (sim_height / CELL_SIZE) + 1;
    sh->cell_size = CELL_SIZE;

    int total_cells = sh->grid_w * sh->grid_h;

    // Reset cell counts before recounting this frame.
    for (int i = 0; i < total_cells; i++) {
        sh->cell_count[i] = 0;
    }

    // ── PART 1: cell assignment ──
    for (int i = 0; i < count; i++) {
        int cell_x = (int)(p[i].pos.x / sh->cell_size);
        int cell_y = (int)(p[i].pos.y / sh->cell_size);

        if (cell_x < 0) cell_x = 0;
        if (cell_x >= sh->grid_w) cell_x = sh->grid_w - 1;
        if (cell_y < 0) cell_y = 0;
        if (cell_y >= sh->grid_h) cell_y = sh->grid_h - 1;

        int cell_id = cell_x + cell_y * sh->grid_w;
        sh->cell_count[cell_id]++;
    }

    // ── PART 2: counting sort ──
    sh->cell_start[0] = 0;
    for (int i = 1; i < total_cells; i++) {
        sh->cell_start[i] = sh->cell_start[i - 1] + sh->cell_count[i - 1];
    }

    static int pos[GRID_CELLS];
    for (int i = 0; i < total_cells; i++) {
        pos[i] = sh->cell_start[i];
    }

    for (int i = 0; i < count; i++) {
        int cell_x = (int)(p[i].pos.x / sh->cell_size);
        int cell_y = (int)(p[i].pos.y / sh->cell_size);

        if (cell_x < 0) cell_x = 0;
        if (cell_x >= sh->grid_w) cell_x = sh->grid_w - 1;
        if (cell_y < 0) cell_y = 0;
        if (cell_y >= sh->grid_h) cell_y = sh->grid_h - 1;

        int cell_id = cell_x + cell_y * sh->grid_w;
        sh->sorted_ids[pos[cell_id]] = i;
        pos[cell_id]++;
    }
}

// ─────────────────────────────────────────────────────────────
// sh_query
//
// Given a position (x, y), finds all particles in the surrounding
// 3x3 block of grid cells and writes their indices into out_ids.
// ─────────────────────────────────────────────────────────────
void sh_query(SpatialHash* sh, float x, float y, int* out_ids, int* out_count, int max_out) {
    *out_count = 0;

    int cx = (int)(x / sh->cell_size);
    int cy = (int)(y / sh->cell_size);

    for (int iy = cy - 1; iy <= cy + 1; iy++) {
        for (int ix = cx - 1; ix <= cx + 1; ix++) {
            if (ix < 0 || ix >= sh->grid_w || iy < 0 || iy >= sh->grid_h) {
                continue;
            }

            int cell_id = ix + iy * sh->grid_w;
            int start   = sh->cell_start[cell_id];
            int end     = start + sh->cell_count[cell_id];

            for (int j = start; j < end; j++) {
                if (*out_count >= max_out) {
                    return;
                }
                out_ids[*out_count] = sh->sorted_ids[j];
                (*out_count)++;
            }
        }
    }
}