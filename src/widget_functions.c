#include "../libs/ck.h"
#include "../libs/ck_internal.h"

void enqueue_line(Position start, Position end, bool erase, GLfloat color[3], float thickness, drawQueue **queue) {
	if (start.x == end.x && start.y == end.y) return;

	drawQueue *newLine = malloc(sizeof(drawQueue));
	if (!newLine) {
		fprintf(stderr, "Failed to allocate memory for new line\n");
		return;
	}
	newLine->val.start = start;
	newLine->val.end = end;
	newLine->val.thickness = thickness;
	newLine->val.color[0] = color[0];
	newLine->val.color[1] = color[1];
	newLine->val.color[2] = color[2];
	newLine->val.erase = erase;
	newLine->next = NULL;

	if (*queue == NULL) {
		*queue = newLine;
	} else {
		drawQueue *current = *queue;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = newLine;
	}
}

void dequeue_line(drawQueue **queue) {
	if (queue == NULL || *queue == NULL) return;

	drawQueue *temp = *queue;
	*queue = (*queue)->next;
	free(temp);

}

int queue_length(drawQueue **queue) {
	int length = 0;
	drawQueue *current = *queue;
	while (current != NULL) {
		length++;
		current = current->next;
	}
	return length;
}

int draw_line_to_canvas(Position start, Position end, bool erase, GLfloat color[3], float thickness, Widget *canvas) {
	if (!canvas) {
		fprintf(stderr, "Canvas is NULL\n");
		return -1;
	}

	if (start.x == end.x && start.y == end.y) return 0;

	enqueue_line(start, end, erase, color, thickness, &(((canvasData*)canvas->data)->lineQueue));
	return 0;
}

int set_widget_texture(Ck *ck, Widget *widget, const char *texture_path) {
	if (!widget || !texture_path) return -1;

	Window *win = ck->windows[0];
	if (!win) {
		fprintf(stderr, "No window available to set texture\n");
		return -1;
	}
	GLuint textureID = load_texture(texture_path);
	if (textureID == 0) {
		fprintf(stderr, "Failed to load texture: %s\n", texture_path);
		return -1;
	}
	int textureCount = 0;
	while (win->textures[textureCount] != 0) textureCount++;
	win->textures = realloc(win->textures, sizeof(GLuint) * (textureCount + 2));
	win->textures[textureCount] = textureID;
	win->textures[textureCount + 1] = 0;
	widget->texture_index = textureCount;
	return 0;
}

int textbox_width(Widget *widget) {
	int width = 0;
	char *copy = strdup(widget->text);
	char *line = strtok(copy, "\n");
	while (line != NULL) {
		int lineWidth = text_width(line, widget->font->glyphs);
		if (lineWidth > width) {
			width = lineWidth;
		}
		line = strtok(NULL, "\n");
	}
	free(copy);
	return width + 20;
}