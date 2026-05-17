#include <stdio.h>
#include "processing.h"


static QueueHandle_t imu_queue ; 

// ---------
// Tasks
// ---------

void task_mpu (void *param)
{

    imu_struct_t imu_packet ; 

    uint8_t data[14];
    TickType_t lastTick = xTaskGetTickCount() ; 

    
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
        
        vTaskDelayUntil(&lastTick , pdMS_TO_TICKS(10)) ; 
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

            printf(">Time:%ld, X:%f, Y:%f , Z:%f , Roll:%f , Pitch:%f, zero:%f \r\n" , imu_packet.timeStamp , angle_x , angle_y , angle_z , roll_axl , pitch_axl , 0.0f) ; 
        }
    }

}

void processing_init()
{
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