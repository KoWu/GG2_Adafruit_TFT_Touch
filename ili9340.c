/*
 * ili9340.c
 *
 *  Created on: 28.11.2015
 *      Author: Admin
 */

#include "mraa.h"
#include "ili9340.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>


static mraa_gpio_context dc, cs;
static int fd, speed;

void ili9340_init(int spifd)
{
	dc = mraa_gpio_init(9);
	cs = mraa_gpio_init(10);
	fd = spifd;

	mraa_gpio_dir(dc, MRAA_GPIO_OUT);
	mraa_gpio_dir(cs, MRAA_GPIO_OUT);
	mraa_gpio_write(dc, 1);
	mraa_gpio_write(cs, 1);

	ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
}

void ili9340_write(uint8_t d)
{
    struct spi_ioc_transfer msg;
    memset(&msg, 0, sizeof(msg));

    msg.tx_buf = (unsigned long) &d;
    msg.speed_hz = speed;
    msg.bits_per_word = 8;
    msg.len = 1;
    ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
}

void ili9340_writeBlock(uint32_t* d, uint32_t size)
{
	struct spi_ioc_transfer msg;
    memset(&msg, 0, sizeof(msg));
	msg.speed_hz = speed;
	msg.bits_per_word = 32;
	msg.len = size;
	msg.tx_buf = (unsigned long) d;
	ioctl(fd, SPI_IOC_MESSAGE(1), &msg);
}

void ili9340_cmd(uint8_t c)
{
	mraa_gpio_write(dc, 0);
	mraa_gpio_write(cs, 0);
	ili9340_write(c);
	mraa_gpio_write(cs, 1);
}

void ili9340_data(uint8_t d)
{
	mraa_gpio_write(dc, 1);
	mraa_gpio_write(cs, 0);
	ili9340_write(d);
	mraa_gpio_write(cs, 1);
}

void ili9340_begin()
{
	ili9340_cmd(0xEF);
	ili9340_data(0x03);
	ili9340_data(0x80);
	ili9340_data(0x02);

	ili9340_cmd(0xCF);
	ili9340_data(0x00);
	ili9340_data(0XC1);
	ili9340_data(0X30);

	ili9340_cmd(0xED);
	ili9340_data(0x64);
	ili9340_data(0x03);
	ili9340_data(0X12);
	ili9340_data(0X81);

	ili9340_cmd(0xE8);
	ili9340_data(0x85);
	ili9340_data(0x00);
	ili9340_data(0x78);

	ili9340_cmd(0xCB);
	ili9340_data(0x39);
	ili9340_data(0x2C);
	ili9340_data(0x00);
	ili9340_data(0x34);
	ili9340_data(0x02);

	ili9340_cmd(0xF7);
	ili9340_data(0x20);

	ili9340_cmd(0xEA);
	ili9340_data(0x00);
	ili9340_data(0x00);

	ili9340_cmd(ILI9341_PWCTR1);    //Power control
	ili9340_data(0x23);   //VRH[5:0]

	ili9340_cmd(ILI9341_PWCTR2);    //Power control
	ili9340_data(0x10);   //SAP[2:0];BT[3:0]

	ili9340_cmd(ILI9341_VMCTR1);    //VCM control
	ili9340_data(0x3e);
	ili9340_data(0x28);

	ili9340_cmd(ILI9341_VMCTR2);    //VCM control2
	ili9340_data(0x86);  //--

	ili9340_cmd(ILI9341_MADCTL);    // Memory Access Control
	ili9340_data(0x48);

	ili9340_cmd(ILI9341_PIXFMT);
	ili9340_data(0x55);

	ili9340_cmd(ILI9341_FRMCTR1);
	ili9340_data(0x00);
	ili9340_data(0x18);

	ili9340_cmd(ILI9341_DFUNCTR);    // Display Function Control
	ili9340_data(0x08);
	ili9340_data(0x82);
	ili9340_data(0x27);

	ili9340_cmd(0xF2);    // 3Gamma Function Disable
	ili9340_data(0x00);

	ili9340_cmd(ILI9341_GAMMASET);    //Gamma curve selected
	ili9340_data(0x01);

	ili9340_cmd(ILI9341_GMCTRP1);    //Set Gamma
	ili9340_data(0x0F);
	ili9340_data(0x31);
	ili9340_data(0x2B);
	ili9340_data(0x0C);
	ili9340_data(0x0E);
	ili9340_data(0x08);
	ili9340_data(0x4E);
	ili9340_data(0xF1);
	ili9340_data(0x37);
	ili9340_data(0x07);
	ili9340_data(0x10);
	ili9340_data(0x03);
	ili9340_data(0x0E);
	ili9340_data(0x09);
	ili9340_data(0x00);

	ili9340_cmd(ILI9341_GMCTRN1);    //Set Gamma
	ili9340_data(0x00);
	ili9340_data(0x0E);
	ili9340_data(0x14);
	ili9340_data(0x03);
	ili9340_data(0x11);
	ili9340_data(0x07);
	ili9340_data(0x31);
	ili9340_data(0xC1);
	ili9340_data(0x48);
	ili9340_data(0x08);
	ili9340_data(0x0F);
	ili9340_data(0x0C);
	ili9340_data(0x31);
	ili9340_data(0x36);
	ili9340_data(0x0F);

	ili9340_cmd(ILI9341_MADCTL);
	ili9340_data(0x20);

	ili9340_cmd(ILI9341_SLPOUT);    //Exit Sleep

	usleep(120000);

	ili9340_cmd(ILI9341_DISPON);    //Display on
}

void ili9340_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

	ili9340_cmd(ILI9341_CASET); // Column addr set
	ili9340_data(x0 >> 8);
	ili9340_data(x0 & 0xFF);     // XSTART
	ili9340_data(x1 >> 8);
	ili9340_data(x1 & 0xFF);     // XEND

	ili9340_cmd(ILI9341_PASET); // Row addr set
	ili9340_data(y0>>8);
	ili9340_data(y0);     // YSTART
	ili9340_data(y1>>8);
	ili9340_data(y1);     // YEND

	ili9340_cmd(ILI9341_RAMWR); // write to RAM
}

void ili9340_drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if((x < 0) ||(x >= ILI9341_TFTWIDTH) || (y < 0) || (y >= ILI9341_TFTHEIGHT))
	  return;

	ili9340_setAddrWindow(x, y, x+1, y+1);

	mraa_gpio_write(dc, 1);
	mraa_gpio_write(cs, 0);

	ili9340_write(color >> 8);
	ili9340_write(color);

	mraa_gpio_write(cs, 1);
}

void ili9340_fillRectData(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t* data)
{
	uint16_t n;
	ili9340_setAddrWindow(x, y, x+w-1, y+h-1);

	mraa_gpio_write(dc, 1);
	mraa_gpio_write(cs, 0);
	for (n = h; n; n--) {
		ili9340_writeBlock(data, w*2);
		data += w/2;
	}
	mraa_gpio_write(cs, 1);
}

void ili9340_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	int n;
	uint16_t* data;

	if((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT))
		return;
	if((x + w - 1) >= ILI9341_TFTWIDTH)
		w = ILI9341_TFTWIDTH  - x;
	if((y + h - 1) >= ILI9341_TFTHEIGHT)
		h = ILI9341_TFTHEIGHT - y;

	data = malloc(w*h*2);
	for (n = 0; n < w*h; n++)
		data[n] = color;
	ili9340_fillRectData(x, y, w, h, (uint32_t*)data);
	free(data);
}

void ili9340_fillScreen(uint16_t color) {
	ili9340_fillRect(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, color);
}

void ili9340_WordToDword(uint16_t* buf, uint32_t len)
{
	int n;
	uint32_t dword, *dwordptr;
	dwordptr = (uint32_t*)buf;
	for (n = len/4; n; n--) {
		dword = *dwordptr;
		*dwordptr = (dword << 16) | (dword >> 16);
		dwordptr++;
	}
}

//0xf800 == blau 1111100000000000
//0x03e0 == grün 0000011111100000
//0x001f == rot  0000000000011111
uint16_t ili9340_color(uint8_t r, uint8_t g, uint8_t b)
{
	return ((((uint16_t)(b >> 3)) << 11) | (((uint16_t)(g >> 2)) << 5) | ((uint16_t)(r >> 3)));
}
