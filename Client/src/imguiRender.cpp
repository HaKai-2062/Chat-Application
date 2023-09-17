#include <string>
#include <fstream>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#include "imguiRender.h"
#include "render.h"

#include "playerStruct.h"

bool show_demo_window = true;
const char* fileName = "secretChatHistory.txt";

void initializeImGui(GLFWwindow* window, const char* glsl_version)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();  (void)io;

	// Setup Dear ImGui context
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void onImGuiFrameStart()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
}

void processConsoleInput(char* messageBuffer)
{
	s_PlayerInfo* clientInfo = s_PlayerInfo::Get();
	strncpy_s(clientInfo->playerMessage, messageBuffer, sizeof(clientInfo->playerMessage));

	std::fstream uidlFile(fileName, std::fstream::app);
	if (uidlFile.is_open())
	{
		uidlFile << clientInfo->playerName << ": " << messageBuffer << "\n";
		uidlFile.close();
	}
}

uint8_t setupConnectionModal(std::string& playerName, std::string& ipAddress, uint16_t portNumber)
{
	bool popupOpen = false;
	ImGui::OpenPopup("Connect to server");
	popupOpen = ImGui::BeginPopupModal("Connect to server", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	std::string myPort = std::to_string(portNumber);

	if (popupOpen)
	{
		ImGui::Text("Name:");
		ImGui::InputText("##playerName", &playerName);

		ImGui::Text("IP");
		ImGui::InputText("##ip", &ipAddress);

		ImGui::Text("Port");
		ImGui::InputText("##port", &myPort, ImGuiInputTextFlags_CharsDecimal);

		if (ImGui::Button("Connect"))
		{
			ImGui::EndPopup();
			ImGui::Render();
			return 1;
		}
		if (ImGui::Button("Quit"))
		{
			ImGui::EndPopup();
			return 2;
		}
	}

	ImGui::EndPopup();
	ImGui::Render();

	return 0;
}

void onImGuiRender()
{
	bool scroll = true;
	bool scrollToBotton = false;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

	ImGui::Begin("##MessageLogger");

	ImGui::Text("Search");
	ImGui::SameLine();
	ImGuiTextFilter filter;
	filter.Draw("##search", 180);

	ImGui::SameLine();
	ImGui::Checkbox("Auto Scroll", &scroll);

	ImGui::Separator();

	// Reserve enough left-over height for 1 separator + 1 input text
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
		ImGui::SetCursorPosY(8.0f);

		std::string messageHistory;
		std::ifstream myFile(fileName);
		while (std::getline(myFile, messageHistory))
		{
			if (!filter.PassFilter(messageHistory.c_str()))
				continue;
			ImGui::TextUnformatted(messageHistory.c_str());
		}
		myFile.close();

		if (scrollToBotton || (scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);

		scrollToBotton = false;
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	ImGui::Separator();

	static char messageBuffer[256];
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100.0f);
	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::InputText("##chatConsole", messageBuffer, IM_ARRAYSIZE(messageBuffer), flags))
	{
		processConsoleInput(messageBuffer);

		for (uint32_t i = 0; i < sizeof(messageBuffer); i++)
			messageBuffer[i] = '\0';
		ImGui::SetKeyboardFocusHere(-1);
		scrollToBotton = true;
	}

	ImGui::SameLine();
	if (ImGui::Button(("Send"), ImVec2(100.0f, 0.0f)))
	{
		processConsoleInput(messageBuffer);

		for (uint32_t i = 0; i < sizeof(messageBuffer); i++)
			messageBuffer[i] = '\0';
		scrollToBotton = true;
	}
	ImGui::End();
	ImGui::Render();
}

void onImGuiFrameEnd()
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();  (void)io;

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = getContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		makeContextCurrent(backup_current_context);
	}
}

void onImGuiCleanUp()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}