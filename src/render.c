#define _USE_MATH_DEFINES
#include "../libs/ck.h"
#include "../libs/ck_internal.h"

static inline void render_text(textRenderParameters params) {
	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glUseProgram(params.shaderProgram);

	int width, height;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
	float screenSize[] = { (float)width, (float)height };
	glUniform2fv(glGetUniformLocation(params.shaderProgram, "screenSize"), 1, screenSize);
	glUniform3fv(glGetUniformLocation(params.shaderProgram, "textColor"), 1, params.color);

	glActiveTexture(GL_TEXTURE0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int lineCount = 0;
	const char* p = params.text;
	uint32_t codepoint;
	int bytes;
	
	while (*p) {
		bytes = utf8_decode(p, &codepoint);
		if (bytes == 0) {
			p++;
			continue;
		}
		if (codepoint == '\n') {
			lineCount++;
		}
		p += bytes;
	}
	
	params.y += lineCount * params.font->ascender * params.scale;
	float lineStart = params.x;
	
	p = params.text;
	while (*p) {
		bytes = utf8_decode(p, &codepoint);
		if (bytes == 0) {
			p++;
			continue;
		}
		
		if (codepoint == '\n') {
			params.x = lineStart;
			params.y -= params.font->ascender * params.scale;
			p += bytes;
			continue;
		}
		Glyph *glyph = (Glyph *)hashmap_get(params.font->glyphs, (long long int)codepoint);
		if (!glyph) {
			p += bytes;
			continue;
		}

		float xpos = params.x + glyph->bearingX * params.scale;
		float ypos = params.y - (glyph->height - glyph->bearingY + params.font->descender) * params.scale;

		float w = glyph->width * params.scale;
		float h = glyph->height * params.scale;

		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos,     ypos,       0.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f }
		};
		
		glBindTexture(GL_TEXTURE_2D, glyph->textureID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		params.x += (glyph->advance) * params.scale;
		p += bytes;
	}
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glUseProgram(0);
}

static inline void render_texture(textureRenderParameters params) {
	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glUseProgram(params.shaderProgram);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.textureID);

	int width, height;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
	float screenSize[] = { (float)width, (float)height };
	glUniform2fv(glGetUniformLocation(params.shaderProgram, "screenSize"), 1, screenSize);
	glUniform3fv(glGetUniformLocation(params.shaderProgram, "tintColor"), 1, params.color);
	glUniform1f(glGetUniformLocation(params.shaderProgram, "intensity"), params.intensity);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float xpos = params.x;
	float ypos = params.y;

	float w = params.width;
	float h = params.height;

	GLfloat vertices[6][4] = {
		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos + w, ypos,       1.0f, 1.0f },
		{ xpos,     ypos,       0.0f, 1.0f },

		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos + w, ypos + h,   1.0f, 0.0f },
		{ xpos + w, ypos,       1.0f, 1.0f }
	};
		
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glUseProgram(0);
}

static inline void render_line(GLuint lineShaderProgram, Line line, int width, int height) {
	glUseProgram(lineShaderProgram);

	glUniform3fv(glGetUniformLocation(lineShaderProgram, "lineColor"), 1, line.color);
	glUniform1i(glGetUniformLocation(lineShaderProgram, "erase"), line.erase);
	glEnable(GL_BLEND);
	if (line.erase)
		glBlendFunc(GL_ZERO, GL_ZERO);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform2f(glGetUniformLocation(lineShaderProgram, "screenSize"), (float)width, (float)height);


	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	printf("drawing line from (%d, %d) to (%d, %d) with thickness %.2f\n", line.start.x, line.start.y, line.end.x, line.end.y, line.thickness);
	float dx = line.start.x - line.end.x;
	float dy = line.start.y - line.end.y;
	float length = sqrt(dx * dx + dy * dy);
	
	if (length > 0) {
		float perpX = -dy / length * line.thickness * 0.5f;
		float perpY = dx / length * line.thickness * 0.5f;

		Position lineVertices[6] = {
			{line.start.x + perpX, line.start.y + perpY},  // Top-left
			{line.start.x - perpX, line.start.y - perpY},  // Bottom-left
			{line.end.x + perpX, line.end.y + perpY},      // Top-right

			{line.start.x - perpX, line.start.y - perpY},  // Bottom-left
			{line.end.x - perpX, line.end.y - perpY},      // Bottom-right
			{line.end.x + perpX, line.end.y + perpY}       // Top-right
		};

		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Position), lineVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Position), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	const int circleSegments = 16;
	float radius = line.thickness * 0.5f;

	for (int cap = 0; cap < 2; cap++) {
		Position center = (cap == 0) ? line.start : line.end;

		Position circleVertices[circleSegments * 3];

		for (int i = 0; i < circleSegments; i++) {
			float angle1 = (float)i / circleSegments * 2.0f * M_PI;
			float angle2 = (float)(i + 1) / circleSegments * 2.0f * M_PI;

			circleVertices[i * 3] = (Position){center.x, center.y};
			circleVertices[i * 3 + 1] = (Position){
				center.x + cos(angle1) * radius,
				center.y + sin(angle1) * radius
			};
			circleVertices[i * 3 + 2] = (Position){
				center.x + cos(angle2) * radius,
				center.y + sin(angle2) * radius
			};
		}

		glBufferData(GL_ARRAY_BUFFER, circleSegments * 3 * sizeof(Position), circleVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Position), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, circleSegments * 3);
	}
		
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glUseProgram(0);
}

void render_wrapped_text(Widget *widget, Window *win) {
	textRenderParameters textParams = {
		.shaderProgram = win->shaderPrograms[0],
		.font = widget->font,
		.scale = 1.0f,
		.color = {widget->text_color[0], widget->text_color[1], widget->text_color[2]}
	};
	
	int total_lines = line_count(widget->text, widget->size.width, widget->font->glyphs);
	int line_index = 0;
	
	const char *text_ptr = widget->text;
	
	while (*text_ptr) {
		const char *line_start = text_ptr;
		const char *line_end = text_ptr;
		
		while (*line_end && *line_end != '\n') {
			uint32_t codepoint;
			int bytes = utf8_decode(line_end, &codepoint);
			if (bytes > 0) {
				line_end += bytes;
			} else {
				line_end++;
			}
		}
		
		int line_length = line_end - line_start;
		char *current_line = malloc(line_length + 1);
		strncpy(current_line, line_start, line_length);
		current_line[line_length] = '\0';
		
		const char *line_ptr = current_line;
		
		while (*line_ptr) {
			int segment_width = 0;
			const char *char_start = line_ptr;
			const char *char_end = line_ptr;
			
			while (*char_end) {
				uint32_t codepoint;
				int bytes = utf8_decode(char_end, &codepoint);
				if (bytes == 0) {
					char_end++;
					continue;
				}
				
				Glyph *glyph = (Glyph *)hashmap_get(widget->font->glyphs, (long long int)codepoint);
				if (!glyph) {
					char_end += bytes;
					continue;
				}
				
				if (segment_width + glyph->advance > widget->size.width) {
					break;
				}
				
				segment_width += glyph->advance;
				char_end += bytes;
			}
			
			int segment_length = char_end - char_start;
			if (segment_length > 0) {
				char *substr = malloc(segment_length + 1);
				strncpy(substr, char_start, segment_length);
				substr[segment_length] = '\0';
				
				int offset_x = get_alignment_offset_x(widget->text_alignment, widget->size, substr, widget->font);
				int offset_y = get_alignment_offset_y(widget->text_alignment, widget->size, widget->font->ascender, total_lines, line_index);
				textParams.text = substr;
				textParams.x = widget->position.x + offset_x;
				textParams.y = widget->position.y + offset_y;
				render_text(textParams);
				
				free(substr);
			}
			line_index++;
			line_ptr = char_end;
			
			if (char_end == char_start) {
				if (*line_ptr) {
					uint32_t codepoint;
					int bytes = utf8_decode(line_ptr, &codepoint);
					line_ptr += (bytes > 0) ? bytes : 1;
				}
			}
		}
		
		free(current_line);
		
		text_ptr = line_end;
		if (*text_ptr == '\n') {
			text_ptr++;
		}
	}
}

void render_text_with_resize(Widget *widget, Window *win) {

}

static inline void set_bound(textureRenderParameters params) {	
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilMask(0xFF);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	render_texture(params);
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

int render_widget(Widget *widget, Window *win) {
	if (!widget || !win) return -1;

	textureRenderParameters params = {
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 0.0f, 0.0f, 0.0f },
		.intensity = 0.0f,
		.textureID = 0
	};
	set_bound(params);

	textureRenderParameters textureParams = {
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 0.0f, 0.0f, 0.0f },
		.intensity = (widget->state * 0.4f),
		.textureID = win->textures[widget->texture_index]
	};
	render_texture(textureParams);
	render_wrapped_text(widget, win);

	glDisable(GL_STENCIL_TEST);

	return 0;
}

int render_canvas(Widget *widget, Window *win) {
	if (!widget || !win) return -1;

	if (!widget->data) {
		fprintf(stderr, "Widget data is NULL\n");
		return -1;
	}

	canvasData *canvas = (canvasData *)widget->data;
	if (!canvas) {
		fprintf(stderr, "Canvas data is NULL\n");
		return -1;
	}

	textureRenderParameters params = {
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 0.0f, 0.0f, 0.0f },
		.intensity = 0.0f,
		.textureID = 0
	};
	set_bound(params);

	textureRenderParameters textureParams = {
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 0.0f, 0.0f, 0.0f },
		.intensity = 0.0f,
		.textureID = win->textures[widget->texture_index]
	};
	render_texture(textureParams);

	if (canvas->lineQueue) {
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glBindFramebuffer(GL_FRAMEBUFFER, canvas->FBO);
		glViewport(0, 0, widget->size.width, widget->size.height);

		while (canvas->lineQueue) {
			Line line = canvas->lineQueue->val;
			render_line(win->shaderPrograms[2], line, widget->size.width, widget->size.height);
			dequeue_line(&canvas->lineQueue);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	}

	textureParams = (textureRenderParameters){
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 1.0f, 1.0f, 1.0f },
		.intensity = 0.0f,
		.textureID = canvas->bitmap
	};
	render_texture(textureParams);

	render_wrapped_text(widget, win);

	glDisable(GL_STENCIL_TEST);

	return 0;
}

int render_textbox(Widget *widget, Window *win) {
	if (!widget || !win) return -1;
	if (!widget->font) {
		fprintf(stderr, "Widget font is NULL\n");
		return -1;
	}

	textboxData *data = (textboxData *)widget->data;

	textureRenderParameters params = {
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 0.0f, 0.0f, 0.0f },
		.intensity = 0.0f,
		.textureID = 0
	};
	set_bound(params);

	textureRenderParameters textureParams = {
		.shaderProgram = win->shaderPrograms[1],
		.x = widget->position.x,
		.y = widget->position.y,
		.width = widget->size.width,
		.height = widget->size.height,
		.color = { 0.0f, 0.0f, 0.0f },
		.intensity = 0.0f,
		.textureID = win->textures[widget->texture_index]
	};
	render_texture(textureParams);

	if (!data->autoresize)
		render_wrapped_text(widget, win);
	else

	glDisable(GL_STENCIL_TEST);
	
	return 0;
}

static inline int render_context(Context *ctx, Window *win) {
	if (!ctx || !win) {
		return -1;
	}

	signal_emit(ctx, REDRAW);

	if (!ctx->widgets) {
		return 0;
	}

	for (int i = 0; i < ctx->widget_count; i++) {
		if (ctx->widgets[i]->render_func) {
			signal_emit(ctx->widgets[i], REDRAW);
			if (ctx->widgets[i]->render_func(ctx->widgets[i], win) != 0) {
				fprintf(stderr, "Widget %d render function failed\n", i);
				return -1;
			}
		}
	}

	return 0;
}

int render_window(Window *win) {
	if (!win || !win->window || !win->context) {
		return -1;
	}
	signal_emit(win, REDRAW);
	glfwMakeContextCurrent(win->window);
	glClearColor(win->context->clear_color[0], win->context->clear_color[1],
				 win->context->clear_color[2], win->context->clear_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (win->context) {
		if (render_context(win->context, win) != 0) {
			fprintf(stderr, "Context render function failed\n");
		}
	}
	return 0;
}