#ifndef __LEDS_AW20XXX_H__
#define __LEDS_AW20XXX_H__

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/workqueue.h>


//#include "gpio.h"
//#include "i2c.h"
#include "aw20xxx_reg.h"
#include "aw_lamp_interface.h"
#include "aw_breath_algorithm.h"


#define AWINIC_UART_DEBUG
#ifdef AWINIC_UART_DEBUG
#define AWLOGD(format, arg...)  printf(format, ##arg)
#else
#define AWLOGD(format, arg...)  do {} while (0)
#endif

/*********************************************************
 * chip select
 ********************************************************/
#define AW20036 1
#define AW20054 0
#define AW20072 0
#define AW20108 0

/*********************************************************
 *
 * i2c addr
 *
 ********************************************************/
#define AW20036_DEVICE_ADDR		0x3a
#define AW20054_DEVICE_ADDR		0x3a
#define AW20072_DEVICE_ADDR		0x3a
#define AW20108_DEVICE_ADDR		0x3a

#if AW20036
#define AW20XXX_DEVICE_ADDR		AW20036_DEVICE_ADDR
#elif AW20054
#define AW20XXX_DEVICE_ADDR		AW20054_DEVICE_ADDR
#elif AW20072
#define AW20XXX_DEVICE_ADDR		AW20072_DEVICE_ADDR
#elif AW20108
#define AW20XXX_DEVICE_ADDR		AW20108_DEVICE_ADDR
#endif

/*********************************************************
 *
 * chip info
 *
 ********************************************************/
#define AW20XXX_RSTR			0x01
#define AW20XXX_CHIPID			0x18

enum aw20xxx_reg_page {
	AW20XXX_REG_PAGE0 = 0xC0,
	AW20XXX_REG_PAGE1 = 0xC1,
	AW20XXX_REG_PAGE2 = 0xC2,
	AW20XXX_REG_PAGE3 = 0xC3,
	AW20XXX_REG_PAGE4 = 0xC4,
	AW20XXX_REG_PAGE5 = 0xC5,
};

enum aw20xxx_imax {
	AW20XXX_IMAX_10mA = 0x00,
	AW20XXX_IMAX_20mA = 0X01,
	AW20XXX_IMAX_30mA = 0x02,
	AW20XXX_IMAX_40mA = 0x03,
	AW20XXX_IMAX_60mA = 0x04,
	AW20XXX_IMAX_80mA = 0x05,
	AW20XXX_IMAX_120mA = 0x06,
	AW20XXX_IMAX_160mA = 0x07,
	AW20XXX_IMAX_3P3mA = 0x08,
	AW20XXX_IMAX_6P7mA = 0x09,
	AW20XXX_IMAX_10P0mA = 0x0A,
	AW20XXX_IMAX_13P3mA = 0x0B,
	AW20XXX_IMAX_20P0mA = 0X0C,
	AW20XXX_IMAX_26P7mA = 0x0D,
	AW20XXX_IMAX_40P0mA = 0x0E,
	AW20XXX_IMAX_53P6mA = 0x0F,
};

enum aw20xxx_display_size{
	AW20XXX_CURRENT_SWITCH_1 = 0x00,
	AW20XXX_CURRENT_SWITCH_2 = 0X01,
	AW20XXX_CURRENT_SWITCH_3 = 0X02,
	AW20XXX_CURRENT_SWITCH_4 = 0x03,
	AW20XXX_CURRENT_SWITCH_5 = 0X04,
	AW20XXX_CURRENT_SWITCH_6 = 0X05,
	AW20XXX_CURRENT_SWITCH_7 = 0x06,
	AW20XXX_CURRENT_SWITCH_8 = 0X07,
	AW20XXX_CURRENT_SWITCH_9 = 0X08,
	AW20XXX_CURRENT_SWITCH_MAX,
};
#if AW20036
#define AW20XXX_CURRENT_SWITCH AW20XXX_CURRENT_SWITCH_3
#elif AW20054
#define AW20XXX_CURRENT_SWITCH AW20XXX_CURRENT_SWITCH_6
#elif AW20072
#define AW20XXX_CURRENT_SWITCH AW20XXX_CURRENT_SWITCH_6
#elif AW20108
#define AW20XXX_CURRENT_SWITCH AW20XXX_CURRENT_SWITCH_9
#endif

/*********************************************************
 *
 * pag num
 *
 ********************************************************/
#define AW20036_REG_NUM_PAG1		36
#define AW20036_REG_NUM_PAG2		36
#define AW20036_REG_NUM_PAG3		36
#define AW20036_REG_NUM_PAG4		72
#define AW20036_REG_NUM_PAG5		72

#define AW20054_REG_NUM_PAG1		69
#define AW20054_REG_NUM_PAG2		69
#define AW20054_REG_NUM_PAG3		69
#define AW20054_REG_NUM_PAG4		138
#define AW20054_REG_NUM_PAG5		138

#define AW20072_REG_NUM_PAG1		72
#define AW20072_REG_NUM_PAG2		72
#define AW20072_REG_NUM_PAG3		72
#define AW20072_REG_NUM_PAG4		144
#define AW20072_REG_NUM_PAG5		144

#define AW20108_REG_NUM_PAG1		108
#define AW20108_REG_NUM_PAG2		108
#define AW20108_REG_NUM_PAG3		108
#define AW20108_REG_NUM_PAG4		216
#define AW20108_REG_NUM_PAG5		216


#if AW20036
#define AW20XXX_REG_NUM_PAG1		AW20036_REG_NUM_PAG1
#define AW20XXX_REG_NUM_PAG2		AW20036_REG_NUM_PAG2
#define AW20XXX_REG_NUM_PAG3		AW20036_REG_NUM_PAG3
#define AW20XXX_REG_NUM_PAG4		AW20036_REG_NUM_PAG4
#define AW20XXX_REG_NUM_PAG5		AW20036_REG_NUM_PAG5
#elif AW20054
#define AW20XXX_REG_NUM_PAG1		AW20054_REG_NUM_PAG1
#define AW20XXX_REG_NUM_PAG2		AW20054_REG_NUM_PAG2
#define AW20XXX_REG_NUM_PAG3		AW20054_REG_NUM_PAG3
#define AW20XXX_REG_NUM_PAG4		AW20054_REG_NUM_PAG4
#define AW20XXX_REG_NUM_PAG5		AW20054_REG_NUM_PAG5
#elif AW20072
#define AW20XXX_REG_NUM_PAG1		AW20072_REG_NUM_PAG1
#define AW20XXX_REG_NUM_PAG2		AW20072_REG_NUM_PAG2
#define AW20XXX_REG_NUM_PAG3		AW20072_REG_NUM_PAG3
#define AW20XXX_REG_NUM_PAG4		AW20072_REG_NUM_PAG4
#define AW20XXX_REG_NUM_PAG5		AW20072_REG_NUM_PAG5
#elif AW20108
#define AW20XXX_REG_NUM_PAG1		AW20108_REG_NUM_PAG1
#define AW20XXX_REG_NUM_PAG2		AW20108_REG_NUM_PAG2
#define AW20XXX_REG_NUM_PAG3		AW20108_REG_NUM_PAG3
#define AW20XXX_REG_NUM_PAG4		AW20108_REG_NUM_PAG4
#define AW20XXX_REG_NUM_PAG5		AW20108_REG_NUM_PAG5
#endif

#if AW20036
#define LED_NUM		36
#elif AW20054
#define LED_NUM		54
#elif AW20072
#define LED_NUM		72
#elif AW20108
#define LED_NUM		108
#endif

#define AW_I2C_RETRIES				2
#define AW_I2C_RETRY_DELAY			1
#define AW_READ_CHIPID_RETRIES			2
#define AW_READ_CHIPID_RETRY_DELAY		1

#define AW20XXX_I2C_ADDR			(0x3a)
#define AW20XXX_I2C_RW_RETRIES			(3)
#define AW20XXX_READ_CHIPID_RETRIES		(3)
#define AW20XXX_REGADD_SIZE_8BIT		(1)
#define AW20XXX_REGDATA_SIZE_8BIT		(1)
#define AW20XXX_REGDATA_TIMEOUT			(100)

#define AW20XXX_READ_CHIPID_ERROR		-1
#define AW20XXX_PULL_EN_ERROR			-2

struct aw20xxx {
	//I2C_HandleTypeDef *hi2cx;
	//GPIO_TypeDef *gpio_port;
	uint8_t chipid;
	uint8_t en_flag;

	//uint16_t irq_gpio_pin;
	//uint16_t en_gpio_pin;

	uint32_t imax;
	uint8_t effect;
	uint8_t cfg;
	uint32_t rgbcolor;
	uint32_t rgbbrightness;

    struct device *     dev;
    struct i2c_client * i2c_client;
    u8          i2c_addr;

    u32         irq_gpio_pin;
    u32         en_gpio_pin;
};

/*********************************************************
 *
 * algorithm variable
 *
 ********************************************************/

static unsigned char aw20xxx_page4_data[AW20XXX_REG_NUM_PAG4] = {0};
//static unsigned char dim_data[LED_NUM];
static unsigned char fade_data[LED_NUM];
static AW_COLORFUL_INTERFACE_STRUCT aw20xxx_interface;
static ALGO_DATA_STRUCT algo_data;

#define RGB_NUM (LED_NUM/3)

/* static unsigned char brightness_breath[AW20XXX_REG_NUM_PAG4] = {0}; */
static unsigned char loop_end[RGB_NUM] = {0};
static AW_COLOR_STRUCT source_color[RGB_NUM];
static AW_COLOR_STRUCT destination_color[RGB_NUM];
static ALGO_DATA_STRUCT aw20xxx_algo_data[RGB_NUM];
static unsigned char breath_phase_nums[RGB_NUM] = {0};
static unsigned char breath_cur_phase[RGB_NUM] = {0};
static unsigned char breath_cur_loop[RGB_NUM] = {0};
static unsigned char colorful_cur_frame[RGB_NUM] = {0};
static unsigned char colorful_total_frames[RGB_NUM] = {0};
static unsigned char colorful_phase_nums[RGB_NUM] = {0};
static unsigned char colorful_cur_phase[RGB_NUM] = {0};
static unsigned char colorful_cur_color_index[RGB_NUM] = {0};

/****************** led effect ctrl *********************/
void aw20xxx_all_on(void);
void aw20xxx_led_Ribbon(int times);
void aw20xxx_all_breath_forever(void);
void aw20xxx_rgb_multi_breath(const AW_MULTI_BREATH_DATA_STRUCT *data);

int aw20xxx_play(void);
//extern GetBrightnessFuncPtr aw_get_breath_brightness_algo_func(BREATH_ALGO_ID algo_id);


#endif
