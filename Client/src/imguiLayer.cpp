#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#include "imguiLayer.h"
#include "renderer.h"
// included playerStruct.h because of fileName
// TDL: find a better way to stream data 
//		instead of reading/writing to a file
#include "playerStruct.h"

// Very bad to do
#include "client.h"

bool scroll = true;
bool scrollToBotton = false;
bool show_demo_window = false;

void ImGuiLayer::initializeImGui(GLFWwindow* window, const char* glsl_version)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();  (void)io;

	// Set default font
	ImFontConfig fontConfig;
	fontConfig.FontDataOwnedByAtlas = false;
	ImFont* robotoFont = io.Fonts->AddFontFromFileTTF("fonts/RobotoRegular.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesDefault());
	io.FontDefault = robotoFont;

	// Setup Dear ImGui context
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGuiLayer::DarkTheme();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImGuiLayer::onImGuiFrameStart()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
}

uint8_t ImGuiLayer::setupConnectionModal(std::string& playerName, std::string& ipAddress, std::string& portNumber)
{
	s_PlayerInfo* clientInfo = s_PlayerInfo::Get();
	bool popupOpen = false;
	ImGui::OpenPopup("Connect to server");
	popupOpen = ImGui::BeginPopupModal("Connect to server", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (popupOpen)
	{
		ImGui::Text("Name:");
		ImGui::InputText("##playerName", &playerName);

		ImGui::SameLine();
		ImGui::ColorEdit4("##nameColorPicker", &clientInfo->color[0], ImGuiColorEditFlags_NoInputs);

		ImGui::Text("IP");
		ImGui::InputText("##ip", &ipAddress);

		ImGui::Text("Port");
		ImGui::InputText("##port", &portNumber, ImGuiInputTextFlags_CharsDecimal);

		if (ImGui::Button("Connect"))
		{
			ImGui::EndPopup();
			if (playerName.empty() || ipAddress.empty() || portNumber.empty())
				return 0;
			return 1;
		}
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(20.0f, 20.0f));
		ImGui::SameLine();
		if (ImGui::Button("Quit"))
		{
			ImGui::EndPopup();
			return 2;
		}
	}

	ImGui::EndPopup();

	return 0;
}

void ImGuiLayer::onImGuiRender()
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

	ImGui::Begin("##messageLogger");

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
		if (myFile.is_open())
		{
			while (std::getline(myFile, messageHistory))
			{
				s_PlayerInfo* clientInfo = s_PlayerInfo::Get();
				if (!filter.PassFilter(messageHistory.c_str()))
					continue;

				// name|color.x|y|z|message				
				// The message doesnt follow our condition so skip it
				if (std::count(messageHistory.begin(), messageHistory.end(), '|') != 5)
					continue;

				char* colorAndMessage = nullptr;
				char* colorArray = nullptr;
				char* message = nullptr;
				char* name = nullptr;

				// if it does not fall under our format then simply skip that line by continue;
				// it can be triggered when our history is being downloaded

				name = strtok_s(messageHistory.data(), "|", &colorAndMessage);

				float color[4] =
				{
					std::strtof(strtok_s(colorAndMessage, "|", &colorAndMessage), nullptr),
					std::strtof(strtok_s(colorAndMessage, "|", &colorAndMessage), nullptr),
					std::strtof(strtok_s(colorAndMessage, "|", &colorAndMessage), nullptr),
					std::strtof(strtok_s(colorAndMessage, "|", &message)		, nullptr)
				};

				ImGui::TextColored({ color[0],color[1],color[2],color[3] }, name);
				ImGui::SameLine();
				ImGui::TextUnformatted(":");
				ImGui::SameLine();
				ImGui::TextUnformatted(message);
			}
			myFile.close();
		}

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
		if (messageBuffer[0] != '\0')
		{
			ImGuiLayer::processConsoleInput(messageBuffer);
			strcpy_s(messageBuffer, "");
			ImGui::SetKeyboardFocusHere(-1);
			scrollToBotton = true;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button(("Send"), ImVec2(100.0f, 0.0f)))
	{
		if (messageBuffer[0] != '\0')
		{
			ImGuiLayer::processConsoleInput(messageBuffer);
			strcpy_s(messageBuffer, "");
			scrollToBotton = true;
		}
	}
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("##playerList");


	uint32_t playerSize = static_cast<uint32_t>(Client::playerList.size());

	ImGui::Text("Online [");
	ImGui::SameLine();
	ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, std::to_string(playerSize).c_str());
	ImGui::SameLine();
	ImGui::Text("]");

	ImGui::Separator();

	const float footer_height_to_reserve2 = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
	if (ImGui::BeginChild("ScrollingRegion2", ImVec2(0, -footer_height_to_reserve2), false, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
		ImGui::SetCursorPosY(8.0f);
		
		for (auto i = Client::playerList.begin(); i != Client::playerList.end(); i++)
		{
			ImVec4 color = { i->second.color[0], i->second.color[1], i->second.color[2], i->second.color[3] };
			const char* playerNmae = i->second.name;

			ImGui::TextColored(color, playerNmae);
		}
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	

	// TDL: Programmatically docking
	/*
	ImGuiID parent_node = ImGui::DockBuilderAddNode();
	ImGui::DockBuilderSetNodePos(parent_node, ImGui::GetWindowPos());
	ImGui::DockBuilderSetNodeSize(parent_node, ImGui::GetWindowSize());
	ImGuiID nodeA;
	ImGuiID nodeB;
	ImGui::DockBuilderSplitNode(parent_node, ImGuiDir_Up, 0.8f, &nodeB, &nodeA);

	ImGui::DockBuilderDockWindow("A", nodeA);
	ImGui::DockBuilderDockWindow("B", nodeB);
	*/

	ImGui::End();
}

void ImGuiLayer::ImGuiRendered()
{
	ImGui::Render();
}

void ImGuiLayer::onImGuiFrameEnd()
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();  (void)io;

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = Renderer::getContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		Renderer::makeContextCurrent(backup_current_context);
	}
}

void ImGuiLayer::onImGuiCleanUp()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::AwaitingConnection()
{
	bool popupOpen = false;
	ImGui::OpenPopup("Connecting......");
	popupOpen = ImGui::BeginPopupModal("Connecting......", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (popupOpen)
	{
		ImGui::Text("Attempting to connect to the server");
		if (Client::m_FailedTheConnection)
		{
			ImGui::Text("Failed to connect to the server");
			ImGui::Text("Make sure you have entered correct IP Address and port number");
		}
		else if (Client::m_WaitingForConnection)
		{
			ImGui::Text("BeepBop...");
		}
	}

	ImGui::EndPopup();
}

void ImGuiLayer::processConsoleInput(char* messageBuffer)
{
	s_PlayerInfo* clientInfo = s_PlayerInfo::Get();
	strncpy_s(clientInfo->message, messageBuffer, sizeof(clientInfo->message));

	std::ofstream myFile(fileName, std::ios::app);
	if (myFile.is_open())
	{
		myFile << clientInfo->name << "|"
			<< std::setprecision(2)
			<< clientInfo->color[0] << '|'
			<< clientInfo->color[1] << '|'
			<< clientInfo->color[2] << '|'
			<< clientInfo->color[3]
			<< "|" << messageBuffer << "\n";
		myFile.close();
	}
}

// Dark Theme for ImGui
// Source: https://github.com/ocornut/imgui/issues/707#issuecomment-917151020
void ImGuiLayer::DarkTheme()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 6.00f);
	style.ItemSpacing = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 1;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 3;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 4;
}