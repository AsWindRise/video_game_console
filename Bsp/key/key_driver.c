#include "key_driver.h"

uint8_t key_val, key_up, key_down, key_old,key_down_data,key_up_data;

uint8_t key_read()
{
    uint8_t key_value = 0;

    // 1. SW1 (PE0) -> ±àºÅ 1
    if(HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) 
        key_value = 1; 

    // 2. SW2 (PE1) -> ±àºÅ 2
    if(HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) 
        key_value = 2; 

    // 3. SW3 (PE2) -> ±àºÅ 3
    if(HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_RESET) 
        key_value = 3; 

    // 4. SW4 (PE3) -> ±àºÅ 4
    if(HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_RESET) 
        key_value = 4; 

    // 5. SK_Pin (PE4) -> ±àºÅ 5
    if(HAL_GPIO_ReadPin(SK_GPIO_Port, SK_Pin) == GPIO_PIN_RESET) 
        key_value = 5; 

        
    return key_value;
}

void key_task()
{
	key_val = key_read();
	key_down = key_val & (key_old ^ key_val);
	key_up = ~key_val & (key_old ^ key_val);
	key_old = key_val;
	
	if(key_down != 0)
	{
		key_down_data = key_down;
	}
	
	if(key_up != 0)
	{
		key_up_data = key_up;
	}
}
