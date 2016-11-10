/******************************************************************************
LSM9DS0.c
LSM9DS0 Library Source File
Author:
	In Hwan “Chris” Baek @ Wireless Health Institute, UCLA
Based on C++ libraries by:
	Jim Lindblom @ SparkFun Electronics
	Original Creation Date: February 14, 2014 (Happy Valentines Day!)
	Modified 14 Jul 2015 by Mike Hord to add Edison support
	https://github.com/sparkfun/SparkFun_9DOF_Block_for_Edison_CPP_Library
License Information:
	GPL
	Distributed as-is; no warranty is given.
	
This file implements all functions for the LSM9DS0. Functions here range
from higher level stuff, like reading/writing LSM9DS0 registers to low-level,
hardware reads and writes.

** Supports only I2C connections!!! **

Development environment specifics:
	Code developed in Intel's Eclipse IOT-DK
	This code requires the Intel mraa library to function; for more
	information see https://github.com/intel-iot-devkit/mraa
******************************************************************************/

#include <stdio.h>
#include <mraa/i2c.h>
#include "LSM9DS0.h"

#define XM_ADDR 0x1d
#define GYRO_ADDR 0x6B

mraa_i2c_context gyro_init()
{
	mraa_i2c_context gyro;
	gyro = mraa_i2c_init(1);

	if (gyro == NULL) {
		fprintf(stderr, "Failed to initialize I2C.\n");
		exit(1);
	}
	mraa_i2c_address(gyro, GYRO_ADDR);
	
	uint8_t gyro_id = mraa_i2c_read_byte_data(gyro, WHO_AM_I_G);
	if (gyro_id != 0xD4) {
		fprintf(stderr, "Gyroscope ID does not match.\n");
	}
	
	/* CTRL_REG1_G sets output data rate, bandwidth, power-down and enables
	Bits[7:0]: DR1 DR0 BW1 BW0 PD Zen Xen Yen
	DR[1:0] - Output data rate selection
		00=95Hz, 01=190Hz, 10=380Hz, 11=760Hz
	BW[1:0] - Bandwidth selection (sets cutoff frequency)
		 Value depends on ODR. See datasheet table 21.
	PD - Power down enable (0=power down mode, 1=normal or sleep mode)
	Zen, Xen, Yen - Axis enable (o=disabled, 1=enabled)	*/
	//gWriteByte(CTRL_REG1_G, 0x0F); // Normal mode, enable all axes
	mraa_i2c_write_byte_data(gyro, 0x0F, CTRL_REG1_G);		

	
	/* CTRL_REG2_G sets up the HPF
	Bits[7:0]: 0 0 HPM1 HPM0 HPCF3 HPCF2 HPCF1 HPCF0
	HPM[1:0] - High pass filter mode selection
		00=normal (reset reading HP_RESET_FILTER, 01=ref signal for filtering,
		10=normal, 11=autoreset on interrupt
	HPCF[3:0] - High pass filter cutoff frequency
		Value depends on data rate. See datasheet table 26.
	*/
	//gWriteByte(CTRL_REG2_G, 0x00); // Normal mode, high cutoff frequency
	mraa_i2c_write_byte_data(gyro, 0x00, CTRL_REG2_G);		

	
	/* CTRL_REG3_G sets up interrupt and DRDY_G pins
	Bits[7:0]: I1_IINT1 I1_BOOT H_LACTIVE PP_OD I2_DRDY I2_WTM I2_ORUN I2_EMPTY
	I1_INT1 - Interrupt enable on INT_G pin (0=disable, 1=enable)
	I1_BOOT - Boot status available on INT_G (0=disable, 1=enable)
	H_LACTIVE - Interrupt active configuration on INT_G (0:high, 1:low)
	PP_OD - Push-pull/open-drain (0=push-pull, 1=open-drain)
	I2_DRDY - Data ready on DRDY_G (0=disable, 1=enable)
	I2_WTM - FIFO watermark interrupt on DRDY_G (0=disable 1=enable)
	I2_ORUN - FIFO overrun interrupt on DRDY_G (0=disable 1=enable)
	I2_EMPTY - FIFO empty interrupt on DRDY_G (0=disable 1=enable) */
	// Int1 enabled (pp, active low), data read on DRDY_G:
	//gWriteByte(CTRL_REG3_G, 0x88);
	mraa_i2c_write_byte_data(gyro, 0x88, CTRL_REG3_G);		

	
	/* CTRL_REG4_G sets the scale, update mode
	Bits[7:0] - BDU BLE FS1 FS0 - ST1 ST0 SIM
	BDU - Block data update (0=continuous, 1=output not updated until read
	BLE - Big/little endian (0=data LSB @ lower address, 1=LSB @ higher add)
	FS[1:0] - Full-scale selection
		00=245dps, 01=500dps, 10=2000dps, 11=2000dps
	ST[1:0] - Self-test enable
		00=disabled, 01=st 0 (x+, y-, z-), 10=undefined, 11=st 1 (x-, y+, z+)
	SIM - SPI serial interface mode select
		0=4 wire, 1=3 wire */
	//gWriteByte(CTRL_REG4_G, 0x00); // Set scale to 245 dps
	mraa_i2c_write_byte_data(gyro, 0x00, CTRL_REG4_G);
	
	/* CTRL_REG5_G sets up the FIFO, HPF, and INT1
	Bits[7:0] - BOOT FIFO_EN - HPen INT1_Sel1 INT1_Sel0 Out_Sel1 Out_Sel0
	BOOT - Reboot memory content (0=normal, 1=reboot)
	FIFO_EN - FIFO enable (0=disable, 1=enable)
	HPen - HPF enable (0=disable, 1=enable)
	INT1_Sel[1:0] - Int 1 selection configuration
	Out_Sel[1:0] - Out selection configuration */
	//gWriteByte(CTRL_REG5_G, 0x00);
	mraa_i2c_write_byte_data(gyro, 0x00, CTRL_REG5_G);
	
	return gyro;
}

mraa_i2c_context accel_init()
{
	mraa_i2c_context accel;
	accel = mraa_i2c_init(1);

	if (accel == NULL) {
		fprintf(stderr, "Failed to initialize I2C.\n");
		exit(1);
	}
	mraa_i2c_address(accel, XM_ADDR);
	
	uint8_t accel_id = mraa_i2c_read_byte_data(accel, WHO_AM_I_XM);
	if (accel_id != 0x49) {
		fprintf(stderr, "Accelerometer/Magnetometer ID does not match.\n");
	}
	
	/* CTRL_REG0_XM (0x1F) (Default value: 0x00)
	Bits (7-0): BOOT FIFO_EN WTM_EN 0 0 HP_CLICK HPIS1 HPIS2
	BOOT - Reboot memory content (0: normal, 1: reboot)
	FIFO_EN - Fifo enable (0: disable, 1: enable)
	WTM_EN - FIFO watermark enable (0: disable, 1: enable)
	HP_CLICK - HPF enabled for click (0: filter bypassed, 1: enabled)
	HPIS1 - HPF enabled for interrupt generator 1 (0: bypassed, 1: enabled)
	HPIS2 - HPF enabled for interrupt generator 2 (0: bypassed, 1 enabled)   */
	//xmWriteByte(CTRL_REG0_XM, 0x00);
	mraa_i2c_write_byte_data(accel, 0x00, CTRL_REG0_XM);		
	/* CTRL_REG1_XM (0x20) (Default value: 0x07)
	Bits (7-0): AODR3 AODR2 AODR1 AODR0 BDU AZEN AYEN AXEN
	AODR[3:0] - select the acceleration data rate:
		0000=power down, 0001=3.125Hz, 0010=6.25Hz, 0011=12.5Hz, 
		0100=25Hz, 0101=50Hz, 0110=100Hz, 0111=200Hz, 1000=400Hz,
		1001=800Hz, 1010=1600Hz, (remaining combinations undefined).
	BDU - block data update for accel AND mag
		0: Continuous update
		1: Output registers aren't updated until MSB and LSB have been read.
	AZEN, AYEN, and AXEN - Acceleration x/y/z-axis enabled.
		0: Axis disabled, 1: Axis enabled									 */	
	//xmWriteByte(CTRL_REG1_XM, 0x57); // 100Hz data rate, x/y/z all enabled
	mraa_i2c_write_byte_data(accel, 0x57, CTRL_REG1_XM);		
	
	//Serial.println(xmReadByte(CTRL_REG1_XM));
	/* CTRL_REG2_XM (0x21) (Default value: 0x00)
	Bits (7-0): ABW1 ABW0 AFS2 AFS1 AFS0 AST1 AST0 SIM
	ABW[1:0] - Accelerometer anti-alias filter bandwidth
		00=773Hz, 01=194Hz, 10=362Hz, 11=50Hz
	AFS[2:0] - Accel full-scale selection
		000=+/-2g, 001=+/-4g, 010=+/-6g, 011=+/-8g, 100=+/-16g
	AST[1:0] - Accel self-test enable
		00=normal (no self-test), 01=positive st, 10=negative st, 11=not allowed
	SIM - SPI mode selection
		0=4-wire, 1=3-wire													 */
	//xmWriteByte(CTRL_REG2_XM, 0x00); // Set scale to 2g
	mraa_i2c_write_byte_data(accel, 0x00, CTRL_REG2_XM);		
	
	/* CTRL_REG3_XM is used to set interrupt generators on INT1_XM
	Bits (7-0): P1_BOOT P1_TAP P1_INT1 P1_INT2 P1_INTM P1_DRDYA P1_DRDYM P1_EMPTY
	*/
	// Accelerometer data ready on INT1_XM (0x04)
	//xmWriteByte(CTRL_REG3_XM, 0x04);
	mraa_i2c_write_byte_data(accel, 0x04, CTRL_REG3_XM);		

	return accel;
}

mraa_i2c_context mag_init()
{
	mraa_i2c_context mag;
	mag = mraa_i2c_init(1);

	if (mag == NULL) {
		fprintf(stderr, "Failed to initialize I2C.\n");
		exit(1);
	}
	mraa_i2c_address(mag, XM_ADDR);
	
	uint8_t mag_id = mraa_i2c_read_byte_data(mag, WHO_AM_I_XM);
	if (mag_id != 0x49) {
		fprintf(stderr, "Accelerometer/Magnetometer ID does not match.\n");
	}
		
	/* CTRL_REG5_XM enables temp sensor, sets mag resolution and data rate
	Bits (7-0): TEMP_EN M_RES1 M_RES0 M_ODR2 M_ODR1 M_ODR0 LIR2 LIR1
	TEMP_EN - Enable temperature sensor (0=disabled, 1=enabled)
	M_RES[1:0] - Magnetometer resolution select (0=low, 3=high)
	M_ODR[2:0] - Magnetometer data rate select
		000=3.125Hz, 001=6.25Hz, 010=12.5Hz, 011=25Hz, 100=50Hz, 101=100Hz
	LIR2 - Latch interrupt request on INT2_SRC (cleared by reading INT2_SRC)
		0=interrupt request not latched, 1=interrupt request latched
	LIR1 - Latch interrupt request on INT1_SRC (cleared by readging INT1_SRC)
		0=irq not latched, 1=irq latched 									 */
	//xmWriteByte(CTRL_REG5_XM, 0x94); // Mag data rate - 100 Hz, enable temperature sensor
	mraa_i2c_write_byte_data(mag, 0x94, CTRL_REG5_XM);
	
	/* CTRL_REG6_XM sets the magnetometer full-scale
	Bits (7-0): 0 MFS1 MFS0 0 0 0 0 0
	MFS[1:0] - Magnetic full-scale selection
	00:+/-2Gauss, 01:+/-4Gs, 10:+/-8Gs, 11:+/-12Gs							 */
	//xmWriteByte(CTRL_REG6_XM, 0x00); // Mag scale to +/- 2GS
	mraa_i2c_write_byte_data(mag, 0x00, CTRL_REG6_XM);
	
	/* CTRL_REG7_XM sets magnetic sensor mode, low power mode, and filters
	AHPM1 AHPM0 AFDS 0 0 MLP MD1 MD0
	AHPM[1:0] - HPF mode selection
		00=normal (resets reference registers), 01=reference signal for filtering, 
		10=normal, 11=autoreset on interrupt event
	AFDS - Filtered acceleration data selection
		0=internal filter bypassed, 1=data from internal filter sent to FIFO
	MLP - Magnetic data low-power mode
		0=data rate is set by M_ODR bits in CTRL_REG5
		1=data rate is set to 3.125Hz
	MD[1:0] - Magnetic sensor mode selection (default 10)
		00=continuous-conversion, 01=single-conversion, 10 and 11=power-down */
	//xmWriteByte(CTRL_REG7_XM, 0x00); // Continuous conversion mode
	mraa_i2c_write_byte_data(mag, 0x00, CTRL_REG7_XM);
	
	/* CTRL_REG4_XM is used to set interrupt generators on INT2_XM
	Bits (7-0): P2_TAP P2_INT1 P2_INT2 P2_INTM P2_DRDYA P2_DRDYM P2_Overrun P2_WTM
	*/
	//xmWriteByte(CTRL_REG4_XM, 0x04); // Magnetometer data ready on INT2_XM (0x08)
	mraa_i2c_write_byte_data(mag, 0x04, CTRL_REG4_XM);
	
	/* INT_CTRL_REG_M to set push-pull/open drain, and active-low/high
	Bits[7:0] - XMIEN YMIEN ZMIEN PP_OD IEA IEL 4D MIEN
	XMIEN, YMIEN, ZMIEN - Enable interrupt recognition on axis for mag data
	PP_OD - Push-pull/open-drain interrupt configuration (0=push-pull, 1=od)
	IEA - Interrupt polarity for accel and magneto
		0=active-low, 1=active-high
	IEL - Latch interrupt request for accel and magneto
		0=irq not latched, 1=irq latched
	4D - 4D enable. 4D detection is enabled when 6D bit in INT_GEN1_REG is set
	MIEN - Enable interrupt generation for magnetic data
		0=disable, 1=enable) */
	//xmWriteByte(INT_CTRL_REG_M, 0x09); // Enable interrupts for mag, active-low, push-pull
	mraa_i2c_write_byte_data(mag, 0x09, INT_CTRL_REG_M);
	
	return mag;
}

float calc_accel_res(accel_scale_t a_scale)
{
	// Possible accelerometer scales (and their register bit settings) are:
	// 2 g (000), 4g (001), 6g (010) 8g (011), 16g (100). Here's a bit of an 
	// algorithm to calculate g/(ADC tick) based on that 3-bit value:
	float a_res = a_scale == A_SCALE_16G ? 16.0 / 32768.0 : 
		   (((float) a_scale + 1.0) * 2.0) / 32768.0;
		   
	return a_res;
}

float calc_gyro_res(gyro_scale_t g_scale)
{
	// Possible gyro scales (and their register bit settings) are:
	// 245 DPS (00), 500 DPS (01), 2000 DPS (10). Here's a bit of an algorithm
	// to calculate DPS/(ADC tick) based on that 2-bit value:
	float g_res;
	
	switch (g_scale)
	{
	case G_SCALE_245DPS:
		g_res = 245.0 / 32768.0;
		break;
	case G_SCALE_500DPS:
		g_res = 500.0 / 32768.0;
		break;
	case G_SCALE_2000DPS:
		g_res = 2000.0 / 32768.0;
		break;
	}
	
	return g_res;
}

float calc_mag_res(mag_scale_t m_scale)
{
	// Possible magnetometer scales (and their register bit settings) are:
	// 2 Gs (00), 4 Gs (01), 8 Gs (10) 12 Gs (11). Here's a bit of an algorithm
	// to calculate Gs/(ADC tick) based on that 2-bit value:
	float m_res = m_scale == M_SCALE_2GS ? 2.0 / 32768.0 : 
	       (float) (m_scale << 2) / 32768.0;
	       
	return m_res;
}

void set_accel_ODR(mraa_i2c_context accel, accel_odr_t a_rate)
{
	// We need to preserve the other bytes in CTRL_REG1_XM. So, first read it:
	uint8_t temp = mraa_i2c_read_byte_data(accel, CTRL_REG1_XM);
	// Then mask out the accel ODR bits:
	temp &= 0xFF^(0xF << 4);
	// Then shift in our new ODR bits:
	temp |= (a_rate << 4);
	// And write the new register value back into CTRL_REG1_XM:
	mraa_i2c_write_byte_data(accel, temp, CTRL_REG1_XM);		
}

void set_gyro_ODR(mraa_i2c_context gyro, gyro_odr_t g_rate)
{
	// We need to preserve the other bytes in CTRL_REG1_G. So, first read it:
	uint8_t temp = mraa_i2c_read_byte_data(gyro, CTRL_REG1_G);
	// Then mask out the gyro ODR bits:
	temp &= 0xFF^(0xF << 4);
	// Then shift in our new ODR bits:
	temp |= (g_rate << 4);
	// And write the new register value back into CTRL_REG1_G:
	mraa_i2c_write_byte_data(gyro, temp, CTRL_REG1_G);
}

void set_mag_ODR(mraa_i2c_context mag, mag_odr_t m_rate)
{
	// We need to preserve the other bytes in CTRL_REG5_XM. So, first read it:
	uint8_t temp = mraa_i2c_read_byte_data(mag, CTRL_REG5_XM);
	// Then mask out the mag ODR bits:
	temp &= 0xFF^(0x7 << 2);
	// Then shift in our new ODR bits:
	temp |= (m_rate << 2);
	// And write the new register value back into CTRL_REG5_XM:
	mraa_i2c_write_byte_data(mag, temp, CTRL_REG5_XM);
}

void set_accel_scale(mraa_i2c_context accel, accel_scale_t a_scale)
{
	// We need to preserve the other bytes in CTRL_REG2_XM. So, first read it:
	uint8_t temp = mraa_i2c_read_byte_data(accel, CTRL_REG2_XM);
	// Then mask out the accel scale bits:
	temp &= 0xFF^(0x3 << 3);
	// Then shift in our new scale bits:
	temp |= a_scale << 3;
	// And write the new register value back into CTRL_REG2_XM:
	mraa_i2c_write_byte_data(accel, temp, CTRL_REG2_XM);
}

void set_gyro_scale(mraa_i2c_context gyro, gyro_scale_t g_scale)
{
	// We need to preserve the other bytes in CTRL_REG4_G. So, first read it:
	uint8_t temp = mraa_i2c_read_byte_data(gyro, CTRL_REG4_G);
	// Then mask out the gyro scale bits:
	temp &= 0xFF^(0x3 << 4);
	// Then shift in our new scale bits:
	temp |= g_scale << 4;
	// And write the new register value back into CTRL_REG4_G:
	mraa_i2c_write_byte_data(gyro, temp, CTRL_REG4_G);
}

void set_mag_scale(mraa_i2c_context mag, mag_scale_t m_scale)
{
	// We need to preserve the other bytes in CTRL_REG6_XM. So, first read it:
	uint8_t temp = mraa_i2c_read_byte_data(mag, CTRL_REG6_XM);
	// Then mask out the mag scale bits:
	temp &= 0xFF^(0x3 << 5);
	// Then shift in our new scale bits:
	temp |= m_scale << 5;
	// And write the new register value back into CTRL_REG6_XM:
	mraa_i2c_write_byte_data(mag, temp, CTRL_REG6_XM);
}

data_t read_accel(mraa_i2c_context accel, float a_res)
{
	int16_t raw_accel_x, raw_accel_y, raw_accel_z;
	uint8_t data_byte[6];
	data_t data;
	
	data_byte[0] = mraa_i2c_read_byte_data(accel, OUT_X_L_A);
	data_byte[1] = mraa_i2c_read_byte_data(accel, OUT_X_H_A);
	data_byte[2] = mraa_i2c_read_byte_data(accel, OUT_Y_L_A);
	data_byte[3] = mraa_i2c_read_byte_data(accel, OUT_Y_H_A);
	data_byte[4] = mraa_i2c_read_byte_data(accel, OUT_Z_L_A);
	data_byte[5] = mraa_i2c_read_byte_data(accel, OUT_Z_H_A);

	raw_accel_x = ((data_byte[1] << 8) | data_byte[0]);
	raw_accel_y = ((data_byte[3] << 8) | data_byte[2]);
	raw_accel_z = ((data_byte[5] << 8) | data_byte[4]);

	//printf("\t%d \t%d \t%d\n", raw_accel_x, raw_accel_y, raw_accel_z);
	data.x = raw_accel_x * a_res;
	data.y = raw_accel_y * a_res;
	data.z = raw_accel_z * a_res;

	return data;
}

data_t read_gyro(mraa_i2c_context gyro, float g_res)
{
	int16_t raw_gyro_x, raw_gyro_y, raw_gyro_z;
	uint8_t data_byte[6];
	data_t data;
	
	data_byte[0] = mraa_i2c_read_byte_data(gyro, OUT_X_L_G);
	data_byte[1] = mraa_i2c_read_byte_data(gyro, OUT_X_H_G);
	data_byte[2] = mraa_i2c_read_byte_data(gyro, OUT_Y_L_G);
	data_byte[3] = mraa_i2c_read_byte_data(gyro, OUT_Y_H_G);
	data_byte[4] = mraa_i2c_read_byte_data(gyro, OUT_Z_L_G);
	data_byte[5] = mraa_i2c_read_byte_data(gyro, OUT_Z_H_G);

	raw_gyro_x = ((data_byte[1] << 8) | data_byte[0]);
	raw_gyro_y = ((data_byte[3] << 8) | data_byte[2]);
	raw_gyro_z = ((data_byte[5] << 8) | data_byte[4]);

	//printf("\t%d \t%d \t%d\n", raw_gyro_x, raw_gyro_y, raw_gyro_z);
	data.x = raw_gyro_x * g_res;
	data.y = raw_gyro_y * g_res;
	data.z = raw_gyro_z * g_res;

	return data;
}

data_t read_mag(mraa_i2c_context mag, float m_res)
{
	int16_t raw_mag_x, raw_mag_y, raw_mag_z;
	uint8_t data_byte[6];
	data_t data;
	
	data_byte[0] = mraa_i2c_read_byte_data(mag, OUT_X_L_M);
	data_byte[1] = mraa_i2c_read_byte_data(mag, OUT_X_H_M);
	data_byte[2] = mraa_i2c_read_byte_data(mag, OUT_Y_L_M);
	data_byte[3] = mraa_i2c_read_byte_data(mag, OUT_Y_H_M);
	data_byte[4] = mraa_i2c_read_byte_data(mag, OUT_Z_L_M);
	data_byte[5] = mraa_i2c_read_byte_data(mag, OUT_Z_H_M);

	raw_mag_x = ((data_byte[1] << 8) | data_byte[0]);
	raw_mag_y = ((data_byte[3] << 8) | data_byte[2]);
	raw_mag_z = ((data_byte[5] << 8) | data_byte[4]);

	//printf("\t%d \t%d \t%d\n", raw_gyro_x, raw_gyro_y, raw_gyro_z);
	data.x = raw_mag_x * m_res;
	data.y = raw_mag_y * m_res;
	data.z = raw_mag_z * m_res;

	return data;
}

int16_t read_temp(mraa_i2c_context xm)
{
	uint8_t temp[2]; // We'll read two bytes from the temperature sensor into temp	
	temp[0] = mraa_i2c_read_byte_data(xm, OUT_TEMP_L_XM); // Read 2 bytes, beginning at OUT_TEMP_L_M
	temp[1] = mraa_i2c_read_byte_data(xm, OUT_TEMP_H_XM);
	int16_t temperature = temp[0] | (temp[1] & 0x0F) <<8; // Temperature is a 12-bit signed integer
	return temperature;
}

data_t calc_gyro_offset(mraa_i2c_context gyro, float g_res)
{
	data_t offset;
	data_t new_gyro_data;
	int i;
	
	offset = read_gyro(gyro, g_res);
	printf("Calculating the offsets...\n");
	
	for (i=2; i<102; i++) {
		new_gyro_data = read_gyro(gyro, g_res);
		offset.x = ((i-1)/(float)i)*offset.x + new_gyro_data.x/i;
		offset.y = ((i-1)/(float)i)*offset.y + new_gyro_data.y/i;
		offset.z = ((i-1)/(float)i)*offset.z + new_gyro_data.z/i;
		usleep(100000);
	}
	
	return offset;
}