#ifndef MPU6500_H
#define MPU6500_H

#include "stm32h7xx_hal.h"

extern int16_t accel_raw[3];
extern int16_t gyro_raw[3];

// CS pin — check your CubeMX pinout, which GPIO port and pin is PA4?
#define MPU6500_CS_PORT    GPIOA
#define MPU6500_CS_PIN     GPIO_PIN_4

void MPU6500_Write(uint8_t reg, uint8_t data);
uint8_t MPU6500_Read(uint8_t reg);
uint8_t MPU6500_Init(void);
void MPU6500_ReadAll(void);


#endif
