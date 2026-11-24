/*
 * mpu6050.h
 *
 *  Created on: Nov 24, 2026
 *      Author: ZachNew28
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "stm32f4xx_hal.h"

typedef struct
{
	float accel_x_g;
	float accel_y_g;
	float accel_z_g;

	float gyro_x_dps;
	float gyro_y_dps;
	float gyro_z_dps;
} MPU6050_Sample;

HAL_StatusTypeDef MPU6050_Init(I2C_HandleTypeDef *i2c);

HAL_StatusTypeDef MPU6050_Read(
		I2C_HandleTypeDef *i2c,
		MPU6050_Sample *sample
);


#endif /* INC_MPU6050_H_ */
