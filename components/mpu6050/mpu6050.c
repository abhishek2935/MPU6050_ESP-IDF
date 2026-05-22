#include "mpu6050.h"
#include "i2c_bus.h"

#include "freertos/FreeRTOS.h"

#define SDA 6
#define SCL 7 
#define I2C_PORT I2C_NUM_0


static i2c_master_dev_handle_t mpu;
void mpu6050_i2c_init()
{
    i2c_master_bus_handle_t bus_handle = get_bus_handle() ; 

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7 ,
        .device_address = MPU_ADDR,
        .scl_speed_hz = 400000  // can go to 400k 
    };

    i2c_master_bus_add_device(bus_handle,&dev_config,&mpu);
}

void mpu_write(uint8_t reg , uint8_t data)
{
    uint8_t buf[2] = {reg , data} ; 

    i2c_master_transmit(
        mpu,
        buf,
        sizeof(buf),
        pdMS_TO_TICKS(1000)
    );
}


void mpu_read(uint8_t reg , uint8_t *data , size_t len)
{
    i2c_master_transmit_receive(
        mpu,
        &reg,
        1,
        data,
        len,
        pdMS_TO_TICKS(1000)
    );
}


void mpu_init()
{
    mpu_write(PW_MGMT,0x00); // sensor wake up
}
