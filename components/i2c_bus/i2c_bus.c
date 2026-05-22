#include <stdio.h>
#include "i2c_bus.h"

#define SDA 6 
#define SCL 7 
#define I2C_PORT I2C_NUM_0

i2c_master_bus_handle_t bus_handle ; 

void i2c_bus_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = SDA , 
        .scl_io_num = SCL , 
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7 , 
        .flags.enable_internal_pullup = true 
    };

    i2c_new_master_bus(&bus_config , &bus_handle);
}

i2c_master_bus_handle_t get_bus_handle(void)
{
    return bus_handle ; 
}