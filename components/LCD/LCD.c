#include <stdio.h>
#include "LCD.h"
#include "processing.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "rom/ets_sys.h"

i2c_master_dev_handle_t lcd_handle;

#define EN 0x04
#define RW 0x02
#define RS 0x01
#define BL 0x08

void lcd_i2c_init(void)
{
    i2c_master_bus_handle_t bus_handle = get_bus_handle();

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_ADDER,
        .scl_speed_hz = 100000
    };

    i2c_master_bus_add_device(
        bus_handle,
        &dev_config,
        &lcd_handle
    );
}

static void lcd_write(uint8_t data)
{
    i2c_master_transmit(
        lcd_handle,
        &data,
        1,
        pdMS_TO_TICKS(100)
    );
}

static void lcd_send_nibble(uint8_t nibble, uint8_t mode)
{
    uint8_t data = nibble | mode | BL;

    lcd_write(data | EN);
    ets_delay_us(1);

    lcd_write(data & ~EN);
    ets_delay_us(50);
}

static void lcd_send_byte(uint8_t value, uint8_t mode)
{
    lcd_send_nibble(value & 0xF0, mode);
    lcd_send_nibble((value << 4) & 0xF0, mode);
}

void lcd_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);

    if (cmd == 0x01 || cmd == 0x02)
    {
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void lcd_data(uint8_t data)
{
    lcd_send_byte(data, RS);
}

void lcd_init(void)
{
    vTaskDelay(pdMS_TO_TICKS(50));

    lcd_send_nibble(0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send_nibble(0x30, 0);
    ets_delay_us(150);

    lcd_send_nibble(0x30, 0);
    ets_delay_us(150);

    lcd_send_nibble(0x20, 0);
    ets_delay_us(150);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
}

void lcd_print(char *str)
{
    while (*str)
    {
        lcd_data(*str++);
    }
}


void task_lcd(void * param)
{
    display_struct_t disp_packet;

    char buffer[32];

    for(;;)
    {
        if(xQueueReceive(disp_queue, &disp_packet, portMAX_DELAY) == pdPASS)
        {
            snprintf(buffer,sizeof(buffer), "Roll: %ld",disp_packet.timeStamp);
            lcd_cmd(0x01);
            lcd_print(buffer);
        }
        vTaskDelay(pdMS_TO_TICKS(200)) ; 
    }
}