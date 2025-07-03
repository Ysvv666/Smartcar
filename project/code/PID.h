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
	
	float OutLeft;
	float OutRight;
} PID_t;

void PID_Update(PID_t *p);

#endif
