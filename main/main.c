#include <stdio.h>

#include "mpu6050.h"
#include "processing.h"


void app_main(void)
{
    mpu6050_i2c_init();
    mpu_init();
    
    processing_init() ; 

}