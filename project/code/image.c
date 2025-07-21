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
#define ROI_Y_START  32                // ROI起始行（感兴趣区域）
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
		for (int i = 32; i < height; i++) {
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
		for (i = Image_Delete; i < MT9V03X_H - 1; i++){
				for (j = 1; j < (MT9V03X_W - 1); j++){
				    //统计八个方向的像素值只滤除白色噪点防止八邻域陷入某个很靠近赛道的白色循环
						num=image[i-1][j-1]+image[i-1][j]+image[i-1][j+1]
							 +image[i][j - 1]              +image[i][j + 1]
							 +image[i+1][j-1]+image[i+1][j]+image[i+1][j+1];
//			      if (num >= threshold_max && image[i][j] == 0){
//								image[i][j] = 255;
//						}
						//当某白色点周围的八个点 小于等于三个白点 即大于等于五个黑点 时判定为黑点
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
								dir_l[l_data_statics - 1] = i; // 记录生长方向//l_data_statics在上面已经+1 所以这里-1表示第一个数据
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
										if(candidates_l[j][0]!=points_l[l_data_statics-1][0] && candidates_l[j][1]!=points_l[l_data_statics-1][1]
			              && candidates_l[j][0]!=points_l[l_data_statics-2][0] && candidates_l[j][1]!=points_l[l_data_statics-2][1]
			              && candidates_l[j][0]!=points_l[l_data_statics-3][0] && candidates_l[j][1]!=points_l[l_data_statics-3][1]
			              && candidates_l[j][0]!=points_l[l_data_statics-4][0] && candidates_l[j][1]!=points_l[l_data_statics-4][1]
										){
												center_l[0] = candidates_l[j][0];
												center_l[1] = candidates_l[j][1];
										}
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
										if(candidates_r[j][0]!=points_r[r_data_statics-1][0] && candidates_r[j][1]!=points_r[r_data_statics-1][1]
			              && candidates_r[j][0]!=points_r[r_data_statics-2][0] && candidates_r[j][1]!=points_r[r_data_statics-2][1]
			              && candidates_r[j][0]!=points_r[r_data_statics-3][0] && candidates_r[j][1]!=points_r[r_data_statics-3][1]
			              && candidates_r[j][0]!=points_r[r_data_statics-4][0] && candidates_r[j][1]!=points_r[r_data_statics-4][1]
										){
												center_r[0] = candidates_r[j][0];
												center_r[1] = candidates_r[j][1];
										}
								}
						}
				} 
				else {
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
				//************左/右边界循迹到尽头中点--->左/右边界停止爬线**************
				if (center_l[0] >= 94 && center_l[1] < 60 ){
						center_l[0] = points_l[l_data_statics - 1][0];//x
						center_l[1] = points_l[l_data_statics - 1][1];//y
						l_data_statics--;
				}
				if (center_r[0] <= 94 && center_r[1] < 60 ){
						center_r[0] = points_r[r_data_statics - 1][0];//x
						center_r[1] = points_r[r_data_statics - 1][1];//y
						r_data_statics--;				
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
				r_border[i]=186;
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
///************************************************************
//最长白列
//***********************************************************/
//void Horizontal_line(void)
//{
//  uint8 i,j;
//  if(Pixle[Row-1][Col/2]==0)
//  {
//    if(Pixle[Row-1][5]==white)
//      midline[Row]=5;
//    else if(Pixle[Row-1][Col-5]==white)
//      midline[Row]=Col-5;
//    else
//      midline[Row]=Col/2;
//  }
//  else
//	{
//    midline[Row]=Col/2;		
//	}
//	
//  for(i=Row-1;i>0;i--)
//  {
//    for(j=midline[i+1];j>=0;j--)
//    {
//      if(Pixle[i][j]==0||j==0)
//      {
//        leftline[i]=j;
//        break;
//      }
//    }
//    for(j=midline[i+1];j<=Col-1;j++)
//    {
//      if(Pixle[i][j]==0||j==Col-1)
//      {
//        rightline[i]=j;
//        break;
//      }
//    }
//    midline[i]=(leftline[i]+rightline[i])/2;
//   if(Pixle[i-1][midline[i]]==0||i==0)
//    {
//   for(j=i;j>0;j--)
//   {
//    midline[j]=midline[i];
//    leftline[j]=midline[i];
//    rightline[j]=midline[i];
//   }
//      break;
//    }
//  }
//}
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
//				if(Buzzer_ChuJie_flag==0){
//						Buzzer_On_Count(1);
//						Buzzer_ChuJie_flag=1;
//				}
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
//		ips200_show_string(0, 0,"Stop_Num:");
//		ips200_show_uint(72, 0,Stop_sum, 2);
		
		if(Stop_sum >4 && en_location1>50000){
				YueJie_flag=1;
				if(Buzzer_Stop_flag==0){
						Buzzer_On_Count(1);
						Buzzer_Stop_flag=1;
				}
		}
}	
/**                    
  * @brief 左边丢线
  * @param 无
  * @retval 左边丢线的数量
  */
uint8 Lost_Left(void){
		uint8 Lost_Left_Sum=0;
		uint8 i=0;
		for(i=119;i>32;i--){
				if(l_border[i]==1){
						Lost_Left_Sum++;
				}
		}
		return Lost_Left_Sum;
}
/**                    
  * @brief 右边丢线
  * @param 无
  * @retval 右边丢线的数量
  */
uint8 Lost_Right(void){
		uint8 Lost_Right_Sum=0;
		uint8 i=0;
		for(i=119;i>32;i--){
				if(r_border[i]==186){
						Lost_Right_Sum++;
				}
		}
		return Lost_Right_Sum;
}

/**                    
  * @brief 判断十字
  * @param 无
  * @retval 无
  */
uint8 Judge_Cross(void){
		uint8 leftlost =Lost_Left();
		uint8 rightlost=Lost_Right();
		if(leftlost>=30 && rightlost>=30)return 1;
		return 0;
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
		for(int i=32+6;i<120-4;i++){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上左拐点
        if(left_up_point==0&&
        l_border[i-1]-l_border[i]<=3&&
        l_border[i-2]-l_border[i-1]<=3&&
        l_border[i-3]-l_border[i-2]<=3&&
        l_border[i-4]-l_border[i-3]<=3&&
        l_border[i-5]-l_border[i-4]<=3&&
        (l_border[i]-l_border[i+2])>=5&&
        (l_border[i]-l_border[i+3])>=8&&
        (l_border[i]-l_border[i+4])>=8&&
				image_copy[i-3][l_border[i]]==0&&//上面是黑色
				image_copy[i+3][l_border[i]]==255&&//下面是白色
				image_copy[i-3][l_border[i]-3]==0&&  //左上面是黑色			因为侧入的时候左边可能是白色 左上防止误判
				image_copy[i][l_border[i]+3]==255//右边是白色
				){           
            left_up_point=i;
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
		for(int i=119-5;i>left_up_point+4;i--){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上左拐点
        if(left_down_point==0&&
        (l_border[i]-l_border[i+1])<=3&&
        (l_border[i+1]-l_border[i+2])<=3&&
        (l_border[i+2]-l_border[i+3])<=3&&
        (l_border[i+3]-l_border[i+4])<=3&&
        (l_border[i]-l_border[i-3])>=5&&//不从i-1开始防止噪声干扰
        (l_border[i]-l_border[i-4])>=8&&
        (l_border[i]-l_border[i-5])>=8&&
				image_copy[i][l_border[i]+3]==255&&//右边是白色
				image_copy[i-3][l_border[i]]==255&&//上面是白色
				image_copy[i+3][l_border[i]]==255&&//下面是白色
				image_copy[i+3][l_border[i]-3]==0  //左下面是黑色			
				){           
            left_down_point=i;
						break;
        }
		}
}
/**                   
	 * @brief 找右上拐点
	 * @param 无
	 * @retval 无
	 */
uint8 right_up_point=0;
void Get_Right_Up_Point(void){
		right_up_point=0;
		uint8 i=0;
		for(int i=32+6;i<120-4;i++){
        if(right_up_point==0&&
				(r_border[i]-r_border[i-1])<=3&&
				(r_border[i-1]-r_border[i-2])<=3&&
				(r_border[i-2]-r_border[i-3])<=3&&
				(r_border[i-3]-r_border[i-4])<=3&&
				(r_border[i-4]-r_border[i-5])<=3&&
				r_border[i+2]-r_border[i]>=5 &&
				r_border[i+3]-r_border[i]>=8&&
				r_border[i+4]-r_border[i]>=8&&
				image_copy[i-3][r_border[i]]==0&&//上面是黑色
				image_copy[i+3][r_border[i]]==255&&//下面是白色
				image_copy[i][r_border[i]-3]==255&&  //左面是白色			
				image_copy[i-3][r_border[i]+3]==0//右上边是黑色
				
				){           
            right_up_point=i;
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
		for(int i=119-5;i>right_up_point+4;i--){
        //点i下面2个连续相差不大并且点i与上面边3个点分别相差很大，认为有上左拐点
        if(right_down_point==0&&
        (r_border[i+1]-r_border[i])<=3&&
        (r_border[i+2]-r_border[i+1])<=3&&
        (r_border[i+3]-r_border[i+2])<=3&&
        (r_border[i+4]-r_border[i+3])<=3&&
        (r_border[i-2]-r_border[i])>=5&&//不从i-1开始防止噪声干扰
        (r_border[i-3]-r_border[i])>=8&&
        (r_border[i-4]-r_border[i])>=8&&
				image_copy[i-3][r_border[i]]==255&&//上面是白色
				image_copy[i+3][r_border[i]]==255&&//下面是白色
				image_copy[i][r_border[i]-3]==255&&  //左面是白色			
				image_copy[i+3][r_border[i]+3]==0//右下边是黑色
				
				){           
            right_down_point=i;
						break;
        }
		}
}
/**
	* @brief  左边补线
	* @param  x1 起点x坐标
	* @param  y1 起点y坐标
	* @param  x2 终点x坐标
	* @param  y2 终点y坐标
  * @retval 无
	*/
//(x1,y1)--->(x2,y2)    y1<y2
void left_draw_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
{	
    uint8 hx;
    uint8 a1=y1;
    uint8 a2=y2;
		uint8 i;
		uint8 t;
//防止输入越界
    if(x1>=186)x1=186;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=Image_Delete)y1=Image_Delete;

    if(x2>=186)x2=186;
    else if(x2<=2)x2=2;
    if(y2>=MT9V03X_H-3)y2=MT9V03X_H-3;
    else if(y2<=Image_Delete)y2=Image_Delete;
//防止for循环范围错误、
    if(a1>a2)//坐标互换
    {
        t=a1;
        a1=a2;
        a2=t;
    }
//计算斜率补线
    for(i=a1;i<a2;i++)
    {
        hx=x1+(i-y1)*(x2-x1)/(y2-y1);//使用斜率补线
        //防止补线越界
        if(hx>=186)hx=186;
        else if(hx<=1)hx=1;
        l_border[i]=hx;
    }
}
/**
	* @brief  右边补线
	* @param  x1 起点x坐标
	* @param  y1 起点y坐标
	* @param  x2 终点x坐标
	* @param  y2 终点y坐标
  * @retval 无
	*/
//(x1,y1)--->(x2,y2)    y1<y2
void right_draw_line(uint8 x1,uint8 y1,uint8 x2,uint8 y2)
{	
    uint8 hx;
    uint8 a1=y1;
    uint8 a2=y2;
		uint8 i;
		uint8 t;
//防止输入越界
    if(x1>=186)x1=186;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=Image_Delete)y1=Image_Delete;

    if(x2>=186)x2=186;
    else if(x2<=2)x2=2;
    if(y2>=MT9V03X_H-3)y2=MT9V03X_H-3;
    else if(y2<=Image_Delete)y2=Image_Delete;
//防止for循环范围错误、
    if(a1>a2)//坐标互换
    {
        t=a1;
        a1=a2;
        a2=t;
    }
//计算斜率补线
    for(i=a1;i<a2;i++)
    {
        hx=x1+(i-y1)*(x2-x1)/(y2-y1);//使用斜率补线
        //防止补线越界
        if(hx>=186)hx=186;
        else if(hx<=1)hx=1;
        r_border[i]=hx;
    }
}

/**
* @brief 最小二乘法
* @param uint8 begin                输入起点
* @param uint8 end                  输入终点
* @param uint8 *border              输入需要计算斜率的边界首地址
* @retval 返回说明
*/
float Slope_Calculate(uint8 begin, uint8 end, uint8 *border)
{
    float xsum = 0, ysum = 0, xysum = 0, x2sum = 0;
    int16 i = 0;
    float result = 0;
    static float resultlast;
 
    for (i = begin; i < end; i++)
    {
        xsum += i;
        ysum += border[i];
        xysum += i * (border[i]);
        x2sum += i * i;
 
    }
    if ((end - begin)*x2sum - xsum * xsum) //判断除数是否为零
    {
        result = ((end - begin)*xysum - xsum * ysum) / ((end - begin)*x2sum - xsum * xsum);
        resultlast = result;
    }
    else
    {
        result = resultlast;
    }
    return result;
}
/**
	* @brief  左上单拐点补线
	* @param  x1 起点x坐标
	* @param  y1 起点y坐标
  * @retval 无
	*/
//(x1,y1)--->(x2,y2)    y1<y2
void left_up_point_draw_line(uint8 x1,uint8 y1)
{	
    uint8 hx;
		uint8 i;
		float k;
		k=Slope_Calculate(left_up_point-7,left_up_point, l_border);
//防止输入越界
    if(x1>=186)x1=186;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=Image_Delete)y1=Image_Delete;
//计算斜率补线
    for(i=y1;i<120;i++)
    {
        hx=x1+(i-y1)*k;//使用斜率补线
        //防止补线越界
        if(hx>=186)hx=186;
        else if(hx<=1)hx=1;
        l_border[i]=hx;
    }
}
/**
  * @brief  右上单拐点补线
	* @param  x1 起点x坐标
	* @param  y1 起点y坐标
  * @retval 无
	*/
//(x1,y1)--->(x2,y2)    y1<y2
void right_up_point_draw_line(uint8 x1,uint8 y1)
{	
    uint8 hx;
		uint8 i;
		float k;
		k=Slope_Calculate(right_up_point-7,right_up_point, r_border);
//防止输入越界
    if(x1>=186)x1=186;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=Image_Delete)y1=Image_Delete;
//计算斜率补线
    for(i=y1;i<120;i++)
    {
        hx=x1+(i-y1)*k;//使用斜率补线
        //防止补线越界
        if(hx>=186)hx=186;
        else if(hx<=1)hx=1;
        r_border[i]=hx;
    }
}
/**
	* @brief  左下单拐点补线
	* @param  x1 起点x坐标
	* @param  y1 起点y坐标
  * @retval 无
	*/
void left_down_point_draw_line (uint8 x1,uint8 y1){
    uint8 hx;
		uint8 i;
		float k;
		k=Slope_Calculate(left_down_point,left_down_point+10, l_border);
//防止输入越界
    if(x1>=186)x1=186;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=Image_Delete)y1=Image_Delete;
//计算斜率补线
    for(i=y1;i>32;i--)
    {
        hx=x1+(i-y1)*k;//使用斜率补线
        //防止补线越界
        if(hx>=186)hx=186;
        else if(hx<=1)hx=1;
        l_border[i]=hx;
    }
}
/**
	* @brief  左下单拐点补线
	* @param  x1 起点x坐标
	* @param  y1 起点y坐标
  * @retval 无
	*/
void right_down_point_draw_line (uint8 x1,uint8 y1){
    uint8 hx;
		uint8 i;
		float k;
		k=Slope_Calculate(right_down_point,right_down_point+10, r_border);
//防止输入越界
    if(x1>=186)x1=186;
    else if(x1<=2)x1=2;
    if(y1>=MT9V03X_H-3)y1=MT9V03X_H-3;
    else if(y1<=Image_Delete)y1=Image_Delete;
//计算斜率补线
    for(i=y1;i>32;i--)
    {
        hx=x1+(i-y1)*k;//使用斜率补线
        //防止补线越界
        if(hx>=186)hx=186;
        else if(hx<=1)hx=1;
        r_border[i]=hx;
    }
}

/**                    
  * @brief 十字补线
  * @param 无
  * @retval 无
  */
uint8 Cross_flag=0;
void myCross_fill(void){
		Cross_flag=0;
		if(Judge_Cross()){
				Cross_flag=1;
				Buzzer_On_Count(1);
				Get_Left_Up_Point();
				Get_Right_Up_Point();
				Get_Left_down_Point();
				Get_Right_down_Point();
				//四个拐点都存在
				if(left_up_point!=0 && right_up_point!=0 && left_down_point!=0 && right_down_point!=0){
						Buzzer_On_Count(1);
						Cross_flag=10;
						left_draw_line(l_border[left_up_point],left_up_point,l_border[left_down_point],left_down_point);
						right_draw_line(r_border[right_up_point],right_up_point,r_border[right_down_point],right_down_point);}		
				//右拐的时候 右下拐点先消失
				else if(left_up_point!=0 && right_up_point!=0 && left_down_point!=0 && right_down_point == 0){
						Buzzer_On_Count(1);
						Cross_flag=11;
						left_draw_line(l_border[left_up_point],left_up_point,l_border[left_down_point],left_down_point);
						right_up_point_draw_line(r_border[right_up_point],right_up_point);}								
				//左拐的时候 左下拐点先消失
				else if(left_up_point!=0 && right_up_point!=0 && left_down_point == 0 && right_down_point !=0){
						Buzzer_On_Count(1);
						Cross_flag=12;
						left_up_point_draw_line(l_border[left_up_point],left_up_point); 
						right_draw_line(r_border[right_up_point],right_up_point,r_border[right_down_point],right_down_point);}		
				//上两个拐点存在，下两个拐点消失，十字进行中
				else if(left_up_point!=0 && right_up_point!=0 && left_down_point == 0 && right_down_point ==0){
						Buzzer_On_Count(1);
						Cross_flag=13;
						left_up_point_draw_line (l_border[left_up_point] ,left_up_point); 
						right_up_point_draw_line(r_border[right_up_point],right_up_point);}								
				//四个拐点都消失，出十字
				else if(left_up_point==0 && right_up_point==0 && left_down_point == 0 && right_down_point ==0){
						Buzzer_On_Count(1);
						Cross_flag=0;
				}
//以下是八邻域爬线存在的找线问题
//********左上方拐点由于圆环导致找不到拐点
				else if(left_up_point==0 && right_up_point!=0 && left_down_point != 0 && right_down_point !=0){
						Buzzer_On_Count(1);
						Cross_flag=3;
						left_down_point_draw_line (l_border[left_down_point] ,left_down_point);
						right_draw_line(r_border[right_up_point],right_up_point,r_border[right_down_point],right_down_point);}	
				//左上方拐点由于圆环导致找不到拐点 若此时为右转弯 右下方拐点先消失
				else if(left_up_point==0 && right_up_point!=0 && left_down_point != 0 && right_down_point ==0){
						Buzzer_On_Count(1);
						Cross_flag=31;
						left_down_point_draw_line (l_border[left_down_point] ,left_down_point);
						right_up_point_draw_line(r_border[right_up_point],right_up_point);}								
				//左上方拐点 由于圆环导致找不到拐点 若此时为左转弯 左下方拐点先消失
				else if(left_up_point==0 && right_up_point!=0 && left_down_point == 0 && right_down_point !=0){
						Buzzer_On_Count(1);
						Cross_flag=32;
						left_draw_line(l_border[33],33,l_border[119],119);
						right_draw_line(r_border[right_up_point],right_up_point,r_border[right_down_point],right_down_point);}	
//********右上方拐点由于圆环导致找不到拐点
				else if(left_up_point!=0 && right_up_point==0 && left_down_point != 0 && right_down_point !=0){
						Buzzer_On_Count(1);
						Cross_flag=4;
						left_draw_line(l_border[left_up_point],left_up_point,l_border[left_down_point],left_down_point);
						right_down_point_draw_line (r_border[right_down_point] ,right_down_point);}
				//右上方拐点由于圆环导致找不到拐点 若此时为左转弯 左下方拐点先消失
				else if(left_up_point!=0 && right_up_point==0 && left_down_point == 0 && right_down_point !=0){
						Buzzer_On_Count(1);
						Cross_flag=41;
						left_up_point_draw_line (l_border[left_up_point] ,left_up_point); 
						right_down_point_draw_line (r_border[right_down_point] ,right_down_point);}
						//右上方拐点由于圆环导致找不到拐点 若此时为右转弯 右下方拐点先消失
				else if(left_up_point!=0 && right_up_point==0 && left_down_point != 0 && right_down_point ==0){
						Buzzer_On_Count(1);
						Cross_flag=42;
						left_draw_line(l_border[left_up_point],left_up_point,l_border[left_down_point],left_down_point);
						right_draw_line(r_border[33],33,r_border[119],119);}	
//八邻域导致的“圆环”十字进行中 承接上面
//********左上方拐点由于圆环导致找不到拐点 该“特殊十字”进行中 下面两个拐点消失
				else if(left_up_point==0 && right_up_point!=0 && left_down_point == 0 && right_down_point ==0){
						Buzzer_On_Count(1);
						Cross_flag=33;
						left_draw_line(l_border[33],33,l_border[119],119);
						right_up_point_draw_line(r_border[right_up_point],right_up_point);}		
//********右上方拐点由于圆环导致找不到拐点 该“特殊十字”进行中 下面两个拐点消失
				else if(left_up_point!=0 && right_up_point==0 && left_down_point == 0 && right_down_point ==0){
						Buzzer_On_Count(1);
						Cross_flag=43;
						left_up_point_draw_line (l_border[left_up_point] ,left_up_point); 
						right_draw_line(r_border[33],33,r_border[119],119);}	
//显示左右上下拐点
				ips200_show_string(0, 32,"left_up:");
				ips200_show_uint(88, 32,left_up_point, 2);
				ips200_show_string(0, 48,"left_down:");
				ips200_show_uint(88, 48,left_down_point, 2);
				ips200_show_string(0, 64,"right_up:");
				ips200_show_uint(88, 64,right_up_point, 2);
				ips200_show_string(0, 80,"right_down:");
				ips200_show_uint(88, 80,right_down_point, 2);
//显示当前标志位									
				ips200_show_string(0, 16,"Cross_flag:");
				ips200_show_uint(88, 16,Cross_flag, 2);
		}else{//出十字的时候
				ips200_show_string(0, 16,"Cross_flag:");
				ips200_show_uint(88, 16,Cross_flag, 2);
		}
}
/**                    
  * @brief 判断一组边线是直道还是弯道
	* @param *border 边线数组名
	* @retval 1：直线，0：弯道
  */
uint8	Judge_ZhiXian(uint8 *border){
		uint8 i=0;
		for(i=96;i>86;i++){
				if(my_abs(border[i]-border[i-32])>32){
						return 0;//变化大 弯道
				}
		}
		return 1;//默认返回为直道
}
/**                    
  * @brief 判断圆环
  * @param 无
  * @retval 无
  */

uint8 Left_Circle_Flag=0;
uint8 Right_Circle_Flag=0;
void Judge_Circle(void){
		uint8 leftlost=Lost_Left();
		uint8 rightlost=Lost_Right();
		if(leftlost>=30 || rightlost<=5 ){//左边丢线 可能是左环岛或者左弯道
				if(Judge_ZhiXian(r_border)==1){//如果右边是直道 说明是左圆环
						Left_Circle_Flag=1;
						Buzzer_On_Count(1);
				}
		}
		if(leftlost<=5  || rightlost>=30){//右边丢线 可能是右环岛或者右弯道
				if(Judge_ZhiXian(l_border)==1){//如果左边是直道 说明是右圆环
						Right_Circle_Flag=1;
						Buzzer_On_Count(1);
				}
		}
}
/**                    
  * @brief 找左圆环圆环最右端
  * @param 无
  * @retval 无
  */
uint8 left_circle_point=0;
void Get_Left_Circle_Point(void){
		left_circle_point=0;
		uint8 i=0;
		for(int i=119-5;i>32+5;i--){
        if(left_circle_point==0&&
        (l_border[i]-l_border[i+2])>=0&&
        (l_border[i]-l_border[i+3])>=0&&
        (l_border[i]-l_border[i+4])>=0&&
        (l_border[i]-l_border[i-2])>=0&&
        (l_border[i]-l_border[i-3])>=0&&
        (l_border[i]-l_border[i-4])>=0&&
				image_copy[i][l_border[i]-3]==0&&//左边是黑色	
				image_copy[i][l_border[i]+3]==255//右边是白色
				){           
            left_circle_point=i;
						break;    //找到就break 防止误判
        }
		}
}
	
/**                    
  * @brief 找右圆环圆环最左端
  * @param 无
  * @retval 无
  */
uint8 right_circle_point=0;
void Get_Right_Circle_Point(void){
		right_circle_point=0;
		uint8 i=0;
		for(int i=119-5;i>32+5;i--){
        if(right_circle_point==0&&
        (r_border[i]-r_border[i+2])<=0&&
        (r_border[i]-r_border[i+3])<=0&&
        (r_border[i]-r_border[i+4])<=0&&
        (r_border[i]-r_border[i-2])<=0&&
        (r_border[i]-r_border[i-3])<=0&&
        (r_border[i]-r_border[i-4])<=0&&
				image_copy[i][r_border[i]-3]==255&&//左边是白色			
				image_copy[i][r_border[i]+3]==0    //右边是黑色
				){           
            right_circle_point=i;
						break;    //找到就break 防止误判
        }
		}
}
/**                    
  * @brief 圆环状态
  * @param 无
  * @retval 无
  */
uint8 Left_Circle_Status=0;
uint8 Right_Circle_Status=0;
void myCircle_fill(void){
		if(Right_Circle_Flag==0 && Left_Circle_Flag==0){
				Left_Circle_Status=0;
			  Right_Circle_Status=0;
				Judge_Circle();
		}
//进入右圆环
		if(Right_Circle_Flag==1){
					uint8 Right_Circle_Status=1;//设置右圆环状态标志位
					switch(Right_Circle_Status){//这里的switch函数主要用于判断进入下一状态的标志元素，判断到了就进入下一元素
							case 1://第一阶段 识别到圆环 补线挡路
									Get_Right_down_Point();//找右下拐点
									Get_Right_Circle_Point();//找到黑色圆圈最左端
									if(right_down_point==0 && right_circle_point!=0)
									Right_Circle_Status=2;break;//直到右下拐点消失,右上黑色圆圈最左端出现 进入第二状态
							case 2://第二阶段 右下拐点消失 依旧补线挡路
									Get_Right_Up_Point();//第二状态下会一直找右上黑色圆圈最左边进行拉线挡路,但是状态switch要一直判断右上拐点是否出现 一出现就进入第三状态
									if(right_up_point!=0)  Right_Circle_Status=3;break;//直到找到右上拐点进入第三状态
							case 3://第三状态 右上拐点出现 补线进环进入第三状态 入环！！！
									Get_Right_Up_Point();//右上拐点一旦消失就进入下一状态即将如环ing
									Get_Left_Up_Point();
									if(right_up_point==0 && left_up_point!=0)Right_Circle_Status=4;break;
							case 4://if左拐点消失那就进入循迹状态
									Get_Left_Up_Point();
									if(left_up_point==0)Right_Circle_Status=5;break;
							case 5://找到左下拐点,拉线出环
									Get_Left_down_Point();
									if(left_down_point!=0)Right_Circle_Status=6;break;
							case 6://左拐点消失 继续手动拉线出去
									Get_Left_down_Point();
									if(left_down_point==0)Right_Circle_Status=7;break;
							case 7://发现右上拐点进入状态8
									Get_Right_Up_Point();
									if(right_up_point!=0)Right_Circle_Status=8;break;
							case 8://右上拐点消失，出环！！！！
									Get_Right_Up_Point();
									if(right_up_point==0){
											Right_Circle_Status=0;
											Right_Circle_Flag=0;
									}break;
					}
//显示左右上下拐点        ***************这里还需要改一下 加入圆环最左端拐点等等.............................
				ips200_show_string(0, 32,"left_up:");
				ips200_show_uint(88, 32,left_up_point, 2);
				ips200_show_string(0, 48,"left_down:");
				ips200_show_uint(88, 48,left_down_point, 2);
				ips200_show_string(0, 64,"right_up:");
				ips200_show_uint(88, 64,right_up_point, 2);
				ips200_show_string(0, 80,"right_down:");
				ips200_show_uint(88, 80,right_down_point, 2);				
				ips200_show_string(0, 96,"r_cir_p:");
				ips200_show_uint(88, 96,right_circle_point, 2);
				ips200_show_string(0, 112,"l_cir_p:");
				ips200_show_uint(88, 112,left_circle_point, 2);
//显示当前标志位		
				ips200_show_string(0, 0,"Circle_Flag:");
				ips200_show_uint(96, 0,Right_Circle_Flag, 1);					
				ips200_show_string(0, 16,"Circle_Status:");
				ips200_show_uint(88, 16,Right_Circle_Status, 2);
		}else{//出圆环的时候
				ips200_show_string(0, 0,"Circle_Flag:");
				ips200_show_uint(96, 0,Right_Circle_Flag, 1);
				ips200_show_string(0, 16,"Circle_Status:");
				ips200_show_uint(112, 16,Right_Circle_Status, 2);
		}
//处理右圆环
		if(Right_Circle_Status==1){
				Get_Right_down_Point();//找右下拐点				
				right_down_point_draw_line (r_border[right_down_point] ,right_down_point);//拉线挡路
		}
		if(Right_Circle_Status==2){ 
				Get_Right_Circle_Point();//找到黑色圆圈最左端
				right_draw_line(r_border[right_circle_point],right_circle_point,r_border[115],115);//直接从这个点拉到右道路右下角挡路
		}
		if(Right_Circle_Status==3){ 
				Get_Right_Up_Point();//switch找到右上拐点,那我也得赶紧找出来然后拉线入环！！
				right_draw_line(r_border[right_up_point],right_up_point,l_border[115],115);//直接从这个点拉到左道路右下角
		}
		if(Right_Circle_Status==4){ 
				Get_Left_Up_Point();//switch状态机发现右上拐点消失 左上拐点出现，那我就拉左拐点入环！！！
				left_draw_line(l_border[left_up_point],left_up_point,l_border[115],115);//直接从这个点拉到左道路右下角
		}
		if(Right_Circle_Status==5){
				//啥也不干
				Buzzer_On_Count(1);//额还是响一下吧表示进入第五阶段了

		} 
		if(Right_Circle_Status==6){//拉线出环
				Get_Left_down_Point();
				left_down_point_draw_line (l_border[left_down_point] ,left_down_point);//拉线
		}
		if(Right_Circle_Status==7){//拉线出环
				left_draw_line(120,32,1,115);//直接从这个点拉到左道路右下角		
		}
		if(Right_Circle_Status==8){//发现右上拐点补线出环
				Get_Right_Up_Point();
				right_up_point_draw_line(r_border[right_up_point] ,right_up_point);//拉线
		}

//进入左圆环
//		if(Left_Circle_Flag==1){
//					uint8 left_Circle_Status=1;//设置左圆环状态标志位
//					switch(left_Circle_Status){//这里的switch函数主要用于判断进入下一状态的标志元素，判断到了就进入下一元素
//							case 1://第一阶段 识别到圆环 补线挡路
//									Get_Left_down_Point();//找左下拐点
//									if(left_down_point==0)left_Circle_Status=2;break;//直到左下拐点消失进入第二状态
//							case 2://第二阶段 左下拐点消失 依旧补线挡路
//									Get_Left_Up_Point();//第二状态下会一直找左上黑色圆圈最左边进行拉线挡路,但是状态switch要一直判断左上拐点是否出现 一出现就进入第三状态
//									if(left_up_point!=0)  left_Circle_Status=3;break;//直到找到右上拐点进入第三状态
//							case 3://第三状态时如果左上拐点出现 补线进环进入第三状态 入环！！！
//									Get_Left_Up_Point();//右上拐点一旦消失就进入下一状态即将如环ing
//									Get_Left_Up_Point();
//									if(left_up_point==0 && left_up_point!=0)left_Circle_Status=4;break;
//							case 4://if右拐点消失那就进入循迹状态
//									Get_Right_Up_Point();
//									if(right_up_point==0)left_Circle_Status=5;break;
//							case 5://找到右下拐点,拉线出环
//									Get_Right_down_Point();
//									if(right_down_point!=0)left_Circle_Status=6;break;
//							case 6://右下拐点消失 继续手动拉线出去
//									Get_Right_down_Point();
//									if(right_down_point==0)left_Circle_Status=7;break;
//							case 7://发现左上拐点进入状态8
//									Get_Left_Up_Point();
//									if(left_up_point!=0)left_Circle_Status=8;break;
//							case 8://左上拐点消失，出环！！！！
//									Get_Left_Up_Point();
//									if(left_up_point==0){
//											left_Circle_Status=0;
//											Left_Circle_Flag=0;
//									}break;
//					}
//		}
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
uint16_t Sum_QuanZhong;
uint16_t Sum_ZhongZhi;	
char pid_flag;          //1时即可进入pid中断输出
void image_process(void)
{
		uint16 i;
//大津法 动态阈值*********************************************************
		Best_thrsod=OtsuThreshold(image_copy,MT9V03X_W,MT9V03X_H);
//	  ips200_show_string(MT9V03X_W,Image_Down,"YuZhi:");
//		ips200_show_uint  (MT9V03X_W,16+Image_Down, Best_thrsod, 3);
//	  ips200_show_string(212,16+Image_Down,"^~^");
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
//判断十字并补线**********************************************************
		if(Right_Circle_Flag==0 && Left_Circle_Flag==0){//防止进入圆环状态机时误判十字 
				myCross_fill();		
		}
//判断圆环并补线**********************************************************
		if(Cross_flag==0){//防止进入十字误判圆环 十字其实也不容易误判圆环 额。。。好吧可能角度问题也会误判圆环的
				myCircle_fill();
		}
//保护处理**************************************************************	
		Motor_Protection();//电机过热过快保护
		ChuJie_Test(image_copy);//出界保护	
		Stop_Test(image_copy);//斑马线处理
////显示图像**************************************************************	
		ips200_show_gray_image(0, Image_Down, (const uint8 *)image_copy, MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, Best_thrsod);
		for(i=0;i<data_statics_l;i++){//左边界
				ips200_draw_point(points_l[i][0]+1, points_l[i][1]+Image_Down, RGB565_BLUE);
		}
		for(i=0;i<data_statics_r;i++){//右边界
				ips200_draw_point(points_r[i][0]-1, points_r[i][1]+Image_Down, RGB565_RED);
		}
		for(i=0;i< MT9V03X_H-1;i++)
		{
				center_line[i]=(l_border[i]+r_border[i])/2;//求中线
		}
//清零***************************************
		Sum_ZhongZhi  = 0;
		Sum_QuanZhong = 0;
//加权计算中值
//		for(i=77;i>75;i--){
//				Sum_ZhongZhi  += center_line[i]*QuanZhong[i];   
//				Sum_QuanZhong += QuanZhong[i];
//		}
//			ZhongZhi=(uint8_t)(Sum_ZhongZhi/Sum_QuanZhong);//这次中值
//互补滤波得到输出中值
//		ZhongZhi1=(uint8_t)(Sum_ZhongZhi/Sum_QuanZhong);//这次中值
//		ZhongZhi=(uint8_t)((ZhongZhi1*95+ZhongZhi0*5)/100);//互补滤波得到输出中值
//		ZhongZhi0=ZhongZhi;//记录上一次中值          加权中值
//单点中值
//			ZhongZhi=center_line[114];//单点
//多点平均中值
		ZhongZhi=(uint8_t)((center_line[60]+center_line[70]+center_line[80]+center_line[90])/4);//互补滤波得到输出中值
//显示中值
		ips200_show_string (MT9V03X_W,Image_Down,"Middle");
		ips200_show_uint   (MT9V03X_W,16+Image_Down, ZhongZhi,3);
//pid
		pid_flag=1;          //1时即可进入pid中断输出
}

