#include "zf_common_headfile.h"

PID_t Inner = {					//内环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 0,					    //比例项权重
	.Ki = 0,					    //积分项权重
	.Kd = 0,					    //微分项权重
	.OutMax = 100,				//输出限幅的最大值
	.OutMin = -100,				//输出限幅的最小值
	.Target=30,           //速度环控速
};

PID_t Outer = {					//外环PID结构体变量，定义的时候同时给部分成员赋初值
	.Kp = 0.5,					  //比例项权重
	.Ki = 0.001,					//积分项权重
	.Kd = 6,					    //微分项权重
	.OutMax = 50,				  //输出限幅的最大值
	.OutMin = -50,				//输出限幅的最小值
	.Target=64,           //差速环
};
// **************************** 变量区域 ****************************
int32 encoder1;
int32 encoder2;
uint8_t Best_thrsod;

// **************************** 代码区域 ****************************
int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);                                              // 初始化芯片时钟 工作频率为 120MHz
    debug_init(); 	// 初始化默认 Debug UART
		system_delay_ms(300);
	
    // 此处编写用户代码 例如外设初始化代码等
    mt9v03x_init();              //总钻风初始化
    ips200_init(IPS200_TYPE);		 //ips初始化 已在文件开头宏定义
	  key_init (10);               //按键初始化//按键扫描周期 以毫秒为单位
		menu_init();        				 //菜单初始化                
		
		gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY1 输入 默认高电平 上拉输入
		gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY2 输入 默认高电平 上拉输入
		gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY3 输入 默认高电平 上拉输入
		gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);                               // 初始化 KEY4 输入 默认高电平 上拉输入
	
		encoder_quad_init(TIM3_ENCODER, TIM3_ENCODER_CH1_B4, TIM3_ENCODER_CH2_B5);
		encoder_dir_init(TIM4_ENCODER, TIM4_ENCODER_CH1_B6, TIM4_ENCODER_CH2_B7);
    
		pit_ms_init(TIM2_PIT, 10); 	 //中断2用于按键处理 10ms
		pit_ms_init(TIM5_PIT, 100);	 //中断5用于编码器
		interrupt_set_priority(TIM2_IRQn, 0);//设置中断2优先级
		interrupt_set_priority(TIM5_IRQn, 1);//设置中断2优先级

		// 此处编写用户代码 例如外设初始化代码等
		ips200_clear();
		menu_display();
    while(1)
    {
//图像显示*******************************************
        if(mt9v03x_finish_flag)
        {	
					//原图像 copy //188 120
					memcpy(image_copy, mt9v03x_image, MT9V03X_H*MT9V03X_W);
//          ips200_displayimage03x((const uint8 *)image_copy, MT9V03X_W, MT9V03X_H);
					//二值化后
          ips200_show_string(0, 120,"After_Handle:");
					Best_thrsod=OtsuThreshold(image_copy,MT9V03X_W,MT9V03X_H);//大津法 动态阈值	
					ips200_show_gray_image(0, 136, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, Best_thrsod);
          mt9v03x_finish_flag = 0;
        }
//*************************************************
			
//按键处理*******************************************
//TIM2_IRQHandler
//*************************************************
				
    }
}
