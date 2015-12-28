/*
 * bitmap.c
 *
 *  Created on: 29.11.2015
 *      Author: Admin
 */

#include "image.h"
#include "ili9340.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * loads an image from bitmap.
 */
int imageLoad(IMAGE *image, const char* file)
{
	int x,y;
	FILE* fd;
	BMP_HEADER header;

	fd = fopen(file, "r");
	if (!fd) {
		return -1;
	}
	if (fread(&header, 1, sizeof(BMP_HEADER), fd) != sizeof(BMP_HEADER)) {
		return -2;
	}
	if (header.magic != 19778) {
		return -3;
	}
	image->data = malloc(header.width * header.height * 2);
	image->height = header.height;
	image->width = header.width;
	fseek(fd, header.startaddr, SEEK_SET);

	for (y = header.height - 1; y >= 0; y--) {
		for (x = 0; x < header.width; x++) {
			uint8_t bgr[3];
			if (fread(bgr, 3, 1, fd) != 1) {
				return -4;
			}
			((uint16_t*)(image->data))[header.width * y + x] = ili9340_color(bgr[2], bgr[1], bgr[0]);
		}
		fseek(fd, header.width & 3, SEEK_CUR);
	}
	ili9340_WordToDword((uint16_t*)image->data, header.width * header.height * 2);

	fclose(fd);
	return 0;
}

void imageFree(IMAGE *image)
{
	if (image) {
		free(image->data);
	}
}
