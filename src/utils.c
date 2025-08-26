#include "../libs/ck.h"
#include "../libs/ck_internal.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/external/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../libs/external/stb_image_resize2.h"

char* read_file(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open file: %s\n", filename);
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		fprintf(stderr, "Failed to seek to end of file: %s\n", filename);
		fclose(file);
		return NULL;
	}

	long length = ftell(file);
	if (length < 0) {
		fprintf(stderr, "Failed to determine file size: %s\n", filename);
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to beginning of file: %s\n", filename);
		fclose(file);
		return NULL;
	}

	char* buffer = malloc(length + 1);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate memory for file content\n");
		fclose(file);
		return NULL;
	}

	size_t readLength = fread(buffer, 1, length, file);
	if (readLength != length) {
		fprintf(stderr, "Failed to read the entire file: %s\n", filename);
		free(buffer);
		fclose(file);
		return NULL;
	}

	buffer[length] = '\0';

	fclose(file);
	return buffer;
}

GLuint load_texture(const char* filename) {
	int width, height, channels;
	
	stbi_set_flip_vertically_on_load(1);
	
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
	if (!data) {
		fprintf(stderr, "Failed to load image: %s\n", filename);
		return 0;
	}

	GLenum format;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA;
	else {
		fprintf(stderr, "Unsupported number of channels: %d\n", channels);
		stbi_image_free(data);
		return 0;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width,
		height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error: %d\n", error);
		glDeleteTextures(1, &texture);
		stbi_image_free(data);
		return 0;
	}

	stbi_image_free(data);

	return texture;
}

GLuint load_shader(const char* vertexPath, const char* fragmentPath) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint shaderProgram = glCreateProgram();

	char* vertexCode = read_file(vertexPath);
	glShaderSource(vertexShader, 1, (const char**)&vertexCode, NULL);
	glCompileShader(vertexShader);
	
	GLint success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n%s\n\n", infoLog, vertexPath);
	}

	char* fragmentCode = read_file(fragmentPath);
	glShaderSource(fragmentShader, 1, (const char**)&fragmentCode, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n%s\n\n", infoLog, fragmentPath);
	}

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}

	free(vertexCode);
	free(fragmentCode);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint generate_texture(int width, int height, const unsigned char* data) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return texture;
}

Position mouse_position(Window* window) {
	Position pos;
	double x, y;
	glfwGetCursorPos(window->window, &x, &y);
	pos.x = (int)x;
	pos.y = window->height - (int)y;
	return pos;
}

void mouse_state(Window *win, int *left, int *right, int *middle) {
	if (!win || !win->window) return;
	*left = glfwGetMouseButton(win->window, GLFW_MOUSE_BUTTON_LEFT);
	*right = glfwGetMouseButton(win->window, GLFW_MOUSE_BUTTON_RIGHT);
	*middle = glfwGetMouseButton(win->window, GLFW_MOUSE_BUTTON_MIDDLE);
}

Window* ck_active_window(Ck* ck) {
	if (!ck) return NULL;
	for (size_t i = 0; i < ck->window_count; ++i) {
		Window* w = ck->windows[i];
		if (glfwGetWindowAttrib(w->window, GLFW_FOCUSED))
			return w;
	}
	return NULL;
}

Window* ck_window_under_cursor(Ck* ck) {
	for (size_t i = 0; i < ck->window_count; ++i) {
		Window* w = ck->windows[i];
		if (glfwGetWindowAttrib(w->window, GLFW_HOVERED))
			return w;
	}
	return NULL;
}

void mouse_state_check(Window *win) {
	static int last_state;
	if (!win || !win->context) return;
	if (!glfwGetWindowAttrib(win->window, GLFW_FOCUSED)) return;
	int left = glfwGetMouseButton(win->window, GLFW_MOUSE_BUTTON_LEFT);
	int right = glfwGetMouseButton(win->window, GLFW_MOUSE_BUTTON_RIGHT);

	Position mousePos = mouse_position(win);

	int on_widget = 0;

	for (int i = 0; i < win->context->widget_count; i++) {
		Widget *widget = win->context->widgets[i];
		if (mousePos.x >= widget->position.x && mousePos.x <= widget->position.x + widget->size.width &&
			mousePos.y >= widget->position.y && mousePos.y <= widget->position.y + widget->size.height) {
			on_widget = 1;
			if (left == GLFW_PRESS) {
				if (last_state != GLFW_PRESS) {
					widget->state = 2;
					signal_emit(widget, CLICK);
				}
			} else {
				widget->state = 1;
				signal_emit(widget, HOVER);
			}
		} else {
			if (widget->state != 0) {
				widget->state = 0;
			}
		}
	}

	if (!on_widget && left == GLFW_PRESS && last_state != GLFW_PRESS)
		signal_emit(win, CLICK);
}

int get_alignment_offset_x(enum ALIGNMENT alignment, Size size, const char *text, Font *font) {
	int pos = 0;
	int text_width = 0;
	int line_count = 1;
	int max_width = 0;
	const char *p = text;

	while (*p) {
		uint32_t codepoint;
		int bytes = utf8_decode(p, &codepoint);
		if (!bytes)
			p++;
		else {
			if (codepoint == '\n') {
				if (max_width < text_width)
					max_width = text_width;
				line_count++;
				text_width = 0;
			}
			Glyph *glyph = (Glyph *)hashmap_get(font->glyphs, codepoint);
			if (glyph)
				text_width += glyph->advance;
			p += bytes;
		}
	}

	int ascender = font->ascender * line_count;
	text_width = max_width > text_width ? max_width : text_width;

	switch (alignment)
	{
	case ALIGN_LEFT:
		pos = 0;
		break;
	case ALIGN_CENTER:
		pos = (size.width - text_width) / 2;
		break;
	case ALIGN_RIGHT:
		pos = size.width - text_width;
		break;
	case ALIGN_TOP:
		pos = (size.width - text_width) / 2;
		break;
	case ALIGN_BOTTOM:
		pos = (size.width - text_width) / 2;
		break;
	case ALIGN_TOP_LEFT:
		pos = 0;
		break;
	case ALIGN_TOP_RIGHT:
		pos = size.width - text_width;
		break;
	case ALIGN_BOTTOM_LEFT:
		pos = 0;
		break;
	case ALIGN_BOTTOM_RIGHT:
		pos = size.width - text_width;
		break;
	default:
		break;
	}

	return pos;
}

int get_alignment_offset_y(enum ALIGNMENT alignment,Size size, int ascender, int total_lines, int line_index) {
	int total_height = ascender * total_lines;
	int top_pos = 0;
	switch (alignment)
	{
	case ALIGN_BOTTOM:
	case ALIGN_BOTTOM_LEFT:
	case ALIGN_BOTTOM_RIGHT:
		top_pos = total_height;
		break;
	case ALIGN_CENTER:
	case ALIGN_LEFT:
	case ALIGN_RIGHT:
		top_pos = size.height/2 + total_height/2;
		break;
	case ALIGN_TOP:
	case ALIGN_TOP_LEFT:
	case ALIGN_TOP_RIGHT:
		top_pos = size.height;
		break;
	}
	return top_pos - (line_index + 1) * ascender;
}

int utf8_decode(const char *str, uint32_t *codepoint) {
	if (!str || !codepoint) return 0;
	
	const unsigned char *s = (const unsigned char *)str;
	
	if (s[0] < 0x80) {
		// ASCII character (0xxxxxxx)
		*codepoint = s[0];
		return 1;
	} else if ((s[0] & 0xE0) == 0xC0) {
		// 2-byte sequence (110xxxxx 10xxxxxx)
		if ((s[1] & 0xC0) != 0x80) return 0;
		*codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
		return 2;
	} else if ((s[0] & 0xF0) == 0xE0) {
		// 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
		if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return 0;
		*codepoint = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
		return 3;
	} else if ((s[0] & 0xF8) == 0xF0) {
		// 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
		if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return 0;
		*codepoint = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
		return 4;
	}
	
	return 0;
}

int utf8_strlen(const char *str) {
	if (!str) return 0;
	
	int len = 0;
	uint32_t codepoint;
	int bytes;
	
	while (*str) {
		bytes = utf8_decode(str, &codepoint);
		if (bytes == 0) {
			str++;
		} else {
			str += bytes;
			len++;
		}
	}
	
	return len;
}

int text_width(const char* text, HashMap* glyphs) {
	int width = 0;
	int max_width = 0;
	uint32_t codepoint;
	int bytes;
	while (*text) {
		bytes = utf8_decode(text, &codepoint);
		if (!bytes)
			text++;
		else {
			text += bytes;
			if (codepoint == '\n') {
				max_width = width > max_width ? width : max_width;
				width = 0;
				continue;
			}
			Glyph *g = (Glyph *)hashmap_get(glyphs, codepoint);
			width += g->advance;
		}
	}
	return width > max_width ? width : max_width;
}

int line_count(const char *str, int width, HashMap *glyphs) {
	int linecount = 0;

	char *cpy = malloc(strlen(str) + 1);
	strcpy(cpy, str);
	char *line = strtok(cpy, "\n");
	while (line) {
		linecount += (text_width(line, glyphs) + width - 1) / width;
		line = strtok(NULL, "\n");
	}
	free(cpy);
	return linecount;
}