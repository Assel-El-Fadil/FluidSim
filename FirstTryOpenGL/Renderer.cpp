#include "Renderer.hpp"
#include "FluidSimulation.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

void drawFilledCircle(float x, float y, float r, float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    const int numSegments = 20;
    for (int i = 0; i <= numSegments; i++) {
        float theta = i * (2.0f * PI / numSegments);
        glVertex2f(x + r * cosf(theta), y + r * sinf(theta));
    }
    glEnd();
}

void drawContainer() {
    float left = leftBound();
    float right = rightBound();
    float bottom = bottomBound();
    float top = topBound();

    glColor3f(0.25f, 0.3f, 0.4f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glVertex2f(right, top);
    glVertex2f(left, top);
    glEnd();
}

void draw() {
    drawContainer();

    // Draw particles colored by velocity magnitude
    for (const auto& p : particles) {
        float speed = sqrtf(p.vx * p.vx + p.vy * p.vy);
        float t = clamp_val(speed / 4.0f, 0.0f, 1.0f);

        // Interpolate color: Calm deep blue (0.15, 0.45, 0.95) -> Fast bright cyan (0.75, 0.95, 1.0)
        float r = 0.15f + t * 0.60f;
        float g = 0.45f + t * 0.50f;
        float b = 0.95f + t * 0.05f;

        drawFilledCircle(p.x, p.y, BALL_RADIUS, r, g, b);
    }

    // Draw mouse interaction visualizer
    if (isMouseDown) {
        if (mouseButton == GLFW_MOUSE_BUTTON_LEFT) {
            drawFilledCircle(mouseX, mouseY, 0.02f, 0.2f, 1.0f, 0.4f);
        } else if (mouseButton == GLFW_MOUSE_BUTTON_RIGHT) {
            drawFilledCircle(mouseX, mouseY, 0.02f, 1.0f, 0.3f, 0.3f);
        }
    }
}
