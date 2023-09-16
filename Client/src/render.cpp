#include <stdio.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

GLFWwindow* createGLFWwindow()
{
    return glfwCreateWindow(1280, 720, "Chat Application (Networking Library by OneLoneCoder)", nullptr, nullptr);
}

bool crossButtonPressed(GLFWwindow* window)
{
    return glfwWindowShouldClose(window);
}

void onFrameStart()
{
    glfwPollEvents();
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool createGLFWContext()
{
    return glfwInit();
}

void initializeGLFW(GLFWwindow* window)
{
    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
}

GLFWwindow* getContext()
{
    return glfwGetCurrentContext();
}

void makeContextCurrent(GLFWwindow* backup_current_context)
{
    glfwMakeContextCurrent(backup_current_context);
}

void onFrameEnd(GLFWwindow* window)
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void rendererSwapBuffers(GLFWwindow* window)
{
    glfwSwapBuffers(window);
}

void rendererCleanUp(GLFWwindow* window)
{
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
}