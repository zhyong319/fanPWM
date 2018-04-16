
#include<GPIO.h>
#include<syszz.h>
/**********功能描述：GPIO 16 初始化设置为软件PWM输出，PWM上限100 ***/
/**********参数：无**/
/**********返回值：无*/
void Init_Pin(void)
{
	wiringPiSetup();
	softPwmCreate(PWM_PIN, 0, PWM_MAX_VALUE);

}



/**********功能描述：风扇,PWM自检，风扇由低速向高速转动***/
/**********参数：  无********************************/
/**********返回值：无********************************/
void FanCheckSelf(void)
{	
		softPwmWrite(PWM_PIN, 30);
		sleep(5);
		FanStop();
}



/**********功能描述：风扇启停控制***/
/******************************************/
/**********返回值：无********************************/
void FanControl(SYSDATA sys)
{
	int pwmValue = sys.fanPWM;
	softPwmWrite(PWM_PIN, pwmValue);
}

void FanPWM(int pwmValue)
{
	softPwmWrite(PWM_PIN , pwmValue);
}

void FanStop(void)
{
	softPwmWrite(PWM_PIN, 0);
}
