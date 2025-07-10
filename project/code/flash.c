#include "flash.h"
#define FLASH_SECTION_INDEX       (127)    // 存储数据用的扇区 倒数第一个扇区
#define FLASH_PAGE_INDEX          (3)      // 存储数据用的页码 倒数第一个页码
//***************************因为最后一个扇区最后一个页码基本不会用到，防止数据覆盖
/**
* @brief  菜单参数保存
* @param   无
*/
extern PID_t Outer;
void menu_save(void)
{
    if(flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))                      // 判断是否有数据
    {
        flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);                // 擦除这一页
    }
    flash_buffer_clear(); //擦除缓存区
    //写入缓冲区
    flash_union_buffer[0].float_type  = Outer.Kp;
    flash_union_buffer[1].float_type  = Outer.Ki;
    flash_union_buffer[2].float_type  = Outer.Kd;
    flash_union_buffer[3].float_type  = Outer.Kd;
    flash_union_buffer[4].float_type  = Outer.OutMax;
    flash_union_buffer[5].float_type  = Outer.OutMin;
    flash_union_buffer[6].float_type  = Outer.Target;
    flash_union_buffer[7].float_type  = Outer.I_Limit;
    flash_union_buffer[8].float_type  = Outer.Base_Speed;
    flash_union_buffer[9].float_type  = Outer.OutEndMax;
    flash_union_buffer[10].float_type = Outer.OutEndMin;
		
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);        // 向指定 Flash 扇区的页码写入缓冲区数据
}

/**
* @brief  调好的pid参数取出
* @param   无
*/
void menu_read(void)
{
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);           // 将数据从 flash 读取到缓冲区ad_page_to_buffer;
    //参数读出
    Outer.Kp        = flash_union_buffer[0].float_type;
    Outer.Ki        = flash_union_buffer[1].float_type;
    Outer.Kd        = flash_union_buffer[2].float_type;
    Outer.Kd        = flash_union_buffer[3].float_type;
    Outer.OutMax    = flash_union_buffer[4].float_type;
    Outer.OutMin    = flash_union_buffer[5].float_type;
    Outer.Target    = flash_union_buffer[6].float_type;
    Outer.I_Limit   = flash_union_buffer[7].float_type;
    Outer.Base_Speed= flash_union_buffer[8].float_type;
    Outer.OutEndMax = flash_union_buffer[9].float_type;
    Outer.OutEndMin = flash_union_buffer[10].float_type;
    
    flash_buffer_clear(); //擦除缓存区
}

