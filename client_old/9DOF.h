#ifndef __9DOF_H__
#define __9DOF_H__

#include <stdio.h>
#include <mraa/i2c.h>
#include "LSM9DS0.h"

typedef struct {
	data_t accel_data, gyro_data, mag_data;
	data_t gyro_offset;
	int16_t temperature;
	float a_res, g_res, m_res;
	mraa_i2c_context accel, gyro, mag;
	accel_scale_t a_scale;
	gyro_scale_t g_scale;
	mag_scale_t m_scale;
} NINEDOF;

NINEDOF *ninedof_init(accel_scale_t a, gyro_scale_t g, mag_scale_t m);
void ninedof_read(NINEDOF *ninedof);
void ninedof_print(NINEDOF* ninedof);

#endif // 9DOF_H
