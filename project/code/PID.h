#ifndef _PID_H_
#define _PID_H_

#include "zf_common_headfile.h"

typedef struct {
	float Target;
	float Actual;
	float Out;
	float Out0;
	float Out1;
	
	float Kp;
	float Kpa;
	float Kpb;
	float Ki;
	float Kd;
	
	float Error0;
	float Error1;
	float Error2;
	float ErrorInt;
	
	float OutMax;
	float OutMin;
	
	float I_Limit;
////写了串级pid，以下变量就此告一段落
//	float Base_Speed;

//	float OutLeft;
//	float OutRight;

//	float OutEndMax;
//	float OutEndMin;
} PID_t;

void PID_Position_Update(PID_t *p);
void PID_Increase_Update(PID_t *p);
void PID_Three_Update(PID_t *p);
void PID_Two_Update(PID_t *p);

#endif
