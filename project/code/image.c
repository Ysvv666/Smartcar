#include "image.h"
//绝对值函数***********************************************
int my_abs(int8 a){
	int8 x =0;
	if(a>=0){x=a;}
	else if(a<0){x=-a;}
	return x;
}
//大津法计算阈值***********************************************
//传入二维图像数组*********************************************
#define GRAY_SCALE 256
#define ROI_Y_START  0                // ROI起始行（感兴趣区域）
#define ROI_Y_END 188                 // ROI结束行
#define MIN_GRAY_THRESHOLD 20         // 最小灰度阈值
#define MAX_GRAY_THRESHOLD 230        // 最大灰度阈值
int OtsuThreshold(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height)
{																			
		int hist[GRAY_SCALE] = {0};
		double prob[GRAY_SCALE] = {0};					
		double cum_prob[GRAY_SCALE] = {0};			
		double cum_mean[GRAY_SCALE] = {0};			
		int threshold = 0;			
		double max_var = 0;				
		int pixel_count = 0;			

		// 计算ROI区域的灰度直方图（只处理图像感兴趣部分）
		for (int y = ROI_Y_START; y < ROI_Y_END; y++) {
				for (int x = 0; x < width; x++) {
						hist[image[y][x]]++;
						pixel_count++;
				}
		}
	
		// 计算概率分布
		for (int i = 0; i < GRAY_SCALE; i++) {
				prob[i] = (double)hist[i] / pixel_count;
		}
		// 计算累计概率和累计均值
		cum_prob[0] = prob[0];
		cum_mean[0] = 0 * prob[0];
		for (int i = 1; i < GRAY_SCALE; i++) {
				cum_prob[i] = cum_prob[i-1] + prob[i];
				cum_mean[i] = cum_mean[i-1] + i * prob[i];
		}

		// 计算全局均值
		double global_mean = cum_mean[GRAY_SCALE-1];

		// 寻找最大类间方差对应的阈值
		for (int k = MIN_GRAY_THRESHOLD; k < MAX_GRAY_THRESHOLD; k++) {
				if (cum_prob[k] == 0 || cum_prob[k] == 1) continue;
				
				double variance = (global_mean * cum_prob[k] - cum_mean[k]) * 
												 (global_mean * cum_prob[k] - cum_mean[k]) / 
												 (cum_prob[k] * (1 - cum_prob[k]));
				
				if (variance > max_var) {
						max_var = variance;
						threshold = k;
				}
		}
		return threshold;
}
//图像二值化（binarize）函数***********************************************
void binarizeImage(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height, int threshold) {
		for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
						image[i][j] = (image[i][j] > threshold) ? 255 : 0;
				}
		}
}
/**
 * 在二值图像中寻找左右边界的起始点 寻找两个边界的边界点作为八邻域循环的起始点
 * @param start_row 搜索起始行
 * @return 1表示成功找到左右起点，0表示未找到或者只找到一个起点
 */
uint8 start_point_l[2]={0,0};//左边起点的x，y值
uint8 start_point_r[2]={0,0};//右边起点的x，y值
uint8 l_found=0;//找到分界标志位
uint8 r_found=0;

uint8 get_start_point(unsigned char image[MT9V03X_H][MT9V03X_W],uint8 start_row)
{
		// 清零起点坐标
		start_point_l[0] = 0; // x
		start_point_l[1] = 0; // y
		start_point_r[0] = 0; // x
		start_point_r[1] = 0; // y
		// 清零找到标志位
		l_found = 0;
		r_found = 0;

		// 从中间往左边搜索左边界起点
		for (int i = 188 / 2; i > 0; i--) {
				start_point_l[0]=i;         //x
				start_point_l[1]=start_row; //y
		// 找到黑白边界：白色像素左侧是黑色像素
				if(image[start_row][i]==255 && image[start_row][i-1]==0){
						l_found = 1;
						break;
				}
		}

		// 从中间往右边搜索右边界起点
		for (int i = 188 / 2; i < 188;i++){
				// 避免数组越界        
				start_point_r[0]=i;        //x
				start_point_r[1]=start_row;//y
				// 找到黑白边界：白色像素右侧是黑色像素
				if(image[start_row][i]==255 && image[start_row][i+1]==0){
						r_found=1;
						break;
				}
		}
		return(l_found && r_found)? 1:0;
}
/**
 * 基于八邻域搜索的左右边界跟踪算法
 * @param break_flag 最大迭代次数
 * @param image 二值图像数据（二维数组）
 * @param l_stastic 左边界点统计计数器
 * @param r_stastic 右边界点统计计数器
 * @param l_start_x 左边界起点x坐标
 * @param l_start_y 左边界起点y坐标
 * @param r_start_x 右边界起点x坐标
 * @param r_start_y 右边界起点y坐标
 * @param hightest 跟踪到的最高点y坐标
 */
// 定义找点的最大数组成员个数
#define USE_NUM (MT9V03X_H * 3)
// 存放点的x，y坐标
uint16 points_l[USE_NUM][2] = {{0,0}};//左线坐标
uint16 points_r[USE_NUM][2] = {{0,0}};//右线坐标
uint16 dir_r[USE_NUM] = {0};//存储右边生长方向
uint16 dir_l[USE_NUM] = {0};//存储左边生长方向
uint16 data_statics_l = 0;//统计左边找到点的个数
uint16 data_statics_r = 0;//统计右边找到点的个数
uint8 hightest = 0;//最高点

void search_l_r(uint16 break_flag, uint8 image[MT9V03X_H][MT9V03X_W], uint16 *l_stastic, uint16 *r_stastic, 
								uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8* hightest){
// {{-1,-1},{ 0,-1},{ 1,-1},
//  {-1, 0},  中心   { 1, 0},
//  {-1, 1},{ 0, 1},{ 1, 1}}; 
									
		// 定义八邻域搜索方向左边界搜索方向（顺时针）
		static const int8 SEEDS_L[8][2] = 
		{{ 0, 1},{-1, 1},{-1, 0},{-1,-1},{ 0,-1},{ 1,-1},{ 1, 0},{ 1, 1}};
		// 定义八邻域搜索方向右边界搜索方向（逆时针）
		static const int8 SEEDS_R[8][2] = 
		{{ 0, 1},{ 1, 1},{ 1, 0},{ 1,-1},{ 0,-1},{-1,-1},{-1, 0},{-1, 1}};   
		
		// 初始化中心点
		uint8 center_l[2] = { l_start_x, l_start_y };
		uint8 center_r[2] = { r_start_x, r_start_y };

		// 存储当前搜索的八邻域点
		uint8 search_l[8][2];
		uint8 search_r[8][2];
		
		// 存储候选点
		uint8 candidates_l[8][2];
		uint8 candidates_r[8][2];
		
		uint8 candidate_count_l = 0;
		uint8 candidate_count_r = 0;

		uint16 l_data_statics = *l_stastic; // 左边界点计数器
		uint16 r_data_statics = *r_stastic; // 右边界点计数器

		// 开始邻域搜索循环
		while (break_flag--)//break_flag为循环次数
		{
				// 生成左边界当前中心点的八邻域
				for (int i = 0; i < 8; i++) {
						search_l[i][0] = center_l[0] + SEEDS_L[i][0];
						search_l[i][1] = center_l[1] + SEEDS_L[i][1];
				}
				
				// 生成右边界当前中心点的八邻域
				for (int i = 0; i < 8; i++) {
						search_r[i][0] = center_r[0] + SEEDS_R[i][0];
						search_r[i][1] = center_r[1] + SEEDS_R[i][1];
				}

				// 保存当前中心点到结果数组
				points_l[l_data_statics][0] = center_l[0];
				points_l[l_data_statics][1] = center_l[1];
				l_data_statics++;
				
				points_r[r_data_statics][0] = center_r[0];
				points_r[r_data_statics][1] = center_r[1];
				r_data_statics++;

				// 重置候选点计数器
				candidate_count_l = 0;
				candidate_count_r = 0;

				// 寻找左边界候选点
				for (int i = 0; i < 8; i++) {
						// 检查是否越界
						if (search_l[i][0] < 0 || search_l[i][0] >= MT9V03X_W || 
								search_l[i][1] < 0 || search_l[i][1] >= MT9V03X_H) {
								continue;
						}
						
						// 检查是否为边界点：黑像素右侧是白像素
						if (image[search_l[i][1]][search_l[i][0]] == 0 && 
								image[search_l[(i + 1) & 7][1]][search_l[(i + 1) & 7][0]] == 255) {
								
								candidates_l[candidate_count_l][0] = search_l[i][0];
								candidates_l[candidate_count_l][1] = search_l[i][1];
								dir_l[l_data_statics - 1] = i; // 记录生长方向
								candidate_count_l++;
						}
				}

				// 寻找右边界候选点
				for (int i = 0; i < 8; i++) {
						// 检查是否越界
						if (search_r[i][0] < 0 || search_r[i][0] >= MT9V03X_W || 
								search_r[i][1] < 0 || search_r[i][1] >= MT9V03X_H) {
								continue;
						}
						
						// 检查是否为边界点：黑像素左侧是白像素
						if (image[search_r[i][1]][search_r[i][0]] == 0 && 
								image[search_r[(i + 1) & 7][1]][search_r[(i + 1) & 7][0]] == 255) {
								
								candidates_r[candidate_count_r][0] = search_r[i][0];
								candidates_r[candidate_count_r][1] = search_r[i][1];
								dir_r[r_data_statics - 1] = i; // 记录生长方向
								candidate_count_r++;
						}
				}

				// 更新左边界中心点：选择Y坐标最小的候选点（向上生长优先）
				if (candidate_count_l > 0) {
						center_l[0] = candidates_l[0][0];
						center_l[1] = candidates_l[0][1];
						
						for (int j = 1; j < candidate_count_l; j++) {
								if (candidates_l[j][1] < center_l[1]) {
										center_l[0] = candidates_l[j][0];
										center_l[1] = candidates_l[j][1];
								}
						}
				} else {
						// 没有找到候选点，回溯
						if (l_data_statics > 1) {
								l_data_statics--;
								center_l[0] = points_l[l_data_statics-1][0];
								center_l[1] = points_l[l_data_statics-1][1];
						} else {
								break; // 无法回溯，退出
						}
				}

				// 更新右边界中心点
				if (candidate_count_r > 0) {
						center_r[0] = candidates_r[0][0];
						center_r[1] = candidates_r[0][1];
						
						for (int j = 1; j < candidate_count_r; j++) {
								if (candidates_r[j][1] < center_r[1]) {
										center_r[0] = candidates_r[j][0];
										center_r[1] = candidates_r[j][1];
								}
						}
				} else {
						// 没有找到候选点，回溯
						if (r_data_statics > 1) {
								r_data_statics--;
								center_r[0] = points_r[r_data_statics-1][0];
								center_r[1] = points_r[r_data_statics-1][1];
						} else {
								break; // 无法回溯，退出
						}
				}

				// 检查终止条件
				// 1. 左右边界相遇
				if (my_abs(center_r[0] - center_l[0]) < 2 && my_abs(center_r[1] - center_l[1]) < 2) {
						*hightest = (center_r[1] + center_l[1]) >> 1;
						break;
				}

				// 2. 高度平衡控制
				if (center_r[1] < center_l[1]) {
						// 右边界高于左边界，左边界等待
						l_data_statics--;
						center_l[0] = points_l[l_data_statics-1][0];
						center_l[1] = points_l[l_data_statics-1][1];
					
//可根据赛道弯曲程度调整（急弯X可增大，直道X可减小)*************************************
				} else if (center_r[1] > center_l[1] + 5) {
					// 右边界明显低于左边界，右边界等待
//center_r[1] > center_l[1] + X*************************************
//直道场景：赛道左右边界几乎平行，高度差很小，可以减小阈值（如 2-3），使跟踪更精确。
//急弯场景：赛道弯曲剧烈，左右边界高度差可能很大，需要增大阈值（如 8-10），避免边界过早停止生长。
					
						
						r_data_statics--;
						center_r[0] = points_r[r_data_statics-1][0];
						center_r[1] = points_r[r_data_statics-1][1];
				}
		}

		// 返回统计结果
		*l_stastic = l_data_statics;
		*r_stastic = r_data_statics;
		
		// 计算最高点
		if (l_data_statics > 0 && r_data_statics > 0) {
				uint8 l_highest = points_l[0][1];
				uint8 r_highest = points_r[0][1];
				
				for (int i = 1; i < l_data_statics; i++) {
						if (points_l[i][1] < l_highest) {
								l_highest = points_l[i][1];
						}
				}
				
				for (int i = 1; i < r_data_statics; i++) {
						if (points_r[i][1] < r_highest) {
								r_highest = points_r[i][1];
						}
				}
				
				*hightest = (l_highest < r_highest) ? l_highest : r_highest;
		} else {
				*hightest = 0;
		}
}

/*
 *功能说明：从八邻域边界里提取需要的边线
 *total_L	：找到的点的总数
 *example： get_left(data_stastics_l );
 */
uint8 l_border[120];//左线数组
uint8 r_border[120];//右线数组
uint8 center_line[120];//中线数组
void get_left	(uint16 total_L)
{
	uint8 i = 0;
	uint16 j = 0;
	uint8 h = 0;
	//初始化
	for (i = 0;i<120;i++)
	{
		l_border[i] = 0;
	}
	h = 120 - 2;
	//左边
	for (j = 0; j < total_L; j++)
	{
		if (points_l[j][1] == h)
		{
			l_border[h] = points_l[j][0]+1;
		}
		else continue; //每行只取一个点，没到下一行就不记录
		h--;
		if (h == 0) 
		{
			break;//到最后一行退出
		}
	}
}
/*
 *从八邻域边界里提取需要的边线
 *total_R  ：找到的点的总数
 *example：get_right(data_stastics_r);
 */
void get_right(uint16 total_R)
{
	uint8 i = 0;
	uint16 j = 0;
	uint8 h = 0;
	for (i = 0; i < 120; i++)
	{
		r_border[i] = 188;//右边线初始化放到最右边，左边线放到最左边，这样八邻域闭合区域外的中线就会在中间，不会干扰得到的数据
	}
	h = 120 - 2;
	//右边
	for (j = 0; j < total_R; j++)
	{
		if (points_r[j][1] == h)
		{
			r_border[h] = points_r[j][0] - 1;
		}
		else continue;//每行只取一个点，没到下一行就不记录
		h--;
		if (h == 0)break;//到最后一行退出
	}
}

//定义膨胀和腐蚀的阈值区间
//当中心像素为黑色 (0) 且周围白色像素足够多时，将其变为白色 (255)，实现膨胀效果
//当中心像素为白色 (255) 且周围黑色像素足够多时，将其变为黑色 (0)，实现腐蚀效果
#define threshold_max	255*5//此参数可根据自己的需求调节
#define threshold_min	255*2//此参数可根据自己的需求调节
void image_filter(uint8(*bin_image)[188])//形态学滤波，简单来说就是膨胀和腐蚀的思想
{
	uint16 i, j;
	uint32 num = 0;


	for (i = 1; i < 120 - 1; i++)
	{
		for (j = 1; j < (188 - 1); j++)
		{
			//统计八个方向的像素值
			num =
				bin_image[i - 1][j - 1] + bin_image[i - 1][j] + bin_image[i - 1][j + 1]
				+ bin_image[i][j - 1] + bin_image[i][j + 1]
				+ bin_image[i + 1][j - 1] + bin_image[i + 1][j] + bin_image[i + 1][j + 1];


			if (num >= threshold_max && bin_image[i][j] == 0)
			{

				bin_image[i][j] = 255;//白  可以搞成宏定义，方便更改

			}
			if (num <= threshold_min && bin_image[i][j] == 255)
			{

				bin_image[i][j] = 0;//黑

			}

		}
	}

}
void image_draw_rectan(uint8(*image)[188])
{

	uint8 i = 0;
	for (i = 0; i < 120; i++)
	{
		image[i][0] = 0;
		image[i][1] = 0;
		image[i][188 - 1] = 0;
		image[i][188 - 2] = 0;

	}
	for (i = 0; i < 188; i++)
	{
		image[0][i] = 0;
		image[1][i] = 0;
		//image[image_h-1][i] = 0;

	}
}
uint8_t Best_thrsod;    //八邻域得到的阈值
uint8_t ZhongZhi=94;

void image_process(void)
{
		uint16 i;
		uint8 hightest = 0;//定义一个最高行，tip：这里的最高指的是y值的最小
		
		Best_thrsod=OtsuThreshold(image_copy,MT9V03X_W,MT9V03X_H);//大津法 动态阈值	
	  ips200_show_string(188,Image_Down,"YuZhi:");
		ips200_show_uint  (188,16+Image_Down, Best_thrsod, 3);
	  ips200_show_string(212,16+Image_Down,"^@^");
  	binarizeImage(image_copy,MT9V03X_W,MT9V03X_H,Best_thrsod);//二值化
//提取赛道边界\预处理
		image_filter(image_copy);//滤波       __
		image_draw_rectan(image_copy);//加黑框| |
//清零	
		data_statics_l = 0;
		data_statics_r = 0;
	if (get_start_point(image_copy,120-1)){//找到起点了，再执行八领域，没找到就一直找 (不用while防止程序死循环，最好先放到赛道上)
//八邻域提取边界				
				search_l_r((uint16)USE_NUM, image_copy, 
				&data_statics_l ,  &data_statics_r, 
				start_point_l[0], start_point_l[1],
				start_point_r[0], start_point_r[1], &hightest);
//从爬取的边界线内提取边线，这个才是最有用的边线			
				get_left(data_statics_l);
				get_right(data_statics_r);
		}

/*如果有元素处理就都写在这里
 *
 *		
 *	
 *
 *		
 */
//显示图像
		ips200_show_gray_image(0, Image_Down, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, Best_thrsod);
//根据最终循环次数画出边界点
		for (i = 0; i < data_statics_l; i++)
		{
			ips200_draw_point(points_l[i][0]+2, points_l[i][1]+Image_Down, RGB565_BLUE);//显示起点
		}
		for (i = 0; i < data_statics_r; i++)
		{
			ips200_draw_point(points_r[i][0]-2, points_r[i][1]+Image_Down, RGB565_RED);//显示起点
		}

		for (i = hightest; i < 120-1; i++)
		{
			center_line[i] = (l_border[i] + r_border[i]) >> 1;//求中线
			//求中线最好最后求，不管是补线还是做状态机，全程最好使用一组边线，中线最后求出，不能干扰最后的输出
			//当然也有多组边线的找法 但是不建议
			ips200_draw_point(center_line[i], i+Image_Down, RGB565_GREEN);   //显示起点 显示中线	
			ips200_draw_point(l_border[i],    i+Image_Down, RGB565_GREEN);   //显示起点 显示左边线
			ips200_draw_point(r_border[i],    i+Image_Down, RGB565_GREEN);   //显示起点 显示右边线
		}
      //			ZhongZhi=center_line[5];
			ZhongZhi=(center_line[0]
						   +center_line[1]
						   +center_line[2]
						 	 +center_line[3]
							 +center_line[4])/5;  //加权平均获取中值
			ips200_show_string(188,32+Image_Down,"Middle");
			ips200_show_uint  (188,48+Image_Down, ZhongZhi, 3);

}


////***************************************************************
////* 函数名称：uint8_t GetOSTU(uint8_t tmImage[IMAGEH][IMAGEW])
////* 功能说明：求阈值大小
////* 参数说明：有三个输入变量，第一个是指针P，指向待处理的图像的第一个位置，指针为全局变量，
////*           还有两个宏定义LQOSTU_MIN02，LQOSTU_MAX02 ，他们分别表示要计算阈值的区域的大小
////* 函数返回：阈值大小 cam_thr
////* 修改时间：2018年3月27日
////*           第一次修改
////* 备 注：
////Ostu方法又名最大类间差方法，通过统计整个图像的直方图特性来实现全局阈值T的自动选取，其算法步骤为：
////1) 先计算图像的直方图，即将图像所有的像素点按照0~255共256个bin，统计落在每个bin的像素点数量
////2) 归一化直方图，也即将每个bin中像素点数量除以总的像素点
////3) i表示分类的阈值，也即一个灰度级，从0开始迭代
////4) 通过归一化的直方图，统计0~i 灰度级的像素(假设像素值在此范围的像素叫做前景像素) 所占整幅图像的比例w0，并统计前景像素的平均灰度u0；统计i~255灰度级的像素(假设像素值在此范围的像素叫做背景像素) 所占整幅图像的比例w1，并统计背景像素的平均灰度u1；
////5) 计算前景像素和背景像素的方差 g = w0*w1*(u0-u1) (u0-u1)
////6) i++；转到4)，直到i为256时结束迭代
////7）将最大g相应的i值作为图像的全局阈值
////缺陷:OSTU算法在处理光照不均匀的图像的时候，效果会明显不好，因为利用的是全局像素信息。
////******************************************************************************
//uint8_t zzkLQGetOSTU0200(uint8 LQOSTU_MIN,uint8 LQOSTU_MAX)
//{ 
//  int16_t j;
//  uint32_t Amount = 0;
//  uint32_t PixelBack = 0;
//  uint32_t PixelIntegralBack = 0;
//  uint32_t PixelIntegral = 0;
//  int32_t PixelIntegralFore = 0;
//  int32_t PixelFore = 0;
//  double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma; // 类间方差;
//  int16_t MinValue, MaxValue;
//  uint8_t Threshold = 0;             //
//  uint8_t Threshold0 = 0;             //
//  uint8 *tmImagep;
//  uint8 *imgp;
//  int tmImage_size=188*(LQOSTU_MAX-LQOSTU_MIN+1)/3-1;
//  imgp=(uint8_t *)mt9v03x_image;
//  tmImagep=imgp+188*LQOSTU_MIN;
//  for (j = 0; j < 256; j++)
//  {
//    HistoGram[j] = 0; //初始化灰度直方图
//    LQ_ary[j]=0;
//  }
//  while(tmImage_size)
//  {
//    HistoGram[*tmImagep]++; //统计灰度级中每个像素在整幅图像中的个数
//    tmImagep=tmImagep+3;
//    tmImage_size--;
//  }

//  for (MinValue = 0; MinValue < 256 && HistoGram[MinValue] == 0; MinValue++) ;        //获取最小灰度的值
//  for (MaxValue = 255; MaxValue > MinValue && HistoGram[MinValue] == 0; MaxValue--) ; //获取最大灰度的值

//  if (MaxValue == MinValue)
//    return MaxValue;         // 图像中只有一个颜色
//  if (MinValue + 1 == MaxValue)
//    return MinValue;        // 图像中只有二个颜色

//  for (j = MinValue; j <= MaxValue; j++)
//    Amount += HistoGram[j];        //  像素总数  256

//  PixelIntegral = 0;
//  for (j = MinValue; j <= MaxValue; j++)
//  {
//    PixelIntegral += HistoGram[j] * j;//灰度值总数  256
//  }
//  SigmaB = -1;
//  //  str.Format("%d",MinValue);AfxMessageBox(str);
//  for (j = MinValue; j < MaxValue; j++)
//  {
//    PixelBack = PixelBack + HistoGram[j];    //前景像素点数
//    PixelFore = Amount - PixelBack;         //背景像素点数
//    OmegaBack = (double)PixelBack / Amount;//前景像素百分比
//    OmegaFore = (double)PixelFore / Amount;//背景像素百分比
//    PixelIntegralBack += HistoGram[j] * j;  //前景灰度值
//    PixelIntegralFore = PixelIntegral - PixelIntegralBack;//背景灰度值
//    MicroBack = (double)PixelIntegralBack / PixelBack;   //前景灰度百分比
//    MicroFore = (double)PixelIntegralFore / PixelFore;   //背景灰度百分比
//    Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);//计算类间方差
//    if (Sigma > SigmaB)                    //遍历最大的类间方差g //找出最大类间方差以及对应的阈值
//    {
//      SigmaB = Sigma;
//      Threshold = j;
//    }
//  }
//  //============求阈值左右像素点的比例，防止出现部分强光现象=========
//  int LQ_cam_jishu,LQ_black=0,LQ_white=0;
//  float LQ_thr_rate=0;
//  int LQGTU_jishu;
//  cam_thr_min=1;
//  for(LQGTU_jishu=2;LQGTU_jishu<Threshold;LQGTU_jishu++)
//  {
//    LQ_black=LQ_black+HistoGram[LQGTU_jishu];//统计小于阈值的黑点个数
//    if(HistoGram[LQGTU_jishu]>HistoGram[cam_thr_min])
//      cam_thr_min=LQGTU_jishu;//找到小于阈值的一个峰值
//  }
//  cam_thr_max=Threshold;
//  for(LQGTU_jishu=Threshold+1;LQGTU_jishu<256;LQGTU_jishu++)
//  {
//    LQ_white=LQ_white+HistoGram[LQGTU_jishu];//统计大于阈值的白点的个数
//    if(HistoGram[LQGTU_jishu]>HistoGram[cam_thr_max])
//      cam_thr_max=LQGTU_jishu;//找到大于阈值的一个峰值
//  }
//  LQ_thr_rate=LQ_black*1.0/LQ_white;//统计黑点与白点的比例
//  int LQ_G_white=0;
//  for(LQGTU_jishu=LQOSTU_QIANGGUANG;LQGTU_jishu<256;LQGTU_jishu++)
//  {
//    LQ_G_white=LQ_G_white+HistoGram[LQGTU_jishu];//统计亮度大于200的点数
//  }
//  //str.Format("%d",LQ_G_white);AfxMessageBox(str);
//  if(LQ_G_white>LQOSTU_QIANGNUM&&0)//亮度大于200的个数大于2000，强光散射
//  {
//    cam_strong_light=1;//强光标志位
//    cam_thr_mid=(cam_thr_max-cam_thr_min)/LQOSTU_THRERR;
//  }
//  else if(LQ_thr_rate>3&&0)
//  {
//    cam_wesk_light=1;  //弱光标志位
//    for(LQGTU_jishu=3;LQGTU_jishu<254;LQGTU_jishu++)//滤波
//    {
//      LQ_ary[LQGTU_jishu]=(HistoGram[LQGTU_jishu-2]+HistoGram[LQGTU_jishu-1]+HistoGram[LQGTU_jishu]+HistoGram[LQGTU_jishu+1]+HistoGram[LQGTU_jishu+2])/5;
//    }
//    for(LQGTU_jishu=cam_thr_min-3;LQGTU_jishu>3;LQGTU_jishu--)
//    {
//      if(LQ_ary[LQGTU_jishu]>LQ_ary[LQGTU_jishu+1]&&LQ_ary[LQGTU_jishu+1]>LQ_ary[LQGTU_jishu+2])
//        break;
//    }
//    int LQ_max,LQ_min=LQGTU_jishu;
//    for(;LQGTU_jishu<cam_thr_min;LQGTU_jishu++)//计算信新的阈值
//    {
//      if(HistoGram[LQGTU_jishu]<HistoGram[LQ_min])
//        LQ_min=LQGTU_jishu;
//    }
//    Threshold=LQ_min+10;
//    cam_thr_max=cam_thr_min;
//    //    str.Format("%d",Threshold);AfxMessageBox(str);
//    LQ_max=2;
//    for(LQGTU_jishu=2;LQGTU_jishu<Threshold;LQGTU_jishu++)
//    {
//      if(HistoGram[LQGTU_jishu]>HistoGram[LQ_max])
//        LQ_max=LQGTU_jishu;
//    }
//    // str.Format("%d",Threshold);AfxMessageBox(str);
//    cam_thr_min=cam_thr_max;
//    cam_thr_mid=(cam_thr_max-cam_thr_min)/LQOSTU_THRERR;
//  }
//  else
//  {
//      if(Threshold0-Threshold>20)
//          Threshold=Threshold0-20;
//      else if(Threshold0-Threshold<-20)
//          Threshold=Threshold0+20;
//    //  str.Format("%d",cam_thr);AfxMessageBox(str);
//    //  cam_thr=200;
//    cam_normal_light=1;//正常光标志位
////  Threshold=Threshold-5;
//    cam_thr_mid=(cam_thr_max-cam_thr_min)/LQOSTU_THRERR;
//  }
//  //=================================================================
//  Threshold0=Threshold;
//  return Threshold;                        //返回最佳阈值;
//}







