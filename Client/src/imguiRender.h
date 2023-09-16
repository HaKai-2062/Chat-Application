#pragma once
struct GLFWwindow;

// very bad to do this D:
extern bool quit;

void initializeImGui(GLFWwindow* window, const char* glsl_version);
void processConsoleInput(char* messageBuffer);
void createConnectionModal();
void onImGuiRender();
void onImGuiFrameStart();
void onImGuiFrameEnd();
void onImGuiCleanUp();