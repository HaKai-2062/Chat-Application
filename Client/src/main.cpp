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

void Save(std::string& playerName, std::string& ipAddress, std::string& portString, std::string& colorString);
void Load(std::string& playerName, std::string& ipAddress, std::string& portString, std::string& colorString);

int main(int, char**)
{
    std::string playerName = "", ipAddress = "127.0.0.1";
    std::string portString = "60000", colorString = "1.0,1.0,1.0,1.0";

    // Load variables on application startup
    Load(playerName, ipAddress, portString, colorString);
    
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
                    uint16_t port = atoi(portString.c_str());
                    myClient = new Client(pName, ip, port);

                    // Clear all the data in the file after reading it for first boot
                    std::ofstream clearFile(fileName, std::ofstream::out | std::ofstream::trunc);
                    if (clearFile.is_open())
                        clearFile.close();

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
            returnType = ImGuiLayer::setupConnectionModal(playerName, ipAddress, portString, colorString);

        ImGuiLayer::ImGuiRendered();
        
        // Quit button was pressed
        if (returnType == 2)
            break;

        Renderer::onFrameEnd(window);
        ImGuiLayer::onImGuiFrameEnd();
        Renderer::rendererSwapBuffers(window);
    }

    // Clear messages History and Save variables before quitting
    Save(playerName, ipAddress, portString, colorString);

    ImGuiLayer::onImGuiCleanUp();
    Renderer::rendererCleanUp(window);
}

void Save(std::string& playerName, std::string& ipAddress, std::string& portString, std::string& colorString)
{
    std::ofstream file(fileName, std::ofstream::out | std::ofstream::trunc);

    if (file.is_open())
    {
        file << playerName << '\n';
        file << ipAddress << '\n';
        file << portString << '\n';
        file << colorString << '\n';
    }
    file.close();
}

void Load(std::string& playerName, std::string& ipAddress, std::string& portString, std::string& colorString)
{
    std::ifstream file(fileName);

    if (file.fail())
    {
        std::ofstream newFile(fileName, std::ofstream::out | std::ofstream::trunc);
        if (newFile.is_open())
        {
            newFile << playerName << '\n';
            newFile << ipAddress << '\n';
            newFile << portString << '\n';
            newFile << colorString << '\n';
            newFile.close();
        }
        return;
    }

    if (file.is_open())
    {
        // This count apparently needs the file to be closed
        uint32_t lineCount = static_cast<uint32_t>(std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n'));
        file.close();

        if (lineCount == 4)
        {
            file = std::ifstream(fileName);
            if (file.is_open())
            {
                std::getline(file, playerName);
                std::getline(file, ipAddress);
                std::getline(file, portString);
                std::getline(file, colorString);
            }
        }
    }
    file.close();
}
