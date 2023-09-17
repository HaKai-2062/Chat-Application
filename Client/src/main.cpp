#include <iostream>
#include <stdexcept>

#include "renderer.h"
#include "imguiLayer.h"

// This includes alot of code to cpp
// Figure out a way to abstract into a header file
#include "client.h"

std::unordered_map<uint32_t, olc::net::playerStruct> Client::playerList;

int main(int, char**)
{
    std::string playerName = "", ipAddress = "127.0.0.1";
    uint16_t port = 60'000;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";

    if (!Renderer::createGLFWContext())
        throw std::runtime_error("Failed to initialize GLFW");

    GLFWwindow* window = Renderer::createGLFWwindow();
    if (window == nullptr)
        throw std::runtime_error("Failed to initialize window");

    Renderer::initializeGLFW(window);

    ImGuiLayer::initializeImGui(window, glsl_version);

    Client* myClient = nullptr;
    bool modalOpened = false;

    // 0: Default value and connectionModal will stay open in this
    // 1: Connect button was pressed
    // 2: Quit button was pressed
    uint8_t returnType = 0;

    while (!Renderer::crossButtonPressed(window))
    {
        Renderer::onFrameStart();
        ImGuiLayer::onImGuiFrameStart();

        // Connect Button was pressed
        if (returnType == 1)
        {
            if (!modalOpened)
            {
                const char* pName = playerName.c_str();
                const char* ip = ipAddress.c_str();
                myClient = new Client(pName, ip, port);
                modalOpened = true;
            }

            myClient->OnClientUpdate();

            if (myClient && !myClient->HasClientConnected())
            {
                // We are attempting to connect so create a Connecting....
                //std::cout << "Attempting to connect to the server.....\n";
               // std::cout << "Downloading Server Messages\n";
                //ImGuiLayer::
                // add some wait ???

            }
            else if (myClient && myClient->HasClientConnected())
            {
                // Client is connected and has the server chat history
                ImGuiLayer::onImGuiRender();
            }
        }
        
        // Keep displaying the connectionModal
        if (returnType == 0)
            returnType = ImGuiLayer::setupConnectionModal(playerName, ipAddress, port);

        ImGuiLayer::ImGuiRendered();
        
        // Quit button was pressed
        if (returnType == 2)
            break;

        Renderer::onFrameEnd(window);
        ImGuiLayer::onImGuiFrameEnd();
        Renderer::rendererSwapBuffers(window);
    }

    ImGuiLayer::onImGuiCleanUp();
    Renderer::rendererCleanUp(window);
}