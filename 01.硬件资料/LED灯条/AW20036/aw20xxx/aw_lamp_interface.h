#ifndef _AW_LAMP_INTERFACE_H_
#define _AW_LAMP_INTERFACE_H_

//#include "stm32h7xx.h"
#include "aw_breath_algorithm.h"

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} AW_COLOR_STRUCT;

typedef struct {
	unsigned int time[5];		/* Tdelay,Trise,Ton,Tfall,Toff; */
	unsigned int repeat_nums;	/* breath times */
	unsigned char  fadeh;		/* breath max brightness */
	unsigned char  fadel;		/* breath min brightness */
	unsigned char color_nums;	/* The number of colors per lamp */
	const AW_COLOR_STRUCT *rgb_color_list; /* The address of the color list */

} AW_MULTI_BREATH_DATA_STRUCT;


typedef struct {
	GetBrightnessFuncPtr getBrightnessfunc;
	ALGO_DATA_STRUCT *p_algo_data;
	AW_COLOR_STRUCT *p_color_1;
	AW_COLOR_STRUCT *p_color_2;
} AW_COLORFUL_INTERFACE_STRUCT;

void aw_set_colorful_rgb_data(uint8_t rgb_idx, uint8_t *dim_reg,AW_COLORFUL_INTERFACE_STRUCT *p_colorful_interface);
void aw_set_rgb_brightness(unsigned char rgb_idx,unsigned char *fade_reg,unsigned char brightness);

unsigned char aw_get_real_dim(unsigned char led_dim);
#endif
