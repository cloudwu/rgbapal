#include "image.h"

int
main() {
	bitmap_gamma_init(0.5f);
	struct bitmap_rgba bm;
	struct bitmap_index bi;
	bitmap_load("button2.png", &bm);
	bitmap_index(&bm, &bi);
	bitmap_save("pal2.png", &bm);
	bitmap_close(&bm);
	bitmap_index_close(&bi);
	return 0;
}
