#ifndef _zf_common_headfile_h_
#define _zf_common_headfile_h_

#include "stdio.h"
#include "stdint.h"
#include "string.h"


//===================================================芯片 SDK 底层===================================================
#include "hal_device_registers.h"
#include "hal_common.h"
#include "hal_adc.h"
#include "hal_comp.h"
#include "hal_dac.h"
#include "hal_dma.h"
#include "hal_dma_request.h"
#include "hal_exti.h"
#include "hal_fsmc.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_iwdg.h"
#include "hal_rcc.h"
#include "hal_rtc.h"
#include "hal_sdio.h"
#include "hal_spi.h"
#include "hal_syscfg.h"
#include "hal_tim_16b.h"
#include "hal_tim_32b.h"
#include "hal_tim_adv.h"
#include "hal_tim_basic.h"
#include "hal_uart.h"
#include "hal_usb.h"
#include "hal_usb_bdt.h"
#include "hal_wwdg.h"
//===================================================芯片 SDK 底层===================================================

//====================================================开源库公共层====================================================
#include "zf_common_typedef.h"
#include "zf_common_clock.h"
#include "zf_common_debug.h"
#include "zf_common_fifo.h"
#include "zf_common_font.h"
#include "zf_common_function.h"
#include "zf_common_interrupt.h"
//====================================================开源库公共层====================================================

//===================================================芯片外设驱动层===================================================
#include "zf_driver_adc.h"
#include "zf_driver_delay.h"
#include "zf_driver_encoder.h"
#include "zf_driver_exti.h"
#include "zf_driver_flash.h"
#include "zf_driver_gpio.h"
#include "zf_driver_iic.h"
#include "zf_driver_pit.h"
#include "zf_driver_pwm.h"
#include "zf_driver_sdio.h"
#include "zf_driver_soft_iic.h"
#include "zf_driver_soft_spi.h"
#include "zf_driver_spi.h"
#include "zf_driver_timer.h"
#include "zf_driver_uart.h"
//===================================================芯片外设驱动层===================================================

//===================================================外接设备驱动层===================================================
#include "zf_device_absolute_encoder.h"
#include "zf_device_bluetooth_ch9141.h"
#include "zf_device_camera.h"
#include "zf_device_dl1a.h"
#include "zf_device_dm1xa.h"
#include "zf_device_gps_tau1201.h"
#include "zf_device_icm20602.h"
#include "zf_device_imu660ra.h"
#include "zf_device_imu963ra.h"
#include "zf_device_ips114.h"
#include "zf_device_ips200.h"
#include "zf_device_key.h"
#include "zf_device_mpu6050.h"
#include "zf_device_mt9v03x.h"
#include "zf_device_oled.h"
#include "zf_device_ov7725.h"
#include "zf_device_scc8660.h"
#include "zf_device_sdcard.h"
#include "zf_device_tft180.h"
#include "zf_device_tsl1401.h"
#include "zf_device_type.h"
#include "zf_device_virtual_oscilloscope.h"
#include "zf_device_wifi_uart.h"
#include "zf_device_wireless_uart.h"
//===================================================外接设备驱动层===================================================

//===================================================应用组件层===================================================
#include "ffconf.h"
#include "ff.h"
//===================================================应用组件层===================================================
//===================================================用户自定义文件===================================================
#include "PID.h"
#include "menu.h"
#include "image.h"
//===================================================用户自定义文件===================================================

//===================================================用户自定义数据===================================================                                  
#define IPS200_TYPE (IPS200_TYPE_SPI) // 双排排针 并口两寸屏 这里宏定义填写 IPS200_TYPE_PARALLEL8
#define KEY1                    (E2)
#define KEY2                    (E3)
#define KEY3                    (E4)
#define KEY4                    (E5)
//===================================================用户自定义数据===================================================

#endif
