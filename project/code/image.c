#include "image.h"
/**20
  * @brief 计算绝对值
	* @param width  图像宽度
	* @param height 图像高度
  * @retval 绝对值
  */
int my_abs(int a){
	if(a>=0)return a;
	else return -a;
}
/**
  * @brief 大津法计算阈值
  * @param image[MT9V03X_H][MT9V03X_W] 传入二维图像数组
  * @param width  图像宽度
  * @param height 图像高度
  * @retval 最佳阈值
  */
#define GRAY_SCALE 256
#define ROI_Y_START  0                // ROI起始行（感兴趣区域）
#define ROI_Y_END MT9V03X_H           // ROI结束行
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
/**
  * @brief 图像二值化（binarize）函数
  * @param image[MT9V03X_H][MT9V03X_W] 传入二维图像数组
  * @param width  图像宽度
  * @param height 图像高度
  * @param threshold 阈值
  * @retval 无
  */
void binarizeImage(unsigned char image[MT9V03X_H][MT9V03X_W], int width, int height, int threshold) {
		for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
						image[i][j] = (image[i][j] > threshold) ? 255 : 0;
				}
		}
}
/**
  * @brief 八邻域图像滤波
  * @param image 传入二维图像数组
  * @retval 无
  */
//定义膨胀和腐蚀的阈值区间
//当中心像素为黑色 (0) 且周围白色像素足够多时，将其变为白色 (255)，实现膨胀效果
//当中心像素为白色 (255) 且周围黑色像素足够多时，将其变为黑色 (0)，实现腐蚀效果
#define threshold_max	255*5//此参数可根据自己的需求调节
#define threshold_min	255*3//此参数可根据自己的需求调节
void image_filter(uint8 image[MT9V03X_H][MT9V03X_W])//形态学滤波，简单来说就是膨胀和腐蚀的思想
{
		uint16 i, j;
		uint32 num = 0;
		for (i = 1; i < MT9V03X_H - 1; i++){
				for (j = 1; j < (MT9V03X_W - 1); j++){
				    //统计八个方向的像素值
						num=image[i-1][j-1]+image[i-1][j]+image[i-1][j+1]
							 +image[i][j - 1]              +image[i][j + 1]
							 +image[i+1][j-1]+image[i+1][j]+image[i+1][j+1];
			      if (num >= threshold_max && image[i][j] == 0){
								image[i][j] = 255;
						}
						if (num <= threshold_min && image[i][j] == 255)
						{
								image[i][j] = 0;
						}
		    }
	  } 
}
/**                          __
	* @brief 图像预处理，给图像套一个黑框| |（并且上方舍弃高度为32的图像,两个字母长度）
  * @param image 传入二维图像数组
  * @retval 无
  */
void image_draw_rectan(uint8 image[MT9V03X_H][MT9V03X_W])
{

		uint8 i=0;
		uint8 j=0;
		for(i=0;i<MT9V03X_H;i++)
		{
				image[i][0] = 0;
				image[i][1] = 0;
				image[i][MT9V03X_W - 1] = 0;
				image[i][MT9V03X_W - 2] = 0;
		}
		for(i=0;i<MT9V03X_W;i++)
		{
				for(j=0;j<Image_Delete;j++){
							image[j][i] = 0; 
				}				
		}
}
/**
  * @brief 在二值图像中寻找左右边界的起始点 寻找两个边界的边界点作为八邻域循环的起始点
  * @param image 传入二维图像数组
  * @param start_row 搜索起始行
  * @retval 1表示成功找到左右起点，0表示未找到或者只找到一个起点
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
		if(image[start_row][MT9V03X_W/2]==255){
				// 从中间往左边搜索左边界起点
				for (int i=MT9V03X_W/2;i>0;i--) {
						start_point_l[0]=i;         //x
						start_point_l[1]=start_row; //y：118
				// 找到黑白边界：白色像素左侧是黑色像素
						if(image[start_row][i]==255&&image[start_row][i-1]==0){
								l_found = 1;
								break;
						}
				}
				// 从中间往右边搜索右边界起点
				for (int i=MT9V03X_W/2;i<MT9V03X_W-1;i++){
						// 避免数组越界        
						start_point_r[0]=i;        //x
						start_point_r[1]=start_row;//y
						// 找到黑白边界：白色像素右侧是黑色像素
						if(image[start_row][i]==255 && image[start_row][i+1]==0){
								r_found=1;
								break;
						}
				}
		}
		else if(image[start_row][MT9V03X_W*3/4]==255){
				// 从3/4往左边搜索左边界起点
				for (int i=MT9V03X_W*3/4;i>0;i--) {
						start_point_l[0]=i;         //x
						start_point_l[1]=start_row; //y：118
				// 找到黑白边界：白色像素左侧是黑色像素
						if(image[start_row][i]==255&&image[start_row][i-1]==0){
								l_found = 1;
								break;
						}
				}
				// 从3/4往右边搜索右边界起点
				for (int i=MT9V03X_W*3/4;i<MT9V03X_W-1;i++){
						// 避免数组越界        
						start_point_r[0]=i;        //x
						start_point_r[1]=start_row;//y
						// 找到黑白边界：白色像素右侧是黑色像素
						if(image[start_row][i]==255 && image[start_row][i+1]==0){
								r_found=1;
								break;
						}
				}
		}
		else if(image[start_row][MT9V03X_W*1/4]==255){
				// 从1/4往左边搜索左边界起点
				for (int i=MT9V03X_W*1/4;i>0;i--) {
						start_point_l[0]=i;         //x
						start_point_l[1]=start_row; //y：118
				// 找到黑白边界：白色像素左侧是黑色像素
						if(image[start_row][i]==255&&image[start_row][i-1]==0){
								l_found = 1;
								break;
						}
				}
				// 从1/4往右边搜索右边界起点
				for (int i=MT9V03X_W*1/4;i<MT9V03X_W-1;i++){
						// 避免数组越界        
						start_point_r[0]=i;        //x
						start_point_r[1]=start_row;//y
						// 找到黑白边界：白色像素右侧是黑色像素
						if(image[start_row][i]==255 && image[start_row][i+1]==0){
								r_found=1;
								break;
						}
				}
		}
		else if(image[start_row][MT9V03X_W*1/8]==255){
				// 从1/8往左边搜索左边界起点
				for (int i=MT9V03X_W*1/8;i>0;i--) {
						start_point_l[0]=i;         //x
						start_point_l[1]=start_row; //y：118
				// 找到黑白边界：白色像素左侧是黑色像素
						if(image[start_row][i]==255&&image[start_row][i-1]==0){
								l_found = 1;
								break;
						}
				}
				// 从1/8往右边搜索右边界起点
				for (int i=MT9V03X_W*1/8;i<MT9V03X_W-1;i++){
						// 避免数组越界        
						start_point_r[0]=i;        //x
						start_point_r[1]=start_row;//y
						// 找到黑白边界：白色像素右侧是黑色像素
						if(image[start_row][i]==255 && image[start_row][i+1]==0){
								r_found=1;
								break;
						}
				}
		}		
		else if(image[start_row][MT9V03X_W*7/8]==255){
				// 从1/8往左边搜索左边界起点
				for (int i=MT9V03X_W*7/8;i>0;i--) {
						start_point_l[0]=i;         //x
						start_point_l[1]=start_row; //y：118
				// 找到黑白边界：白色像素左侧是黑色像素
						if(image[start_row][i]==255&&image[start_row][i-1]==0){
								l_found = 1;
								break;
						}
				}
				// 从1/8往右边搜索右边界起点
				for (int i=MT9V03X_W*7/8;i<MT9V03X_W-1;i++){
						// 避免数组越界        
						start_point_r[0]=i;        //x
						start_point_r[1]=start_row;//y
						// 找到黑白边界：白色像素右侧是黑色像素
						if(image[start_row][i]==255 && image[start_row][i+1]==0){
								r_found=1;
								break;
						}
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
  * @param hightest 跟踪到的最高点y坐标,相当于道路的尽头
  * @retval 无
  */
// 定义找点的最大数组成员个数
// 存放点的x，y坐标
uint16 points_l[USE_NUM][2] = {{0,0}};//左线坐标
uint16 points_r[USE_NUM][2] = {{0,0}};//右线坐标
uint16 dir_l[USE_NUM] = {0};//存储左边生长方向
uint16 dir_r[USE_NUM] = {0};//存储右边生长方向
uint16 data_statics_l = 0;  //统计左边找到点的个数
uint16 data_statics_r = 0;  //统计右边找到点的个数
uint8  hightest = 0;        //最高点
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
				points_l[l_data_statics][0] = center_l[0];//x
				points_l[l_data_statics][1] = center_l[1];//y
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
								dir_l[l_data_statics - 1] = i; // 记录生长方向//l_data_statics在上面已经+1 所以这里-1
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
								
								candidates_r[candidate_count_r][0] = search_r[i][0];//x
								candidates_r[candidate_count_r][1] = search_r[i][1];//y
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
				} 
				else{
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
				//左右边界相遇终止******************************
				//由于我在图像上方加了一个超级大黑款，因此图像相遇真是手拿把掐！！！qwqwqwqwqwqwqwQWQWQWQWQWQWQWWQ
				if (my_abs(center_r[0] - center_l[0]) < 2 && my_abs(center_r[1] - center_l[1]) < 2) {
						*hightest = (center_r[1] + center_l[1]) >> 1;
						break;
				}
				//左右边界相遇终止********************************		
		*l_stastic = l_data_statics;
		*r_stastic = r_data_statics;
		}
}
/**
  * @brief 从八邻域边界里提取需要的边线
  * @param total_L	：找到的点的总数
	*	@example： get_left(data_stastics_l);
  * @retval 无
  */
uint8 l_border[MT9V03X_H];//左线数组
uint8 r_border[MT9V03X_H];//右线数组
uint8 center_line[MT9V03X_H];//中线数组
void get_left	(uint16 total_L)
{
		uint16 i = 0;
		uint16 j = 0;
		uint8  h = 0;
		//初始化
		for(i=0;i<MT9V03X_H;i++)
		{
				l_border[i]=1;
		}
		for(j=0;j<total_L;j++)
		{
				h=points_l[j][1];
				if(points_l[j][0]>l_border[h]){
						l_border[h]=points_l[j][0];
				}
				else continue;
		}
}

/**
  * @brief 从八邻域边界里提取需要的边线
  * @param total_R	：找到的点的总数
	*	@example get_right(data_stastics_r);
  * @retval 无
  */
void get_right(uint16 total_R)
{
		uint16 i = 0;
		uint16 j = 0;
		uint8  h = 0;
		//初始化
		for(i=0;i<MT9V03X_H;i++)
		{
				r_border[i]=188-2;
		}
		for(j=0;j<total_R;j++)
		{
				h=points_r[j][1];
				if(points_r[j][0]<r_border[h]){
						r_border[h]=points_r[j][0];
				}	
				else continue;
		}
}
/**                    
  * @brief 出界紧急停止 取图像下面中间10*60个点
  * @param image 传入二维图像数组
  * @retval 无
  */
uint8 YueJie_flag=0;
uint8 Buzzer_ChuJie_flag=0;//
void ChuJie_Test(uint8 image[MT9V03X_H][MT9V03X_W]){
		uint8 i=0;
		uint8 j=0;
		uint16 sum=0;
		for(i=110;i<120;i++){
				for(j=64;j<124;j++){
						if(image[i][j]==0){
								sum++;
						}
				}
		}
		if(sum >550){
				YueJie_flag=1;
				if(Buzzer_ChuJie_flag==0){
						Buzzer_On_Count(1);
						Buzzer_ChuJie_flag=1;
				}
		}
}	
/**                    
  * @brief 到达斑马线停止
  * @param image 传入二维图像数组
  * @retval 无
  */
uint8 Stop_flag=0;
uint8 Buzzer_Stop_flag=0;//
void Stop_Test(uint8 image[MT9V03X_H][MT9V03X_W]){
		uint8 i=0;
		uint8 j=0;
		uint16 Stop_sum=0;
		for(i=30;i<90;i++){
					if((image[115][i]==0 && image[115][i+1]==255)||(image[115][i]==255 && image[115][i+1]==0)){
							Stop_sum++;
					}
		} 
		ips200_show_string(0, 0,"Stop_Num:");
		ips200_show_uint(72, 0,Stop_sum, 2);
		
		if(Stop_sum >4 && en_location1>50000){
				YueJie_flag=1;
				if(Buzzer_Stop_flag==0){
						Buzzer_On_Count(1);
						Buzzer_Stop_flag=1;
				}
		}
}	
/**                    
  * @brief 判断十字
  * @param 无
  * @retval 无
  */
uint8 Corss_flag=0;
uint8 Judge_Cross(void){
		uint8 DiuXian_Flag_lsum=0;
		uint8 DiuXian_Flag_rsum=0;
		uint8 i=0;
		for(i=119;i>32;i--){
				if(l_border[i]==1){
						DiuXian_Flag_lsum++;
				}
				if(r_border[i]==188-2){
						DiuXian_Flag_rsum++;
				}			
		}	
		if(DiuXian_Flag_lsum>=30 && DiuXian_Flag_rsum>=30){
	      Corss_flag=1;
				return 1;
		}else {
				Corss_flag=0;
				return 0;
		}
}

/**                    
	* @brief 找左上拐点
  * @param 无
  * @retval 无
  */
uint8 left_up_point=0;
void Get_Left_Up_Point(void){
		left_up_point=0;
		uint8 i=0;
		for(int i=32+4;i<120-4;i++){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上左拐点
        if(left_up_point==0&&
        l_border[i-1]-l_border[i]<=3&&
        l_border[i-2]-l_border[i-1]<=3&&
        l_border[i-3]-l_border[i-2]<=3&&
        (l_border[i]-l_border[i+2])>=8&&
        (l_border[i]-l_border[i+3])>=10&&
        (l_border[i]-l_border[i+4])>=10){           
            left_up_point=i;
						break;
        }
		}
}/**                   
	 * @brief 找右上拐点
	 * @param 无
	 * @retval 无
	 */
uint8 right_up_point=0;
void Get_Right_Up_Point(void){
		right_up_point=0;
		uint8 i=0;
		for(int i=32+4;i<120-4;i++){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上右拐点
        if(right_up_point==0&&
				(r_border[i]-r_border[i-1])<=3&&
				(r_border[i-1]-r_border[i-2])<=3&&
				(r_border[i-2]-r_border[i-3])<=3&&
				r_border[i+2]-r_border[i]>=8 &&
				r_border[i+3]-r_border[i]>=10&&
				r_border[i+4]-r_border[i]>=10){           
            right_up_point=i;
						break;

        }
		}
}
/**                    
	* @brief 找左下拐点
  * @param 无
  * @retval 无
  */
uint8 left_down_point=0;
void Get_Left_down_Point(void){
		left_down_point=0;
		uint8 i=0;
		for(int i=119-3;i>left_up_point+4;i++){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上左拐点
        if(left_down_point==0&&
        (l_border[i]-l_border[i+1])<=3&&
        (l_border[i+1]-l_border[i+2])<=3&&
        (l_border[i+2]-l_border[i+3])<=3&&
        (l_border[i]-l_border[i-2])>=8&&//不从i-1开始防止噪声干扰
        (l_border[i]-l_border[i-3])>=10&&
        (l_border[i]-l_border[i-4])>=10){           
            left_down_point=i;
						break;
        }
		}
}
/**                    
	* @brief 找右下拐点
  * @param 无
  * @retval 无
  */
uint8 right_down_point=0;
void Get_Right_down_Point(void){
		right_down_point=0;
		uint8 i=0;
		for(int i=119-3;i>right_up_point+4;i++){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上左拐点
        if(right_down_point==0&&
        (r_border[i+1]-r_border[i])<=3&&
        (r_border[i+2]-r_border[i+1])<=3&&
        (r_border[i+3]-r_border[i+2])<=3&&
        (r_border[i-2]-r_border[i])>=8&&//不从i-1开始防止噪声干扰
        (r_border[i-3]-r_border[i])>=8&&
        (r_border[i-4]-r_border[i])>=8){           
            right_down_point=i;
						break;
        }
		}
}
/**
	* @brief  左边补线
	* @param  x1 起点x坐标
	* @param  x2 终点x坐标
	* @param  y1 起点y坐标
	* @param  y2 终点y坐标
	*/
void left_draw_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
{
    uint8 hx;
    uint8 a1=y1;
    uint8 a2=y2;
    //防止越界以及参数输入错误
    if(y1>y2)
    {
        uint8 t=y1;
        y1=y2;
        y2=t;
    }

    if(x1>=MT9V03X_W-3)x1=MT9V03X_W-3;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=2)y1=2;

    if(x2>=MT9V03X_W-3)x2=MT9V03X_W-3;
    else if(x2<=2)x2=2;
    if(y2>=MT9V03X_H-3)y2=MT9V03X_H-3;
    else if(y2<=2)y2=2;

    for(uint8 i=a1;i<a2;i++)
    {
        hx=x1+(i-y1)*(x2-x1)/(y2-y1);//使用斜率补线
        //防止补线越界
        if(hx>=MT9V03X_W-3)hx=MT9V03X_W-3;
        else if(hx<=2)hx=2;
        l_border[i]=hx;
    }
}
/**
	* @brief  右边补线(虽然和左边补线是一样的，但还是要分开，因为会有单边补线)
	* @param  x1 起点x坐标
	* @param  x2 终点x坐标
	* @param  y1 起点y坐标
	* @param  y2 终点y坐标
	*/
void right_draw_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
{
    uint8 hx;
    uint8 a1=y1;
    uint8 a2=y2;
    //防止越界以及参数输入错误
    if(y1>y2)
    {
        uint8 t=y1;
        y1=y2;
        y2=t;
    }

    if(x1>=MT9V03X_W-3)x1=MT9V03X_W-3;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=2)y1=2;

    if(x2>=MT9V03X_W-3)x2=MT9V03X_W-3;
    else if(x2<=2)x2=2;
    if(y2>=MT9V03X_H-3)y2=MT9V03X_H-3;
    else if(y2<=2)y2=2;

    for(uint8 i=a1;i<a2;i++)
    {
        hx=x1+(i-y1)*(x2-x1)/(y2-y1);//使用斜率补线
        //防止补线越界
        if(hx>=MT9V03X_W-3)hx=MT9V03X_W-3;
        else if(hx<=2)hx=2;
        r_border[i]=hx;
    }
}
/**
	* @brief  左边界延长
	* @param  start_point 延长起点
	* @param  end_point   延长终点
	*/
void lenthen_l_border(uint8 start_point,uint8 end_point)
{
    float k;
    //防止越界
    if(start_point>=MT9V03X_H-1)start_point=MT9V03X_H-1;
    if(start_point<32)start_point=32;
    if(end_point>=MT9V03X_H-1)end_point=MT9V03X_H-1;
    if(end_point<32)end_point=32;
    
    if(end_point<start_point)
    {
        uint8 t=start_point;
        start_point=end_point;
        end_point=t;
    }

    if(start_point<=37)//起点过于靠上，直接连线
    {
        left_draw_line(l_border[start_point],start_point,l_border[end_point],end_point);
    }
    else
    {
        k=(float)(l_border[start_point]-l_border[start_point-4])/5.0;//斜率
        for(uint8 i=start_point;i<=end_point;i++)
        {
            l_border[i]=l_border[start_point]+(int)(i-start_point)*k;//使用斜率延长

            if(l_border[i]<1)//防止越界
            {
                l_border[i]=1;
            }

            if(l_border[i]>=MT9V03X_W-2)//防止越界
            {
                l_border[i]=MT9V03X_W-2;
            }
        }
    }
}
/**
	* @brief  右边界延长
	* @param  start_point 延长起点
	* @param  end_point   延长终点
	*/
void lenthen_r_border(uint8 start_point,uint8 end_point)
{
    float k;
    //防止越界
    if(start_point>=MT9V03X_H-1)start_point=MT9V03X_H-1;
    if(start_point<32)start_point=32;
    if(end_point>=MT9V03X_H-1)end_point=MT9V03X_H-1;
    if(end_point<32)end_point=32;
    
    if(end_point<start_point)
    {
        uint8 t=start_point;
        start_point=end_point;
        end_point=t;
    }

    if(start_point<=5)//起点过于靠上，直接连线
    {
        left_draw_line(r_border[start_point],start_point,r_border[end_point],end_point);
    }
    else
    {
        k=(float)(r_border[start_point]-r_border[start_point-4])/5.0;//斜率
        for(uint8 i=start_point;i<=end_point;i++)
        {
            r_border[i]=r_border[start_point]+(int)(i-start_point)*k;//使用斜率延长

            if(r_border[i]<1)//防止越界
            {
                r_border[i]=1;
            }
            
            if(r_border[i]>=MT9V03X_W-2)//防止越界
            {
                r_border[i]=MT9V03X_W-2;
            }
        }
    }
}


/**                    
  * @brief 十字补线
  * @param 无
  * @retval 无
  */
void myCross_fill(void){
		uint8 Cross_flag=0;
		if(Judge_Cross()){

				Buzzer_On_Count(1);
				Get_Left_Up_Point();
				Get_Right_Up_Point();
				if(left_up_point!=0 && right_up_point!=0){
						Buzzer_On_Count(1);
						Get_Left_down_Point();
						Get_Right_down_Point();
						Cross_flag=1;
								if(left_down_point!=0 && right_down_point!=0)
								{		
										Buzzer_On_Count(1);
										Cross_flag=11;
										left_draw_line(l_border[left_up_point],left_up_point,l_border[left_down_point],left_down_point);//左边补线
										right_draw_line(r_border[right_up_point],right_up_point,r_border[right_down_point],right_down_point);//右边补线								
								}
								else if(left_down_point!=0 && right_down_point==0)//如果左边有下拐点，右边没有
								{
										Buzzer_On_Count(1);
										Cross_flag=12;
										left_draw_line(l_border[left_up_point],left_up_point,l_border[left_down_point],left_down_point);//左边补线
										lenthen_r_border(right_up_point-1,MT9V03X_H-2);//右边延长
								}
								else if(left_down_point==0 && right_down_point!=0)//如果左边有下拐点，右边没有
								{
										Buzzer_On_Count(1);
										Cross_flag=13;
										right_draw_line(r_border[right_up_point],right_up_point,r_border[right_down_point],right_down_point);//右边补线
										lenthen_l_border(left_up_point-1,MT9V03X_H-2);//左边延长
								}
								else if(left_down_point==0 && right_down_point==0)
								{
										Buzzer_On_Count(1);
										Cross_flag=14;
										lenthen_l_border(left_up_point-1,MT9V03X_H-2);//左边延长
										lenthen_r_border(right_up_point-1,MT9V03X_H-2);//右边延长
								}
						ips200_show_string(0, 32,"lu");
						ips200_show_uint(88, 32,left_up_point, 1);
						ips200_show_string(0, 48,"ru");
						ips200_show_uint(88, 48,right_up_point, 1);
						ips200_show_string(0, 64,"ld");
						ips200_show_uint(88, 64,left_down_point, 1);
						ips200_show_string(0, 80,"rd");
						ips200_show_uint(88, 80,right_down_point, 1);
									
						ips200_show_string(0, 16,"Cross_flag:");
						ips200_show_uint(88, 16,Cross_flag, 2);
				}
		}
}

/**                    
  * @brief 图像处理汇总
  * @param 无
  * @retval 无
  */
uint8_t QuanZhong[MT9V03X_H]={
1,1,1,1,1,1,1,1,1,1,            //第一行
1,1,1,1,1,1,1,1,1,1,            //第二行
1,1,1,1,1,1,1,1,1,1,            //第三行
1,1,1,1,1,1,1,1,1,1,            //第四行
2,2,2,2,2,2,2,2,2,2,            //第五行
3,3,3,3,3,3,3,3,3,3,            //第六行                                
6,6,6,6,6,6,6,6,6,6,            //第七行
7,8,9,10,11,12,13,14,15,16,     //第八行
17,18,19,20,20,20,20,19,18,17,  //第九行
16,15,14,13,12,11,10,9,8,7,     //第十行
6,6,6,6,6,6,6,1,1,1,            //第十一行
1,1,1,1,1,1,1,1,1,1           	//第十二行
};//12行 120个点
uint8_t Best_thrsod;    //八邻域得到的阈值
uint8_t ZhongZhi0=94;
uint8_t ZhongZhi1=94;
uint8_t ZhongZhi=94;
uint8_t Image_Down=120; //图像下移距离
uint32_t Sum_QuanZhong;
uint32_t Sum_ZhongZhi;	
char pid_flag;          //1时即可进入pid中断输出
void image_process(void)
{
		uint16 i;
//大津法 动态阈值*********************************************************
		Best_thrsod=OtsuThreshold(image_copy,MT9V03X_W,MT9V03X_H);
	  ips200_show_string(MT9V03X_W,Image_Down,"YuZhi:");
		ips200_show_uint  (MT9V03X_W,16+Image_Down, Best_thrsod, 3);
	  ips200_show_string(212,16+Image_Down,"^~^");
//二值化图像*************************************************************
  	binarizeImage(image_copy,MT9V03X_W,MT9V03X_H,Best_thrsod);
//滤波*****************************************************************
		image_filter(image_copy);
//提取赛道边界\预处理                         __
		image_draw_rectan(image_copy);//加黑框| |
//清零*****************************************************************
		data_statics_l = 0;
		data_statics_r = 0;
	  if(get_start_point(image_copy,MT9V03X_H-1)){//找到起点了，再执行八领域，没找到就一直找 (不用while防止程序死循环，最好先放到赛道上)
//八邻域提取边界	**********************************************************				
				search_l_r((uint16)USE_NUM, image_copy, 
				&data_statics_l ,  &data_statics_r, 
				start_point_l[0], start_point_l[1],
				start_point_r[0], start_point_r[1], &hightest);
//从爬取的边界线内提取边线，这个才是最有用的边线************************************
				get_left (data_statics_l);
				get_right(data_statics_r);
		}		
//判断丢线条——即遇到十字****************************************************
//判断十字并执行补线操作****************************************************
		myCross_fill();		
//保护处理**************************************************************	
		Motor_Protection();//电机过热保护
		ChuJie_Test(image_copy);//出界保护	
		Stop_Test(image_copy);//斑马线处理
//显示图像**************************************************************	
//		ips200_show_gray_image(0, Image_Down, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, Best_thrsod);
//		for(i=0;i<data_statics_l;i++){//左边界
//				ips200_draw_point(points_l[i][0]+1, points_l[i][1]+Image_Down, RGB565_BLUE);
//		}
//		for(i=0;i<data_statics_r;i++){//右边界
//				ips200_draw_point(points_r[i][0]-1, points_r[i][1]+Image_Down, RGB565_RED);
//		}
		for(i=0;i< MT9V03X_H-1;i++)
		{
				center_line[i]=(l_border[i]+r_border[i])/2;//求中线
//				ips200_draw_point(center_line[i],i+Image_Down, RGB565_GREEN );   //显示起点 显示中线	
//				if(l_border[i]+4>187)l_border[i]=183;
//				if(r_border[i]-4<1)r_border[i]=5;
//				ips200_draw_point(l_border[i]+4,i+Image_Down, RGB565_PURPLE);   //显示起点 显示中线	
//				ips200_draw_point(r_border[i]-4,i+Image_Down, RGB565_PURPLE);   //显示起点 显示中线	
		}
//清零***************************************
		Sum_ZhongZhi  = 0;
		Sum_QuanZhong = 0;
//加权计算中值
		for(i=118;i>115;i--){
				Sum_ZhongZhi +=center_line[i]*QuanZhong[i];   
				Sum_QuanZhong+=QuanZhong[i];
		}
		ZhongZhi1=(uint8_t)(Sum_ZhongZhi/Sum_QuanZhong);//这次中值
		ZhongZhi=(uint8_t)((ZhongZhi1*95+ZhongZhi0*5)/100);//互补滤波得到输出中值
		ZhongZhi0=ZhongZhi;//记录上一次中值
//		ZhongZhi=center_line[114];//单点
//显示中值
		ips200_show_string (MT9V03X_W,32+Image_Down,"Middle");
		ips200_show_uint   (MT9V03X_W,48+Image_Down, ZhongZhi,3);
//pid
		pid_flag=1;          //1时即可进入pid中断输出
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
//  int tmImage_size=MT9V03X_W*(LQOSTU_MAX-LQOSTU_MIN+1)/3-1;
//  imgp=(uint8_t *)mt9v03x_image;
//  tmImagep=imgp+MT9V03X_W*LQOSTU_MIN;
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







