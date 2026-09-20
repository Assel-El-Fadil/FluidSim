#include "FluidSimulation.hpp"
#include "SphKernels.hpp"
#include "SpatialGrid.hpp"
#include <algorithm>
#include <cmath>
#include <omp.h>

// Window State
float gWindowWidth = SCREENWIDTH;
float gWindowHeight = SCREENHEIGHT;
float gAspect = 1.0f;

// Physics Parameters
float gravity = -9.81f;           // Gravity acceleration (m/s^2)
float RAD = 0.045f;              // SPH Smoothing radius (h)
float MASS = 0.85f;              // Particle mass
float targetDensity = 1000.0f;   // Rest density (rho_0)
float pressureStiffness = 140.0f;// Gas constant (k)
float viscosityMultiplier = 0.01f;// Dynamic viscosity (mu)
float cohesionMultiplier = 0.08f; // Cohesion / Surface tension strength

// Interactive State
bool isPaused = false;
bool isMouseDown = false;
int mouseButton = -1;
float mouseX = 0.0f;
float mouseY = 0.0f;

// Particle Vectors
std::vector<Particle> particles;
std::vector<float> densities;
std::vector<Force> forces;

// Bounds functions
float leftBound()   { return (gAspect >= 1.0f) ? -gAspect : -1.0f; }
float rightBound()  { return (gAspect >= 1.0f) ?  gAspect :  1.0f; }
float bottomBound() { return (gAspect < 1.0f)  ? -1.0f / gAspect : -1.0f; }
float topBound()    { return (gAspect < 1.0f)  ?  1.0f / gAspect :  1.0f; }

// --- Scene Initialization ---
void init_scene(int numParticles) {
    particles.clear();
    particles.reserve(numParticles);

    int cols = 28;
    float spacing = BALL_RADIUS * 2.8f;
    float startX = -((cols * spacing) / 2.0f);
    float startY = 0.2f;

    for (int i = 0; i < numParticles; i++) {
        int col = i % cols;
        int row = i / cols;

        Particle p;
        p.x = startX + col * spacing;
        p.y = startY + row * spacing;
        p.vx = 0.0f;
        p.vy = 0.0f;

        particles.push_back(p);
    }
}

// -- Init scene with two separate blocks of particles --
void init_scene_two_blocks(int numParticles) {
    particles.clear();
    particles.reserve(numParticles);

	// left block
    int cols = 14;
    float spacing = BALL_RADIUS * 2.8f;
    float startY = 0.2f;
    float startX = -0.9f;
    for (int i = 0; i < numParticles / 2; i++) {
        int col = i % cols;
        int row = i / cols;

        Particle p;
        p.x = startX + col * spacing;
        p.y = startY + row * spacing;
        p.vx = 0.0f;
        p.vy = 0.0f;

        particles.push_back(p);
    }
    // right block
    startX = 0.9f - (cols * spacing);
    for (int i = 0; i < numParticles / 2; i++) {
        int col = i % cols;
        int row = i / cols;
        Particle p;
        p.x = startX + col * spacing;
        p.y = startY + row * spacing;
        p.vx = 0.0f;
        p.vy = 0.0f;
        particles.push_back(p);
    }
}

// --- Density Computation ---
void compute_densities() {
    densities.resize(particles.size());
    
    #pragma omp parallel for
    for (int i = 0; i < (int)particles.size(); i++) {
        float density = 0.0f;
        float px = particles[i].x;
        float py = particles[i].y;

        int cx = getCellX(px);
        int cy = getCellY(py);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int ncx = cx + dx;
                int ncy = cy + dy;
                if (ncx < 0 || ncx >= GRID_DIM || ncy < 0 || ncy >= GRID_DIM) continue;

                int cellIdx = getCellIndex(ncx, ncy);
                int j = gridHead[cellIdx];
                while (j != -1) {
                    float rx = px - particles[j].x;
                    float ry = py - particles[j].y;
                    float dist = sqrtf(rx * rx + ry * ry);
                    if (dist < RAD) {
                        density += MASS * densitySmoothingKernel(RAD, dist);
                    }
                    j = particleNext[j];
                }
            }
        }
        densities[i] = std::max(density, targetDensity * 0.1f);
    }
}

// --- Forces Computation ---
void compute_forces() {
    forces.assign(particles.size(), { 0.0f, 0.0f });

    #pragma omp parallel for
    for (int i = 0; i < (int)particles.size(); i++) {
        Particle& pi = particles[i];
        float rho_i = densities[i];
        float P_i = std::max(0.0f, pressureStiffness * (rho_i - targetDensity));

        float fx = 0.0f;
        float fy = 0.0f;

        int cx = getCellX(pi.x);
        int cy = getCellY(pi.y);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int ncx = cx + dx;
                int ncy = cy + dy;
                if (ncx < 0 || ncx >= GRID_DIM || ncy < 0 || ncy >= GRID_DIM) continue;

                int cellIdx = getCellIndex(ncx, ncy);
                int j = gridHead[cellIdx];
                while (j != -1) {
                    if (i != j) {
                        Particle& pj = particles[j];
                        float rx = pi.x - pj.x;
                        float ry = pi.y - pj.y;
                        float r = sqrtf(rx * rx + ry * ry);

                        if (r > 1e-6f && r < RAD) {
                            float nx = rx / r;
                            float ny = ry / r;

                            float rho_j = densities[j];
                            float P_j = std::max(0.0f, pressureStiffness * (rho_j - targetDensity));

                            // 1. SPH Pressure Force (Repulsion)
                            float grad = pressureGradMagnitude(RAD, r); // < 0
                            float p_coeff = -MASS * (P_i / (rho_i * rho_i) + P_j / (rho_j * rho_j)) * grad;
                            fx += p_coeff * nx;
                            fy += p_coeff * ny;

                            // 2. Viscosity Force (Relative velocity damping)
                            float lap = laplacianViscosityKernel(RAD, r);
                            float v_coeff = viscosityMultiplier * MASS * lap / rho_j;
                            fx += v_coeff * (pj.vx - pi.vx);
                            fy += v_coeff * (pj.vy - pi.vy);

                            // 3. Cohesion / Surface Tension Force (Attraction)
                            float coh_scale = cohesionMultiplier * MASS * powf(RAD - r, 3.0f);
                            fx -= coh_scale * nx;
                            fy -= coh_scale * ny;
                        }
                    }
                    j = particleNext[j];
                }
            }
        }

        // Add External Gravity
        fy += gravity;

        // Interactive Mouse Force
        if (isMouseDown) {
            float mdx = mouseX - pi.x;
            float mdy = mouseY - pi.y;
            float mdist = sqrtf(mdx * mdx + mdy * mdy);
            if (mdist < 0.35f && mdist > 1e-4f) {
                float mdirX = mdx / mdist;
                float mdirY = mdy / mdist;
                float mForce = (1.0f - mdist / 0.35f) * 40.0f;
                if (mouseButton == GLFW_MOUSE_BUTTON_LEFT) {
                    // Attract fluid toward cursor
                    fx += mdirX * mForce;
                    fy += mdirY * mForce;
                }
                else if (mouseButton == GLFW_MOUSE_BUTTON_RIGHT) {
                    // Repel fluid away from cursor
                    fx -= mdirX * mForce;
                    fy -= mdirY * mForce;
                }
            }
        }

        forces[i] = { fx, fy };
    }
}

// --- Boundary Collisions & Integration Substep ---
void integrate_substep(float dt) {
    build_spatial_grid(particles);
    compute_densities();
    compute_forces();

    float left = leftBound() + BALL_RADIUS;
    float right = rightBound() - BALL_RADIUS;
    float bottom = bottomBound() + BALL_RADIUS;
    float top = topBound() - BALL_RADIUS;

    #pragma omp parallel for
    for (int i = 0; i < (int)particles.size(); i++) {
        Particle& p = particles[i];

        // Symplectic Euler Integration
        p.vx += forces[i].fx * dt;
        p.vy += forces[i].fy * dt;

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // Wall collisions with damping & friction
        const float restitution = 0.25f;
        const float friction = 0.95f;

        if (p.x < left) {
            p.x = left;
            p.vx = -p.vx * restitution;
            p.vy *= friction;
        }
        else if (p.x > right) {
            p.x = right;
            p.vx = -p.vx * restitution;
            p.vy *= friction;
        }

        if (p.y < bottom) {
            p.y = bottom;
            p.vy = -p.vy * restitution;
            p.vx *= friction;
        }
        else if (p.y > top) {
            p.y = top;
            p.vy = -p.vy * restitution;
            p.vx *= friction;
        }
    }
}

void updateSimulation(float frameDt) {
    if (isPaused) return;

    // Sub-stepping for physical stability & smooth motion
    const int NUM_SUBSTEPS = 8;
    float subDt = std::min(frameDt / (float)NUM_SUBSTEPS, 0.0015f);

    for (int step = 0; step < NUM_SUBSTEPS; step++) {
        integrate_substep(subDt);
    }
}
