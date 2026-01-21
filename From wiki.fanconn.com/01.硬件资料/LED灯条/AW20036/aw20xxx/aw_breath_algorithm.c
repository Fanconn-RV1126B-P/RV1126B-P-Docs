#include "aw_breath_algorithm.h"
#include "aw_type.h"

static const AW_U8 gamma_brightness[] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 10, 12, 14, 16, 18, 20, 22,
	24, 26, 29, 32, 35, 38, 41, 44,
	47, 50, 53, 57, 61, 65, 69, 73,
	77, 81, 85, 89, 94, 99, 104, 109,
	114, 119, 124, 129, 134, 140, 146, 152,
	158, 164, 170, 176, 182, 188, 195, 202,
	209, 216, 223, 230, 237, 244, 251, 255,
};

static const AW_U8 gamma_steps = sizeof(gamma_brightness)/sizeof(AW_U8);

AW_U8 linear_brightness[256];
AW_S16 steps = 0;
const AW_U8 *p_data = AW_NULL;

/*
Perform a better visual LED breathing effect,
we recommend using a gamma corrected value
to set the LED intensity.
This results in a reduced number of steps for
the LED intensity setting, but causes the change
in intensity to appear more linear to the human eye.
*/
static AW_U8 algorithm_get_correction(ALGO_DATA_STRUCT *p_algo_data)
{
	AW_U16 start_idx = 0;
	AW_S32 end_idx = 0;

	if (0 == p_algo_data->cur_frame)
		start_idx = p_algo_data->data_start;
	else if ((p_algo_data->total_frames - 1) == p_algo_data->cur_frame)
		start_idx = p_algo_data->data_end;
	else if (p_algo_data->data_end >= p_algo_data->data_start) {
		/* get the start index in gamma array */
		while (start_idx < steps) {
			if (p_data[start_idx] >= p_algo_data->data_start)
				break;
			start_idx++;
		}

		if (start_idx >= steps)
			start_idx = steps - 1;

		/* get the end index in gamma array */
		end_idx = steps - 1;
		while (end_idx >= 0) {
			if (p_data[end_idx] <= p_algo_data->data_end) {
				break;
			}
			end_idx--;
		}

		if (end_idx < 0)
			end_idx = 0;

		/* get current index */
		start_idx +=
		  (end_idx-start_idx) * p_algo_data->cur_frame / (p_algo_data->total_frames-1);
		/* get start index */
		start_idx = p_data[start_idx];
	} else {
		/* get the start index in gamma array */
		while (start_idx < steps) {
			if (p_data[start_idx] >= p_algo_data->data_end)
				break;
			start_idx++;
		}

		if (start_idx >= steps)
			start_idx = steps - 1;

		/* get the end index in gamma array */
		end_idx = steps - 1;
		while (end_idx >= 0) {
			if (p_data[end_idx] <= p_algo_data->data_start)
				break;
			end_idx--;
		}
		if (end_idx < 0)
			end_idx = 0;
		/* get currrent index */
		end_idx -= (end_idx-start_idx)*p_algo_data->cur_frame/p_algo_data->total_frames;
		/* get start index */
		start_idx = p_data[end_idx];
	}

	return start_idx;
}

GetBrightnessFuncPtr aw_get_breath_brightness_algo_func(BREATH_ALGO_ID algo_id)
{
	GetBrightnessFuncPtr get_brightness_func_ptr = AW_NULL;

//	switch (algo_id) {
//	case BREATH_ALGO_GAMMA_CORRECTION:
//	{
//			steps = gamma_steps;
//			p_data = gamma_brightness;
//
//		//get_brightness_func_ptr = algorithm_get_correction(algo_data);
//	}
//		break;
//	default:
//		break;
//	}

	return get_brightness_func_ptr;
}


