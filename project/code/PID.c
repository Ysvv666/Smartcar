#include "PID.h"

void PID_Update(PID_t *p)
{
	p->Error1 = p->Error0;			
	p->Error0 = p->Target - p->Actual;	
	
//��ֹ��ʹ��Ki���ҵ���Ϊ0�����ɶ������Ӱ��	**************
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
	
//����޷�***************************************
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}	
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}	
//*********************************************
	
//���ٴ�***************************************
	p->OutLeft=25+(p->Out);
	p->OutRight=25-(p->Out);
//*********************************************
	
//����޷�***************************************
	if (p->OutLeft >  100){p->OutLeft  = 100;}
	if (p->OutLeft < -100){p->OutLeft  =-100;}
	
	if (p->OutRight>  100){p->OutRight = 100;}
	if (p->OutRight< -100){p->OutRight =-100;}
//*********************************************
}












