#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image.h"

int
bitmap_load(const char *filename, struct bitmap_rgba *img) {
	int ch = 0;
	img->pixel = (stbi_uc *)stbi_load(filename, &img->w, &img->h, &ch, 4);
	if (img->pixel == NULL)
		return 0;
	return 1;
}

int
bitmap_save(const char *filename, struct bitmap_rgba *img) {
	if (img->pixel == NULL)
		return 1;
	return stbi_write_png(filename, img->w, img->h, 4, img->pixel, 0);
}

void
bitmap_close(struct bitmap_rgba *img) {
	if (img->pixel) {
		stbi_image_free(img->pixel);
		img->pixel = NULL;
	}
}

