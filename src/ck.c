#include "../libs/ck.h"
#include "../libs/ck_internal.h"

Ck *initCK() {
	Ck *ck = malloc(sizeof(Ck));
	if (!ck) {
		fprintf(stderr, "Failed to allocate memory for Ck\n");
		return NULL;
	}
	
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not init FreeType Library\n");
		glfwTerminate();
		free(ck);
		return NULL;
	}

	ck->ft = ft;
	ck->window_count = 0;
	ck->windows = NULL;

	glfwSetErrorCallback(error_callback);
	return ck;
}

void destroyCK(Ck *ck) {
	if (ck) {
		if (ck->ft) {
			FT_Done_FreeType(ck->ft);
		}
		free(ck);
		glfwTerminate();
	}
}

int loopCK(Ck *ck) {
	if (!ck) {
		fprintf(stderr, "Invalid Ck\n");
		return -1;
	}

	while (ck->window_count) {
		for (int i = 0; i < ck->window_count; i++) {
			if (render_window(ck->windows[i]) != 0) {
				fprintf(stderr, "Failed to render window\n");
				return -1;
			}
			mouse_state_check(ck->windows[i]);
			glfwSwapBuffers(ck->windows[i]->window);
		}
		glfwPollEvents();
	}
	return 0;
}