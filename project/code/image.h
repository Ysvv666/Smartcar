#ifndef _image_h_
#define _image_h_

#include "zf_common_headfile.h"

#define USE_NUM 264
#define Image_Delete  32        //大概两个字母像素长度
// 外部变量声明（全局变量需在头文件中声明，方便其他文件使用）

extern uint8_t  start_point_l[2];
extern uint8_t  start_point_r[2];
extern uint8_t  l_found;
extern uint8_t  r_found;

extern uint16_t points_l[USE_NUM][2];
extern uint16_t points_r[USE_NUM][2];
extern uint16_t dir_l[USE_NUM];
extern uint16_t dir_r[USE_NUM];
extern uint8_t  l_border[MT9V03X_H];
extern uint8_t  r_border[MT9V03X_H];
extern uint8_t  center_line[MT9V03X_H];
extern uint16_t data_statics_l;
extern uint16_t data_statics_r;
extern uint8_t  hightest;

extern uint8_t  QuanZhong[MT9V03X_H];
extern uint8_t  Best_thrsod;
extern uint8_t  ZhongZhi0;
extern uint8_t  ZhongZhi1;
extern uint8_t  ZhongZhi;
extern uint8_t  Image_Down; //图像下移距离
extern uint16_t Sum_QuanZhong;
extern uint16_t Sum_ZhongZhi;

extern uint8_t  YueJie_flag;
extern uint8_t  Buzzer_ChuJie_flag;
extern uint8_t  Buzzer_Stop_flag;
extern char     pid_flag;



int my_abs(int a);
int OtsuThreshold(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height);
void binarizeImage(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height, int threshold);
uint8 get_start_point(unsigned char image[MT9V03X_H][MT9V03X_W], uint8 start_row);
void search_l_r(uint16 break_flag, uint8 image[MT9V03X_H][MT9V03X_W], uint16 *l_stastic, uint16 *r_stastic, 
                uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8* hightest);
void get_left(uint16 total_L);
void get_right(uint16 total_R);

void ChuJie_Test(uint8 image[MT9V03X_H][MT9V03X_W]);
void Stop_Test(uint8 image[MT9V03X_H][MT9V03X_W]);
uint8 Judge_Cross(void);

float Slope_Calculate(uint8 begin, uint8 end, uint8 *border);
bool calculate_s_i(uint8 start, uint8 end, uint8 *border, float *slope_rate, float *intercept);
uint8 cross_fill(uint8 image[MT9V03X_H][MT9V03X_W], uint8 *l_border, uint8 *r_border,
                 uint16 total_num_l, uint16 total_num_r, uint16 *dir_l, uint16 *dir_r,
                 uint16(*points_l)[2], uint16(*points_r)[2]);

void image_filter(uint8 image[MT9V03X_H][MT9V03X_W]);
void image_draw_rectan(uint8 image[MT9V03X_H][MT9V03X_W]);
void image_process(void);
#endif
