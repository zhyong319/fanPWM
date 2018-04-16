#include<stdio.h>
#include<time.h>
#include<CharacterLib.h>
#include<I2CLCD.h>
#include<GPIO.h>
#include<syszz.h>
#include<stdlib.h>
#include<string.h>
#include<SysError.h>

#define ZONE0 0
#define ZONE1 1
int LCDFD = 0; //保存液晶文件句柄


#define WARN_TEMPRETURE 30
#define PWMLEVEL 30
#define FAN_START_WORK_TEMPRETURE ((PWMLEVEL/2)+WARN_TEMPRETURE)
#define LOW_TEMPRATURE_COUNT 36 //36为3分钟，根据刷新频率计算得出

bool DEBUGMODE = 0;

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
int FanProg(SYSDATA* sys) //计算控制风扇PWM，并返回风扇PWM数据
{	
	int temprature = sys->averageValue;
	static int lowTempratureCounter = 0; //低温时的计数
	int pwmInt=0;
	pwmInt = (temprature - WARN_TEMPRETURE) * 2; //让PWM值在0——100之间，对应的温度为20——70度之间
	if ((temprature < WARN_TEMPRETURE) || (pwmInt < PWMLEVEL))
	{
		lowTempratureCounter++;
		if (lowTempratureCounter > LOW_TEMPRATURE_COUNT)
		{
			pwmInt = 0;
			FanStop();
			lowTempratureCounter = LOW_TEMPRATURE_COUNT+1;
			
		}
		else
		{
			pwmInt = PWMLEVEL;//保持最小转速
		}
	}
	else if (temprature < FAN_START_WORK_TEMPRETURE)
	{
		pwmInt = PWMLEVEL;
		lowTempratureCounter = 0;
	}
	else if (temprature >= FAN_START_WORK_TEMPRETURE)
	{
		lowTempratureCounter = 0;
		if (pwmInt >= PWM_MAX_VALUE)
		{
			pwmInt = PWM_MAX_VALUE;		
		}
	}
	if (DEBUGMODE == true)
	{
		printf("fan PWM=%d , lowTempratureCounter=%d , CPU temprature=%d\n", pwmInt, lowTempratureCounter, temprature);
	}

	sys->pFreeCounter = &lowTempratureCounter; 
	return pwmInt;
}

void DisplayInfo(int fd , SYSDATA sys)
{
	//////系统时间   Count计数
	OLED_P16x16Ch(LCDFD, 0, 0, F16X16, 0);
	OLED_P16x16Ch(LCDFD, 16, 0, F16X16, 1);
	OLED_P8x16Str(LCDFD, 32, 0, F8X16, 11);

	OLED_P8x16Str(LCDFD, 40, 0, F8X16, (sys.pTmNow->tm_hour) / 10);
	OLED_P8x16Str(LCDFD, 48, 0, F8X16, (sys.pTmNow->tm_hour) % 10);
	OLED_P8x16Str(LCDFD, 56, 0, F8X16, 11);
	OLED_P8x16Str(LCDFD, 64, 0, F8X16, (sys.pTmNow->tm_min) / 10);
	OLED_P8x16Str(LCDFD, 72, 0, F8X16, (sys.pTmNow->tm_min) % 10);

	if (*sys.pFreeCounter >= LOW_TEMPRATURE_COUNT)
	{
		OLED_P8x16Str(LCDFD, 112, 0, F8X16, 20);
		OLED_P8x16Str(LCDFD, 120, 0, F8X16, 20);
	}
	else if (*sys.pFreeCounter < 0)
	{
		OLED_P8x16Str(LCDFD, 112, 0, F8X16, 19);
		OLED_P8x16Str(LCDFD, 120, 0, F8X16, 19);
	}
	else
	{
		OLED_P8x16Str(LCDFD, 112, 0, F8X16, (*sys.pFreeCounter) / 10);
		OLED_P8x16Str(LCDFD, 120, 0, F8X16, (*sys.pFreeCounter) % 10);
	}

/*	OLED_P16x16Ch(LCDFD, 16, 0, F16X16, 0);
	OLED_P16x16Ch(LCDFD, 36, 0, F16X16, 1);
	OLED_P16x16Ch(LCDFD, 56, 0, F16X16, 4);
	OLED_P16x16Ch(LCDFD, 76, 0, F16X16, 2);
	OLED_P16x16Ch(LCDFD, 96, 0, F16X16, 3)*/;

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
	
	if ((sys.averageValue < WARN_TEMPRETURE) || ((sys.fanPWM < PWMLEVEL))) //the temprature fall FAN_START_WORK_TEMPRATURE
	{
		OLED_P8x16Str(LCDFD, 96, 6, F8X16, 20); //-
		OLED_P8x16Str(LCDFD, 104, 6, F8X16, 20);//-	
		OLED_P8x16Str(LCDFD, 112, 6, F8X16, 20); //-
		OLED_P8x16Str(LCDFD, 120, 6, F8X16, 20); //-	
		return;
	
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
tm* GetNowTime()
{
	time_t timer;//time_t就是long int 类型
	struct tm *tblock;
	timer = time(NULL);
	tblock = localtime(&timer);
	return tblock;
}

bool IsNumber(char *str) //判断字符数组是否是数据
{
	char *p = str;
	while (*p != '\0')
	{
		if ((*p < '0') || (*p > '9'))
			return false;
		p++;
	}
	return true;
}

int main(int argc, char* argv[])
{

	for (int ii = 0; ii < argc; ii++)
	{

		printf("argv[%d] = %s\n", ii, argv[ii]);
	}
	
	SYSDATA sysData;

	int ParameterInt=0; //保存传入的参数INT
	
	if (argc > 1) //如果程序带参数
	{
		if ( ! IsNumber( argv[1] ) ) //如果参数是字符
		{
			if (0 == strcmp("debug" , argv[1] ) ) //如果参数为DEBUG字符
			{
				DEBUGMODE = true;
				if (IsNumber(argv[2])) //如果第三个参数是数据
				{
					ParameterInt = atoi(argv[2]);
				}
			}
			else if (0 == strcmp("stop", argv[1])) //如果为stop字符
			{
				Init_Pin(); //初始化GPIO   
				LCDFD = GetLCDFD();//猎取液晶操作句柄
				OLED_Init(LCDFD);//初始化液晶
				OLED_CLS(LCDFD); //清屏
				FanStop();
				return EXIT_OK;
			}
			else
			{
				DEBUGMODE = false;
			}
		}
		else if(IsNumber(argv[1])) //如果参数是数据
		{
			ParameterInt = atoi(argv[1]); 

		}
		else //出错
		{
			printf("There is has a error ,system exited!\n\n");
			printf("This app's paremeter can be with DEBUG | 0-100\n");
			return EXIT_PAREMETER_ERROR;
		}
	}

	if (DEBUGMODE == false)
	{
		daemon(0, 0);   //将程序设置为daemon模式  后台守护进程模式
	}
	Init_Pin(); //初始化GPIO   
	LCDFD = GetLCDFD();//猎取液晶操作句柄
	OLED_Init(LCDFD);//初始化液晶
	LCDCheckSelf(LCDFD);
	Draw_BMP(LCDFD, 0, 0, 127, 7, ubuntu); //显示欢迎画面
	sleep(2);
	OLED_CLS(LCDFD); //清屏
	
	while (1)
	{
		sysData.pTmNow = GetNowTime();//获取当时时间
		sysData.bootTimeNow = GetBootTime(); //获取系统启动时间
		GetCPUloadAVG(&sysData.cpuload_x, &sysData.cpuload_y);//计算CPU平均温度
		sysData.averageValue = GetCPUTempreture(); //获取CPU平均温度
		sysData.fanPWM = FanProg(&sysData); //根据CPU温度，获取风扇PWM数据
		if (ParameterInt > 0) sysData.fanPWM = ParameterInt; //如有参数传入，将计算后的PWM重新赋值，风扇转速恒定
		DisplayInfo(LCDFD, sysData); //更新屏显
		FanControl(sysData); //风扇控制
		sleep(5);
	}
	return EXIT_OK;
}
