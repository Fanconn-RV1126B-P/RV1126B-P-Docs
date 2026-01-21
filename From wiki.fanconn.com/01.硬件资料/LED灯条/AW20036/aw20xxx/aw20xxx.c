/*
 * aw20xxx.c   aw20036, aw20054, aw20072, aw20108 led module
 *
 * Copyright (c) 2021 AWINIC Technology CO., LTD
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */



//#include <stdio.h>
//#include <stdlib.h>
#include "aw20xxx.h"
#include "data.c"

#define AW20XXX_DRIVER_VERSION		"V1.1.0"

static struct aw20xxx *g_aw20xxx;
static struct i2c_client *g_aw20xxx_client;


/*********************************************************
 *
 * I2C interface
 *
 ********************************************************/
//void soft_delay(int ms)
//{
//	HAL_Delay(ms);
//}

static int aw20xxx_write(u8 reg, u8 val)
{
   int rc = 0;
    struct i2c_client *client = g_aw20xxx_client;
    struct i2c_msg msg;
    u8 buf[2] = { reg, val };

    if (!client) {
        pr_info("%s: Invalid params\n", __func__);
        return -EINVAL;
    }

    msg.addr = client->addr;
    msg.flags = 0;
    msg.len = 2;
    msg.buf = buf;

    if (i2c_transfer(client->adapter, &msg, 1) < 1) {
        pr_info("%s: i2c write failed\n", __func__);
        rc = -EIO;
    }

    return rc;
}

static int aw20xxx_write_array(u8 reg,u8 *val, int size)
{
	int i, ret = 0;

	i = 0;
	while (i < size) {
		ret = aw20xxx_write(reg, val[i]);
		if (ret) {
			pr_err("%s failed !\n", __func__);
			break;
		}
		i++;
	}

	return ret;
}


static int aw20xxx_read(u8 reg)
{
    int rc = 0;
    struct i2c_client *client = g_aw20xxx_client;
    struct i2c_msg msg[2];
    u8 buf[2] = {reg};
    if (!client) {
        printk("%s: Invalid params\n", __func__);
        return -EINVAL;
    }

    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &buf[0];

    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = 1;
    msg[1].buf = &buf[1];

    if (i2c_transfer(client->adapter, msg, 2) != 2) {
        rc = -EIO;
    }

    if(rc == 0)
         return buf[1];
    else
         return rc;
}

//static HAL_StatusTypeDef aw20xxx_i2c_read(struct aw20xxx *aw20xxx,
//					  uint8_t reg_addr,
//					  uint8_t *reg_data)
//{
//	uint8_t cnt = 0;
//	uint8_t slave_data[1] = { 0 };
//	HAL_StatusTypeDef status;
//
//	while (cnt < AW20XXX_I2C_RW_RETRIES) {
//		status = HAL_I2C_Mem_Read(aw20xxx->hi2cx,
//					  AW20XXX_DEVICE_ADDR << 1,
//					  reg_addr,
//					  AW20XXX_REGADD_SIZE_8BIT,
//					  slave_data,
//					  AW20XXX_REGDATA_SIZE_8BIT,
//					  AW20XXX_REGDATA_TIMEOUT);
//		if (status != HAL_OK)
//			cnt++;
//		else {
//			*reg_data = slave_data[0];
//			break;
//		}
//	}
//
//	if (status != HAL_OK)
//		AWLOGD("%s: fail! status is %d\n", __func__, status);
//
//	return status;
//}
//
//static HAL_StatusTypeDef aw20xxx_i2c_write(struct aw20xxx *aw20xxx,
//					   uint8_t reg_addr,
//					   uint8_t reg_data)
//{
//	uint8_t cnt = 0;
//	uint8_t slave_data[1] = { 0 };
//	HAL_StatusTypeDef status;
//
//	slave_data[0] = reg_data;
//
//	while (cnt < AW20XXX_I2C_RW_RETRIES) {
//		status = HAL_I2C_Mem_Write(aw20xxx->hi2cx,
//					   AW20XXX_DEVICE_ADDR << 1,
//					   reg_addr,
//					   AW20XXX_REGADD_SIZE_8BIT,
//					   slave_data,
//					   AW20XXX_REGDATA_SIZE_8BIT,
//					   AW20XXX_REGDATA_TIMEOUT);
//		if (status != HAL_OK)
//			cnt++;
//		else
//			break;
//	}
//
//	if (status != HAL_OK)
//		AWLOGD("%s: fail! status is %d\n", __func__, status);
//
//	return status;
//}                       
//
//static HAL_StatusTypeDef aw20xxx_i2c_writes(struct aw20xxx *aw20xxx,
//							uint8_t reg_addr,
//							uint8_t *reg_data,
//							unsigned nums)
//{
//	HAL_StatusTypeDef status;
//	unsigned char cnt = 0;
//
//	while (cnt < AW_I2C_RETRIES) {
//
//	status = HAL_I2C_Mem_Write(aw20xxx->hi2cx,
//					  AW20XXX_DEVICE_ADDR<<1,
//					  reg_addr,
//					  AW20XXX_REGADD_SIZE_8BIT,
//					  reg_data,
//					  nums,
//					  100);
//
//	if (status != HAL_OK)
//		AWLOGD("%s: fail! status is %d\n", __func__, status);
//		cnt++;
//	}
//	return status;
//}

#if 0
static void aw20xxx_i2c_write_bits(struct aw20xxx *aw20xxx,
				   uint8_t reg_addr,
				   uint32_t reg_mask,
				   uint8_t reg_data)
{
	uint8_t val;

	aw20xxx_i2c_read(aw20xxx, reg_addr, &val);
	val &= reg_mask;
	val |= (reg_data & (~reg_mask));
	aw20xxx_i2c_write(aw20xxx, reg_addr, val);
}
#endif

static void aw20xxx_set_page(enum aw20xxx_reg_page page_id)
{
	//aw20xxx_i2c_write(g_aw20xxx, REG_PAGE, (unsigned char)page_id);
    aw20xxx_write(REG_PAGE, (unsigned char)page_id);
};

static void aw20xxx_set_dim_by_idx(int idx, unsigned char dim_val)
{
	dim_val &= 0x3F;
	//aw20xxx_i2c_write(g_aw20xxx, idx, dim_val);
    aw20xxx_write(idx, dim_val);
}

static void aw20xxx_set_fade_by_idx(int idx, unsigned char fade_val)
{
//	aw20xxx_i2c_write(g_aw20xxx, idx, fade_val);
    aw20xxx_write(idx, fade_val);
}

/* pattern0 was adopted to realize the automatic breathing mode */
void aw20xxx_all_breath_forever(void)
{
	int i = 0;

	/* 1. set fade source */
	aw20xxx_set_page(AW20XXX_REG_PAGE3);
	for (i = 0; i < AW20XXX_REG_NUM_PAG3; i++) {
		//aw20xxx_i2c_write(g_aw20xxx, i, 0x01);
        aw20xxx_write(0x00+i, 0x01);
	}

	/* 2. set dim val */
	aw20xxx_set_page(AW20XXX_REG_PAGE1);
	for (i = 0; i < AW20XXX_REG_NUM_PAG1; i++) {
		aw20xxx_set_dim_by_idx(0x00+i, AW20XXX_DBGCTR_DIM);
	}

	/* 3. config page0 */
	aw20xxx_set_page(AW20XXX_REG_PAGE0);

	/* 1. set pattern0_breath max/min level */
//	aw20xxx_i2c_write(g_aw20xxx, REG_FADEH0, 0xff);
//	aw20xxx_i2c_write(g_aw20xxx, REG_FADEL0, 0x1f);

    aw20xxx_write(REG_FADEH0, 0xff);
    aw20xxx_write(REG_FADEL0, 0x1f);

	/* 2. set pattern0_breath T1 and T2 , T3 and T4 */
//	aw20xxx_i2c_write(g_aw20xxx, REG_PAT0T0, 0x30|0x03);
//	aw20xxx_i2c_write(g_aw20xxx, REG_PAT0T1, 0x30|0x03);

    aw20xxx_write(REG_PAT0T0, 0x30|0x03);
    aw20xxx_write(REG_PAT0T1, 0x30|0x03);

	/* 3 set pattern0_breath start/end phase */
	//aw20xxx_i2c_write(g_aw20xxx, REG_PAT0T2, 0x00|0x00);
    aw20xxx_write(REG_PAT0T2, 0x00|0x00);

	/* 4 set pattern0_breath loop times -> forever */
	//aw20xxx_i2c_write(g_aw20xxx, REG_PAT0T3, 0x00);
    aw20xxx_write(REG_PAT0T3, 0x00);

	/* 5. enalbe patern0 */
	//aw20xxx_i2c_write(g_aw20xxx, REG_PATE, 0x01);
    aw20xxx_write(REG_PATE, 0x01);
        
	/* 6. set auto_breath mode */
	//aw20xxx_i2c_write(g_aw20xxx, REG_PAT0CFG, 0x01);
    aw20xxx_write(REG_PAT0CFG, 0x01);

	/* 7. stop run */
	//aw20xxx_i2c_write(g_aw20xxx, REG_PATGO, 0x00);
    aw20xxx_write(REG_PATGO, 0x00);

	/* 8. start breath */
	//aw20xxx_i2c_write(g_aw20xxx, REG_PATGO, 0x01);
    aw20xxx_write(REG_PATGO, 0x01);
}

/*********************************************************
 *
 * light effect
 *
 ********************************************************/
static void aw20xxx_rgb_multi_breath_init(const AW_MULTI_BREATH_DATA_STRUCT *data)
{
	unsigned char i;
	//aw20xxx_interface.getBrightnessfunc = aw_get_breath_brightness_algo_func(BREATH_ALGO_GAMMA_CORRECTION);
	algo_data.cur_frame = 0;
	algo_data.total_frames = 20;
	algo_data.data_start = 0;
	algo_data.data_end = 0;
	aw20xxx_interface.p_algo_data = &algo_data;

	for (i = 0; i < RGB_NUM; i++) {
		colorful_cur_frame[i] = 0;
		colorful_total_frames[i] = 30;
		colorful_cur_color_index[i] = 0;
		colorful_cur_phase[i] = 0;
		colorful_phase_nums[i] = 5;
		breath_cur_phase[i] = 0;
		breath_phase_nums[i] = 5;
		aw20xxx_algo_data[i].cur_frame = 0;
		aw20xxx_algo_data[i].total_frames = data[i].time[0]/20;
		aw20xxx_algo_data[i].data_start = data[i].fadel;
		aw20xxx_algo_data[i].data_end = data[i].fadeh;
		source_color[i].r = 0x00;
		source_color[i].g = 0x00;
		source_color[i].b = 0x00;
		destination_color[i].r = 0x00;
		destination_color[i].g = 0x00;
		destination_color[i].b = 0x00;
	}
}
static void aw20xxx_update_frame_idx(const AW_MULTI_BREATH_DATA_STRUCT *data)
{
	unsigned char i;
	int update_frame_idx = 0;

	for (i = 0; i < RGB_NUM; i++) {
		if (loop_end[i] == 1)
				continue;
		update_frame_idx = 1;

		source_color[i].r = destination_color[i].r;
		source_color[i].g = destination_color[i].g;
		source_color[i].b = destination_color[i].b;

		aw20xxx_algo_data[i].cur_frame++;
		if (aw20xxx_algo_data[i].cur_frame >=
			aw20xxx_algo_data[i].total_frames) {
			aw20xxx_algo_data[i].cur_frame = 0;
			breath_cur_phase[i]++;
			if (breath_cur_phase[i] >= breath_phase_nums[i]) {
				colorful_cur_color_index[i]++;
				breath_cur_phase[i] = 1;
				if (colorful_cur_color_index[i] >= data[i].color_nums) {
					colorful_cur_color_index[i] = 0;
					if (data[i].repeat_nums == 0)
						breath_cur_loop[i] = 0;
					else if (breath_cur_loop[i] >= (data[i].repeat_nums - 1))
						update_frame_idx = 0;
					else
						breath_cur_loop[i]++;
				}
			}
		}
		destination_color[i].r = data[i].rgb_color_list[colorful_cur_color_index[i]].r;
		destination_color[i].g = data[i].rgb_color_list[colorful_cur_color_index[i]].g;
		destination_color[i].b = data[i].rgb_color_list[colorful_cur_color_index[i]].b;
		if (update_frame_idx) {
			aw20xxx_algo_data[i].total_frames =
				(data[i].time[breath_cur_phase[i]])/20;
			if (breath_cur_phase[i] == 1) {
				aw20xxx_algo_data[i].data_start = data[i].fadel;
				aw20xxx_algo_data[i].data_end = data[i].fadeh;
			} else if (breath_cur_phase[i] == 2) {
				aw20xxx_algo_data[i].data_start = data[i].fadeh;
				aw20xxx_algo_data[i].data_end = data[i].fadeh;
			} else if (breath_cur_phase[i] == 3) {
				aw20xxx_algo_data[i].data_start = data[i].fadeh;
				aw20xxx_algo_data[i].data_end = data[i].fadel;
			} else {
				aw20xxx_algo_data[i].data_start = data[i].fadel;
				aw20xxx_algo_data[i].data_end = data[i].fadel;
			}
		} else
			loop_end[i] = 1;
	}
}

static void aw20xxx_frame_display(void)
{
	unsigned char i;
	unsigned char brightness = 0;

	for (i = 0; i < RGB_NUM; i++) {
		aw20xxx_interface.p_color_1 = &source_color[i];
		aw20xxx_interface.p_color_2 = &destination_color[i];
		//aw_set_colorful_rgb_data(i, dim_data, &aw20xxx_interface);
		brightness = aw20xxx_interface.getBrightnessfunc(&aw20xxx_algo_data[i]);
		//aw_set_rgb_brightness(i, fade_data, brightness);
	}
}

static void aw20xxx_update(void)
{
	unsigned char i;

	aw20xxx_set_page(AW20XXX_REG_PAGE4);
	for (i = 0; i < RGB_NUM; i++) {
		//aw20xxx_page4_data[aw20xxx_reg_map[i*6+0]] = aw_get_real_dim(dim_data[i*3 + 0]);
		aw20xxx_page4_data[aw20xxx_reg_map[i*6+1]] = fade_data[i*3 + 0];
		//aw20xxx_page4_data[aw20xxx_reg_map[i*6+2]] = aw_get_real_dim(dim_data[i*3 + 1]);
		aw20xxx_page4_data[aw20xxx_reg_map[i*6+3]] = fade_data[i*3 + 1];
		//aw20xxx_page4_data[aw20xxx_reg_map[i*6+4]] = aw_get_real_dim(dim_data[i*3 + 2]);
		aw20xxx_page4_data[aw20xxx_reg_map[i*6+5]] = fade_data[i*3 + 2];
	}
//	aw20xxx_i2c_writes(g_aw20xxx, 0x00, aw20xxx_page4_data, AW20XXX_REG_NUM_PAG4);
    aw20xxx_write_array(0x00, aw20xxx_page4_data, AW20XXX_REG_NUM_PAG4);
}

void aw20xxx_rgb_multi_breath(const AW_MULTI_BREATH_DATA_STRUCT *data)
{
	aw20xxx_rgb_multi_breath_init(data);
	aw20xxx_frame_display();
	aw20xxx_update();

	while (1) {
		//soft_delay(10);
        msleep(10);
		aw20xxx_update_frame_idx(data);
		aw20xxx_frame_display();
		aw20xxx_update();
	}
}
void aw20xxx_led_Ribbon(int times)
{
	int i = 0;

	aw20xxx_set_page(AW20XXX_REG_PAGE1);
	for (i = 0; i < AW20XXX_REG_NUM_PAG1; i++) {
		aw20xxx_set_dim_by_idx(0x00+i, AW20XXX_DBGCTR_DIM);
	}

	aw20xxx_set_page(AW20XXX_REG_PAGE2);
	for (i = 0; i < AW20XXX_REG_NUM_PAG2; i++) {
		aw20xxx_set_fade_by_idx(0x00+i, AW20XXX_DBGCTR_FDAD);
		//soft_delay(times);
		msleep(times);
	}
}

void aw20xxx_all_on(void)
{
	int i = 0;

	/* write dim for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE1);
	for (i = 0; i < AW20XXX_REG_NUM_PAG1; i++) { /* AW20XXX_REG_NUM_PAG1 */
		aw20xxx_set_dim_by_idx(0x00+i, AW20XXX_DBGCTR_DIM);/* AW20XXX_DBGCTR_DIM */
	}

	/* write fade for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE2);
	for (i = 0; i < AW20XXX_REG_NUM_PAG2; i++) {/* AW20XXX_REG_NUM_PAG2 */
		//aw20xxx_set_fade_by_idx(0x00+i, AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
		aw20xxx_set_fade_by_idx(0x00+i, AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
	}
}

void aw20xxx_blue_on(void)
{
	int i = 0;

	/* write dim for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE1);
	for (i = 0; i < 12; i++) { /* AW20XXX_REG_NUM_PAG1 */
		aw20xxx_set_dim_by_idx(0x00+3*i, AW20XXX_DBGCTR_DIM);/* AW20XXX_DBGCTR_DIM */
	}

	/* write fade for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE2);
	for (i = 0; i < 12; i++) {/* AW20XXX_REG_NUM_PAG2 */
		//aw20xxx_set_fade_by_idx(0x00+i, AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
		aw20xxx_set_fade_by_idx(0x00+3*i, AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
	}
}

void aw20xxx_green_on(void)
{
	int i = 0;

	/* write dim for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE1);
	for (i = 0; i < 12; i++) { /* AW20XXX_REG_NUM_PAG1 */
		aw20xxx_set_dim_by_idx(0x00+(3*i+1), AW20XXX_DBGCTR_DIM);/* AW20XXX_DBGCTR_DIM */
	}

	/* write fade for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE2);
	for (i = 0; i < 12; i++) {/* AW20XXX_REG_NUM_PAG2 */
		//aw20xxx_set_fade_by_idx(0x00+i, AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
		aw20xxx_set_fade_by_idx(0x00+(3*i+1), AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
	}
}

void aw20xxx_red_on(void)
{
	int i = 0;

	/* write dim for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE1);
	for (i = 0; i < 12; i++) { /* AW20XXX_REG_NUM_PAG1 */
		aw20xxx_set_dim_by_idx(0x00+(3*i+2), AW20XXX_DBGCTR_DIM);/* AW20XXX_DBGCTR_DIM */
	}

	/* write fade for brightness */
	aw20xxx_set_page(AW20XXX_REG_PAGE2);
	for (i = 0; i < 12; i++) {/* AW20XXX_REG_NUM_PAG2 */
		//aw20xxx_set_fade_by_idx(0x00+i, AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
		aw20xxx_set_fade_by_idx(0x00+(3*i+2), AW20XXX_DBGCTR_FDAD);/* AW20XXX_DBGCTR_FDAD */
	}
}


//static int32_t aw20xxx_en_pull_high(struct aw20xxx *aw20xxx)
//{
//	if (aw20xxx->gpio_port != NULL && aw20xxx->en_gpio_pin != 0) {
//		HAL_GPIO_WritePin(aw20xxx->gpio_port, aw20xxx->en_gpio_pin,
//						  GPIO_PIN_SET);
//		soft_delay(3);
//		aw20xxx->en_flag = 1;
//		AWLOGD("%s:pin en pull high !\n", __func__);
//	} else {
//		AWLOGD("%s:pin en pull high error! no config gpio info!\n",
//				__func__);
//		return AW20XXX_PULL_EN_ERROR;
//	}
//
//	return 0;
//}

//static void aw20xxx_pin_init(struct aw20xxx *aw20xxx)
//{
//	aw20xxx->hi2cx = &hi2c1;
//	aw20xxx->gpio_port = GPIOA;
//	aw20xxx->irq_gpio_pin = GPIO_PIN_2;
//	aw20xxx->en_gpio_pin = GPIO_PIN_3;
//}

static void aw20xxx_soft_rst(void)
{
	aw20xxx_set_page(AW20XXX_REG_PAGE0);
//	aw20xxx_i2c_write(g_aw20xxx, REG_SWRST, AW20XXX_RSTR);
    aw20xxx_write(REG_SWRST, AW20XXX_RSTR);
//	soft_delay(2);
    msleep(2);
};

/*****************************************************
 *
 * check chip id
 *
 *****************************************************/
static int aw20xxx_read_chipid(struct aw20xxx *aw20xxx)
{
	//int ret = -1;
	uint8_t cnt = 0;
	uint8_t reg_val = 0;

	//AWLOGD("%s, enter\n", __func__);

	aw20xxx_set_page(AW20XXX_REG_PAGE0);

	while (cnt++ < AW_READ_CHIPID_RETRIES) {
		//ret = aw20xxx_i2c_read(aw20xxx, REG_CHIPID, &reg_val);
        reg_val = aw20xxx_read(REG_CHIPID);
		if (reg_val == AW20XXX_CHIPID) {
			pr_err("%s, This Chip is AW20108 REG_ID: 0x%x\n",
				__func__, reg_val);
			return 0;
		} else {
			pr_err("%s, enter, read chip id error ret = %d\n",
				__func__, reg_val);
        }
//		} else {
//			AWLOGD("%s, enter, read chip id is %#x\n",
//				__func__, reg_val);
//		}
		//soft_delay(5);
		msleep(5);
	}
	return -1;
}

static int aw20xxx_chip_init(void)
{
	//unsigned char val = 0;

	/* 1. switch to page0 */
	aw20xxx_set_page(AW20XXX_REG_PAGE0);
	/* 2. soft_reset */
	aw20xxx_soft_rst();
    msleep(100);
	/* 3. config global imx and all led on */
	//aw20xxx_i2c_write(g_aw20xxx, REG_GCCR, AW20XXX_IMAX_30mA<<4 | 0x08);
    aw20xxx_write(REG_GCCR, AW20XXX_IMAX_30mA<<4 | 0x08);
	/* 4. config display size */
	//aw20xxx_i2c_write(g_aw20xxx, REG_SIZE, AW20XXX_CURRENT_SWITCH);
    aw20xxx_write(REG_SIZE, AW20XXX_CURRENT_SWITCH);
	/* 5. exit sleep mode */
	//aw20xxx_i2c_write(g_aw20xxx, REG_WORK_MODE, 0x00);
    aw20xxx_write(REG_WORK_MODE, 0x00);

	return 0;
}

/*********************************************************
 *
 * call interface
 *
 ********************************************************/
int aw20xxx_play(void)
{
	int32_t ret;
//	struct aw20xxx *aw20xxx;
//
//	AWLOGD("%s, enter\n", __func__);
//
//	aw20xxx = (struct aw20xxx *)malloc(sizeof(struct aw20xxx));
//	if (aw20xxx == NULL) {
//		AWLOGD("%s malloc aw20xxx fail\n", __func__);
//		return AW20XXX_READ_CHIPID_ERROR;
//	}

	//aw20xxx_pin_init(aw20xxx);
//	ret = aw20xxx_en_pull_high(aw20xxx);
//	if (ret < 0)
//		AWLOGD("%s aw20108 en pull up fail\n", __func__);

//	g_aw20xxx = aw20xxx;
	ret = aw20xxx_read_chipid(g_aw20xxx);
	if (ret) {
		pr_err("%s:chip not detected!!\n", __func__);
		//free(g_aw20xxx);
		return ret;
	}
	aw20xxx_chip_init();

	/* base fuction*/
	
	//aw20xxx_all_on();
    aw20xxx_green_on();
//	soft_delay(2000);
/*
    msleep(200);
    

	// The interval between the previous lamp and the next lamp is 1000
	aw20xxx_led_Ribbon(100);
	//soft_delay(1000);
	msleep(2000);

	// breath led
	aw20xxx_all_breath_forever();
    msleep(2000);
	*/
	/* light effect */
	//aw20xxx_rgb_multi_breath(aw20xxx_rgb36_data);
	return 0;
}

static int AW20XXX_parse_dt(struct device *dev)
{
    struct device_node *np = dev->of_node;
    int ret = 0;
    //u32 flag_en;
    u32 flag_irq;

//    g_aw20xxx->en_gpio_pin = of_get_named_gpio_flags(np, "en-gpio", 0, &flag_en);
//
//    if (!gpio_is_valid(g_aw20xxx->en_gpio_pin)) {
//        pr_err("en_gpio_pin gpio not specified\n");
//        ret = -EINVAL;
//    }

    g_aw20xxx->irq_gpio_pin = of_get_named_gpio_flags(np, "irq-gpio", 0, &flag_irq);

    if (!gpio_is_valid(g_aw20xxx->irq_gpio_pin)) {
        pr_err("irq_gpio_pin gpio not specified\n");
        ret = -EINVAL;
    }

    return ret;
}


__attribute__((unused))static int AW20XXX_gpio_configure(bool on)
{
    int ret = 0;

    if (on) {
//        ret = gpio_request(g_aw20xxx->en_gpio_pin,
//                   "AW20XXX-en-gpio");
//        if (ret) {
//            pr_err("AW20XXX en gpio request failed\n");
//            goto power_error;
//        }
//        gpio_direction_output(g_aw20xxx->en_gpio_pin,1);
//        msleep(100);
//        ret = gpio_request(g_aw20xxx->irq_gpio_pin,
//                   "AW20XXX-irq-gpio");
        if (ret) {
            pr_err("AW20XXX irq gpio request failed\n");
            goto irq_error;
        gpio_direction_output(g_aw20xxx->irq_gpio_pin,0);
        msleep(100);
        }
    } else {
		goto error;
    }

    return ret;

//power_error:
//    gpio_free(g_aw20xxx->en_gpio_pin);
irq_error:
    gpio_free(g_aw20xxx->irq_gpio_pin);
error:
    return ret;
}


static int AW20XXX_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;

    if (!client || !client->dev.of_node) {
        pr_err("AW20XXX invalid input\n");
        return -EINVAL;
    }

    g_aw20xxx_client = devm_kzalloc(&client->dev,
                 sizeof(struct i2c_client), GFP_KERNEL);
    if (!g_aw20xxx_client) {
        pr_err( "Failed alloc aw20xxx_client memory\n");
        return -ENOMEM;
    }

   g_aw20xxx_client = client; 

    if(!i2c_check_functionality(g_aw20xxx_client->adapter, I2C_FUNC_I2C)) {  
        pr_err( "aw20xxx Failed check I2C functionality\n");
        return -ENODEV;
    }
 
    g_aw20xxx = devm_kzalloc(&g_aw20xxx_client->dev,
                 sizeof(struct aw20xxx), GFP_KERNEL);
    if (!g_aw20xxx) {
        pr_err( "Failed alloc aw20xxx memory\n");
        return -ENOMEM;
    }

    g_aw20xxx->i2c_addr = g_aw20xxx_client->addr;
    g_aw20xxx->i2c_client = g_aw20xxx_client;
    g_aw20xxx->dev = &g_aw20xxx_client->dev;
    pr_info( "aw20xxx I2C address is 0x%x\n",g_aw20xxx->i2c_addr);

    ret = AW20XXX_parse_dt(&g_aw20xxx_client->dev);
    if (ret) {
        pr_err( "aw20xxx Failed to parse device tree\n");
        goto err_dt_parse;
    }

    i2c_set_clientdata(g_aw20xxx_client, g_aw20xxx);

//    ret = AW20XXX_gpio_configure(true);
//    if (ret) {
//        pr_err( "aw20xxx Failed to configure GPIOs\n");
//    }

   
    aw20xxx_play();


    return ret;

//err_i2c_prog:
//    LT9211C_gpio_configure(LONTIUM_FALSE);
err_dt_parse:
    devm_kfree(&g_aw20xxx_client->dev, g_aw20xxx);

    return ret;
}

static int AW20XXX_remove(struct i2c_client *client)
{
    struct aw20xxx *aw20xxx = i2c_get_clientdata(client);

    if(gpio_is_valid(aw20xxx->en_gpio_pin))
        gpio_free(aw20xxx->en_gpio_pin);

    pr_info( "gpio for aw20xxx driver release");
    i2c_set_clientdata(client, NULL);

    devm_kfree(&client->dev, aw20xxx);

    pr_info( "remove aw20xxx driver successfully\n");

    return 0;
}

static struct i2c_device_id  AW20XXX_id[] = {
    { "AW20XXX", 0 },
    {}
};

static const struct of_device_id AW20XXX_match_table[] = {
    { .compatible = "FAN,AW20XXX" },
    {}
};

MODULE_DEVICE_TABLE(of, AW20XXX_match_table);

static struct i2c_driver AW20XXX_driver = {
    .driver         = {
        .name       = " AW20XXX",
        .owner      = THIS_MODULE,
        .of_match_table = AW20XXX_match_table,
    },
    .probe          =  AW20XXX_probe,
    .remove         =  AW20XXX_remove,
    .id_table       =  AW20XXX_id,
};

static int __init  AW20XXX_init(void)
{
    int ret = 0;
    pr_info("AW20XXX driver installing....\n");
    ret = i2c_add_driver(&AW20XXX_driver);
    return ret;
}

subsys_initcall(AW20XXX_init);

static void __exit  AW20XXX_exit(void)
{
    pr_info("AW20XXX driver exited\n");
    i2c_del_driver(&AW20XXX_driver);
}


module_init(AW20XXX_init);
module_exit(AW20XXX_exit);

MODULE_AUTHOR("lp@fanconn.com");
MODULE_DESCRIPTION("AW20036)");
MODULE_LICENSE("GPL");

