#ifndef image_h
#define image_h

#include <stdint.h>

struct bitmap_rgba {
	int w;
	int h;
	uint8_t *pixel;
};

struct bitmap_index {
	int w;
	int h;
	uint8_t pal[256][4];
	uint8_t *pixel;
};

int bitmap_load(const char *filename, struct bitmap_rgba *img);
int bitmap_write(const char *filename, struct bitmap_rgba *img);
void bitmap_close(struct bitmap_rgba *img);

void bitmap_gamma_init(float v);
void bitmap_index(struct bitmap_rgba *img, struct bitmap_index *img_index);
void bitmap_index_close(struct bitmap_index *img_index);

#endif
