#include <GLFW/glfw3.h>
#include <iostream>

#include "Config.hpp"
#include "FluidSimulation.hpp"
#include "Renderer.hpp"

// --- Callbacks ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    gWindowWidth = (float)width;
    gWindowHeight = (float)height;
    gAspect = (width > 0 && height > 0) ? (float)width / (float)height : 1.0f;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (gAspect >= 1.0f) {
        glOrtho(-gAspect, gAspect, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        glOrtho(-1.0f, 1.0f, -1.0f / gAspect, 1.0f / gAspect, -1.0f, 1.0f);
    }

    glMatrixMode(GL_MODELVIEW);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_R) {
            init_scene(NUM_PARTICLES);
            std::cout << "[FluidSim] Scene reset.\n";
        }
        else if (key == GLFW_KEY_SPACE) {
            isPaused = !isPaused;
            std::cout << "[FluidSim] Simulation " << (isPaused ? "PAUSED" : "RESUMED") << "\n";
        }
        else if (key == GLFW_KEY_G) {
            gravity = (gravity < -0.1f) ? 0.0f : -9.81f;
            std::cout << "[FluidSim] Gravity set to " << gravity << "\n";
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // Map screen pixel coords to OpenGL ortho world coordinates
    float normX = (float)xpos / gWindowWidth;
    float normY = (float)ypos / gWindowHeight;

    if (gAspect >= 1.0f) {
        mouseX = (normX * 2.0f - 1.0f) * gAspect;
        mouseY = (1.0f - normY * 2.0f);
    } else {
        mouseX = (normX * 2.0f - 1.0f);
        mouseY = (1.0f - normY * 2.0f) / gAspect;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        isMouseDown = true;
        mouseButton = button;
    } else if (action == GLFW_RELEASE) {
        isMouseDown = false;
        mouseButton = -1;
    }
}

int main(void) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow((int)SCREENWIDTH, (int)SCREENHEIGHT, "SPH 2D Fluid Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    framebuffer_size_callback(window, (int)SCREENWIDTH, (int)SCREENHEIGHT);

    std::cout << "====================================================\n";
    std::cout << " SPH 2D Fluid Simulation Started!\n";
    std::cout << " Controls:\n";
    std::cout << "   - Left Mouse Drag  : Attract fluid to cursor\n";
    std::cout << "   - Right Mouse Drag : Repel fluid from cursor\n";
    std::cout << "   - Press 'R'        : Reset fluid block\n";
    std::cout << "   - Press 'SPACE'    : Pause / Resume simulation\n";
    std::cout << "   - Press 'G'        : Toggle gravity on/off\n";
    std::cout << "====================================================\n";

    init_scene(NUM_PARTICLES);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float frameDt = (float)(currentTime - lastTime);
        lastTime = currentTime;
        if (frameDt > 0.1f) frameDt = 0.016f; // Clamp large frame spikes

        updateSimulation(frameDt);

        glClearColor(0.06f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
