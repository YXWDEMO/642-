#include "main.h"



ScanInfoTypedef  ScanInfo;

bit  Mic_Out_EN;
bit	Key_Out_EN;
bit	Key_Numadd_EN;
u16	Key_Long_Time_Cnt;

void    Get_Usb_Ad(void)
{
	
   BatVoltage();
   BatInfo.Usb_Ad_Volt = GetADCValue(USB_CHANNEL);
	 
   BatInfo.Usb_Ad_Volt =  (u32)BatInfo.Bat_Volt*BatInfo.Usb_Ad_Volt>>12;

}

void   Out_Stop(void)
{

	if(OUTLOAD)
	{
		Smok_Disp_Led_Cnt = 0;		
		Mic_Out_EN = 0;		
		OutInfo.Out_Mos_Interrup_Ctr_Flag = 0;
		OutInfo.Out_Mos_Interrup_Ctr_Flag1 = 0;
		CLR_OUTSTART;CLR_OUTLOAD;
		MOS_OFF;
		MOS1_OFF;
		BatInfo.Bat_Leve_Data = Get_Oil_Or_Bat_Map(Bat_Leve);
		Get_Oil_Leve();		
		Disp_Smok_Stop;
		ClrShortData();
	}

}

void Power_Reset_Port_Scan(void)
{
    if(MIC_IO)
    SET_MICSTART;

		ScanInfo.PortScan_Old  = ScanInfo.PortScan_New;
}

void    Port_Scan(void)
{

//	  if(SysInfo.Power_Rest){return;}


	if(MIC_IO)
		SET_MICSTART;
	else
		CLR_MICSTART;

#if 0
	if(USB_IO)
		SET_USBSTART;
	else
		CLR_USBSTART;			
#else
	Get_Usb_Ad();
	if(BatInfo.Usb_Ad_Volt >= 700)	
		SET_USBSTART;
	else
		CLR_USBSTART;		
#endif

#ifdef KEY_USE
		if(!KEY_IO)
			SET_KEYSTART;
		else
			CLR_KEYSTART;
#endif

	if(ScanInfo.PortScan_New != ScanInfo.PortScan_Old)
	{
		if(ScanInfo.PortScan_Cnt < 3){ScanInfo.PortScan_Cnt++;}
		else
		{

			if(MICSTART&&!MICSTART_OLD)
			{
				SET_OUTSTART;
				Mic_Out_EN = 1;
			}
			if(!MICSTART&&MICSTART_OLD) 
			{
				CLR_OUTSTART;
				Mic_Out_EN = 0;				
				Out_Stop(); 
			}

#ifdef KEY_USE	
			if(KEYSTART&&!KEYSTART_OLD)
			{
				if (!OUTLOAD&&Dispinfo.Disp_Task!=PowerOnReset&&Dispinfo.Disp_Task!=UsbIn&&Dispinfo.Disp_Task!=LowVol&&Dispinfo.Disp_Task!=LowRes&&Dispinfo.Disp_Task!=OverTime&&Dispinfo.Disp_Task!=Dis_Ovp&&Dispinfo.Disp_Task!=HighRes)
				{
					Dispinfo.Disp_Task = change_gear;
					Smg_Paoma_En=0;
					Key_Out_EN = 1;
					ScanInfo.Clr_Key_Num_Cnt = 0;
				}
			}
			if(!KEYSTART&&KEYSTART_OLD)
			{
				Key_Out_EN = 0;
				if(Key_Numadd_EN&&Dispinfo.Disp_Task == change_gear)
				{
					ScanInfo.Key_Num++;
				} 
				else {
					Key_Numadd_EN=1;
				}
				ScanInfo.Clr_Key_Num_Cnt = 0;
								
			}

#endif

			if(USBSTART&&!USBSTART_OLD)
			{
				if(BatInfo.Ovp_En == 1){Disp_Usb_In;}
				// DualReverseDet();	
				SET_USBIN;
				BatInfo.Bat_Charge_Up_Cnt = 0;
				BatInfo.Usb_In_Flag = 1;
			}

			if(!USBSTART&&USBSTART_OLD) 
			{
				CLR_USBIN;
				BatInfo.Usb_Full_Flag = 0;								
				if(BatInfo.Ovp_En == 0){Disp_Usb_Out;}
				BatInfo.Ovp_En = 0;
						BatInfo.Usb_In_Flag = 0;			  
			}

				if(HEATSTART&&!HEATSTART_OLD) {SET_HEATIN;}
					if(!HEATSTART&&HEATSTART_OLD) {CLR_HEATIN;}
	
				ScanInfo.PortScan_Cnt = 0;
				ScanInfo.PortScan_Old  = ScanInfo.PortScan_New;
				}
		}
		else{  ScanInfo.PortScan_Cnt = 0;}
			if (Key_Out_EN)
			{
				if(++Key_Long_Time_Cnt > 2650)//
				{
				Dispinfo.Disp_EN=0;
				Key_Out_EN = 0;
				Dispinfo.Disp_Task = PowerOff;
				}
			}
			else
			{
				Key_Long_Time_Cnt = 0;
			}
	if(ScanInfo.Clr_Key_Num_Cnt){ScanInfo.Clr_Key_Num_Cnt--;}
	else if(Dispinfo.Disp_Task == change_gear)
	{ 
		switch (ScanInfo.Key_Num)
		{
		case 0:
			OutInfo.Out_Gear_Leve=0;//22
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
			break;
		case 1:
			OutInfo.Out_Gear_Leve=1;//10
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
			break;
		case 2:
			OutInfo.Out_Gear_Leve=2;//11
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
			break;
		case 3:
			OutInfo.Out_Gear_Leve=3;//12
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
			break;
		case 4:
			OutInfo.Out_Gear_Leve=4;//13
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
			break;
		case 5:
			OutInfo.Out_Gear_Leve=5;//14
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
			break;
		case 6:
			OutInfo.Out_Gear_Leve=6;//15
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 7:
			OutInfo.Out_Gear_Leve=7;//16
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 8:
			OutInfo.Out_Gear_Leve=8;//17
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 9:
			OutInfo.Out_Gear_Leve=9;//18
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 10:
			OutInfo.Out_Gear_Leve=10;//19
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 11:
			OutInfo.Out_Gear_Leve=11;//20
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 12:
			OutInfo.Out_Gear_Leve=12;//21
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		case 13:
			ScanInfo.Key_Num = 0;
			// 手动执行 case 0 的操作
			OutInfo.Out_Gear_Leve=0;//22
			Set_Disp_Num(1,0,300,0x00,_t_gear_list[ScanInfo.Key_Num]|c_bit_27);
		break;
		default:
			break;
		}
		if((!KEYSTART_OLD))//松开
		{
			Dispinfo.Disp_Task = Idle;
		}		
	}
}
void Res_Deal()
{
	u8   i = 0;
	OutInfo.Out_High_Res_Cnt = 0;
		// 第一路
	i = 50;
	MOS_ON;
	while(i){i--;}
	OutInfo.Out_Adc_Volt = GetADCValue(OUT_CHANNEL);
	MOS_OFF;
		if(OutInfo.Out_Adc_Volt > 4080)
	{
		OutInfo.Out_High_Res_Cnt++;
	}
	//  延时再次采样，排除ADC异常值
	i = 250;
	while(i){i--;}
	i = 50;
	MOS_ON;
	while(i){i--;}
	OutInfo.Out_Adc_Volt = GetADCValue(OUT_CHANNEL);
	MOS_OFF;
	if(OutInfo.Out_Adc_Volt > 4080)
	{
		OutInfo.Out_High_Res_Cnt++;
	}
	if(OutInfo.Out_High_Res_Cnt >= 2)
	{
		SysInfo.Res_Hight=1;
		// 开路
		OutInfo.Out_High_Res_Cnt = 0;
	}
	OutInfo.Out_High_Res_Cnt = 0;
	// 第二路
	i = 50;
	MOS1_ON;
	while(i){i--;}
	OutInfo.Out_Adc_Volt = GetADCValue(OUT1_CHANNEL);
	MOS1_OFF;
	if(OutInfo.Out_Adc_Volt > 4080)
	{
		OutInfo.Out_High_Res_Cnt++;
	}
	//  延时再次采样，排除ADC异常值
	i = 250;
	while(i){i--;}
	//第二路
	i = 50;
	MOS1_ON;
	while(i){i--;}
	OutInfo.Out_Adc_Volt = GetADCValue(OUT1_CHANNEL);
	MOS1_OFF;
	if(OutInfo.Out_Adc_Volt > 4080)
	{
		OutInfo.Out_High_Res_Cnt++;
	}
	if(OutInfo.Out_High_Res_Cnt >= 2)
	{
		SysInfo.Res_Hight=1;
	}
}

void DualReverseDet(void)			// todo: 发热丝 反接保护 检测
{
	u8   i = 0;
	u8 DualDiff = 0;
	u8 error=0;
	u16 AD1_tmp = 0,AD2_tmp = 0;
	if(!SysInfo.Usb_First_Deal)
	{
		return;
	}
	Res_Deal();
	if(SysInfo.Res_Hight)
	{
		return;
	}
	i = 50;
	MOS_ON;
	while(i){i--;}
	OutInfo.Out_Adc_Volt = GetADCValue(OUT_CHANNEL);
	OutInfo.Out_Adc_Volt1 = GetADCValue(OUT1_CHANNEL);
	MOS_OFF;
	AD1_tmp=OutInfo.Out_Adc_Volt;
	if(OutInfo.Out_Adc_Volt1>3000)
	{
		error=1;
	}
	i = 50;
	MOS1_ON;
	while(i){i--;}
	OutInfo.Out_Adc_Volt = GetADCValue(OUT_CHANNEL);
	OutInfo.Out_Adc_Volt1 = GetADCValue(OUT1_CHANNEL);
	MOS1_OFF;
	AD2_tmp=OutInfo.Out_Adc_Volt1;
	if(OutInfo.Out_Adc_Volt>3000)
	{
		error=1;
	}
	ClrShortData();
	DualDiff = AD1_tmp>AD2_tmp ? (AD1_tmp-AD2_tmp) : (AD2_tmp-AD1_tmp);
	if(DualDiff > 30 || error)
	{
		SysInfo.Res_Reverse=1;
	}
	SysInfo.Usb_First_Deal=0;
}
void ShipModeDet()
{
	#if 0
	if(SysInfo.ShipModeTimeCnt > (SysInfo.FlagShipMode_1st ? 35:10))			// todo: 测试逻辑，计时缩小X倍	
	#else
	if(SysInfo.ShipModeTimeCnt > (SysInfo.FlagShipMode_1st ? 352492:337500))				// todo: 首次船运100小时，计数周期1S ，二次船运96小时
	#endif
	{
		SysInfo.FlagShipModeActive = 1;
		SysInfo.FlagShipMode_1st = 0;
		OutInfo.Out_Gear_Leve=3;
		ScanInfo.Key_Num=3;
	}
	else{
		SysInfo.FlagShipModeActive = 0;
	}
}
