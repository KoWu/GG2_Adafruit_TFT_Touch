/*
 * bitmap.h
 *
 *  Created on: 29.11.2015
 *      Author: Admin
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <inttypes.h>

typedef struct __attribute__((packed)) BMP_HEADER_s {
	uint16_t magic;
	uint32_t size;
	char unused[4];
	uint32_t startaddr;
	uint32_t headersize;
	int32_t width;
	int32_t height;
	char colorplanes[2];
	uint16_t bit;
	uint32_t compression;
	uint32_t imagesize;
	char unused2[16];
} BMP_HEADER;

typedef struct IMAGE_s {
	int width;
	int height;
	uint32_t* data;
} IMAGE;


int imageLoad(IMAGE *image, const char* file);
void imageFree(IMAGE* image);

#endif /* IMAGE_H_ */
