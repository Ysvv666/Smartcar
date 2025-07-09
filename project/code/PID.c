#include "PID.h"

void PID_Update(PID_t *p)
{
	p->Error1 = p->Error0;			
	p->Error0 = p->Target - p->Actual;	
	
//防止调节Ki清零后的干扰**************
	if (p->Ki != 0)					
	{
		p->ErrorInt += p->Error0;	
	}
	else			
	{
		p->ErrorInt = 0;		
	}
//********************************************
	
	p->Out = p->Kp * p->Error0
	       + p->Ki * p->ErrorInt
	       + p->Kd *(p->Error0 - p->Error1);
	
//输出限幅***************************************
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
//*********************************************
	
	
//写了串级pid代码，差速大法就此告一段落	
//差速大法***************************************
	p->OutLeft =p->Base_Speed-(p->Out);
	p->OutRight=p->Base_Speed+(p->Out)+1;//由于右轮电机会慢一点
//*********************************************
	
//最终输出限幅***************************************
	if (p->OutLeft >p->OutEndMax){p->OutLeft  =p->OutEndMax;}
	if (p->OutLeft <p->OutEndMin){p->OutLeft  =p->OutEndMin;}
	
	if (p->OutRight>p->OutEndMax){p->OutRight =p->OutEndMax;}
	if (p->OutRight<p->OutEndMin){p->OutRight =p->OutEndMin;}
//*********************************************
}












