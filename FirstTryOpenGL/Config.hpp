#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cmath>

// Global Constants
const float BALL_RADIUS = 0.010f;
const float PI = 3.14159265358979323846f;
const float SCREENWIDTH = 800.0f;
const float SCREENHEIGHT = 800.0f;

const int NUM_PARTICLES = 800;

// Spatial Grid Constants
const int GRID_DIM = 90;

// Data Structures
struct Particle {
    float x = 0.0f, y = 0.0f;
    float vx = 0.0f, vy = 0.0f;
};

struct Force {
    float fx = 0.0f, fy = 0.0f;
};

// Math Helper
template <typename T>
inline T clamp_val(T val, T low, T high) {
    return (val < low) ? low : ((val > high) ? high : val);
}

#endif // CONFIG_HPP
