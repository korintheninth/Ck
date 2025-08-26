#ifndef CK_H
#define CK_H

#include "external/glew-2.1.0/include/GL/glew.h"
#include "external/glfw-3.4.bin.WIN64/include/GLFW/glfw3.h"
#include "external/freetype/ft2build.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include FT_FREETYPE_H

typedef struct Font Font;
typedef struct Window Window;
typedef struct Context Context;
typedef struct Widget Widget;

enum SIGNAL {
	ACTIVATE,
	DEACTIVATE,
	CLICK,
	HOVER,
	RESIZE,
	REDRAW,
	USER_SIGNAL1,
	USER_SIGNAL2,
	USER_SIGNAL3
};

typedef void (*SignalHandler)(void *sender, void *data);

typedef struct SignalManager {
	enum SIGNAL signal;
	SignalHandler handler;
	void *data;
	struct SignalManager *next;
} SignalManager;

enum ALIGNMENT {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_TOP,
	ALIGN_BOTTOM,
	ALIGN_TOP_LEFT,
	ALIGN_TOP_RIGHT,
	ALIGN_BOTTOM_LEFT,
	ALIGN_BOTTOM_RIGHT
};

typedef struct Ck {
	FT_Library ft;
	Window **windows;
	int window_count;
} Ck;

typedef struct Size {
	int width;
	int height;
} Size;

typedef struct Position {
	float x;
	float y;
} Position;

typedef struct Widget {
	Position position;
	Size size;
	Font *font; //Optimise this later, maybe use a font manager
	char *text;
	enum ALIGNMENT text_alignment;
	float text_color[3];
	int texture_index;
	void *data;
	int state; // 0: normal, 1: hovered, 2: clicked
	int (*render_func)(struct Widget *widget, Window *win);
} Widget;

typedef struct Context {
	Widget **widgets;
	int widget_count;
	GLclampf clear_color[4];
} Context;

typedef struct Window {
	GLFWwindow *window;
	int width;
	int height;
	char *title;
	Context *context;
	GLuint shaderPrograms[3];
	GLuint *textures;
} Window;

typedef struct Bucket {
	long long int key;
	void *value;
	struct Bucket *next;
} Bucket;

typedef struct HashMap {
	Bucket **buckets;
	size_t size, count;
} HashMap;

//CK functions

Ck *initCK();
void destroyCK(Ck *ck);
int loopCK(Ck *ck);

//Window functions

Window *create_window(Ck *ck, int width, int height, const char *title);
void destroy_window(Window *win);
void set_window_title(Window *win, const char *title);
int set_window_size(Window *win, int width, int height);

//Context functions

Context *create_context();
void destroy_context(Context *ctx);
int add_widget(Context *ctx, Widget *widget);

//Widget functions

Widget *create_push_button(Ck *ck, Position position, Size size, const char *font_name, const char *text, enum ALIGNMENT text_alignment, float text_color[3]);
Widget *create_canvas(Ck *ck, Position position, Size size, const char *font_name, const char *text, enum ALIGNMENT text_alignment, float text_color[3]);
Widget *create_textbox(Ck *ck, Position position, Size size, const char *font_name, const char *text, float text_color[3], bool autoresize);
int destroy_widget(Widget *widget);
int draw_line_to_canvas(Position start, Position end, bool erase, GLfloat color[3], float thickness, Widget *canvas);
int set_widget_texture(Ck *ck, Widget *widget, const char *texture_path);

//utility functions

//Returns the mouse position relative to the bottom left corner of the window
Position mouse_position(Window* window);
void mouse_state(Window *win, int *left, int *right, int *middle);
Window* ck_active_window(Ck* ck);
Window* ck_window_under_cursor(Ck* ck);

//signal functions

void signal_connect(void *sender, enum SIGNAL signal, SignalHandler func, void *data);
void signal_disconnect(void *sender, enum SIGNAL signal, SignalHandler func);

//HashMap functions

HashMap *hashmap_create(size_t size);
void hashmap_destroy(HashMap *map);
int hashmap_insert(HashMap *map, long long int key, void *value);
void *hashmap_get(HashMap *map, long long int key);
void *hashmap_replace(HashMap *map, long long int key, void* value);
HashMap *hashmap_resize(HashMap *map, int size);
int hashmap_remove(HashMap *map, long long int key);

// UTF-8 support functions
int utf8_decode(const char *str, uint32_t *codepoint);
int utf8_strlen(const char *str);

#endif