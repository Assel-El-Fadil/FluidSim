#ifndef SPH_KERNELS_HPP
#define SPH_KERNELS_HPP

#include "Config.hpp"

// 2D SPH Smoothing Kernels
float densitySmoothingKernel(float h, float r);
float pressureGradMagnitude(float h, float r);
float laplacianViscosityKernel(float h, float r);

#endif // SPH_KERNELS_HPP
