#include "main.h"

bit Smg_Paoma_En;
u16 Disp_Ficker;
u32 Smok_Disp_Data;
u32 Smok_Disp_Data1;
u16 Smok_Disp_Led_Cnt;
bit Flag_Disp_Read_Flag;
u32 Temp_Disp_Data;
u32 Temp_Disp_Data1;
u16 Smok_time;
u16 Paoma_Time;
u16 Paoma_Cnt;
bit Data2_EN;
#define DELAY_COEFFICIENT 1 // �ӳ�ϵ��

#define F_CPU 24000000UL // 24 MHz

#define LED_H                         \
	{                                 \
		GPIOA->BSRR |= LL_GPIO_PIN_0; \
	}
#define LED_L                        \
	{                                \
		GPIOA->BRR |= LL_GPIO_PIN_0; \
	} //  LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_3);}

// һ���� 360NS
#define _0_CODE \
	{           \
		LED_H;  \
		LED_L;  \
	}
#define _1_CODE \
	{           \
		LED_H;  \
		LED_H;  \
		LED_L;  \
	}
// ����
// ʮλ
const u16 _t_list_digit_list_d1[] =
	{
		0x3F,
		0x6,
		0x5B,
		0x4F,
		0x66,
		0x6D,
		0x7D,
		0x7,
		0x7F,
		0x6F,
		0,
};
// ��λ
const u32 _t_list_digit_list_d0[] =
	{
		0x3F00,
		0x600,
		0x5B00,
		0x4F00,
		0x6600,
		0x6D00,
		0x7D00,
		0x700,
		0x7F00,
		0x6F00,

		0,
};
u16 _t_gear_list[13];
const u16 _t_list_paoma[] =
	{
		0,
};

void delay(u16 us)
{
	while (us--)
		; // 500ns
}

Dispinfotypedef Dispinfo;

void Set_Disp_Data(u32 Temp,u32 Temp1)
{
    Flag_Disp_Read_Flag = 0;
    Temp_Disp_Data  = Temp;
    Temp_Disp_Data1 = Temp1;
    Flag_Disp_Read_Flag = 1;
}

void   Set_Disp_Num(u8 num,u16 low,u16 high,u32 data1,u32 data2)
{
	CHRG_ENABLE;
	OutInfo.Out_Mos_Interrup_Ctr_Flag = 0;
	OutInfo.Out_Mos_Interrup_Ctr_Flag1 = 0;
	CLR_OUTSTART;CLR_OUTLOAD;
	CLR1_OUTSTART;CLR1_OUTLOAD;
	MOS_OFF;
	MOS1_OFF;
	Dispinfo.Disp_Time = 0;
	Dispinfo.Disp_Num = num;
	Dispinfo.Disp_Low_Time = low;
	Dispinfo.Disp_High_Time = high;
	//Dispinfo.Disp_Data1  = data1;
	//Dispinfo.Disp_Data2 = data2;
	Set_Disp_Data(data1,data2);	
	Smg_Paoma_En = 0;
	// Dispinfo.Smg_Paoma_Cnt = 0;
	// Dispinfo.Smg_Paoma_Time = 0; 
	ClrShortData();
}

void DISP_Deal(void)
{
	if(Dispinfo.Disp_Num)
	{
		if(++Dispinfo.Disp_Time <= Dispinfo.Disp_Low_Time) 
		{
			Dispinfo.Disp_EN = 0; 
		}
		else if(Dispinfo.Disp_Time > Dispinfo.Disp_High_Time )
		{
			Dispinfo.Disp_Time = 0;
			Dispinfo.Disp_Num--;
			if(!Dispinfo.Disp_Num)
			{
				if(Dispinfo.Disp_Task == PowerOnReset)
				{
					SysInfo.Sleep_Time = 5;
					Dispinfo.Disp_Task = Idle;
					
				}
				else
				{
					Dispinfo.Disp_Task = Idle;
				}
				Dispinfo.Disp_EN = 0;
			}
		}
		else
		{
			Dispinfo.Disp_EN = 1;
		}
	}
	else if (OUTLOAD||OUT1LOAD)
	{
		Smok_Disp_Deal();
	}
	else if (USBIN)
	{
		if (!BatInfo.Ovp_En)
		{
			if (BatInfo.Usb_Ad_Volt >= 600)
			{
				// if (CHRG_IO)
				// {
					if (++BatInfo.OvpOn_Cnt >= 50)
					{
						CHRG_DISABLE;
						BatInfo.OvpOn_Cnt = 0;
						BatInfo.Ovp_En = 1;
						Disp_Ovp_EN;
						Dispinfo.Disp_EN = 0;
					}
					return;
				// }
			}
		}
		else
		{
			if (BatInfo.Usb_Ad_Volt <= 570)
			{
				// if (!CHRG_IO)
				// {
					if (++BatInfo.OvpOff_Cnt >= 50)
					{
						CHRG_ENABLE;
						BatInfo.OvpOff_Cnt = 0;
						BatInfo.Ovp_En = 0;
						Dispinfo.Disp_EN = 1;
					}
				// }
			}
			return;
		}
		// ���������USB,������ʾ����
		if (BatInfo.Usb_In_Flag)
		{
			BatVoltage();
			// ����USB,����Ƿ���������źŻ��ѹ����?4.12V
			if (BatInfo.Bat_Volt >= 4120)
			{
				if (++BatInfo.Usb_Full_Cnt >= 20) // 200ms
				{
					BatInfo.Usb_In_Flag = 0;
					Bat_Leve = 100;
					BatInfo.Usb_Full_Flag = 1;
					BatInfo.Usb_Disp_Long_Cnt = 2000; // ������10s���е�Ϩ��
					// Set_Disp_Data(0,0);
					// Dispinfo.Disp_EN = 0;
				}
			}
			else
			{
				BatInfo.Usb_In_Flag = 0;
				BatInfo.Usb_Full_Cnt = 0;
			}
		}
		if (!BatInfo.Usb_Full_Flag)
		{
			BatVoltage();
			if ((BatInfo.Bat_Volt >= 4150&&Bat_Leve>=99)||Bat_Leve == 100)
			{
				if (++BatInfo.Usb_Full_Cnt >= 300) // 3s
				{
					Bat_Leve = 100;
					BatInfo.Usb_Full_Flag = 1;
					BatInfo.Usb_Disp_Long_Cnt = 2000; 
				}
			}
			else
			{
				BatInfo.Usb_Full_Cnt = 0;
			}
			if (++BatInfo.Usb_Led_Twinkle_Cnt < 50)
			{
				if(BatInfo.Usb_Led_Twinkle_Cnt>25)
				{
					Set_Disp_Data(BatInfo.Bat_Leve_Data|DISP_Bat_MASK,0);
				}
				else if(BatInfo.Usb_Led_Twinkle_Cnt<=25)
				{
					Set_Disp_Data(BatInfo.Bat_Leve_Data,0);
				}
				
			}
			else
			{
				BatInfo.Usb_Led_Twinkle_Cnt=0;
			}
			Dispinfo.Disp_EN = 1;
		}
		else
		{
			Set_Disp_Data(BatInfo.Bat_Leve_Data|DISP_Bat_MASK,0);
			Dispinfo.Disp_EN = 1;
		}
	}
}
void   Disp_Get_Oil_Mask(void)
{
	if(Out_Oil_Time>=16750)
		{
			Dispinfo.Disp_choose_Oil_Mask=c_bit_19|c_bit_20|c_bit_21|c_bit_22;
		}
		else if(Out_Oil_Time<16750&&Out_Oil_Time>=11500) {
			Dispinfo.Disp_choose_Oil_Mask=c_bit_20|c_bit_21|c_bit_22;
		}
		else if(Out_Oil_Time<11500&&Out_Oil_Time>=6250) {
			Dispinfo.Disp_choose_Oil_Mask=c_bit_21|c_bit_22;
		}
		else if(Out_Oil_Time<6250&&Out_Oil_Time>=0) {
			Dispinfo.Disp_choose_Oil_Mask=c_bit_22;
		}
	if(Out1_Oil_Time>=16750)
		{
			Dispinfo.Disp_choose_Oil1_Mask=c_bit_19|c_bit_20|c_bit_21|c_bit_22;
		}
		else if(Out1_Oil_Time<16750&&Out1_Oil_Time>=11500) {
			Dispinfo.Disp_choose_Oil1_Mask=c_bit_20|c_bit_21|c_bit_22;
		}
		else if(Out1_Oil_Time<11500&&Out1_Oil_Time>=6250) {
			Dispinfo.Disp_choose_Oil1_Mask=c_bit_21|c_bit_22;
		}
		else if(Out1_Oil_Time<6250&&Out1_Oil_Time>=0) {
			Dispinfo.Disp_choose_Oil1_Mask=c_bit_22;
		}
		Dispinfo.Disp_choose_Oil1_Mask=Dispinfo.Disp_choose_Oil1_Mask;
		Dispinfo.Disp_choose_Oil_Mask=Dispinfo.Disp_choose_Oil_Mask;
}
void Smok_Disp_Deal(void)
{
	if (Smg_Paoma_En)
	{
		if (OUTLOAD||OUT1LOAD)
		{
			Disp_Get_Oil_Mask();
			if(OutInfo.Out_Mos_Interrup_Ctr_Flag==1)
			{
				if(Out_Oil_Time)
				{
					Set_Disp_Data(Get_Oil_Or_Bat_Map(Bat_Leve)|Dispinfo.Disp_choose_Oil_Mask|c_bit_18,0);
				}
				else{
					if(Dispinfo.Smg_Paoma_Time % 50 == 0)
					{
						Set_Disp_Data(Get_Oil_Or_Bat_Map(Bat_Leve)|Dispinfo.Disp_choose_Oil_Mask|c_bit_18,0);
					}
					else if(Dispinfo.Smg_Paoma_Time%50==25){
						Set_Disp_Data(Get_Oil_Or_Bat_Map(Bat_Leve)|c_bit_18,0);
					}
				}
			}
			else if(OutInfo.Out_Mos_Interrup_Ctr_Flag1==1)
			{
				if(Out1_Oil_Time)
				{
					Set_Disp_Data(Get_Oil_Or_Bat_Map(Bat_Leve)|Dispinfo.Disp_choose_Oil1_Mask|c_bit_18,0);
				}
				else
				{
					if(Dispinfo.Smg_Paoma_Time % 50 == 0)
					{
						Set_Disp_Data(Get_Oil_Or_Bat_Map(Bat_Leve)|Dispinfo.Disp_choose_Oil1_Mask|c_bit_18,0);
					}
					else if(Dispinfo.Smg_Paoma_Time%50==25){
						Set_Disp_Data(Get_Oil_Or_Bat_Map(Bat_Leve)|c_bit_18,0);
					}
				}
			}
			if(++Dispinfo.Smg_Paoma_Time > 50)
				{
					Dispinfo.Smg_Paoma_Time=0;
				}
			
		}
		Dispinfo.Disp_EN = 1;
	} 
	else
	{
		// if(Dispinfo.Smg_Paoma_Time>100)
		// {
		// 	Dispinfo.Smg_Paoma_Time=100;
		// }
		Dispinfo.Smg_Paoma_Time=0;
		OutInfo.Smok_Stop_Step = 0;
		// OutInfo.Smok_Stop_Step_Time = 0;
		Dispinfo.Oil_Flash=1;
		Smg_Paoma_En = 0;
		// Dispinfo.Smg_Paoma_Cnt = 0;
		// Dispinfo.Smg_Paoma_Time = 0;
	}
}

void   Breath_Deal(void)
{
	if (Dispinfo.Breath_EN)
		{
			if (Dispinfo.Breath_Cycle < 100)
			{
				Dispinfo.Breath_Cycle++;
			}
			else
			{
				Dispinfo.Breath_Cycle = 20;

				if (Dispinfo.Breath_Lm == 0)
				{
					// ����
					if (Dispinfo.Breath_Duty < 200)
					{
						Dispinfo.Breath_Duty++;
					}
					else
					{
						Dispinfo.Breath_Lm = 1; // �л�������״̬
					}
				}
				else
				{
					// ����
					if (Dispinfo.Breath_Duty > 0)
					{
						Dispinfo.Breath_Duty--;
					}
					else
					{
						Dispinfo.Breath_Lm = 0; // �л�������״̬
					}
				}
			}
			Dispinfo.Disp_EN = 1;
		}
}

