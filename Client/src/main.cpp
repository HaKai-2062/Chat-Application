#include <iostream>
#include <stdexcept>

#include "renderer.h"
#include "imguiLayer.h"

// This includes alot of code to cpp
// Figure out a way to abstract into a header file
#include "client.h"

std::unordered_map<uint32_t, olc::net::playerStruct> Client::playerList;

bool Client::m_WaitingForConnection = true;
bool Client::m_FailedTheConnection = false;

int main(int, char**)
{
    std::string playerName = "", ipAddress = "127.0.0.1";
    uint16_t port = 60'000;
    std::string portString = std::to_string(port);

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
    bool connectButtonPressed = false;

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
            if (!connectButtonPressed)
            {
                // playerName is empty so end the frame
                // and display setupconectionaModal again
                if (playerName.empty())
                {
                    returnType = 0;
                    ImGuiLayer::ImGuiRendered();
                    Renderer::onFrameEnd(window);
                    ImGuiLayer::onImGuiFrameEnd();
                    Renderer::rendererSwapBuffers(window);

                    continue;
                }
                else
                {
                    const char* pName = playerName.c_str();
                    const char* ip = ipAddress.c_str();
                    myClient = new Client(pName, ip, port);

                    // Create messageHistory file or make it empty
                    std::ofstream file(fileName, std::ofstream::out | std::ofstream::trunc);
                    if (file.is_open())
                        file.close();
                    connectButtonPressed = true;
                }
            }

            myClient->OnClientUpdate();
            
            if (myClient && myClient->m_WaitingForConnection)
            {
                ImGuiLayer::AwaitingConnection();
            }
            else
            {
                ImGuiLayer::onImGuiRender();
            }
        }
        
        // Keep displaying the connectionModal
        if (returnType == 0)
            returnType = ImGuiLayer::setupConnectionModal(playerName, ipAddress, portString);

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