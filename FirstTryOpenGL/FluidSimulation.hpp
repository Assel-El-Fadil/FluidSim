#ifndef FLUID_SIMULATION_HPP
#define FLUID_SIMULATION_HPP

#include "Config.hpp"
#include <vector>
#include <GLFW/glfw3.h>

// Window & Aspect State
extern float gWindowWidth;
extern float gWindowHeight;
extern float gAspect;

// Physics Parameters
extern float gravity;
extern float RAD;
extern float MASS;
extern float targetDensity;
extern float pressureStiffness;
extern float viscosityMultiplier;
extern float cohesionMultiplier;

// Interactive State
extern bool isPaused;
extern bool isMouseDown;
extern int mouseButton;
extern float mouseX;
extern float mouseY;

// Particle Vectors
extern std::vector<Particle> particles;
extern std::vector<float> densities;
extern std::vector<Force> forces;

// Boundary Calculation Functions
float leftBound();
float rightBound();
float bottomBound();
float topBound();

// Solver Functions
void init_scene(int numParticles);
void init_scene_two_blocks(int numParticles);
void compute_densities();
void compute_forces();
void integrate_substep(float dt);
void updateSimulation(float frameDt);

#endif // FLUID_SIMULATION_HPP
