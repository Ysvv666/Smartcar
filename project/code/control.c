#include "control.h"
/**                    
  * @brief 左电机PWM输出
	* @param duty 占空比（0~10000）
  * @retval 无
  */
void Motor_Right_PWM(int16_t duty){


		if(0 <= duty)                                                           // 正转
		{
				gpio_set_level(DIR_R, GPIO_LOW);                                   // DIR输出高电平
				pwm_set_duty(PWM_R, duty );                   // 计算占空比
		}
		else                                                                    // 反转
		{
				gpio_set_level(DIR_R, GPIO_HIGH);                                    // DIR输出低电平
				pwm_set_duty(PWM_R, (-duty) );                // 计算占空比
		}
}
/**                    
  * @brief 右电机PWM输出
	* @param duty 占空比（0~10000）
  * @retval 无
  */
	void Motor_Left_PWM(int16_t duty){

		if(0 <= duty)                                                           // 正转
		{
				gpio_set_level(DIR_L, GPIO_LOW);                                  
				pwm_set_duty(PWM_L, duty  );                   // 计算占空比
		}
		else                                                                    // 反转
		{
				gpio_set_level(DIR_L, GPIO_HIGH);                                    
				pwm_set_duty(PWM_L, (-duty) );                // 计算占空比
		}
}

/**
  * @brief  电机过快保护
  * @param  无
  * @retval 无
  */
extern PID_t Turn_t;
uint8 Motor_Protection_flag=0;
void Motor_Protection(void)
{
    if(my_abs(en_speed1-en_speed2)>8000||
		   en_speed1>8000 || en_speed2>8000){
				 if(Motor_Protection_flag==0){
		//  			cargo=0;到时候再用
					 	 //清除按键状态
						 key_clear_all_state();
						 Buzzer_On_Count(1);
						 //重置编码器数据
						 encoder1=0;
						 encoder2=0;
						 en_speed1 = 0;
						 en_speed2 = 0;
						 en_location1 = 0; 
						 en_location2 = 0;
					   Motor_Protection_flag=1;
				}
	  }     
}
float HandleSpeed(uint8_t Zhongzhi) {
    int error = abs(Zhongzhi - 94);
    
    // 二次函数系数，满足 f(0)=1600, f(7)=950, 开口向下
    float a = -13.2653f;  // 二次项系数
    float b = 13.7755f;   // 一次项系数
    float c = 1600.0f;    // 常数项
    
    // 计算二次函数: f(error) = -13.2653*error² + 13.7755*error + 1600
    float speed = a * error * error + b * error + c;
    
    // 限制最小速度为400
    if (speed < 400) speed = 400.0f;
    
    return speed;
}
float My_HandleSpeed(uint8_t Zhongzhi) {
    int error = abs(Zhongzhi - 94);
		if(error<10)return 1400.0f;
		else if(error>10 && error<20)return 1300.0f;
		else if(error>20 && error<30)return 1200.0f;
		else if(error>30 && error<40)return 1100.0f;
		else if(error>40 && error<50)return 1000.0f;
		else if(error>50 )return 950.0f;
		return 950.0f;
}
float HandleKp(uint8_t Zhongzhi) {
    int error = abs(Zhongzhi - 94);
    // 二次函数系数计算，满足 f(0)=13, f(17)=46, 开口向上
    float a = 0.1145f;    // 二次项系数
    float b = 0.75f;      // 一次项系数
    float c = 13.0f;      // 常数项
    
    // 计算二次函数结果
    float kp = a * error * error + b * error + c;
    
    // 限制最小值为0（实际应用中可能需要根据需求调整）
    if (kp < 0) kp = 0.0f;
    
    return kp;
}