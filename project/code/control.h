#ifndef _control_h_
#define _control_h_

#include "zf_common_headfile.h"

extern uint8 Motor_Protection_flag;

void Motor_Left_PWM(int16_t duty);
void Motor_Right_PWM(int16_t duty);
void Motor_Protection(void);
float HandleSpeed(uint8_t Zhongzhi);
float HandleKp(uint8_t error);

#endif
