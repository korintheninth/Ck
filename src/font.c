#include "../libs/ck.h"
#include "../libs/ck_internal.h"

void load_font(const char* fontPath, FT_Library* ft, FT_Face* face, int fontSize) {
	if (FT_Init_FreeType(ft)) {
		fprintf(stderr, "Could not initialize FreeType library\n");
		exit(EXIT_FAILURE);
	}

	if (FT_New_Face(*ft, fontPath, 0, face)) {
		fprintf(stderr, "Could not load font: %s\n", fontPath);
		exit(EXIT_FAILURE);
	}

	FT_Set_Pixel_Sizes(*face, 0, fontSize);
}

HashMap *generate_font_texture(FT_Face face) {
	HashMap* glyphs = hashmap_create(face->num_glyphs);
	if (!glyphs) {
		fprintf(stderr, "Failed to allocate memory for glyphs\n");
		exit(EXIT_FAILURE);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	FT_ULong charcode;
	FT_UInt gindex;
	
	charcode = FT_Get_First_Char(face, &gindex);
	
	while (gindex != 0) {
		if (FT_Load_Glyph(face, gindex, FT_LOAD_RENDER)) {
			fprintf(stderr, "Failed to load glyph for character: %lu (U+%04lX)\n", charcode, charcode);
			charcode = FT_Get_Next_Char(face, charcode, &gindex);
			continue;
		}

		Glyph* glyph = malloc(sizeof(Glyph));
		if (!glyph) {
			fprintf(stderr, "Failed to allocate memory for glyph\n");
			charcode = FT_Get_Next_Char(face, charcode, &gindex);
			continue;
		}
		
		glyph->width = face->glyph->bitmap.width;
		glyph->height = face->glyph->bitmap.rows;
		glyph->bearingX = face->glyph->bitmap_left;
		glyph->bearingY = face->glyph->bitmap_top;
		glyph->advance = face->glyph->advance.x >> 6;

		glGenTextures(1, &glyph->textureID);
		glBindTexture(GL_TEXTURE_2D, glyph->textureID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			glyph->width,
			glyph->height,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		hashmap_insert(glyphs, (int)charcode, glyph);
		
		charcode = FT_Get_Next_Char(face, charcode, &gindex);
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return glyphs;
}

Font *get_font(const char* fontPath, int fontSize, FT_Library *ft) {
	FT_Face face;
	Font *font = malloc(sizeof(Font));
	if (!font) {
		fprintf(stderr, "Failed to allocate memory for Font\n");
		return NULL;
	}

	load_font(fontPath, ft, &face, fontSize);
	HashMap* glyphs = generate_font_texture(face);
	font->glyphs = glyphs;
	font->lineHeight = face->height >> 6;
	font->ascender = face->ascender >> 6;
	font->descender = face->descender >> 6;
	font->fontSize = fontSize;
	FT_Done_Face(face);

	return font;
}

void free_font(Font *font) {
	if (font) {
		for (int i = 0; hashmap_get(font->glyphs, i); i++) {
			glDeleteTextures(1, &(((Glyph *)hashmap_get(font->glyphs, i))->textureID));
		}
		free(font->glyphs);
		free(font);
	}
}

