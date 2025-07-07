#include "zf_common_headfile.h"
//外环为速度环
PID_t Inner = {					//内环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 0,					    //比例项权重
	.Ki = 0,					    //积分项权重
	.Kd = 0,					    //微分项权重
	.OutMax = 0,				//输出限幅的最大值
	.OutMin = 0,				//输出限幅的最小值
	.Target = 0,           //速度环控速
};
//外环为差速环
PID_t Outer = {					//外环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 20,					  //比例项权重
	.Ki = 0,					    //积分项权重
	.Kd = 6,					    //微分项权重
	.OutMax = 30,				  //PID_Out输出限幅的最大值
	.OutMin = 0,			  	//PID_Out输出限幅的最小值
	.Target = 0,     		  //目标值(偏移量已经计算出的情况下)
	
	.Base_Speed = 15,			//基础速度
	.OutEndMax = 30,			//最终输出限幅的最大值
	.OutEndMin = 0,       //最终输出限幅的最小值
};
// **************************** 变量区域 ****************************
int32 encoder1;
int32 encoder2;
int32 en_speed1;
int32 en_speed2;
int32 en_location1;
int32 en_location2;

uint8_t Best_thrsod;    //八邻域得到的阈值
int8_t line_offset;		  //中值偏差
int8_t filtered_offset; //中值偏差(滤波后)
char Bias_finish_flag=1;//偏差计算结束标志位

// **************************** 代码区域 ****************************
int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init(); 	// 初始化默认 Debug UART
		system_delay_ms(300);
//各种外设初始化
  	ips200_init(IPS200_TYPE);		 //ips200类型SPI初始化 已在文件开头宏定义
    mt9v03x_init();              //总钻风初始化
	  key_init (10);               //按键初始化//按键扫描周期 以毫秒为单位
		menu_init();        				 //菜单初始化                
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
/*														    定时器3：编码器模式
																  定时器4：编码器模式
																  定时器5：电机PWM输出      */
		pit_ms_init(TIM2_PIT, 10); 	 //定时器中断2用于按键处理      10ms
		pit_ms_init(TIM6_PIT, 100);	 //定时器中断6用于编码器获取数据  100ms
		pit_ms_init(TIM7_PIT, 20);	   //定时器中断7用于电机PID控制  20ms
		interrupt_set_priority(TIM2_IRQn, 0);//设置中断2优先级
		interrupt_set_priority(TIM6_IRQn, 1);//设置中断6优先级
    interrupt_set_priority(TIM7_IRQn, 2);//设置中断7优先级
//显示菜单
		menu_display();
    while(1)
    {
//图像显示*******************************************
			if(mt9v03x_finish_flag){//MT9V03X_W为188 MT9V03X_H为120  	
					memcpy(image_copy, mt9v03x_image, MT9V03X_H*MT9V03X_W);
//          ips200_displayimage03x((const uint8 *)image_copy, MT9V03X_W, MT9V03X_H);//(0,0)显示原图
//					__disable_irq();// 进入临界区（禁止中断）
          ips200_show_string(0, 120,"After_Handle:");//(0,136)显示处理后的图像
					Best_thrsod=OtsuThreshold(image_copy,MT9V03X_W,MT9V03X_H);//大津法 动态阈值	
					ips200_show_gray_image(0, 136, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, Best_thrsod);
//原图			ips200_show_gray_image(0, 136, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
          mt9v03x_finish_flag = 0;
//					__enable_irq();// 退出临界区（恢复中断）
			}
			ips200_show_string(0, 256,"Speed1:");
			ips200_show_int(56, 256, en_speed1, 5);
			ips200_show_string(0, 272,"Location1:");
			ips200_show_int(80, 272, en_location1, 5);
			ips200_show_string(0, 288,"Speed2:");
			ips200_show_int(56, 288, en_speed2, 5);
			ips200_show_string(0, 304,"Location2:");
			ips200_show_int(80, 304, en_location2, 5);
			}
//*************************************************	
//图像处理********************************************
			line_offset=ScanLineCenter(image_copy,MT9V03X_W,MT9V03X_H);//中值
      filtered_offset = FilterOffset(line_offset);	//滤波
			Bias_finish_flag=1;
			
//*************************************************	
}
