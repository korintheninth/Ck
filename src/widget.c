#include "../libs/ck.h"
#include "../libs/ck_internal.h"

int destroy_widget(Widget *widget) {
	if (!widget) return -1;

	if (widget->font) {
		free_font(widget->font);
	}

	if (widget->text) {
		free(widget->text);
	}

	if (widget->data) {
		free(widget->data);
	}

	free(widget);
	return 0;
}

static inline Widget *create_widget(Ck *ck, Position position, Size size, const char *font_name,
							const char *text, enum ALIGNMENT text_alignment, float text_color[3]) {

	Font *font = get_font(font_name, 16, &ck->ft);
	if (!font) {
		fprintf(stderr, "Failed to get font: %s\n", font_name);
		return NULL;
	}

	Widget *widget = malloc(sizeof(Widget));
	if (!widget) {
		fprintf(stderr, "Failed to allocate memory for Widget\n");
		free_font(font);
		return NULL;
	}
	if (text) {
		widget->text = malloc(strlen(text) + 1);
		if (!widget->text) {
			fprintf(stderr, "Failed to allocate memory for widget text\n");
			free_font(font);
			free(widget);
			return NULL;
		}
		strcpy(widget->text, text);
	} else {
		widget->text = NULL;
	}
	widget->position = position;
	widget->size = size;
	widget->font = font;
	widget->text_alignment = text_alignment;
	widget->text_color[0] = text_color[0];
	widget->text_color[1] = text_color[1];
	widget->text_color[2] = text_color[2];
	widget->state = 0;
	}

Widget *create_push_button(Ck *ck, Position position, Size size, const char *font_name,
							const char *text, enum ALIGNMENT text_alignment, float text_color[3]) {


	Widget *button = create_widget(ck, position, size, font_name, text, text_alignment, text_color);
	if (!button) {
		fprintf(stderr, "Failed to create push button widget\n");
		return NULL;
	}
	button->texture_index = 0;
	button->data = NULL;
	button->render_func = render_widget;

	signal_emit(button, ACTIVATE);
	return button;
}

Widget *create_canvas(Ck *ck, Position position, Size size, const char *font_name,
						const char *text, enum ALIGNMENT text_alignment, float text_color[3]) {

	Widget *canvas = create_widget(ck, position, size, font_name, text, text_alignment, text_color);
	if (!canvas) {
		fprintf(stderr, "Failed to create canvas widget\n");
		return NULL;
	}

	canvasData *data = malloc(sizeof(canvasData));
	if (!data) {
		fprintf(stderr, "Failed to allocate memory for canvas data\n");
		free_font(canvas->font);
		free(canvas);
		return NULL;
	}

	data->lineQueue = NULL;
	
	data->bitmap = generate_texture(size.width, size.height, NULL);
	
	glGenFramebuffers(1, &data->FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, data->FBO);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data->bitmap, 0);
	
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Framebuffer is not complete\n");
		glDeleteFramebuffers(1, &data->FBO);
		glDeleteTextures(1, &data->bitmap);
		free(data);
		free_font(canvas->font);
		free(canvas);
		return NULL;
	}
	
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glViewport(0, 0, size.width, size.height);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	canvas->data = data;
	canvas->texture_index = 1;
	canvas->render_func = render_canvas;

	signal_emit(canvas, ACTIVATE);

	return canvas;
}

Widget *create_textbox(Ck *ck, Position position, Size size, const char *font_name,
						const char *text, float text_color[3], bool autoresize) {
	Widget *textbox = create_widget(ck, position, size, font_name, text, ALIGN_TOP_LEFT, text_color);
	if (!textbox) {
		fprintf(stderr, "Failed to create textbox widget\n");
		return NULL;
	}
	textbox->data = malloc(sizeof(textboxData));
	((textboxData *)textbox->data)->autoresize = autoresize;
	textbox->texture_index = 2;
	textbox->render_func = render_textbox;

	signal_emit(textbox, ACTIVATE);

	return textbox;
}