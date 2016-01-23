#include "image.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct color {
	uint8_t c[4];
	int index;
	int index_color;
};

struct color_range {
	uint8_t up;
	uint8_t down;
};

void
bitmap_index_close(struct bitmap_index *img_index) {
	if (img_index->pixel) {
		free(img_index->pixel);
		img_index->pixel = NULL;
	}
}

static void
calc_color(struct color *tmp, int sz, uint8_t c[4]) {
	int color[4] = {0,0,0,0};
	int i,j;
	for (i=0;i<sz;i++) {
		for (j=0;j<4;j++) {
			color[j] += tmp[i].c[j];
		}
	}
	for (j=0;j<4;j++) {
		c[j] = color[j] / sz;
	}
}

static int
comp_color(const void *a, const void *b) {
	const struct color *ca = a;
	const struct color *cb = b;
	return memcmp(ca->c,cb->c,4);
}

static int
comp_color_1(const void *a, const void *b) {
	const struct color *ca = a;
	const struct color *cb = b;
	uint8_t aa[4];
	uint8_t bb[4];
	aa[0] = ca->c[1];
	aa[1] = ca->c[0];
	aa[2] = ca->c[2];
	aa[3] = ca->c[3];

	bb[0] = cb->c[1];
	bb[1] = cb->c[0];
	bb[2] = cb->c[2];
	bb[3] = cb->c[3];

	return memcmp(aa,bb,4);
}

static int
comp_color_2(const void *a, const void *b) {
	const struct color *ca = a;
	const struct color *cb = b;
	uint8_t aa[4];
	uint8_t bb[4];
	aa[0] = ca->c[2];
	aa[1] = ca->c[0];
	aa[2] = ca->c[1];
	aa[3] = ca->c[3];

	bb[0] = cb->c[2];
	bb[1] = cb->c[0];
	bb[2] = cb->c[1];
	bb[3] = cb->c[3];

	return memcmp(aa,bb,4);
}

static int
comp_color_3(const void *a, const void *b) {
	const struct color *ca = a;
	const struct color *cb = b;
	uint8_t aa[4];
	uint8_t bb[4];
	aa[0] = ca->c[3];
	aa[1] = ca->c[0];
	aa[2] = ca->c[1];
	aa[3] = ca->c[2];

	bb[0] = cb->c[3];
	bb[1] = cb->c[0];
	bb[2] = cb->c[1];
	bb[3] = cb->c[2];

	return memcmp(aa,bb,4);
}

/*

static int
find_med(struct color *tmp, int sz, int ch) {
	int med = sz / 2;
	int i;
	for (i = med; i>0; i--) {
		int c = tmp[i].c[ch];
		int nc = tmp[i+1].c[ch];
		if (c!=nc)
			return i;
	}
	return 1;
}


static int
find_med(struct color *tmp, int sz, int ch) {
	int i;
	int b = 0;
	int c = tmp[0].c[ch];
	int p = 0;
	for (i=1;i<sz;i++) {
		int c2 = tmp[i].c[ch];
		int b2 = c2 - c;
		if (b2 > 0) {
			c=c2;
			if (b2 > b) {
				p = i;
				b = b2;
			}
		}
	}
	return p;
}
*/

static void
mediancut(struct color *tmp, int sz, struct bitmap_index *idx, int pfrom, int pn) {
	if (pn == 1) {
		uint8_t c[4];
		calc_color(tmp, sz, c);
		memcpy(idx->pal[pfrom],c,4);
		int i;
		for (i=0;i<sz;i++) {
			tmp[i].index_color = pfrom;
		}
		return;
	}
	if (sz <= pn) {
		int i;
		for (i=0;i<sz;i++) {
			memcpy(idx->pal[pfrom+i],tmp[i].c,4);
			tmp[i].index_color = pfrom+i;
		}
		for (;i<pn;i++) {
			idx->pal[i][0] = 0;
			idx->pal[i][1] = 0;
			idx->pal[i][2] = 0;
			idx->pal[i][3] = 0;
		}
		return;
	}
	struct color_range range[4];
	int i,j;
	for (i=0;i<4;i++) {
		range[i].up = range[i].down = tmp[0].c[i];
	}
	for (i=1;i<sz;i++) {
		for (j=0;j<4;j++) {
			int c = tmp[i].c[j];
			if (c < range[j].up) {
				range[j].up = c;
			} else if (c > range[j].down) {
				range[j].down = c;
			}
		}
	}
	int ch = 0, range_c = range[0].down - range[0].up;
	for (i=1;i<4;i++) {
		int t = range[i].down - range[i].up;
		if (t > range_c) {
			ch = i;
			range_c = t;
		}
	}
	switch (ch) {
	case 0:
		qsort(tmp, sz, sizeof(struct color), comp_color);
		break;
	case 1:
		qsort(tmp, sz, sizeof(struct color), comp_color_1);
		break;
	case 2:
		qsort(tmp, sz, sizeof(struct color), comp_color_2);
		break;
	case 3:
		qsort(tmp, sz, sizeof(struct color), comp_color_3);
		break;
	}
	// med :  sz == pal_n : pn
	int med = sz / 2;
	int pal_n = pn / 2;
//	int med = find_med(tmp, sz, ch);
//	int pal_n = med * pn / sz;
//	printf("med(%d) : sz(%d) == pal_n(%d) : pn(%d)\n",med, sz, pal_n, pn);
//	if (pal_n == 0) {
//		++ pal_n;
//	}
	mediancut(tmp, med, idx, pfrom, pal_n);
	mediancut(tmp+med, sz - med, idx, pfrom + pal_n, pn - pal_n);
}

static int
unique_color(struct color *tmp, struct bitmap_rgba *img, int sz) {
	int i;
	for (i=0;i<sz;i++) {
		tmp[i].c[0] = img->pixel[i*4];
		tmp[i].c[1] = img->pixel[i*4+1];
		tmp[i].c[2] = img->pixel[i*4+2];
		tmp[i].c[3] = img->pixel[i*4+3];
		tmp[i].index = i;
		tmp[i].index_color = -1;
	}
	qsort(tmp, sz, sizeof(struct color), comp_color);
	int n = 0;
	for (i=1;i<sz;i++) {
		if (memcmp(tmp[i].c, tmp[n].c, 4)==0) {
			tmp[i].index_color = tmp[n].index;
		} else {
			++n;
			struct color t;
			t = tmp[n];
			tmp[n] = tmp[i];
			tmp[i] = t;
		}
	}
	++n;
	return n;
}

static void
gen_index(struct color *tmp, int ncolor, int sz, struct bitmap_index *img_index) {
	int i;
	uint8_t *pixel = img_index->pixel;
	for (i=0;i<ncolor;i++) {
		pixel[tmp[i].index] = tmp[i].index_color;
	}
	for (i=ncolor;i<sz;i++) {
		int index = tmp[i].index_color;
		pixel[tmp[i].index] = pixel[index];
	}
}

static void
convert_origin(struct bitmap_rgba *img, struct bitmap_index *img_index) {
	int sz = img->w * img->h;
	int i;
	for (i=0;i<sz;i++) {
		uint8_t *c = img_index->pal[img_index->pixel[i]];
		memcpy(&img->pixel[i*4], c, 4);
	}
}

static uint8_t gamma[256];
static uint8_t gamma_inv[256];

#include <math.h>

void
bitmap_gamma_init(float v) {
	int i,j;
	for (i=0;i<256;i++) {
		double c = i / 256.0;
		c = pow(c, v);
		int g = (int)(c * 256);
		if (g == 256)
			g = 255;
		gamma[i] = g;
	}
	j = 0;
	for (i=0;i<256;i++) {
		int g; 
		for (;;) {
			g = gamma[j];
			if (g < i)
				++j;
			else {
				break;
			}
		}
		gamma_inv[i] = j;
	}
}

static void
rgb2yuv(struct color *c, int n) {
	int i;
	for (i=0;i<n;i++) {
		int r = c->c[0];
		int g = c->c[1];
		int b = c->c[2];
		int y = (int)(0.299 * r + 0.587 * g + 0.114 * b);
		int u = (int)(-0.169 * r - 0.331 * g + 0.499 * b) + 128;
		int v = (int)(0.499 * r - 0.418 *g - 0.0813 * b) + 128;
		c->c[0] = gamma[y];
		c->c[1] = u;
		c->c[2] = v;
		c->c[3] = gamma[c->c[3]];

		++c;
	}
}

static inline uint8_t
clamp(int c) {
	if (c < 0)
		return 0;
	if (c > 255)
		return 255;
	return c;
}

static void
yuv2rgb(struct bitmap_index *img_index) {
	int i;
	for (i=0;i<256;i++) {
		uint8_t *c = img_index->pal[i];
		int y = gamma_inv[c[0]];
		int u = c[1];
		int v = c[2];
		int r = (int)(y + 1.402 * (v-128));
		int g = (int)(y - 0.344 * (u - 128) - 0.714 * (v - 128));
		int b = (int)(y + 1.772 * (u - 128));
		c[0] = clamp(r);
		c[1] = clamp(g);
		c[2] = clamp(b);
		c[3] = gamma_inv[c[3]];
	}
}

void
bitmap_index(struct bitmap_rgba *img, struct bitmap_index *img_index) {
	int sz = img->w * img->h;
	struct color * tmp = malloc(sz * sizeof(struct color));
	int n = unique_color(tmp, img, sz);
//	printf("total color = %d\n", n);
	rgb2yuv(tmp, n);
	mediancut(tmp, n, img_index, 0, 256);
	img_index->pixel = malloc(sz);
	img_index->w = img->w;
	img_index->h = img->h;
	gen_index(tmp, n, sz, img_index);
	yuv2rgb(img_index);
	convert_origin(img, img_index);
	free(tmp);
}
