/* --------------------------------
program:mas2018.12.3
revise date:2018.12.15
function: 
.port initialize
.T8P2 timer
.T8P1 pwm
.sleep
 ----------------------------------*/
#include <hic.h>
#include "mas.h"

static volatile unsigned char bit_use1 @0x0011;
//key press tag
static volatile sbit kp_tag				@(unsigned) &bit_use1 * 8 + 0;//key press tag
static volatile sbit kp_sad_tag			@(unsigned) &bit_use1 * 8 + 1;//key press shake avoid delay tag
static volatile sbit kr_sad_tag			@(unsigned) &bit_use1 * 8 + 2;//key relax shake avoid delay tag
//long press key tag
static volatile sbit klp_tag			@(unsigned) &bit_use1 * 8 + 3;//key long press tag
static volatile sbit kp_count_tag		@(unsigned) &bit_use1 * 8 + 4;
static volatile sbit sw_load_command    @(unsigned) &bit_use1 * 8 + 5;
static volatile sbit charge_tag			@(unsigned) &bit_use1 * 8 + 6;
static volatile sbit sleep_tag			@(unsigned) &bit_use1 * 8 + 7;

static volatile unsigned char bit_use2 @0x0012;
static volatile sbit c_0	@(unsigned) &bit_use2 * 8 + 0;
static volatile sbit c_1	@(unsigned) &bit_use2 * 8 + 1;
static volatile sbit c_2	@(unsigned) &bit_use2 * 8 + 2;

static volatile sbit chrg_sad_tag	@(unsigned) &bit_use2 * 8 + 4;


unsigned int intCount = 0;

unsigned char t_kps = 0;//key press shake avoid delay time
unsigned char t_krs = 0;//key relax shake avoid delay time
unsigned char t_klp = 0;//key long press time
unsigned char t_sd = 0;//sleep delay time
unsigned char t_chrg_sad = 0;
unsigned char kp_count = 0;//key press count

void Ram_Clr(void) //clear ram
{
	_asm 
  { 
	CLR   IAAL;
	CLR   IAAH;
	CLR   IAD;
	INC   IAAL,1;
	JBS   IAAL,6;
	GOTO  $-3;
  } 
}

void port_init(void)
{
	//porta
	PA = 0;
	ANS = 0xff;//0=AN,1=IO
	PAT = 0x38;//0=out, 1=in
	N_PAU = 0xe7;//0=enable, 1=disable
	N_PAD = 0xdf; 
	//portb
	PB = 0;
	PBT = 0x00;
}

void clock_init(void)
{
	OSCP = 0x55;
	OSCC = 0xd0;//4M
	while(!HSOSCF);//wait for clock steady
	_asm{nop;}
	while(!SW_HS);
	_asm{nop;}

	//LPM = 0;//mode idle0
}

void wdt_init(void)
{
	WDTC = 0x17;//clock source=internal RC 32k, perscale=256
	WDTP = 0xff;//overfolow time=256*256/32=2048ms
}

void clrwdt(void)
{
	_asm{cwdt;}
}

void pints_init(void)
{
	INTC0 = 0;//key int. shield
	//PINT2 set
	PINTS = 0x10;//pint2 pa4 int.	
	PEG2= 0;//falling edge trigger
	PIF2 = 0;//clear int. flag
	PIE2 = 1;//pint2 int. enable
}

void kint_init(void)
{
	//key int. set
	INTC0 = 0;//mask all key int.
	KMSK4 = 1;//enable kin4 int.
	KIF = 0;//clear int. flag
	KIE = 1;//key int. enable
}

void pwm_init(void)
{
	//pwm T8P1 set
	pwm_period = 244;//周期
	pwm_set(122);
	/*------T8PnC设置：
	T8PnC.7=1 //pwm模式
	T8PnC[6:3]=0000//后分频
	T8PnC.2=0 //关闭pwm模块
	T8PnC[1:0]=01//预分频
	T8PnC	= 1000 0001
			= 0x81
	 ------------*/
	#ifdef PwmFreq_2K
	T8P1C = 0x81;//Fpwm=2k
	#endif

	#ifdef PwmFreq_0K5
	T8P1C = 0x82;//Fpwm=0.5k
	#endif
	
    T8P1TIE = 0;//禁止T8P1定时中断
	sw_pwm_module = ENABLE;//使能pwm模块
	/*---------T8P1OC设置：(default=0x00)
	T8P1OC.7=1//pwm1输出关闭
	T8P1OC[3:2]=00//PWM10输出关闭
	T8P1OC[1:0]=00//PA1输出PWM11 
	T8P1OC=1000 0000
		  =0x80
	--------------*/
	T8P1OC = 0x80;
	sw_pwm = PWM_OFF;//pwm1输出关闭
}

void timer_init()
{
	GIE = 0;//int. disable
	//timer T8P2 set
    T8P2P = 244;    //设置T8P2P周期寄存器初始值
    T8P2C = 0x79;   //设置T8P2定时器模式，预分频4,后分频16
    T8P2TIF = 0;    //清除T8P2中断标志
    T8P2TIE = 1;	//使能T8P2定时中断
	sw_timer_module = DISABLE;//关闭定时模块
}


void var_init(void)
{
	bit_use1 = 0;
	bit_use2 = 0;

}

void init(void)
{
	Ram_Clr();
	clock_init();
	wdt_init();
	port_init();
	timer_init();
	pwm_init();

	#ifdef PINT_KEY
	pints_init();
	#endif

	#ifdef KINT_KEY
	kint_init();
	#endif

	var_init();

}

void key_check(void)
{
	//key press check
	if(KEY_RELAX == key)
		{
			//kps_tag reset
			kp_sad_tag = 0;
			t_kps = 0;
			//key release ack.
			if(kr_sad_tag)
				kp_tag = 0;		
		}
		else 
			{
				//krs_tag reset
				kr_sad_tag = 0;
				t_krs = 0;
				//key press ack.
				if(kp_sad_tag)
					kp_tag = 1;				
			}
		//key press count
		if(kp_tag && (!kp_count_tag))
			{
				kp_count_tag = 1;
				kp_count++;
				if(kp_count > 4)
					kp_count = 0;						
			}
		//tag reset
		if(!kp_tag)
			{
				//key long press tag reset
				klp_tag = 0;
				t_klp = 0;
				//key press count tag reset
				kp_count_tag = 0;
			}
		//key press count reset
		if(!sw_load_command)
			kp_count = 0;
		//sleep_tag reset
		if(sw_load_command)
			{
				sleep_tag = 0;
				t_sd = 0;
			}
}

void charge_check(void)
{
	//if(chrg)
		//charge_tag = 1;
		//else charge_tag = 0;
	if(chrg_sad_tag)
		charge_tag = 1;
		else charge_tag = 0;
	if(!chrg)
	{
		chrg_sad_tag = 0;
		t_chrg_sad = 0;
	}
}

void moto_op(void)
{   
	if(charge_tag)
		{
			sw_load_command = 0;

		}
	if(sw_load_command)
	{
		switch(kp_count)
		{
			case 0:
			{	
				pwm_set(PWM_LOW);
				sw_pwm = PWM_ON;
			}break;
			case 1:
			{
				pwm_set(PWM_MID);
				sw_pwm = PWM_ON;
			}break;
			case 2:
			{
				pwm_set(PWM_HIG);
				sw_pwm = PWM_ON;
			}break;
			case 3:
			{
				pwm_set(PWM_HIG);
				sw_pwm = c_1;
			}break;
			case 4:
			{
				pwm_set(PWM_MID);
				sw_pwm = c_2;
			}break;
		}
	}
	else 
		{
			pwm_set(PWM_NULL);
			sw_pwm = PWM_OFF;
		}

}

void sleep(void)
{
	if(sleep_tag)
	{
		sw_timer_module = DISABLE;
		sw_pwm_module = DISABLE;
		//sleep_tag reset
		sleep_tag = 0;
		t_sd = 0;
		_asm{nop;}
		clrwdt();
		RCEN = 0;//idel状态下，关闭WDT内部RC时钟
		_asm
		{	
			idle;
			nop;
		}
		RCEN = 1;//开启WDT内部RC时钟
		sw_timer_module = ENABLE;
		sw_pwm_module = ENABLE;
	}
}

void isr(void) interrupt
{
	//pa4 int.
	#ifdef PINT_KEY
    if(PIE2 && PIF2) 
    {
		PIF2 = 0;  		
		//sleep_tag reset
		sleep_tag = 0;
		t_sd = 0;
	}
	#endif

	#ifdef KINT_KEY
	if(KIE && KIF)
	{
		PA4 = 0;
		KIF = 0;  
		//sleep_tag reset
		sleep_tag = 0;
		t_sd = 0;							 
	}
	#endif

	//t8p2 timer_128 int.
	if(T8P2TIE && T8P2TIF)
    {					 //进入T8P1中断
        T8P2TIF = 0;	 //清除T8P1中断标志
		intCount++;		
		//timer 1/4s 		
		if(0 == (intCount & (ONE_4_SECOND_COUNT_MASK)))
		{
			//last time of key long press 
			if(kp_tag && (!klp_tag))
				{
					t_klp++;
					if(t_klp >= TIME_KLP)
						{
							klp_tag = 1;
							sw_load_command = !sw_load_command;
						}
				}	
				
			//sleep delay timer
			if((!sw_load_command) && (!sleep_tag))	
				{
					t_sd++;
					if(t_sd >= TIME_SD)
						sleep_tag = 1;
				}
		}//end 1/4s timer
	//led_disp
	c_0 = LED_OFF;
	if((intCount & LED_FLICKER_COUNT_MASK) < LED_FLICKER_COUNT_END)
		c_0 = LED_ON;
	//pwm pulse
	if((intCount & PULSE1_COUNT_MASK) < PULSE1_COUNT_END)
		c_1 = 1;
		else c_1 = 0;
	if((intCount & PULSE2_COUNT_MASK) < PULSE2_COUNT_END)
		c_2 = 1;
		else c_2 = 0;		
			
	//key press shake avoid delay				
	if((KEY_PRESS == key) && (!kp_sad_tag))
		{			
		   	t_kps++;	//防抖动延时
		   	if(t_kps >= SHAKE_AVOID_DELAY)
		   		kp_sad_tag = 1;
		}
	//key relax shake avoid delay
	if((KEY_RELAX == key) && (!kr_sad_tag))
		{			
		   	t_krs++;	//防抖动延时
		   	if(t_krs >= SHAKE_AVOID_DELAY)
		   		kr_sad_tag = 1;
		}
	//chrg shake avoid delay
	if(chrg && (!chrg_sad_tag))
		{
			t_chrg_sad++;
			if(t_chrg_sad >= TIME_CHRG_SAD)
				chrg_sad_tag = 1;
		}
				     
    }//end t8p2 timer_128 int.

}

void main() 
{
	init();
	GIE = 1;//总中断使能
	sw_timer_module = ENABLE;//开启定时模块
	while(1)
	{
		clrwdt();
		key_check();
		charge_check();
		moto_op();
		sleep();

	}
}