#include "PID.h"	

/**                    
  * @brief 位置式PID
  * @param PID_t 结构体
  * @retval 无
  */
void PID_Position_Update(PID_t *p)
{
		p->Error1 = p->Error0;			
		p->Error0 = p->Target - p->Actual;	
//防止调节Ki清零后的干扰**************
		if (p->Ki != 0)					
		{
				p->ErrorInt += p->Error0;	
					if (p->ErrorInt> p->I_Limit) {p->ErrorInt= p->I_Limit;}	
					else if (p->ErrorInt < -(p->I_Limit) ) {p->ErrorInt= -(p->I_Limit);}	
		}
		else			
		{
			p->ErrorInt = 0;		
		}
//********************************************
//pid输出计算************************************
//	p->Out1 = p->Kp * p->Error0
//	       +  p->Ki * p->ErrorInt
//	       +  p->Kd *(p->Error0 - p->Error1);//这一次pid输出
//	p->Out=(p->Out1)*0.8+(p->Out0)*0.2;        //互补滤波计算pid最终输出值
	p->Out  = p->Kp * p->Error0
	       +  p->Ki * p->ErrorInt
	       +  p->Kd *(p->Error0 - p->Error1);//这一次pid输出
//输出限幅***************************************
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
//*********************************************
//	p->Out0=p->Out;//记住上一次pid输出
}	


/**                    
  * @brief 平滑三次PID
  * @param PID_t 结构体
  * @retval 无
  */
/**
  *直道过于抖动或将要“入”弯姿态不好调一次项Kpa （=13）
  *弯道不平滑、力度不够、调三次项Kpb（=0.02）
  *Kd微分乘上的误差改进为角速度差Kd*gyro（=12.5）
  *若转弯甩尾、打滑，可以用三次项gyro
  *gyro为角速度，定时间内读取陀螺仪z轴数据再清零即可得到！
  */
void PID_Three_Update(PID_t *p)
{
		p->Error1 = p->Error0;			
		p->Error0 = p->Target - p->Actual;	
//pid输出计算************************************
		p->Out  = p->Kp5 * (p->Error0*p->Error0*p->Error0*p->Error0*p->Error0)
					 +  p->Kpa * (p->Error0*p->Error0*p->Error0)
					 +  p->Kpb *  p->Error0
					 +  p->Kd	 * (-(mpu6050_gyro_z+8));//这一次pid输出
//输出限幅***************************************
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
}	
/**                    
  * @brief 平滑二次PID（类似于模糊式和分段式pid）
  * @param PID_t 结构体
  * @retval 无
****************Kp随着误差曲线变化而变化*************
  */
void PID_KpTwo_Update(PID_t *p)
{
		p->Error1 = p->Error0;			
		p->Error0 = p->Target - p->Actual;	
//防止调节Ki清零后的干扰****************************
		if (p->Ki != 0)					
		{
				p->ErrorInt += p->Error0;	
					if (p->ErrorInt> p->I_Limit) {p->ErrorInt= p->I_Limit;}	
					else if (p->ErrorInt < -(p->I_Limit) ) {p->ErrorInt= -(p->I_Limit);}	
		}
		else			
		{
			p->ErrorInt = 0;		
		}
//********************************************
//pid输出计算************************************
	p->Kp=HandleKp(ZhongZhi);//Kp曲线变化，需要更改时更改函数a，b，c
	p->Out  = p->Kp * p->Error0
	       +  p->Ki * p->ErrorInt
	       +  p->Kd *(p->Error0 - p->Error1);//这一次pid输出
//输出限幅***************************************
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
}	
/**                    
  * @brief 增量式PID
  * @param PID_t 结构体
  * @retval 无
  */
void PID_Increase_Update(PID_t *p)
{
		p->Error2 = p->Error1;           // 保存上上次误差
    p->Error1 = p->Error0;           // 保存上次误差
    p->Error0 = p->Target - p->Actual;  // 计算当前误差	//防止调节Ki清零后的干扰**************
		
		if (p->Ki != 0)					
		{
				p->ErrorInt += p->Error0;	
					//增量式pid 积分项限幅*****************************
					if (p->ErrorInt> p->I_Limit) {p->ErrorInt= p->I_Limit;}	
					else if (p->ErrorInt < -(p->I_Limit) ) {p->ErrorInt= -(p->I_Limit);}	
		}
		else			
		{
			p->ErrorInt = 0;		
		}
	//pid输出计算************************************
		 float increment= p->Kp * (p->Error0 - p->Error1) + 
                      p->Ki * p->Error0 + 
                      p->Kd * (p->Error0 - 2 * p->Error1 + p->Error2);
    p->Out = p->Out0 + increment;
    p->Out = p->Out * 0.8 + p->Out0 * 0.2;  // 低通滤波	//输出限幅***************************************
		if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
		if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
	//*********************************************
		p->Out0=p->Out;//记住上一次pid输出		
}

