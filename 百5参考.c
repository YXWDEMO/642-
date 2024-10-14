#include "main.h"

bit Smok_Down_EN;
bit Low_Bat_Flag;
u8 CC;
u8 Charge_Vol_offset;

const u16 _Bat_Leve_digit_List[] =
	{
		3350, 3400, 3433, 3458, 3475, 3490, 3503, 3515, 3526, 3537, //+11
		3549, 3559, 3569, 3579, 3589, 3599, 3609, 3619, 3628, 3637, //+10
		3646, 3654, 3661, 3669, 3676, 3683, 3690, 3696, 3701, 3705, //+87654
		3709, 3712, 3715, 3718, 3721, 3724, 3727, 3730, 3733, 3736, //+43
		3739, 3742, 3745, 3748, 3751, 3754, 3757, 3760, 3763, 3766, //+3
		3768, 3771, 3774, 3777, 3780, 3784, 3788, 3793, 3798, 3803, //+345
		3808, 3813, 3819, 3825, 3831, 3837, 3843, 3849, 3856, 3863, //+567
		3870, 3877, 3884, 3891, 3898, 3905, 3912, 3920, 3928, 3936, //+78
		3944, 3952, 3960, 3968, 3976, 3985, 3994, 4004, 4014, 4024, //+8910
		4036, 4048, 4060, 4072, 4090, 4105, 4110, 4120, 4130, 4150, //+910
};

BatInfoTypedef BatInfo;

void BatVoltage(void)
{

	BatInfo.adc_value = GetADCValue(BAT_CHANNEL); // sample vfent
	BatInfo.test_value_1v2 = Adc_CalBatRef();
	BatInfo.Bat_Volt = (u32)BatInfo.test_value_1v2 * 4095ul / BatInfo.adc_value; // 1.2V
}

void Get_Power_Rest_BatLeve(void) //计算Bat_Leve和Bat_Leve_map
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
	if(Bat_Leve%5)
			BatInfo.Bat_Leve_temp = Bat_Leve + (5-Bat_Leve%5);
	else
		BatInfo.Bat_Leve_temp = Bat_Leve;
	BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(BatInfo.Bat_Leve_temp);
}

void Smok_Start_Bat_Leve_Deal(void)
{
	u8 temp;
	if (!BatInfo.Bat_Leve_Delay_Cnt) // 防止快吸
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
	if(Bat_Leve%5)
		BatInfo.Bat_Leve_temp = Bat_Leve + (5-Bat_Leve%5);
	else
		BatInfo.Bat_Leve_temp = Bat_Leve;
	BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(BatInfo.Bat_Leve_temp);
}

void Get_Bat_Leve(void)
{	
	if (OUTLOAD)  //吸烟时
	{
		u8 Smoke_Down_CNT = 0;
		BatInfo.Bat_Leve_Delay_Cnt = 25;
		BatInfo.Bat_Charge_Up_Cnt = 0;
		if(OutInfo.Set_Out_Volt == SET_OUT_VOL1)  Smoke_Down_CNT = 61;
		else Smoke_Down_CNT = 60;
		if (++BatInfo.Smok_Time_Down_Max_Cnt > Smoke_Down_CNT) // 5.0S
		{
			BatInfo.Smok_Time_Down_Max_Cnt = 0;
			Smok_Down_EN = 1;
		}
		if(!Smok_Down_EN)
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
	else  //停止吸烟2.5s后
	{
		BatVoltage();
//		if(Bat_Leve>=20) CC=6;
//		else  CC=5;
		
		if (BatInfo.Usb_In_Flag)  //USBIN
		{
			if (Bat_Leve <= 99)
			{
				if(Bat_Leve < 90)  Charge_Vol_offset = 30;
				else  Charge_Vol_offset = 10;
				if (BatInfo.Bat_Volt > (_Bat_Leve_digit_List[Bat_Leve]+Charge_Vol_offset))
				{
					if (BatInfo.Bat_Charge_Up_Cnt < 100)  //
					{
						BatInfo.Bat_Charge_Up_Cnt++;
					} // 45S
					else
					{
						BatInfo.Bat_Charge_Up_Cnt = 0;
						Bat_Leve++;
						BatInfo.Bat_Low_Lock = 0;
						BatInfo.Smok_Time_Down_Max_Cnt = 0;
						Smok_Down_EN = 0;
					}
				}
			}
			else
			{
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
	}
	if(Bat_Leve%5)
		BatInfo.Bat_Leve_temp = Bat_Leve + (5-Bat_Leve%5);
	else
		BatInfo.Bat_Leve_temp = Bat_Leve;
	BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(BatInfo.Bat_Leve_temp);
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
			OilOrBat_Map = _t_list_digit_list_d0[temp] | _t_list_digit_list_d1[0] ;
		}
		else
		{
			OilOrBat_Map = _t_list_digit_list_d0[temp % 10] | _t_list_digit_list_d1[temp / 10];
		}
	}
	OilOrBat_Map |= DISP_BMP_PERCENT_MASK;
	return OilOrBat_Map;
}
