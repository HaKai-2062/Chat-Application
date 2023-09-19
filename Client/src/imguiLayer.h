#pragma once
struct GLFWwindow;

namespace ImGuiLayer {
	void DarkTheme();
	void AwaitingConnection();
	void processConsoleInput(char* messageBuffer);
	uint8_t setupConnectionModal(std::string& playerName, std::string& ipAddress, std::string& portNumber, std::string& colorString);


	void initializeImGui(GLFWwindow* window, const char* glsl_version);
	void onImGuiFrameStart();
	void onImGuiRender();
	void ImGuiRendered();
	void onImGuiFrameEnd();
	void onImGuiCleanUp();
}

