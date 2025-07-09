#include "zf_common_headfile.h"
// **************************** 变量区域 ****************************
//写了串级pid代码，差速大法的变量告一段落2025.7.9
PID_t Outer = {					//外环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 1, 	  			  //比例项权重
	.Ki = 0,					    //积分项权重
	.Kd = 1,					    //微分项权重
	.OutMax = 5,				  //PID_Out输出限幅的最大值
	.OutMin = 0,			  	//PID_Out输出限幅的最小值
	.Target = 188/2,     	//目标值
	
	.Base_Speed = 10,			//基础速度
	.OutEndMax = 15,			//最终输出限幅的最大值
	.OutEndMin = 0,       //最终输出限幅的最小值
};

//外环为差速环
//PID_t Outer = {					//外环PID结构体变量，定义的时候同时给部分成员赋初值
//	.Kp = 15, 					  //比例项权重
//	.Ki = 0,					    //积分项权重
//	.Kd = 10,					    //微分项权重
//	.OutMax = 1500,				  //PID_Out输出限幅的最大值
//	.OutMin = 0,			  	//PID_Out输出限幅的最小值
//	.Target = 188/2,     	//目标值
//};
//内环为速度环
PID_t Inner_Left = {					//内环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 10,					    //比例项权重
	.Ki = 0,					    //积分项权重
	.Kd = 8,					    //微分项权重
	.OutMax = 2500,				//输出限幅的最大值
	.OutMin = 0,				//输出限幅的最小值
};
PID_t Inner_Right = {					//内环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 10,					    //比例项权重
	.Ki = 0,					    //积分项权重
	.Kd = 8,					    //微分项权重
	.OutMax = 2500,				//输出限幅的最大值
	.OutMin = 0,				//输出限幅的最小值
};
// **************************** 代码区域 ****************************
int main(void)
{
//					__disable_irq();// 进入临界区（禁止中断）
//					__enable_irq(); // 退出临界区（恢复中断）
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init(); 	// 初始化默认 Debug UART
		system_delay_ms(300);
//各种外设初始化
  	ips200_init(IPS200_TYPE);		 //ips200类型SPI初始化 已在文件开头宏定义
    mt9v03x_init();              //总钻风初始化
	  key_init (10);               //按键初始化//按键扫描周期 以毫秒为单位
//按键初始化		
		gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);       // 初始化 KEY1 输入 默认高电平 上拉输入
		gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);       // 初始化 KEY2 输入 默认高电平 上拉输入
		gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);       // 初始化 KEY3 输入 默认高电平 上拉输入
		gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);       // 初始化 KEY4 输入 默认高电平 上拉输入
//电机引脚初始化
    gpio_init(DIR_L, GPO, GPIO_HIGH, GPO_PUSH_PULL);    // GPIO 初始化为输出 默认上拉输出高
    pwm_init(PWM_L, 17000, 0);                          // PWM 通道初始化频率 17KHz 占空比初始为 0
    gpio_init(DIR_R, GPO, GPIO_HIGH, GPO_PUSH_PULL);    // GPIO 初始化为输出 默认上拉输出高
    pwm_init(PWM_R, 17000, 0);                          // PWM 通道初始化频率 17KHz 占空比初始为 0
//编码器初始化
		encoder_quad_init(TIM3_ENCODER, TIM3_ENCODER_CH1_B4, TIM3_ENCODER_CH2_B5);//encoder1
		encoder_quad_init(TIM4_ENCODER, TIM4_ENCODER_CH1_B6, TIM4_ENCODER_CH2_B7);//encoder2
//定时中断初始化
/*														     定时器3：编码器模式
 *															   定时器4：编码器模式
 *															   定时器5：电机PWM输出
 */
		pit_ms_init(TIM2_PIT, 10); 	 //定时器中断2用于按键处理       10ms
		pit_ms_init(TIM6_PIT, 100);	 //定时器中断6用于编码器获取数据  100ms
		pit_ms_init(TIM7_PIT, 20);	 //定时器中断7用于电机PID控制    20ms
		interrupt_set_priority(TIM2_IRQn, 0);//设置中断2优先级
		interrupt_set_priority(TIM6_IRQn, 1);//设置中断6优先级
    interrupt_set_priority(TIM7_IRQn, 2);//设置中断7优先级
//显示菜单
    menu_display();
    while(1)
    {		
			if(mt9v03x_finish_flag){//MT9V03X_W为188 MT9V03X_H为120  	
					memcpy(image_copy, mt9v03x_image, MT9V03X_H*MT9V03X_W);
					image_process();
					mt9v03x_finish_flag = 0;
			}
		}
}
//(0,0)显示image_copy
//ips200_displayimage03x((const uint8 *)image_copy, MT9V03X_W, MT9V03X_H);
//(0,136)显示二值化后（阈值0，即原图）的image_copy
//ips200_show_gray_image(0, 136, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
//(0,136)显示二值化后（阈值为Best_thrsod）的image_copy
//Best_thrsod=OtsuThreshold(image_copy,MT9V03X_W,MT9V03X_H);//大津法 动态阈值	
//ips200_show_gray_image(0, 136, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, Best_thrsod);
