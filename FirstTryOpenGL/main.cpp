#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>

const float BALL_RADIUS = 0.025f;
const float PI = 3.14159265358979323846;
const float FPS = 60.0;
const float SCREENWIDTH = 700.0f;
const float SCREENHEIGHT = 700.0f;

// Physics
float posX = 0.0f, posY = 0.0f;
float velX = 0.0f, velY = 0.0f;       // initial velocity
float gravity = -5.0f;                // gravity force (units/sec^2)
float bounceDamping = 0.9f;           // how much energy is kept on bounce
float airDamping = 1.0f;            // slight drag for smoothness

struct Particle {
    float x = 0.0f, y = 0.0f;
    float vx = 0.0f, vy = 0.0f;
};

std::vector<Particle> particles;

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

void wall_collision(Particle& p) {
    if (p.x + BALL_RADIUS > 1) { 
        p.x = 1.0f - BALL_RADIUS;
        p.vx = -p.vx * bounceDamping; 
    } else if (p.x - BALL_RADIUS < -1) { 
        p.x = -1.0f + BALL_RADIUS; 
        p.vx = -p.vx * bounceDamping; 
    }

    if (p.y - BALL_RADIUS < -1.0f) {
        p.y = -1.0f + BALL_RADIUS;
        p.vy = -p.vy * bounceDamping; // bounce with energy loss
    }
    if (p.y + BALL_RADIUS > 1.0f) {
        p.y = 1.0f - BALL_RADIUS;
        p.vy = -p.vy * bounceDamping;
    }
}

void update_particles(float dt) {
    for (auto& p : particles) {
        p.vy += gravity * dt;          // apply gravity
        p.x += p.vx * dt;             // update position
        p.y += p.vy * dt;

        // --- Collision with walls (simple elastic, dampened) ---
		wall_collision(p);
	}
}

int main(void)
{
	// --- Initialize GLFW ---
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(SCREENWIDTH, SCREENHEIGHT, "Physics Ball", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
	float lasttime=(float)glfwGetTime();
	float currenttime;
	init_scene(80, 5);

	// --- Main loop ---
    while (!glfwWindowShouldClose(window))
    {   
		currenttime = (float)glfwGetTime();
        float dt = (float)(currenttime-lasttime);
        dt = std::min(dt, 1.0f / 60.0f);
		lasttime = currenttime;

        // --- Physics ---
		update_particles(dt);

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
