#include "../libs/ck.h"

Context *create_context() {
	Context *ctx = (Context *)malloc(sizeof(Context));
	if (!ctx) {
		fprintf(stderr, "Failed to allocate memory for Context\n");
		return NULL;
	}

	ctx->widgets = NULL;
	ctx->widget_count = 0;
	ctx->clear_color[0] = 0.0f;
	ctx->clear_color[1] = 0.0f;
	ctx->clear_color[2] = 0.0f;
	ctx->clear_color[3] = 0.0f;

	return ctx;
}

void destroy_context(Context *ctx) {
	if (ctx) {
		if (ctx->widgets) {
			for (int i = 0; i < ctx->widget_count; i++) {
				destroy_widget(ctx->widgets[i]);
			}
			free(ctx->widgets);
		}
		free(ctx);
	}
}

int add_widget(Context *ctx, Widget *widget) {
	if (!ctx || !widget) {
		return -1;
	}

	Widget **new_widgets = (Widget **)realloc(ctx->widgets, sizeof(Widget *) * (ctx->widget_count + 1));
	if (!new_widgets) {
		fprintf(stderr, "Failed to allocate memory for widgets\n");
		return -1;
	}

	ctx->widgets = new_widgets;
	ctx->widgets[ctx->widget_count] = widget;
	ctx->widget_count++;
	return 0;
}

int remove_widget(Context *ctx, Widget *widget) {
	if (!ctx || !widget) {
		return -1;
	}

	for (int i = 0; i < ctx->widget_count; i++) {
		if (ctx->widgets[i] == widget) {
			destroy_widget(widget);
			memmove(&ctx->widgets[i], &ctx->widgets[i + 1],
				 (ctx->widget_count - i - 1) * sizeof(Widget *));
			ctx->widget_count--;
			return 0;
		}
	}
	return -1;
}