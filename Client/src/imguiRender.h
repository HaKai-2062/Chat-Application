#pragma once
struct GLFWwindow;

void initializeImGui(GLFWwindow* window, const char* glsl_version);
void processConsoleInput(char* messageBuffer);
void onImGuiRender();
void onImGuiFrameStart();
void onImGuiFrameEnd();
void onImGuiCleanUp();

uint8_t setupConnectionModal(std::string& playerName, std::string& ipAddress, uint16_t portNumber);