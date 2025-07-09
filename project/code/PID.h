#ifndef _PID_H_
#define _PID_H_

#include "zf_common_headfile.h"

typedef struct {
	float Target;
	float Actual;
	float Out;
	
	float Kp;
	float Ki;
	float Kd;
	
	float Error0;
	float Error1;
	float ErrorInt;
	
	float OutMax;
	float OutMin;
	
//写了串级pid，以下变量就此告一段落
	float Base_Speed;

	float OutLeft;
	float OutRight;

	float OutEndMax;
	float OutEndMin;
} PID_t;

void PID_Update(PID_t *p);

#endif
