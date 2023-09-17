#include <stdio.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

#include "renderer.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLFWwindow* Renderer::createGLFWwindow()
{
    return glfwCreateWindow(1280, 720, "Chat Application (Networking Library by OneLoneCoder)", nullptr, nullptr);
}

bool Renderer::crossButtonPressed(GLFWwindow* window)
{
    return glfwWindowShouldClose(window);
}

void Renderer::onFrameStart()
{
    glfwPollEvents();
}

bool Renderer::createGLFWContext()
{
    return glfwInit();
}

void Renderer::initializeGLFW(GLFWwindow* window)
{
    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
}

GLFWwindow* Renderer::getContext()
{
    return glfwGetCurrentContext();
}

void Renderer::makeContextCurrent(GLFWwindow* backup_current_context)
{
    glfwMakeContextCurrent(backup_current_context);
}

void Renderer::onFrameEnd(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::rendererSwapBuffers(GLFWwindow* window)
{
    glfwSwapBuffers(window);
}

void Renderer::rendererCleanUp(GLFWwindow* window)
{
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}