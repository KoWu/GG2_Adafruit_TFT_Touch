/*
 * stmpe610.c
 *
 *  Created on: 05.12.2015
 *      Author: Admin
 */
#include "mraa.h"
#include "stmpe610.h"
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


static mraa_gpio_context cs;
static int fd;

void stmpe610_init(int spifd)
{
	cs = mraa_gpio_init(8);
	fd = spifd;

	mraa_gpio_dir(cs, MRAA_GPIO_OUT);
	mraa_gpio_mode(cs, MRAA_GPIO_STRONG);
	mraa_gpio_write(cs, 1);
}

void writeRegister8(uint8_t reg, uint8_t val) {
    struct spi_ioc_transfer msg[2];
    memset(&msg, 0, sizeof(msg));
    msg[0].speed_hz = 1000000;
    msg[0].bits_per_word = 8;
    msg[0].len = 1;
    memcpy(&msg[1], &msg, sizeof(msg[0]));

    msg[0].tx_buf = (unsigned long) &reg;
    msg[1].tx_buf = (unsigned long) &val;
    mraa_gpio_write(cs, 0);
    ioctl(fd, SPI_IOC_MESSAGE(2), &msg);
    mraa_gpio_write(cs, 1);
}

uint8_t readRegister8(uint8_t reg) {
	struct spi_ioc_transfer msg[2];
	uint8_t x,z;
	memset(&msg, 0, sizeof(msg));
	msg[0].speed_hz = 1000000;
	msg[0].bits_per_word = 8;
	msg[0].len = 1;
	memcpy(&msg[1], &msg, sizeof(msg[0]));

	z = 0;
	reg |= 0x80;
	msg[0].tx_buf = (unsigned long) &reg;
	msg[1].tx_buf = (unsigned long) &z;
	msg[1].rx_buf = (unsigned long) &x;
	mraa_gpio_write(cs, 0);
	ioctl(fd, SPI_IOC_MESSAGE(2), &msg);
	mraa_gpio_write(cs, 1);
	return x;
}

uint8_t stmpe610_touched(void) {
  return (readRegister8(STMPE_TSC_CTRL) & 0x80);
}

uint8_t stmpe610_bufferEmpty(void) {
  return (readRegister8(STMPE_FIFO_STA) & STMPE_FIFO_STA_EMPTY);
}

uint8_t stmpe610_bufferSize(void) {
  return readRegister8(STMPE_FIFO_SIZE);
}

void stmpe610_begin()
{
	uint8_t i;
	mraa_gpio_write(cs, 1);

	writeRegister8(STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
	usleep(10000);

	for (i=0; i<65; i++) {
		readRegister8(i);
	}

	writeRegister8(STMPE_SYS_CTRL2, 0x0); // turn on clocks!
	writeRegister8(STMPE_TSC_CTRL, STMPE_TSC_CTRL_XYZ | STMPE_TSC_CTRL_EN); // XYZ and enable!
	writeRegister8(STMPE_INT_EN, STMPE_INT_EN_TOUCHDET);
	writeRegister8(STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_10BIT | (0x6 << 4)); // 96 clocks per conversion
	writeRegister8(STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_6_5MHZ);
	writeRegister8(STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE | STMPE_TSC_CFG_DELAY_1MS | STMPE_TSC_CFG_SETTLE_5MS);
	writeRegister8(STMPE_TSC_FRACTION_Z, 0x6);
	writeRegister8(STMPE_FIFO_TH, 1);
	writeRegister8(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
	writeRegister8(STMPE_FIFO_STA, 0);    // unreset
	writeRegister8(STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
	writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
	writeRegister8(STMPE_INT_CTRL, STMPE_INT_CTRL_POL_HIGH | STMPE_INT_CTRL_ENABLE);
}

void stmpe610_readData(uint16_t *x, uint16_t *y, uint8_t *z) {
	uint8_t data[4], i;

	for (i=0; i<4; i++) {
		data[i] = readRegister8(0xD7);
	}

	*y = data[0];
	*y <<= 4;
	*y |= (data[1] >> 4);
	*x = data[1] & 0x0F;
	*x <<= 8;
	*x |= data[2];
	*z = data[3];

	if (stmpe610_bufferEmpty())
		writeRegister8(STMPE_INT_STA, 0xFF); // reset all ints
}
