#ifndef CK_INTERNAL_H
#define CK_INTERNAL_H

#include "ck.h"

typedef struct Glyph {
	GLuint textureID;
	int width;
	int height;
	int bearingX;
	int bearingY;
	unsigned int advance;
} Glyph;

typedef struct Font {
	HashMap* glyphs;
	int fontSize;
	int lineHeight;
	int ascender;
	int descender;
} Font;

typedef struct textRenderParameters {
	GLuint shaderProgram;
	Font* font;
	const char* text;
	float x;
	float y;
	float scale;
	float color[3];
} textRenderParameters;

typedef struct textureRenderParameters {
	GLuint shaderProgram;
	float x;
	float y;
	int width;
	int height;
	float color[3];
	float intensity;
	GLuint textureID;
} textureRenderParameters;

typedef struct Line {
	Position start;
	Position end;
	float thickness;
	GLfloat color[3];
	bool erase;
} Line;

typedef struct drawQueue {
	Line val;
	struct drawQueue *next;
} drawQueue;

typedef struct canvasData {
	GLuint bitmap;
	GLuint FBO;
	drawQueue *lineQueue;
} canvasData;

typedef struct textboxData {
	bool autoresize;
} textboxData;

//utility functions

char *read_file(const char* filename);
GLuint load_texture(const char* filename);
GLuint load_shader(const char* vertexPath, const char* fragmentPath);
GLuint generate_texture(int width, int height, const unsigned char* data);
void mouse_state_check(Window *win);
int get_alignment_offset_x(enum ALIGNMENT alignment, Size size, const char *text, Font *font);
int get_alignment_offset_y(enum ALIGNMENT alignment,Size size, int ascender, int total_lines, int line_index);
int text_width(const char* text, HashMap* glyphs);
int line_count(const char *str, int width, HashMap *glyphs);

//Font functions

Font* get_font(const char* fontPath, int fontSize, FT_Library *ft);
void free_font(Font *font);

// Render functions

int render_widget(Widget *widget, Window *win);
int render_canvas(Widget *widget, Window *win);
int render_textbox(Widget *widget, Window *win);
int render_window(Window *win);

// Widget functions

//window functions

void window_thread(void *win_ptr);

void enqueue_line(Position start, Position end, bool erase, GLfloat color[3], float thickness, drawQueue **queue);
void dequeue_line(drawQueue **queue);
int queue_length(drawQueue **queue);

//callback functions

void error_callback(int error, const char* description);
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_close_callback(GLFWwindow* window);

//signal functions
void signal_emit(void *sender, enum SIGNAL signal);
#endif