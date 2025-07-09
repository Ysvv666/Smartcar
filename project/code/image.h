#ifndef _image_h_
#define _image_h_

#include "zf_common_headfile.h"
#define Image_Down 120

int my_abs(int a);
int OtsuThreshold(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height);
void binarizeImage(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height, int threshold);
uint8 get_start_point(unsigned char image[MT9V03X_H][MT9V03X_W], uint8 start_row);
void search_l_r(uint16 break_flag, uint8 image[MT9V03X_H][MT9V03X_W], uint16 *l_stastic, uint16 *r_stastic, 
                uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8* hightest);
void get_left(uint16 total_L);
void get_right(uint16 total_R);
void image_filter(uint8 image[MT9V03X_H][MT9V03X_W]);
void image_draw_rectan(uint8 image[MT9V03X_H][MT9V03X_W]);
void image_process(void);

//#define ROW 120
//#define COL 188

//#define LQOSTU_MIN02         0   //大津法计算阈值时的最小行数
//#define LQOSTU_MAX02         (ROW-1) //大津法计算阈值时的最大行数
//#define LQOSTU_THRERR        3     //动态，跳变沿高度取值
//#define LQOSTU_QIANGGUANG    200   //强光阈值，大于阈值的亮度认为是强光
//#define LQOSTU_QIANGNUM      2000   //强光总点数的阈值

//extern uint8 cam_static;//当赛道出界，或者计算的灰度阈值异常时使用
//extern uint8 cam_thr_mid,cam_thr_max,cam_thr_min;
//extern uint8 cam_thr,cam_thr0;//摄像头的阈值
//extern uint8_t HistoGram[256]; //在进行大津的计算阈值时用于统计赛道指定区域的各个阈值的个数数组
//extern int LQ_ary[256];

//extern uint8 cam_strong_light;//强光标志位
//extern uint8 cam_normal_light;//正常光标志位
//extern uint8 cam_wesk_light;  //弱光标志位

//uint8_t zzkLQGetOSTU0200(uint8 LQOSTU_MIN,uint8 LQOSTU_MAX);
//uint8_t zzkLQGetOSTU0200_col(uint8 LQOSTU_MIN,uint8 LQOSTU_MAX,uint8 lie_min,uint8 lie_max);

#endif
