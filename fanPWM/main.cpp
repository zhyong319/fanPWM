#include<stdio.h>
#include<CharacterLib.h>
#include<I2CLCD.h>
#include<GPIO.h>
#include<syszz.h>

#define ZONE0 0
#define ZONE1 1
int LCDFD = 0; //保存液晶文件句柄

//int averageValue = 0; //保存CPU温度的平均值
//int bootTimeNow = 0; //保存系统启动到现在的时间
//int fanStatus =1; //初始变量	
//double x, y; //保存CPU负载值
//int tmp, num1[2], num2[2]; //分别存储CPU负载的整数值（DOUBLE类型分位存储）
//volatile int fanPWM;

#define WARN_TEMPRETURE 35
#define FAN_START_WORK_TEMPRETURE 41


int GetCPUTempreture(void) ///获取CPU的平均温度
{
	int temp0 = 0; //保存ZONE0温度值
	int temp1 = 0; //保存ZONE1温度值
	int temprature;
	temp0 = GetZoneTemp(ZONE0);
	temp1 = GetZoneTemp(ZONE1);
	temprature = (temp0 + temp1) / 2;
	return temprature;
}
int FanProg(int temprature) //计算控制风扇PWM，并返回风扇PWM数据
{	
	if (temprature < FAN_START_WORK_TEMPRETURE)
	{
		FanStop();
		printf("Tempreture less %d C\n ", FAN_START_WORK_TEMPRETURE);
		return 0; // CPU温度低于X度，风扇停止工作
	}
	else
	{
		int pwmInt = (temprature - WARN_TEMPRETURE) * 5; //让PWM值在0——100之间，对应的温度为35——55度之间
		if (pwmInt >= PWM_MAX_VALUE)
		{
			pwmInt = PWM_MAX_VALUE;
		}
		printf("FAN PWM  = %d\n", pwmInt);
		return pwmInt;
	}
	//WriteSysLog(START);

}

void DisplayInfo(int fd , SYSDATA sys)
{
	//////系统运行中
	OLED_P16x16Ch(LCDFD, 16, 0, F16X16, 0);
	OLED_P16x16Ch(LCDFD, 36, 0, F16X16, 1);
	OLED_P16x16Ch(LCDFD, 56, 0, F16X16, 4);
	OLED_P16x16Ch(LCDFD, 76, 0, F16X16, 2);
	OLED_P16x16Ch(LCDFD, 96, 0, F16X16, 3);

	//已运行时间
	OLED_P8x16Str(LCDFD, 0, 2, F8X16, (sys.bootTimeNow / DAY) / 10); //XX天
	OLED_P8x16Str(LCDFD, 8, 2, F8X16, (sys.bootTimeNow / DAY) % 10);
	OLED_P16x16Ch(LCDFD, 16, 2, F16X16, 7);
	OLED_P8x16Str(LCDFD, 32, 2, F8X16, ((sys.bootTimeNow % DAY) / HOUR) / 10);  // xx时
	OLED_P8x16Str(LCDFD, 40, 2, F8X16, ((sys.bootTimeNow % DAY) / HOUR) % 10);
	OLED_P16x16Ch(LCDFD, 48, 2, F16X16, 8);
	OLED_P8x16Str(LCDFD, 64, 2, F8X16, (((sys.bootTimeNow % DAY) % HOUR) / MINUTE) / 10);  // XX分
	OLED_P8x16Str(LCDFD, 72, 2, F8X16, (((sys.bootTimeNow % DAY) % HOUR) / MINUTE) % 10);
	OLED_P16x16Ch(LCDFD, 80, 2, F16X16, 9);
	OLED_P8x16Str(LCDFD, 96, 2, F8X16, (((sys.bootTimeNow % DAY) % HOUR) % MINUTE) / 10);  //XX秒
	OLED_P8x16Str(LCDFD, 104, 2, F8X16, (((sys.bootTimeNow % DAY) % HOUR) % MINUTE) % 10);
	OLED_P16x16Ch(LCDFD, 112, 2, F16X16, 10);

	////负载
	OLED_P16x16Ch(LCDFD, 0, 4, F16X16, 5);
	OLED_P16x16Ch(LCDFD, 16, 4, F16X16, 6);
	OLED_P8x16Str(LCDFD, 32, 4, F8X16, 11);
	int tmp, num1[3], num2[3];
	num1[0] = (int)sys.cpuload_x; //计算第一个浮点数的整数部分
	tmp = (int)((sys.cpuload_x - num1[0]) * 100);
	num1[1] = tmp / 10;
	num1[2] = tmp % 10;

	num2[0] = (int)sys.cpuload_y;//计算第二个浮点数的整数部分
	tmp = (int)((sys.cpuload_y - num2[0]) * 100);
	num2[1] = tmp / 10;
	num2[2] = tmp % 10;
	OLED_P8x16Str(LCDFD, 40, 4, F8X16, num1[0]);
	OLED_P8x16Str(LCDFD, 48, 4, F8X16, 12);
	OLED_P8x16Str(LCDFD, 56, 4, F8X16, num1[1]);
	OLED_P8x16Str(LCDFD, 64, 4, F8X16, num1[2]);

	OLED_P8x16Str(LCDFD, 80, 4, F8X16, num2[0]);
	OLED_P8x16Str(LCDFD, 88, 4, F8X16, 12);
	OLED_P8x16Str(LCDFD, 96, 4, F8X16, num2[1]);
	OLED_P8x16Str(LCDFD, 104, 4, F8X16, num2[2]);
	////CPU温度

	OLED_P8x16Str(LCDFD, 0, 6, F8X16, 10); //C
	OLED_P8x16Str(LCDFD, 8, 6, F8X16, 13); //P
	OLED_P8x16Str(LCDFD, 16, 6, F8X16, 14);//U
	OLED_P8x16Str(LCDFD, 24, 6, F8X16, 11);//:
	OLED_P8x16Str(LCDFD, 32, 6, F8X16, sys.averageValue / 10);
	OLED_P8x16Str(LCDFD, 40, 6, F8X16, sys.averageValue % 10);

	//PWM
	OLED_P8x16Str(LCDFD, 64, 6, F8X16, 15); //P
	OLED_P8x16Str(LCDFD, 72, 6, F8X16, 16); //W
	OLED_P8x16Str(LCDFD, 80, 6, F8X16, 17); //M
	OLED_P8x16Str(LCDFD, 88, 6, F8X16, 11); //:
	
	if (sys.fanPWM < 30)
	{
		OLED_P8x16Str(LCDFD, 96, 6, F8X16, 20); //-
		OLED_P8x16Str(LCDFD, 104, 6, F8X16, 20);//-	
		OLED_P8x16Str(LCDFD, 112, 6, F8X16, 20); //-
		OLED_P8x16Str(LCDFD, 120, 6, F8X16, 18); // %	
		return ;
	}
	else if (sys.fanPWM < 100)
	{
		OLED_P8x16Str(LCDFD, 96, 6, F8X16, (sys.fanPWM / 10));
		OLED_P8x16Str(LCDFD, 104, 6, F8X16,(sys.fanPWM % 10));
		OLED_P8x16Str(LCDFD, 112, 6, F8X16, 18);//%	
		OLED_P8x16Str(LCDFD, 120, 6, F8X16, 19); // " "空字符
		return;
	}
	else if (sys.fanPWM >= 100)
	{
		OLED_P8x16Str(LCDFD, 96, 6, F8X16, (sys.fanPWM / 100));
		OLED_P8x16Str(LCDFD, 104, 6, F8X16, ((sys.fanPWM % 100) / 10));
		OLED_P8x16Str(LCDFD, 112, 6, F8X16, ((sys.fanPWM % 100) % 10));
		OLED_P8x16Str(LCDFD, 120, 6, F8X16, 18);//%
		return;
	}

}
int main(void)
{
	SYSDATA sysData;
	daemon(0, 0);   //将程序设置为daemon模式
	printf("The SYS mornitor is running...\n");
	Init_Pin(); //初始化GPIO   
	LCDFD = GetLCDFD();//猎取液晶操作句柄
	OLED_Init(LCDFD);//初始化液晶
	LCDCheckSelf(LCDFD);

	Draw_BMP(LCDFD, 0, 0, 127, 7, ubuntu);
	sleep(1);
	FanCheckSelf(); //风扇自检
	OLED_CLS(LCDFD);
	while (1)
	{	
		GetCPUloadAVG(&sysData.cpuload_x, &sysData.cpuload_y);
		sysData.bootTimeNow = GetBootTime(); //获取系统启动时间
		sysData.averageValue = GetCPUTempreture(); //获取CPU平均温度
		sysData.fanPWM = FanProg(sysData.averageValue); //根据CPU温度，获取风扇PWM数据
		
		DisplayInfo(LCDFD , sysData); //更新屏显
		FanControl(sysData); //风扇控制
		
		sleep(2);
	}
	printf("The SYS mornitor was stoped !\n");
	return 0;
}
