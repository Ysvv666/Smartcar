#include "control.h"
void Motor_Left_PWM(int8_t duty){
		if(0 <= duty)                                                           // 正转
		{
				gpio_set_level(DIR_L, GPIO_HIGH);                                   // DIR输出高电平
				pwm_set_duty(PWM_L, duty * (PWM_DUTY_MAX / 100));                   // 计算占空比
		}
		else                                                                    // 反转
		{
				gpio_set_level(DIR_L, GPIO_LOW);                                    // DIR输出低电平
				pwm_set_duty(PWM_L, (-duty) * (PWM_DUTY_MAX / 100));                // 计算占空比
		}
}
void Motor_Right_PWM(int8_t duty){
		if(0 <= duty)                                                           // 正转
		{
				gpio_set_level(DIR_R, GPIO_HIGH);                                   // DIR输出高电平
				pwm_set_duty(PWM_R, duty * (PWM_DUTY_MAX / 100));                   // 计算占空比
		}
		else                                                                    // 反转
		{
				gpio_set_level(DIR_R, GPIO_LOW);                                    // DIR输出低电平
				pwm_set_duty(PWM_R, (-duty) * (PWM_DUTY_MAX / 100));                // 计算占空比
		}
}
