#include <iostream>
#include <stdexcept>

#include "render.h"
#include "imguiRender.h"

// This includes alot of code to cpp
// Figure out a way to abstract into a header file
#include "client.h"

int main(int, char**)
{
    std::string playerName = "", ipAddress = "127.0.0.1";
    uint16_t port = 60'000;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";

    if (!createGLFWContext())
        throw std::runtime_error("Failed to initialize GLFW");

    GLFWwindow* window = createGLFWwindow();
    if (window == nullptr)
        throw std::runtime_error("Failed to initialize window");

    initializeGLFW(window);

    initializeImGui(window, glsl_version);

    Client* myClient = nullptr;
    bool clientRegistered = false;

    // 0: this is default value
    // 1: this means connect was pressed
    // 2: this means quit was pressed
    uint8_t returnType = 0;

    while (!crossButtonPressed(window))
    {
        onFrameStart();
        onImGuiFrameStart();

        // Connect Button was pressed
        // Note: Since setupConnectionModal has ImGui::Render() as well
        //       so we have to avoid calling both onImGuiRender() and setipConnectionModal() in same frame
        if (returnType == 1)
        {
            if (!clientRegistered)
            {
                const char* pName = playerName.c_str();
                const char* ip = ipAddress.c_str();
                myClient = new Client(pName, ip, port);
                clientRegistered = true;
            }
            myClient->OnClientUpdate();
            onImGuiRender();
        }
        
        // Keep displaying the connectionModal
        if (returnType == 0)
            returnType = setupConnectionModal(playerName, ipAddress, port);


        // Quit button was pressed
        if (returnType == 2)
            break;

        onFrameEnd(window);
        onImGuiFrameEnd();
        rendererSwapBuffers(window);
    }

    onImGuiCleanUp();
    rendererCleanUp(window);
}
