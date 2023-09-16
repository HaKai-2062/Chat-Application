struct GLFWwindow;

GLFWwindow* createGLFWwindow();
bool createGLFWContext();
bool crossButtonPressed(GLFWwindow* window);

static void glfw_error_callback(int error, const char* description);

void initializeGLFW(GLFWwindow* window);
void onFrameStart();
void onFrameEnd(GLFWwindow* window);
void rendererSwapBuffers(GLFWwindow* window);
void rendererCleanUp(GLFWwindow* window);

GLFWwindow* getContext();
void makeContextCurrent(GLFWwindow* backup_current_context);