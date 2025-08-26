#include "../libs/ck.h"
#include "../libs/ck_internal.h"

Window *create_window(Ck *ck, int width, int height, const char *title) {
	Window *win = (Window *)malloc(sizeof(Window));
	if (!win) {
		fprintf(stderr, "Failed to allocate memory for Window\n");
		return NULL;
	}

	win->width = width;
	win->height = height;
	win->title = malloc(strlen(title) + 1);
	if (!win->title) {
		fprintf(stderr, "Failed to allocate memory for window title\n");
		free(win);
		return NULL;
	}
	strcpy(win->title, title);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	if (ck->window_count == 0)
		win->window = glfwCreateWindow(width, height, title, NULL, NULL);
	else
		win->window = glfwCreateWindow(width, height, title, NULL, ck->windows[0]->window);
	if (!win->window) {
		fprintf(stderr, "Failed to create GLFW window\n");
		free(win);
		free(win->title);
		return NULL;
	}

	glfwSetFramebufferSizeCallback(win->window, framebuffer_size_callback);
	glfwSetWindowCloseCallback(win->window, window_close_callback);

	glfwSetWindowUserPointer(win->window, ck);
	glfwMakeContextCurrent(win->window);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		free(win);
		return NULL;
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debug_callback, NULL);
	
	win->textures = malloc(sizeof(GLuint) * 4);
	if (!win->textures) {
		fprintf(stderr, "Failed to allocate memory for textures\n");
		glfwTerminate();
		free(win->title);
		free(win);
		return NULL;
	}
	win->textures[3] = 0;
		
	if (ck->window_count == 0) {
		win->shaderPrograms[0] = load_shader("shaders/text_vertex.glsl", "shaders/text_fragment.glsl");
		if (!win->shaderPrograms[0]) {
			fprintf(stderr, "Failed to load text shader program\n");
			glfwTerminate();
			free(win);
			free(win->title);
			free(win->textures);
			return NULL;
		}
		win->shaderPrograms[1] = load_shader("shaders/texture_vertex.glsl", "shaders/texture_fragment.glsl");
		if (!win->shaderPrograms[1]) {
			fprintf(stderr, "Failed to load texture shader program\n");
			glfwTerminate();
			free(win);
			free(win->title);
			free(win->textures);
			return NULL;
		}

		win->shaderPrograms[2] = load_shader("shaders/line_vertex.glsl", "shaders/line_fragment.glsl");
		if (!win->shaderPrograms[2]) {
			fprintf(stderr, "Failed to load line shader program\n");
			glfwTerminate();
			free(win);
			free(win->title);
			free(win->textures);
			return NULL;
		}

		win->textures[0] = load_texture("assets/Button.png");
		if (!win->textures[0]) {
			fprintf(stderr, "Failed to load button texture\n");
			glfwTerminate();
			free(win);
			free(win->title);
			free(win->textures);
			return NULL;
		}
		win->textures[1] = load_texture("assets/Canvas.png");
		if (!win->textures[1]) {
			fprintf(stderr, "Failed to load canvas texture\n");
			glfwTerminate();
			free(win);
			free(win->title);
			free(win->textures);
			return NULL;
		}
		win->textures[2] = load_texture("assets/TextBox.png");
	} else {
		win->shaderPrograms[0] = ck->windows[0]->shaderPrograms[0];
		win->shaderPrograms[1] = ck->windows[0]->shaderPrograms[1];
		win->shaderPrograms[2] = ck->windows[0]->shaderPrograms[2];
		win->textures[0] = ck->windows[0]->textures[0];
		win->textures[1] = ck->windows[0]->textures[1];
	}

	if (!ck->windows) {
		ck->windows = malloc(sizeof(Window *));
		if (!ck->windows) {
			fprintf(stderr, "Failed to allocate memory for windows array\n");
			glfwTerminate();
			free(win);
			return NULL;
		}
		ck->windows[0] = win;
		ck->window_count++;
	} else {
		ck->windows = realloc(ck->windows, sizeof(Window *) * (ck->window_count + 1));
		if (!ck->windows) {
			fprintf(stderr, "Failed to reallocate memory for windows array\n");
			glfwTerminate();
			free(win);
			return NULL;
		}
		ck->windows[ck->window_count] = win;
		ck->window_count++;
	}

	return win;
}
void destroy_window(Window *win) {
	Ck *ck = (Ck *)glfwGetWindowUserPointer(win->window);
	if (!ck || !win) {
		return;
	}
	if (ck->windows) {
		for (int i = 0; i < ck->window_count; i++) {
			if (ck->windows[i] == win) {
				for (size_t j = i; j < ck->window_count - 1; j++)
					ck->windows[j] = ck->windows[j+1];
				ck->windows[ck->window_count] = NULL;
				ck->window_count--;
				break;
			}
		}
	}
	if (win) {
		if (win->window)
			glfwDestroyWindow(win->window);
		if (win->title)
			free((char *)win->title);
		free(win);
	}
}

void set_window_title(Window *win, const char *title) {
	if (win && win->window) {
		glfwSetWindowTitle(win->window, title);
		free((char *)win->title);
		win->title = malloc(strlen(title) + 1);
		if (!win->title) {
			fprintf(stderr, "Failed to allocate memory for window title\n");
			return;
		}
		strcpy(win->title, title);
	}
}

int set_window_size(Window *win, int width, int height) {
	if (win && win->window) {
		glfwSetWindowSize(win->window, width, height);
		win->width = width;
		win->height = height;
		return 0;
	}
	return -1;
}