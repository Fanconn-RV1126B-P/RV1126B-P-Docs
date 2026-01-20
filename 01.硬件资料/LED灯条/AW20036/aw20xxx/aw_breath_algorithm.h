#ifndef __AW_BREATH_ALGORITHM_H__
#define __AW_BREATH_ALGORITHM_H__

//#include "stdint.h"
//#include "string.h"
//#include <linux/stdint.h>
#include <linux/string.h>

//#ifdef __cplusplus
//extern "C" {
//#endif /* __cplusplus */
typedef enum {
	BREATH_ALGO_NONE,
	BREATH_ALGO_GAMMA_CORRECTION,
	BREATH_ALGO_LINEAR_CORRECTION,
	BREATH_ALGO_MAX,
} BREATH_ALGO_ID;

typedef struct {
	uint16_t total_frames;
	uint16_t cur_frame;
	uint16_t data_start;
	uint16_t data_end;
} ALGO_DATA_STRUCT;

typedef uint8_t (*GetBrightnessFuncPtr)(ALGO_DATA_STRUCT *);
GetBrightnessFuncPtr aw_get_breath_brightness_algo_func(BREATH_ALGO_ID algo_id);

//#ifdef __cplusplus
//}
//#endif /* __cplusplus */

#endif  /* __AW_BREATH_ALGORITHM_H__ */

