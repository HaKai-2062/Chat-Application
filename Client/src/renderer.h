#pragma once

struct GLFWwindow;

namespace Renderer {
	GLFWwindow* createGLFWwindow();
	bool createGLFWContext();
	bool crossButtonPressed(GLFWwindow* window);

	void initializeGLFW(GLFWwindow* window);
	void onFrameStart();
	void onFrameEnd(GLFWwindow* window);
	void rendererSwapBuffers(GLFWwindow* window);
	void rendererCleanUp(GLFWwindow* window);

	GLFWwindow* getContext();
	void makeContextCurrent(GLFWwindow* backup_current_context);
}
