#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <omp.h>
#include <random>

const float BALL_RADIUS = 0.005f;
const float PI = 3.14159265358979323846;
const float FPS = 120.0;
const float SCREENWIDTH = 700.0f;
const float SCREENHEIGHT = 700.0f;
const float MASS = 5.0f;
const float RAD = 0.25f;

const int NUM_PARTICLES = 500;
const int NUM_ROWS = 5;

// Window
float gWindowWidth = SCREENWIDTH;
float gWindowHeight = SCREENHEIGHT;
float gAspect = 1.0f;

// Physics
float gravity = -9.8f;                // gravity force (units/sec^2)

float targetDensity = 10.0f;
float pressureStiffness = 20.0f;
float viscosityMultiplier = 0.5f;

struct Particle {
    float x = 0.0f, y = 0.0f;
    float vx = 0.0f, vy = 0.0f;
};

struct Force {
    float fx = 0.0f, fy = 0.0f;
};

std::vector<Particle> particles;
std::vector<float> densities;

float distance(Particle p1, Particle p2){
	return sqrt(pow(p1.x - p2.x, 2.0f) + pow(p1.y - p2.y, 2.0f));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gWindowWidth = width;
    gWindowHeight = height;
    gAspect = (float)width / (float)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (gAspect >= 1.0f) {
        // Wide window
        glOrtho(-gAspect, gAspect, -1.0f, 1.0f, -1.0f, 1.0f);
    }
    else {
        // Tall window
        glOrtho(-1.0f, 1.0f, -1.0f / gAspect, 1.0f / gAspect, -1.0f, 1.0f);
    }

    glMatrixMode(GL_MODELVIEW);
}

float leftBound() { return (gAspect >= 1.0f) ? -gAspect : -1.0f; }
float rightBound() { return (gAspect >= 1.0f) ? gAspect : 1.0f; }
float bottomBound() { return (gAspect < 1.0f) ? -1.0f / gAspect : -1.0f; }
float topBound() { return (gAspect < 1.0f) ? 1.0f / gAspect : 1.0f; }

void drawFilledCircle(GLfloat x, GLfloat y) {
    GLfloat twicePi = 2.0f * PI;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center of the circle
    for (int i = 0; i <= 64; i++) {
        glVertex2f(
            x + (BALL_RADIUS * cos(i * twicePi / 64)),
            y + (BALL_RADIUS * sin(i * twicePi / 64))
        );
    }
    glEnd();
}

void draw() {
    for (const auto& p : particles) {
        drawFilledCircle(p.x, p.y);
    }
}

void init_scene(int numParticles, int numRows) {
    for (int i = 0; i < numParticles; i++) {
        float spacing = BALL_RADIUS * 2.2f;

        float x = (i / numRows) * spacing - 0.5f;
        float y = (i % numRows) * spacing + 0.5f;


        Particle p;
		p.x = x;
		p.y = y;
		particles.push_back(p);
    }
}

void wall_collision(Particle& p)
{
    float left = leftBound();
    float right = rightBound();
    float bottom = bottomBound();
    float top = topBound();

    if (p.x + BALL_RADIUS > right) {
        p.x = right - BALL_RADIUS;
        p.vx = -p.vx;
    }
    else if (p.x - BALL_RADIUS < left) {
        p.x = left + BALL_RADIUS;
        p.vx = -p.vx;
    }

    if (p.y + BALL_RADIUS > top) {
        p.y = top - BALL_RADIUS;
        p.vy = -p.vy;
    }
    else if (p.y - BALL_RADIUS < bottom) {
        p.y = bottom + BALL_RADIUS;
        p.vy = -p.vy;
    }
}


void update_particles(float dt, GLFWwindow* window) {
    #pragma omp parallel for
    for (auto& p : particles) {
        p.x += p.vx * dt;             // update position
        p.y += p.vy * dt;

        // --- Collision with walls (simple elastic, dampened) ---
		wall_collision(p);
	}
}

static float densitySmoothingKernel(float effectRadius, float dist) {
	float volume = PI * pow(effectRadius, 8.0f) / 4.0f;
    float result = std::max(0.0f, effectRadius * effectRadius - dist * dist);
	return pow(result, 3.0f)/volume;
}

static float PressureSmoothingKernel(float effectRadius, float dist) {
    if (dist >= effectRadius) return 0.0f;
    float deriv = effectRadius - dist;
    float scale = -24.0f / (PI * pow(effectRadius, 8.0f));
    return scale * deriv * deriv * deriv;
}

static float laplacienViscosityKernel(float effectradius, float dist) {
    if (dist >= effectradius) return 0.0f;
    float scale = 20.0f / (PI * pow(effectradius, 4.0f));
	return scale * (effectradius - dist);
}

Force CalculateViscosity(int ind) {
    Force viscosity;
    for (int i = 0; i < particles.size(); i++) {
        if (i == ind) continue;
		Particle& pi = particles[ind];
		Particle& pj = particles[i];
		viscosity.fx += MASS * (pj.vx - pi.vx) / densities[i] * laplacienViscosityKernel(RAD, distance(pi, pj));
		viscosity.fy += MASS * (pj.vy - pi.vy) / densities[i] * laplacienViscosityKernel(RAD, distance(pi, pj));
    }
	viscosity.fx *= viscosityMultiplier;
	viscosity.fy *= viscosityMultiplier;
	return viscosity;
}

static float DensityToPressure(float density) {
    float p = pressureStiffness * (density - targetDensity);
    return std::max(p, 0.0f);
}

void CalculateDensity(int ind) {
	float density = 0.0f;
	Particle& p = particles[ind];
    for (int i = 0; i < particles.size(); i++) {
        if (i == ind) continue;
		float dist = distance(p, particles[i]);
		float effect = densitySmoothingKernel(RAD, dist);
		density += MASS * effect;
    }
	densities.at(ind) = density;
}

void compute_densities() {
    densities.resize(particles.size());
    #pragma omp parallel for
    for (int i = 0; i < particles.size(); i++) {
        CalculateDensity(i);
    }
}

Force CalculatePressure(int i) {
    Force pressure;
    Particle& pi = particles[i];

    float Pi = DensityToPressure(densities[i]);

    for (int j = 0; j < particles.size(); j++) {
        if (i == j) continue;

        Particle& pj = particles[j];

        float dx = pi.x - pj.x;
        float dy = pi.y - pj.y;
        float r = sqrt(dx * dx + dy * dy);

        if (r < 1e-6f || r >= RAD) continue;

        dx /= r;
        dy /= r;

        float Pj = DensityToPressure(densities[j]);
        float grad = PressureSmoothingKernel(RAD, r);

        float coeff =
            -MASS *
            (Pi / (densities[i] * densities[i]) +
                Pj / (densities[j] * densities[j])) *
            grad;

        pressure.fx += coeff * dx;
        pressure.fy += coeff * dy;
    }

    return pressure;
}

void updateSimulation(float dt, GLFWwindow* window) {

    compute_densities();

    #pragma omp parallel for
    for (int i = 0; i < particles.size(); i++) {
        Force pf = CalculatePressure(i);
		Force vf = CalculateViscosity(i);

        // SPH acceleration
        float ax = pf.fx + vf.fx;
        float ay = pf.fy + vf.fy + gravity;

        particles[i].vx += ax * dt;
        particles[i].vy += ay * dt;
    }

    update_particles(dt, window);
}

int main(void)
{
	// --- Initialize GLFW ---
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(SCREENWIDTH, SCREENHEIGHT, "Physics Ball", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    framebuffer_size_callback(window, SCREENWIDTH, SCREENHEIGHT);

    glfwMakeContextCurrent(window);

	float lasttime=(float)glfwGetTime();
	float currenttime;
	init_scene(NUM_PARTICLES, NUM_ROWS);

	// --- Main loop ---
    while (!glfwWindowShouldClose(window))
    {   
		currenttime = (float)glfwGetTime();
        float dt = (float)(currenttime-lasttime);
        dt = 0.001f;
		lasttime = currenttime;

        // --- Physics ---
        updateSimulation(dt, window);

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(0.2f, 0.6f, 1.0f);
        draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
