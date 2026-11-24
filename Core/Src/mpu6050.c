/*
 * mpu6050.c
 *
 *  Created on: Nov 24, 2026
 *      Author: ZachNew28
 */

#include "mpu6050.h"

// MPU-6050 device addr for STM32 HAL
#define MPU6050_ADDR      (0x68U << 1)

// Register addresses
#define REG_SMPLRT_DIV    0x19
#define REG_CONFIG        0x1A
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_ACCEL_XOUT_H  0x3B
#define REG_PWR_MGMT_1    0x6B
#define REG_WHO_AM_I      0x75

// Sensor sensitivity at our selected ranges
#define ACCEL_SCALE       16384.0f
#define GYRO_SCALE        131.0f

static HAL_StatusTypeDef write_register(
		I2C_HandleTypeDef *i2c,
		uint8_t reg,
		uint8_t value)
{
	return HAL_I2C_Mem_Write(
			i2c,
			MPU6050_ADDR,
			reg,
			I2C_MEMADD_SIZE_8BIT,
			&value,
			1,
			100
	);
}

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *i2c)
{
	if (i2c == NULL)
	{
		return HAL_ERROR;
	}

	uint8_t device_id = 0;

	// Verify sensor identity
	if (HAL_I2C_Mem_Read(
			i2c,
			MPU6050_ADDR,
			REG_WHO_AM_I,
			I2C_MEMADD_SIZE_8BIT,
			&device_id,
			1,
			100) != HAL_OK)
	{
		return HAL_ERROR;
	}

	if (device_id != 0x68)
	{
		return HAL_ERROR;
	}

	// Wake sensor, select gyro PLL clock
	if (write_register(i2c, REG_PWR_MGMT_1, 0x01) != HAL_OK)
	{
		return HAL_ERROR;
	}
	HAL_Delay(100);

	// Enable digital low-pass filter
	if (write_register(i2c, REG_CONFIG, 0x03) != HAL_OK)
	{
		return HAL_ERROR;
	}

	// Configure 100 Hz sample rate
	if (write_register(i2c, REG_SMPLRT_DIV, 9) != HAL_OK)
	{
		return HAL_ERROR;
	}

	//Gyroscope: +/- 250 degrees/second
	if (write_register(i2c, REG_GYRO_CONFIG, 0x00) != HAL_OK)
	{
		return HAL_ERROR;
	}

	// Accelerometer: +/- 2 g
	if (write_register(i2c, REG_ACCEL_CONFIG, 0x00) != HAL_OK)
	{
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef MPU6050_Read(
		I2C_HandleTypeDef *i2c,
		MPU6050_Sample *sample)
{
	if (i2c == NULL || sample == NULL)
	{
		return HAL_ERROR;
	}

	uint8_t data[14];

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
			i2c,
			MPU6050_ADDR,
			REG_ACCEL_XOUT_H,
			I2C_MEMADD_SIZE_8BIT,
			data,
			sizeof(data),
			100
	);

	if (status != HAL_OK)
	{
		return status;
	}

	// Combine high and low bytes into signed values
	int16_t ax = (int16_t)(((uint16_t)data[0] << 8) | data[1]);
	int16_t ay = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
	int16_t az = (int16_t)(((uint16_t)data[4] << 8) | data[5]);

	// Bytes 6-7 contain temperature so we skip to 8
	int16_t gx = (int16_t)(((uint16_t)data[8] << 8) | data[9]);
	int16_t gy = (int16_t)(((uint16_t)data[10] << 8) | data[11]);
	int16_t gz = (int16_t)(((uint16_t)data[12] << 8) | data[13]);

	// Convert raw measurements into physical units
	sample->accel_x_g = ax / ACCEL_SCALE;
	sample->accel_y_g = ay / ACCEL_SCALE;
	sample->accel_z_g = az / ACCEL_SCALE;

	sample->gyro_x_dps = gx / GYRO_SCALE;
	sample->gyro_y_dps = gy / GYRO_SCALE;
	sample->gyro_z_dps = gz / GYRO_SCALE;

	return HAL_OK;
}
