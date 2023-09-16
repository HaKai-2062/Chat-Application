#include <stdexcept>

#include "render.h"
#include "imguiRender.h"

// This includes alot of code to main.cpp
// Figure out a way to use header file for this
#include "client.cpp"

int main(int, char**)
{
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";

    if (!createGLFWContext())
        throw std::runtime_error("Failed to initialize GLFW");

    GLFWwindow* window = createGLFWwindow();
    if (window == nullptr)
        throw std::runtime_error("Failed to initialize window");

    initializeGLFW(window);

    initializeImGui(window, glsl_version);

    while (!crossButtonPressed(window))
    {
		if (quit)
			break;

        onFrameStart();
        onImGuiFrameStart();
        onImGuiRender();
        onFrameEnd(window);
        onImGuiFrameEnd();
        rendererSwapBuffers(window);
    }

    onImGuiCleanUp();
    rendererCleanUp(window);
}
