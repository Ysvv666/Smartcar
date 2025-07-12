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
					//增量式pid 积分项限幅*****************************
					if (p->ErrorInt> p->I_Limit) {p->ErrorInt= p->I_Limit;}	
					else if (p->ErrorInt < -(p->I_Limit) ) {p->ErrorInt= -(p->I_Limit);}	
		}
		else			
		{
			p->ErrorInt = 0;		
		}
//********************************************
//pid输出计算************************************
	p->Out1 = p->Kp * p->Error0
	       +  p->Ki * p->ErrorInt
	       +  p->Kd *(p->Error0 - p->Error1);//这一次pid输出
	p->Out=(p->Out1)*0.8+(p->Out0)*0.2;        //互补滤波计算pid最终输出值
//输出限幅***************************************
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
//*********************************************
	p->Out0=p->Out;//记住上一次pid输出
//*********************************************
////写了串级pid代码，差速大法就此告一段落	
////差速大法***************************************
//	p->OutLeft =p->Base_Speed-(p->Out);
//	p->OutRight=p->Base_Speed+(p->Out)+1;//由于右轮电机会慢一点
////*********************************************
//	
////最终输出限幅***************************************
//	if (p->OutLeft >p->OutEndMax){p->OutLeft  =p->OutEndMax;}
//	if (p->OutLeft <p->OutEndMin){p->OutLeft  =p->OutEndMin;}
//	
//	if (p->OutRight>p->OutEndMax){p->OutRight =p->OutEndMax;}
//	if (p->OutRight<p->OutEndMin){p->OutRight =p->OutEndMin;}
////*********************************************
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
	//********************************************
		
	//********************************************	
		
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
//	//写了串级pid代码，差速大法就此告一段落	
//	//差速大法***************************************
//		p->OutLeft =p->Base_Speed-(p->Out);
//		p->OutRight=p->Base_Speed+(p->Out)+1;//由于右轮电机会慢一点
//	//*********************************************
//		
//	//最终输出限幅***************************************
//		if (p->OutLeft >p->OutEndMax){p->OutLeft  =p->OutEndMax;}
//		if (p->OutLeft <p->OutEndMin){p->OutLeft  =p->OutEndMin;}
//		
//		if (p->OutRight>p->OutEndMax){p->OutRight =p->OutEndMax;}
//		if (p->OutRight<p->OutEndMin){p->OutRight =p->OutEndMin;}
//	//*********************************************
		
}

