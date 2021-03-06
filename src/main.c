/*-
 * Copyright (c) 2020 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/console.h>
#include <sys/callout.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/thread.h>
#include <sys/mbuf.h>

#include <dev/i2c/i2c.h>
#include <dev/ccs811/ccs811.h>

#include "lcd.h"
#include "ccs811.h"
#include "mh_z19b.h"

extern struct mdx_device i2c0;

/* Override cpu_idle() from machdep.c */
void
cpu_idle(void)
{

	/*
	 * gd32v DMA does not work properly in WFI state,
	 * so do nothing here.
	 */
}

static int __unused
ccs811_main(void)
{
	uint16_t eco2;
	uint16_t tvoc;
	char text[16];
	int error;

	lcd_init();

	error = ccs811_init();
	if (error) {
		lcd_update(0, "ccs811");
		lcd_update(1, "error");
		panic("could not initialize CCS811\n");
	}

	sprintf(text, "Co2(ppm)");
	lcd_update(1, text);

	while (1) {
		error = ccs811_read_data(&eco2, &tvoc);
		if (error)
			panic("%s: error %d\n", __func__, error);

		printf("eCo2 %d tvoc %d\n", eco2, tvoc);

		sprintf(text, "%d", eco2);
		lcd_update(0, text);

		mdx_usleep(1000000);
	}

	return (0);
}

int
main(void)
{
	char text[16];
	uint32_t co2;
	int iteration;
	int error;
	int i;

	lcd_init();
	mh_z19b_init();

	sprintf(text, "wait");
	lcd_update(0, text);
	sprintf(text, "sensor");
	lcd_update(1, text);

	/* Measurement range changed, wait a bit. */
	mdx_usleep(5000000);

	sprintf(text, "Co2(ppm)");
	lcd_update(1, text);

	iteration = 0;

	while (1) {

		i = iteration % 5;

		if (i == 0) {
			error = mh_z19b_read_data(&co2);
			if (error) {
				lcd_update(0, "panic");
				lcd_update(1, "err 1");
				panic("%s: error %d\n", __func__, error);
			}
			printf("Co2 %d\n", co2);
			sprintf(text, "%d", co2);
			lcd_update(0, text);

			sprintf(text, "Co2(ppm)");
		} else
			snprintf(text, i+1, "tick");
		lcd_update(1, text);

		iteration++;
		mdx_usleep(1000000);
	}

	return (0);
}
