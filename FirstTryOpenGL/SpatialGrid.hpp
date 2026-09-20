#ifndef SPATIAL_GRID_HPP
#define SPATIAL_GRID_HPP

#include "Config.hpp"
#include <vector>

// Expose Spatial Grid Data
extern std::vector<int> gridHead;
extern std::vector<int> particleNext;

// Spatial Grid Mapping & Hashing Functions
int getCellX(float x);
int getCellY(float y);
int getCellIndex(int cx, int cy);
void build_spatial_grid(const std::vector<Particle>& particles);

#endif // SPATIAL_GRID_HPP
