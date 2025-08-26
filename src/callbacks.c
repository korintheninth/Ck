#include "../libs/ck.h"
#include "../libs/ck_internal.h"

void error_callback(int error, const char* description) {
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	Ck *ck = (Ck *)glfwGetWindowUserPointer(window);

	Window *win = NULL;
	for (size_t i = 0; i < ck->window_count; i++)
		if (ck->windows[i]->window == window)
			win = ck->windows[i];
	
	if (!win)
		return;
	
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);

	if (win) {
		win->width = width;
		win->height = height;
	}
}

void window_close_callback(GLFWwindow* window) {
	Ck *ck = (Ck *)glfwGetWindowUserPointer(window);

	Window *win = NULL;
	for (size_t i = 0; i < ck->window_count; i++)
		if (ck->windows[i]->window == window)
			win = ck->windows[i];
	
	if (!win)
		return;
	destroy_window(win);
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		return;
	}
	fprintf(stderr, "GL_DEBUG: Source: %d, Type: %d, ID: %d, Severity: %d, Message: %s\n", source, type, id, severity, message);
}

void mousebutton_callback(GLFWwindow *window, int button, int action, int mods) {
	Ck *ck = (Ck *)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS)
		signal_emit(ck, CLICK);
}