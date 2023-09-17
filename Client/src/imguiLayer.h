#pragma once
struct GLFWwindow;

namespace ImGuiLayer {
	void DarkTheme();
	void processConsoleInput(char* messageBuffer);
	uint8_t setupConnectionModal(std::string& playerName, std::string& ipAddress, uint16_t portNumber);

	void initializeImGui(GLFWwindow* window, const char* glsl_version);
	void onImGuiRender();
	void onImGuiFrameStart();
	void onImGuiFrameEnd();
	void ImGuiRendered();
	void onImGuiCleanUp();
}

