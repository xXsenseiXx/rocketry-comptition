#include "mpu6500.h"
extern SPI_HandleTypeDef hspi1;


void MPU6500_Write(uint8_t reg, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = reg & 0x7F;  // bit7 = 0 → write
    buf[1] = data;

    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_RESET); // CS low
    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_SET);   // CS high
}

uint8_t MPU6500_Read(uint8_t reg)
{
    uint8_t buf_tx[2];
    uint8_t buf_rx[2] = {0};

    buf_tx[0] = reg | 0x80; // Register address with Read bit (MSB) set to 1
    buf_tx[1] = 0x00;       // Dummy byte to keep the clock running

    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_RESET); // CS low

    // Transmit 2 bytes and receive 2 bytes
    HAL_SPI_TransmitReceive(&hspi1, buf_tx, buf_rx, 2, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_SET);   // CS high

    // buf_rx[0] is garbage received while sending the address
    // buf_rx[1] is the actual data we requested
    return buf_rx[1];
}
uint8_t MPU6500_Init(void)
{
    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_SET);
    HAL_Delay(100);

    // Wake up sensor and switch to SPI mode
    MPU6500_Write(0x6B, 0x00);  // PWR_MGMT_1: wake up
    HAL_Delay(10);
    MPU6500_Write(0x6A, 0x10);  // USER_CTRL: disable I2C, enable SPI
    HAL_Delay(10);

    uint8_t whoami = MPU6500_Read(0x75);
    if (whoami != 0x70)
        return 0;
    return 1;
}

void MPU6500_ReadAll(void)
{
    // We need 15 bytes total: 1 for the address, 14 for the dummy bytes to keep the clock running
    uint8_t buf_tx[15] = {0};
    uint8_t buf_rx[15] = {0};

    buf_tx[0] = 0x3B | 0x80; // Start at ACCEL_XOUT_H (0x3B), Read Bit set

    // Burst Read: 1 Address Byte + 14 Data Bytes
    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, buf_tx, buf_rx, 15, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_SET);

    // Now we combine the High and Low bytes into 16-bit integers
    // buf_rx[0] is garbage (received while sending the address)

    // Accelerometer (Registers 0x3B to 0x40)
    accel_raw[0] = (int16_t)((buf_rx[1] << 8) | buf_rx[2]);  // X axis
    accel_raw[1] = (int16_t)((buf_rx[3] << 8) | buf_rx[4]);  // Y axis
    accel_raw[2] = (int16_t)((buf_rx[5] << 8) | buf_rx[6]);  // Z axis

    // Temperature (Registers 0x41 to 0x42)
    // int16_t temp_raw = (int16_t)((buf_rx[7] << 8) | buf_rx[8]);

    // Gyroscope (Registers 0x43 to 0x48)
    gyro_raw[0] = (int16_t)((buf_rx[9]  << 8) | buf_rx[10]); // X axis
    gyro_raw[1] = (int16_t)((buf_rx[11] << 8) | buf_rx[12]); // Y axis
    gyro_raw[2] = (int16_t)((buf_rx[13] << 8) | buf_rx[14]); // Z axis
}

