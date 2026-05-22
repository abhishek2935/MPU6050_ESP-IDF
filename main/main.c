#include <stdio.h>

#include "mpu6050.h"
#include "processing.h"
#include "LCD.h"

#include "learnTest.h"

#include "driver/i2c_master.h"


void app_main(void)
{
    i2c_bus_init() ; 

    mpu6050_i2c_init();
    mpu_init();
    processing_init() ;  
    
    lcd_i2c_init() ; 
    lcd_init() ; 

    lcd_cmd(0x80) ; 

}