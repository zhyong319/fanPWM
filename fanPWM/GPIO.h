
#include<wiringPi.h>
#include<softPwm.h>
#include<unistd.h>
#include<syszz.h>

#ifndef _GPIO_H
#define _GPIO_H


#define START 1
#define STOP  0
#define TEMP_H 50
#define TEMP_L   40


#define PWM_PIN 16
#define PWM_MAX_VALUE 100

/**********功能描述：GPIO 16 初始化设置为输出 ***/
/**********参数：无**/
/**********返回值：无*/
extern void Init_Pin(void);



/**********功能描述：风扇自检，启动3秒，停5秒，共二个循环***/
/**********参数：  无********************************/
/**********返回值：无********************************/
extern void FanCheckSelf(void);



/**********功能描述：风扇启停控制***/
/**********参数：********************************/
/**********返回值：无********************************/
extern void FanControl(SYSDATA sys);


//功能描述：风扇启停控制
/**********参数：********************************/
/**********返回值：无********************************/
extern void FanStop(void);
#endif

