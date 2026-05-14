#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include <math.h>

#define MPU_ADDR 0x68 

#define SDA 6
#define SCL 7 
#define I2C_PORT I2C_NUM_0

#define PW_MGMT 0x6B 
#define AXL    0x3B 

static i2c_master_bus_handle_t bus_handle ; 
static i2c_master_dev_handle_t mpu ; 


static QueueHandle_t imu_queue ; 

typedef struct 
{
    int16_t ax , ay, az ; 
    int16_t gx,gy,gz ; 
    uint32_t timeStamp ; 
}imu_struct_t;


void i2c_init()
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


// ---------
// Tasks
// ---------


void task_mpu (void *param)
{

    imu_struct_t imu_packet ; 

    uint8_t data[14];
    uint32_t prev_ms = xTaskGetTickCount() * portTICK_PERIOD_MS ; 
    
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    float angle_z = 0.0f;
    
    for(;;)
    {
        mpu_read(AXL,data,14);

        imu_packet.ax = (int16_t)(data[0]<<8) | data[1] ; // raw accel
        imu_packet.ay = (int16_t)(data[2]<<8)| data[3];
        imu_packet.az = (int16_t)(data[4]<<8) | data[5] ;
        
        imu_packet.gx = (int16_t)(data[8]<<8) | data[9] ; // raw gyro
        imu_packet.gy = (int16_t)(data[10]<<8)| data[11];
        imu_packet.gz = (int16_t)(data[12]<<8) | data[13] ;
        
        imu_packet.timeStamp = xTaskGetTickCount() * portTICK_PERIOD_MS ; // current time
 
        xQueueSend(imu_queue , &imu_packet , portMAX_DELAY) ; 
        vTaskDelay(pdMS_TO_TICKS(10));
    }

}

void task_processing(void * param)
{
    imu_struct_t imu_packet ; 

    uint32_t prev_ms = 0 ; 

    float angle_x = 0 , angle_y = 0 , angle_z = 0 ; 

    for(;;)
    {
        if(xQueueReceive(imu_queue,&imu_packet,portMAX_DELAY) == pdPASS)
        {
            float dt = (imu_packet.timeStamp - prev_ms) / 1000.0f ; 
            prev_ms = imu_packet.timeStamp ; 

            if(dt <= 0) continue;

            float ax = imu_packet.ax / 16384.0f ;
            float ay = imu_packet.ay / 16384.0f ;
            float az = imu_packet.az / 16384.0f ;

            float gx = imu_packet.gx / 131.0f ; 
            float gy = imu_packet.gy / 131.0f ; 
            float gz = imu_packet.gz / 131.0f ; 


            // accel
            float roll_axl = atan2(ay ,az) * (180/3.141592653f) ; 
            float pitch_axl = atan2(-ax , sqrt((ay)*(ay) + (az)*(az)))*(180/3.141592653f);

            // gyro - deg
            angle_x +=  gx * (dt) ; 
            angle_y +=  gy * (dt); 
            angle_z +=  gz * (dt) ; 

            printf("%ld , %f , %f , %f , %f , %f , %f \n" , imu_packet.timeStamp , angle_x , angle_y , angle_z , roll_axl , pitch_axl , 0.0f) ; 
        }
    }

}


void app_main(void)
{
    i2c_init();
    mpu_init();

    imu_queue = xQueueCreate(10 , sizeof(imu_struct_t)); 

    if(imu_queue == NULL) 
    {
        printf("empty queue... Failed\n") ; 
        return ; 
    }

    xTaskCreate(
        task_mpu , 
        "task_mpu",
        5000,
        NULL,
        1,
        NULL
    );

    xTaskCreate(task_processing , "processing" , 5000 , NULL , 1 , NULL) ; 
}