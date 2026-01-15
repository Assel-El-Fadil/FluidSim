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
const float MASS = 1.0f;

const int NUM_PARTICLES = 500;
const int NUM_ROWS = 5;

// Window
float gWindowWidth = SCREENWIDTH;
float gWindowHeight = SCREENHEIGHT;
float gAspect = 1.0f;

// Physics
float posX = 0.0f, posY = 0.0f;
float velX = 0.0f, velY = 0.0f;       // initial velocity
float gravity = -5.0f;                // gravity force (units/sec^2)
float bounceDamping = 0.9f;           // how much energy is kept on bounce
float airDamping = 1.0f;            // slight drag for smoothness

float targetDensity = 10.0f;
float pressureStiffness = 20.0f;

struct Particle {
    float x = 0.0f, y = 0.0f;
    float vx = 1.0f, vy = 0.0f;
};

struct PressureForce {
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

void init_scene(int numParticles) {
    /*for (int i = 0; i < numParticles; i++) {
        float spacing = BALL_RADIUS * 2.2f;

        float x = (i / numRows) * spacing - 0.5f;
        float y = (i % numRows) * spacing + 0.5f;


        Particle p;
		p.x = x;
		p.y = y;
		particles.push_back(p);
    }*/
    particles.clear();
    densities.clear();

    std::random_device rd;
    std::mt19937 gen(rd());

    // Keep particles fully inside the window
    std::uniform_real_distribution<float> distX(
        -1.0f + BALL_RADIUS, 1.0f - BALL_RADIUS);
    std::uniform_real_distribution<float> distY(
        -1.0f + BALL_RADIUS, 1.0f - BALL_RADIUS);

    // Small random initial velocities (helps break symmetry)
    std::uniform_real_distribution<float> velDist(-0.1f, 0.1f);

    for (int i = 0; i < numParticles; i++) {
        Particle p;
        p.x = distX(gen);
        p.y = distY(gen);
        p.vx = velDist(gen);
        p.vy = velDist(gen);
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
        p.vx = -p.vx * bounceDamping;
    }
    else if (p.x - BALL_RADIUS < left) {
        p.x = left + BALL_RADIUS;
        p.vx = -p.vx * bounceDamping;
    }

    if (p.y + BALL_RADIUS > top) {
        p.y = top - BALL_RADIUS;
        p.vy = -p.vy * bounceDamping;
    }
    else if (p.y - BALL_RADIUS < bottom) {
        p.y = bottom + BALL_RADIUS;
        p.vy = -p.vy * bounceDamping;
    }
}


void update_particles(float dt, GLFWwindow* window) {
    #pragma omp parallel for
    for (auto& p : particles) {
        //p.vy += gravity * dt;          // apply gravity
        p.x += p.vx * dt;             // update position
        p.y += p.vy * dt;

        // --- Collision with walls (simple elastic, dampened) ---
		wall_collision(p);
	}
}

float densitySmoothingKernel(float effectRadius, float dist) {
	float volume = PI * pow(effectRadius, 8.0f) / 4.0f;
    float result = std::max(0.0f, effectRadius - dist);
	return pow(result, 3.0f)/volume;
}

float PressureSmoothingKernel(float effectRadius, float dist) {
    if (dist >= effectRadius) return 0.0f;
    float deriv = effectRadius - dist;
    float scale = -24.0f / (PI * pow(effectRadius, 8.0f));
    return scale * dist * deriv * deriv;
}

float DensityToPressure(float density) {
    return pressureStiffness * (density - targetDensity);
}

void CalculateDensity(int ind) {
	float density = 0.0f;
	Particle& p = particles[ind];
    for (int i = 0; i < particles.size(); i++) {
        if (i == ind) continue;
		float dist = distance(p, particles[i]);
		float effect = densitySmoothingKernel(0.5f, dist);
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

PressureForce CalculatePressure(int i) {
    PressureForce pressure;
    Particle& pi = particles[i];

    float Pi = DensityToPressure(densities[i]);

    for (int j = 0; j < particles.size(); j++) {
        if (i == j) continue;

        Particle& pj = particles[j];

        float dx = pi.x - pj.x;
        float dy = pi.y - pj.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < 1e-5f || dist >= 0.5f) continue;

        dx /= dist;
        dy /= dist;

        float Pj = DensityToPressure(densities[j]);
        float slope = PressureSmoothingKernel(0.5f, dist);

        float coeff =
            -MASS * MASS *
            (Pi / (densities[i] * densities[i]) +
                Pj / (densities[j] * densities[j])) *
            slope;

        pressure.fx += coeff * dx;
        pressure.fy += coeff * dy;
    }

    return pressure;
}

void updateSimulation(float dt, GLFWwindow* window) {

    compute_densities();

    #pragma omp parallel for
    for (int i = 0; i < particles.size(); i++) {
        PressureForce pf = CalculatePressure(i);
		particles[i].vx = pf.fx / MASS * dt;
		particles[i].vy = pf.fy / MASS * dt;
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
	init_scene(NUM_PARTICLES);

	// --- Main loop ---
    while (!glfwWindowShouldClose(window))
    {   
		currenttime = (float)glfwGetTime();
        float dt = (float)(currenttime-lasttime);
        dt = std::min(dt, 1.0f / FPS);
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
