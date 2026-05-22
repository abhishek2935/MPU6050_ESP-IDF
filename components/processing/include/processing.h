#pragma once

#include "mpu6050.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>

typedef struct {
    uint32_t timeStamp;
    float roll_axl;
    float pitch_axl; 
} display_struct_t;

extern QueueHandle_t disp_queue;

void processing_init() ; 