#include "aw_lamp_interface.h"
#include "aw_type.h"

void aw_get_colorful_rgb_data(
	AW_COLOR_STRUCT *p_rgb_color,
	AW_COLORFUL_INTERFACE_STRUCT *p_colorful_interface)
{
	if (p_rgb_color != NULL
		&& p_colorful_interface != NULL
		&& p_colorful_interface->getBrightnessfunc != NULL
		&& p_colorful_interface->p_algo_data != NULL
		&& p_colorful_interface->p_color_1 != NULL
		&& p_colorful_interface->p_color_2 != NULL) {
		memset( p_rgb_color, 0, sizeof(AW_COLOR_STRUCT) );

		/* get led color of red in RGB lamp */
		if (p_colorful_interface->p_color_1->r != p_colorful_interface->p_color_2->r) {
			p_colorful_interface->p_algo_data->data_start= p_colorful_interface->p_color_1->r;
			p_colorful_interface->p_algo_data->data_end = p_colorful_interface->p_color_2->r;
			p_rgb_color->r = p_colorful_interface->getBrightnessfunc( p_colorful_interface->p_algo_data );
		} else
			p_rgb_color->r = p_colorful_interface->p_color_1->r;

		/* get led color of green in RGB lamp */
		if (p_colorful_interface->p_color_1->g
			!= p_colorful_interface->p_color_2->g) {
			p_colorful_interface->p_algo_data->data_start= p_colorful_interface->p_color_1->g;
			p_colorful_interface->p_algo_data->data_end = p_colorful_interface->p_color_2->g;
			p_rgb_color->g = p_colorful_interface->getBrightnessfunc( p_colorful_interface->p_algo_data );
		} else
			p_rgb_color->g = p_colorful_interface->p_color_1->g;

		/* get led color of blue in RGB lamp */
		if (p_colorful_interface->p_color_1->b
			!= p_colorful_interface->p_color_2->b) {
			p_colorful_interface->p_algo_data->data_start= p_colorful_interface->p_color_1->b;
			p_colorful_interface->p_algo_data->data_end = p_colorful_interface->p_color_2->b;
			p_rgb_color->b = p_colorful_interface->getBrightnessfunc( p_colorful_interface->p_algo_data );
		} else {
			p_rgb_color->b = p_colorful_interface->p_color_1->b;
		}
	}
}
void aw_set_rgb_color(uint8_t rgb_idx, uint8_t* dim_reg, AW_COLOR_STRUCT *p_rgb_dim)
{
	if (p_rgb_dim != NULL) {
		dim_reg[rgb_idx*3 + 0] = p_rgb_dim->r;
		dim_reg[rgb_idx*3 + 1] = p_rgb_dim->g;
		dim_reg[rgb_idx*3 + 2] = p_rgb_dim->b;
	}
}
void aw_set_colorful_rgb_data(uint8_t rgb_idx,uint8_t* dim_reg,
	AW_COLORFUL_INTERFACE_STRUCT * p_colorful_interface)
{
	AW_COLOR_STRUCT rgb_color ;

	if (p_colorful_interface != NULL) {
		aw_get_colorful_rgb_data(&rgb_color, p_colorful_interface );
		aw_set_rgb_color(rgb_idx, dim_reg, &rgb_color );
	}
}
AW_U8 aw_get_real_dim(AW_U8 led_dim)
{
	AW_U8 real_dim = 0;

	/* dim[7:6] register is used for pattern, dim[5:0] is used for dim */
	real_dim = led_dim>>2;
	if (real_dim == 0 && led_dim > 0) {
		real_dim = 1;
	}

	return real_dim;
}
void aw_set_rgb_brightness(unsigned char rgb_idx,
					  unsigned char *fade_reg,
					  unsigned char brightness)
{
	if (AW_NULL != fade_reg) {
		fade_reg[rgb_idx*3 + 0] = brightness;
		fade_reg[rgb_idx*3 + 1] = brightness;
		fade_reg[rgb_idx*3 + 2] = brightness;
	}
}

