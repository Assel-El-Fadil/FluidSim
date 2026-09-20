#include "SpatialGrid.hpp"
#include <algorithm>

std::vector<int> gridHead(GRID_DIM * GRID_DIM, -1);
std::vector<int> particleNext(NUM_PARTICLES, -1);

int getCellX(float x) {
    float norm = (x + 2.0f) / 4.0f;
    int cx = (int)(norm * GRID_DIM);
    return clamp_val(cx, 0, GRID_DIM - 1);
}

int getCellY(float y) {
    float norm = (y + 2.0f) / 4.0f;
    int cy = (int)(norm * GRID_DIM);
    return clamp_val(cy, 0, GRID_DIM - 1);
}

int getCellIndex(int cx, int cy) {
    return cy * GRID_DIM + cx;
}

void build_spatial_grid(const std::vector<Particle>& particles) {
    std::fill(gridHead.begin(), gridHead.end(), -1);
    particleNext.resize(particles.size(), -1);
    for (int i = 0; i < (int)particles.size(); i++) {
        int cx = getCellX(particles[i].x);
        int cy = getCellY(particles[i].y);
        int cellIdx = getCellIndex(cx, cy);
        particleNext[i] = gridHead[cellIdx];
        gridHead[cellIdx] = i;
    }
}
