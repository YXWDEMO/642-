#include "main.h"

bit Smok_Down_EN;
bit Low_Bat_Flag;
u8 Charge_Vol_offset;
const u16 _Bat_Leve_digit_List[] =
	{
		3490, 3530, 3570, 3600, 3610, 3620, 3630, 3640, 3650, 3655, 
		3660, 3665, 3670, 3674, 3678, 3680, 3684, 3688, 3690, 3694, 
		3698, 3700, 3702, 3704, 3708, 3710, 3714, 3718, 3720, 3722, 
		3724, 3728, 3730, 3734, 3738, 3740, 3744, 3748, 3750, 3752, 
		3754, 3758, 3760, 3764, 3768, 3770, 3775, 3780, 3785, 3790, 
		3795, 3800, 3805, 3810, 3815, 3820, 3825, 3830, 3840, 3845, 
		3850, 3855, 3860, 3865, 3870, 3875, 3880, 3885, 3890, 3895, 
		3900, 3905, 3910, 3920, 3924, 3928, 3930, 3935, 3940, 3950, 
		3970, 3980, 3990, 4000, 4010, 4020, 4030, 4040, 4050, 4055, 
		4060, 4065, 4070, 4080, 4090, 4100, 4110, 4120, 4125, 4150,
};

BatInfoTypedef BatInfo;

void BatVoltage(void)
{

	BatInfo.adc_value = GetADCValue(BAT_CHANNEL); // sample vfent
	BatInfo.test_value_1v2 = Adc_CalBatRef();

	BatInfo.Bat_Volt = (u32)BatInfo.test_value_1v2 * 4095ul / BatInfo.adc_value; // 1.2V
}

void Get_Power_Rest_BatLeve(void)
{
	u8 i = 0;

	BatVoltage();
	BatInfo.Bat_Low_Lock = 0;
	if (BatInfo.Bat_Volt <= _Bat_Leve_digit_List[0])
	{
		Bat_Leve = 0;
		BatInfo.Bat_Low_Lock = 1;
	}
	else if (BatInfo.Bat_Volt > _Bat_Leve_digit_List[99])
	{
		Bat_Leve = 100;
	}
	else
	{

		for (i = 1; i < 100; i++)
		{
			if (BatInfo.Bat_Volt < _Bat_Leve_digit_List[i])
			{

				Bat_Leve = i;
				break;
			}
		}
	}

	BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(Bat_Leve);
}

void Smok_Start_Bat_Leve_Deal(void)
{
	u8 temp;
	if (!BatInfo.Bat_Leve_Delay_Cnt) // é˜²æ?¢å¿«å?
	{

		if (Bat_Leve <= 1)
		{
			Bat_Leve = 1;
		}
		else
		{
			temp = (Bat_Leve - 1);

			if (BatInfo.Bat_Volt < _Bat_Leve_digit_List[temp])
			{
				BatInfo.Smok_Time_Down_Max_Cnt = 0;
				Bat_Leve--;
				Smok_Down_EN = 0;
			}
		}
	}
	BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(Bat_Leve);
}

void Get_Bat_Leve(void)
{

	if (OUTLOAD||OUT1LOAD)
	{
		BatInfo.Bat_Leve_Delay_Cnt = 25;
		BatInfo.Usb_Bat_full_Time=0;
		BatInfo.Bat_Charge_Up_Cnt = 0;
		if (++BatInfo.Smok_Time_Down_Max_Cnt > 88) // 5.0S
		{
			BatInfo.Smok_Time_Down_Max_Cnt = 0;
			Smok_Down_EN = 1;
		}

		return;
	}

	if (Smok_Down_EN)
	{
		Smok_Down_EN = 0;
		//		if(!Bat_Need_Charge_Flag)
		//		{
		if (Bat_Leve)
		{
			Bat_Leve--;
		}
		else
		{
			Bat_Leve = 0;
		}
		//		}
	}

	if (BatInfo.Bat_Leve_Delay_Cnt)
	{
		BatInfo.Bat_Leve_Delay_Cnt--;
	}
	else
	{
		BatVoltage();
		if (USBIN)
		{
			if (Bat_Leve <= 99)
			{
				if(Bat_Leve < 90)  Charge_Vol_offset = 30;
				else  Charge_Vol_offset = 10;
				if (BatInfo.Bat_Volt > (_Bat_Leve_digit_List[Bat_Leve]+Charge_Vol_offset))
				{
					if (BatInfo.Bat_Charge_Up_Cnt < 500)
					{
						BatInfo.Bat_Charge_Up_Cnt++;
					} // 45S
					else
					{
						BatInfo.Usb_Bat_full_Time=0;
						BatInfo.Bat_Charge_Up_Cnt = 0;
						Bat_Leve++;
						BatInfo.Bat_Low_Lock = 0;
						BatInfo.Smok_Time_Down_Max_Cnt = 0;
						Smok_Down_EN = 0;
					}
				}
				else if (Bat_Leve == 99)
				{
					BatInfo.Usb_Bat_full_Time++;
						if(BatInfo.Usb_Bat_full_Time==6000)
					{
						BatInfo.Usb_Bat_full_Time=0;
						BatInfo.Bat_Charge_Up_Cnt = 0;
						Bat_Leve = 100;
						BatInfo.Smok_Time_Down_Max_Cnt = 0;
						Smok_Down_EN = 0;
					}
				}
				
			}
			else
			{
				BatInfo.Usb_Bat_full_Time=0;
				BatInfo.Bat_Charge_Up_Cnt = 0;
				Bat_Leve = 100;
				BatInfo.Smok_Time_Down_Max_Cnt = 0;
				Smok_Down_EN = 0;
			}
			if (Bat_Leve >= 100)
			{
				Bat_Leve = 100;
			}
		}
		BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(Bat_Leve);
	}
}
u32 Get_Oil_Or_Bat_Map(u8 OilOrBat_Leve)
{
	// Low_Bat_Flag = 0;
	u32 OilOrBat_Map = 0x00;
	u8 temp = 0;
	temp = OilOrBat_Leve;

	if (temp >= 100)
	{
		OilOrBat_Map = (DISP_DIGIT_1_MASK | _t_list_digit_list_d1[0] | _t_list_digit_list_d0[0]);
	}
	else if (temp < 100)
	{
		if (temp < 10)
		{
			OilOrBat_Map = _t_list_digit_list_d0[temp]| _t_list_digit_list_d1[0];
		}
		else
		{
			OilOrBat_Map = _t_list_digit_list_d0[temp % 10] | _t_list_digit_list_d1[temp / 10];
		}
	}
	OilOrBat_Map |= DISP_BMP_PERCENT_MASK;
	return OilOrBat_Map;
}
