#include "beep.h"
/**
  * @brief  蜂鸣器循环
  * @param  无
  * @retval 无
  */
void Buzzer_On_Count(uint8 Count){
    if(Count>0){
        gpio_set_level(BUZZER, GPIO_HIGH);
				system_delay_ms(200);
			  gpio_set_level(BUZZER, GPIO_LOW);
				system_delay_ms(200);
        Count--;
    }
}


