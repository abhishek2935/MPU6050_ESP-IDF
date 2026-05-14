#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"

#define MPU_ADDR 0x68 
#define PW_MGMT  0x6B 
#define AXL      0x3B 

typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    uint32_t timeStamp;
} imu_struct_t;


void mpu6050_i2c_init(void);
void mpu_init(void);

void mpu_write(uint8_t reg, uint8_t data);
void mpu_read(uint8_t reg, uint8_t *data, size_t len);