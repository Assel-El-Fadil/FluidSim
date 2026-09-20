#include "SphKernels.hpp"

float densitySmoothingKernel(float h, float r) {
    if (r >= h) return 0.0f;
    float h2 = h * h;
    float r2 = r * r;
    float diff = h2 - r2;
    float scale = 4.0f / (PI * powf(h, 8.0f));
    return scale * diff * diff * diff;
}

float pressureGradMagnitude(float h, float r) {
    if (r >= h || r < 1e-6f) return 0.0f;
    float scale = -30.0f / (PI * powf(h, 5.0f));
    float diff = h - r;
    return scale * diff * diff;
}

float laplacianViscosityKernel(float h, float r) {
    if (r >= h) return 0.0f;
    float scale = 20.0f / (PI * powf(h, 5.0f));
    return scale * (h - r);
}
