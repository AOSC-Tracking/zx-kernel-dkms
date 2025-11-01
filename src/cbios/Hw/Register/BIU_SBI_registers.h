/*
* Copyright 1998-2014 VIA Technologies, Inc. All Rights Reserved.
* Copyright 2001-2014 S3 Graphics, Inc. All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sub license,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice (including the
* next paragraph) shall be included in all copies or substantial portions
* of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
* THE AUTHOR(S) OR COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

typedef union _REG_MM8504	//Subsystem_Control_Register
{
    struct
    {
        CBIOS_U32    VSYNC1_INT                     :1;
        CBIOS_U32    ENGINE_BUSY_INT                :1;
        CBIOS_U32    VSYNC2_INT                     :1;
        CBIOS_U32    DIU_BIU_DVO_INT                :1;
        CBIOS_U32    DP2_INT                        :1;
        CBIOS_U32    ENGINE_IDLE_CNTR_EXPIRE_INT    :1;
        CBIOS_U32    DIU_BIU_CEC_INT                :1;
        CBIOS_U32    DP1_INT                        :1;
        CBIOS_U32    CSP_TIMEOUT_INT                :1;
        CBIOS_U32    Reserved_1                     :1;
        CBIOS_U32    Reserved_2                     :1;
        CBIOS_U32    CORRECTABLE_ERR_INT            :1;
        CBIOS_U32    NON_FATAL_ERR_INT              :1;
        CBIOS_U32    FATAL_ERR_INT                  :1;
        CBIOS_U32    UNSUPPORTED_ERR_INT            :1;
        CBIOS_U32    INVALID_CPURW_INT              :1;
        CBIOS_U32    VPP_TIMEOUT_INT                :1;
        CBIOS_U32    SP_TIMEOUT_INT                 :1;
        CBIOS_U32    EXT_PANEL_DETECT_INT           :1;
        CBIOS_U32    GPIO_INT                       :1;
        CBIOS_U32    GOP0_INT                       :1;
        CBIOS_U32    EXT_PANEL2_DETECT_INT          :1;
        CBIOS_U32    HDCP_INT                       :1;
        CBIOS_U32    CHIP_IDLE_CNTR_EXPIRE_INT      :1;
        CBIOS_U32    PECTL_TIMEOUT_INT              :1;
        CBIOS_U32    DAC_SENSE_INT                  :1;
        CBIOS_U32    HDA_CODEC_INT                  :1;
        CBIOS_U32    VCP_TIMEOUT_INT                :1;
        CBIOS_U32    MSVD_TIMEOUT_INT               :1;
        CBIOS_U32    HDA_AUDIO_INT                  :1;
        CBIOS_U32    VSYNC3_INT                     :1;
        CBIOS_U32    Reserved_3                     :1;
    };
    CBIOS_U32    Value;
}REG_MM8504;


typedef union _REG_MM8508	//Command_FIFO_Control_Register
{
    struct
    {
        CBIOS_U32    VSYNC1_INT_EN                     :1;
        CBIOS_U32    ENGINE_BUSY_INT_EN                :1;
        CBIOS_U32    VSYNC2_INT_EN                     :1;
        CBIOS_U32    DIU_BIU_DVO_INT_EN                :1;
        CBIOS_U32    DP2_INT_EN                        :1;
        CBIOS_U32    ENGINE_IDLE_CNTR_EXPIRE_INT_EN    :1;
        CBIOS_U32    DIU_BIU_CEC_INT_EN                :1;
        CBIOS_U32    DP1_INT_EN                        :1;
        CBIOS_U32    CSP_TIMEOUT_INT_EN                :1;
        CBIOS_U32    Reserved_1                        :1;
        CBIOS_U32    Reserved_2                        :1;
        CBIOS_U32    CORRECTABLE_ERR_INT_EN            :1;
        CBIOS_U32    NON_FATAL_ERR_INT_EN              :1;
        CBIOS_U32    FATAL_ERR_INT_EN                  :1;
        CBIOS_U32    UNSUPPORTED_ERR_INT_EN            :1;
        CBIOS_U32    INVALID_CPURW_INT_EN              :1;
        CBIOS_U32    VPP_TIMEOUT_INT_EN                :1;
        CBIOS_U32    SP_TIMEOUT_INT_EN                 :1;
        CBIOS_U32    EXT_PANEL_DETECT_INT_EN           :1;
        CBIOS_U32    GPIO_INT_EN                       :1;
        CBIOS_U32    GOP0_INT_EN                       :1;
        CBIOS_U32    EXT_PANEL2_DETECT_INT_EN          :1;
        CBIOS_U32    HDCP_INT_EN                       :1;
        CBIOS_U32    CHIP_IDLE_CNTR_EXPIRE_INT_EN      :1;
        CBIOS_U32    PECTL_TIMEOUT_INT_EN              :1;
        CBIOS_U32    DAC_SENSE_INT_EN                  :1;
        CBIOS_U32    HDA_CODEC_INT_EN                  :1;
        CBIOS_U32    VCP_TIMEOUT_INT_EN                :1;
        CBIOS_U32    MSVD_TIMEOUT_INT_EN               :1;
        CBIOS_U32    HDA_AUDIO_INT_EN                  :1;
        CBIOS_U32    VSYNC3_INT_EN                     :1;
        CBIOS_U32    Reserved_3                        :1;
    };
    CBIOS_U32    Value;
}REG_MM8508;


typedef union _REG_MM850C    //Wake_Up_and_Advance_Function_Control_Register
{
    struct
    {
        CBIOS_U32    DIU_BIU_HDCPINT_EN_CLR        :1;
        CBIOS_U32    HDCODEC_INT_EN_CLR            :1;
        CBIOS_U32    DP1_INT_EN_CLR                :1;
        CBIOS_U32    DP2_INT_EN_CLR                :1;
        CBIOS_U32    DIU_BIU_CEC1INT_EN_CLR        :1;
        CBIOS_U32    DIU_BIU_CEC2INT_EN_CLR        :1;
        CBIOS_U32    DIU_BIU_TS1_INT_EN_CLR        :1;
        CBIOS_U32    DIU_BIU_TS2_INT_EN_CLR        :1;
        CBIOS_U32    DIU_BIU_HDMIIN_INT_EN_CLR     :1;
        CBIOS_U32    DIU_BIU_HDMIIN5V_INT_EN_CLR   :1;
        CBIOS_U32    DIU_BIU_DVO_INT_EN_CLR        :1;
        CBIOS_U32    DIU_BIU_VX1LOCK_INT_EN_CLR    :1;
        CBIOS_U32    HDA_BIU_INTR_EN_CLR           :1;
        CBIOS_U32    FPVSYNC1_EN_CLR               :1;
        CBIOS_U32    FPVSYNC2_EN_CLR               :1;
        CBIOS_U32    TLM_FENCE_INT_EN_CLR          :1;
        CBIOS_U32    BCI_FLIP_INT_EN_CLR           :1;
        CBIOS_U32    BCI_QWD_INT_EN_CLR            :1;
        CBIOS_U32    ISP_BLS_INT_EN_CLR            :1;
        CBIOS_U32    ISP_CAP0_INT_EN_CLR           :1;
        CBIOS_U32    ISP_CAP1_INT_EN_CLR           :1;
        CBIOS_U32    ISP_CAP2_INT_EN_CLR           :1;
        CBIOS_U32    ISP_A3_INT_EN_CLR             :1;
        CBIOS_U32    ISP_ERR_EN_CLR                :1;
        CBIOS_U32    ISP_MISC_INT_EN_CLR           :1;
        CBIOS_U32    ISP_RESERVED0_INT_EN_CLR      :1;
        CBIOS_U32    ISP_RESERVED1_INT_EN_CLR      :1;
        CBIOS_U32    ISP_RESERVED2_INT_EN_CLR      :1;
        CBIOS_U32    S3VD_INT_EN_CLR               :1;
        CBIOS_U32    DACSENSE_INT1_EN_CLR          :1;
        CBIOS_U32    DACSENSE_INT2_EN_CLR          :1;
        CBIOS_U32    Reserved                      :1;
    };
    CBIOS_U32    Value;
}REG_MM850C;


typedef union _REG_MM8510	//Transaction_Layer/Data_Link_Layer_Status_0_Register
{
	struct
	{
		CBIOS_U32	Reserved_7to0	:8;
		CBIOS_U32	TLR_MEMW_BUSY	:1;
		CBIOS_U32	FB_FIFO_Full	:1;
		CBIOS_U32	FB_FIFO_Empty	:1;
		CBIOS_U32	BIU_MW_FIFO_Empty	:1;
		CBIOS_U32	TLR_RDFF_EMPTY	:1;
		CBIOS_U32	TLR_PHQ_EMPTY	:1;
		CBIOS_U32	TLR_NPHQ_EMPTY	:1;
		CBIOS_U32	TLR_FMTFF_FULL	:1;
		CBIOS_U32	TLM_BusY	:1;
		CBIOS_U32	Reserved_0	:1;
		CBIOS_U32	TLR_BusY	:1;
		CBIOS_U32	Reserved_1	:1;
		CBIOS_U32	Reserved_2	:1;
		CBIOS_U32	Reserved_3	:1;
		CBIOS_U32	Reserved_4	:1;
		CBIOS_U32	CFAIFX_BusY	:1;
		CBIOS_U32	Reserved_5	:1;
		CBIOS_U32	Reserved_6	:1;
		CBIOS_U32	Reserved_7	:6;
	};
	CBIOS_U32	Value;
}REG_MM8510;


typedef union _REG_MM8514	//Transaction_Layer/Data_Link_Layer_Status_1_Register
{
	struct
	{
		CBIOS_U32	PECPL_OSCNT	:7;
		CBIOS_U32	Reserved_0	:1;
		CBIOS_U32	TRANSACTION_OSCNT	:6;
		CBIOS_U32	Reserved_1	:2;
		CBIOS_U32	Reserved_2	:6;
		CBIOS_U32	Reserved_3	:2;
		CBIOS_U32	Reserved_4	:4;
		CBIOS_U32	Reserved_5	:4;
	};
	CBIOS_U32	Value;
}REG_MM8514;


typedef union _REG_MM8524	//CRAx_BIU_Shadow_Software_RESET_Register
{
	struct
	{
		CBIOS_U32	CRA3_C_Software_Reset_register	:8;
		CBIOS_U32	CRA4_C_Software_Reset_register	:8;
		CBIOS_U32	CRA5_C_Software_Reset_register	:8;
		CBIOS_U32	CRA6_C_Software_Reset_register	:8;
	};
	CBIOS_U32	Value;
}REG_MM8524;


typedef union _REG_MM8528	//GTS_Counter_Low_Register
{
	struct
	{
		CBIOS_U32	GTS_low_32_bits	:32;
	};
	CBIOS_U32	Value;
}REG_MM8528;


typedef union _REG_MM852C	//GTS_Counter_High_Register
{
	struct
	{
		CBIOS_U32	GTS_High_32_bits	:32;
	};
	CBIOS_U32	Value;
}REG_MM852C;


typedef union _REG_MM8530	//GTS_Address_0_Register
{
	struct
	{
		CBIOS_U32	GTS_ADDRESS_low	:32;
	};
	CBIOS_U32	Value;
}REG_MM8530;


typedef union _REG_MM8534	//GTS_Address_1_Register
{
	struct
	{
		CBIOS_U32	GTS_ADDRESS_high	:32;
	};
	CBIOS_U32	Value;
}REG_MM8534;


typedef union _REG_MM8538	//Reserved
{
	struct
	{
		CBIOS_U32	Reserved	:32;
	};
	CBIOS_U32	Value;
}REG_MM8538;


typedef union _REG_MM853C	//Stop/Trap_CSP_Control_Register
{
	struct
	{
		CBIOS_U32	issue_Stop_to_CSP	:1;
		CBIOS_U32	Trap_clear	:1;
		CBIOS_U32	Reserved_31to2	:30;
	};
	CBIOS_U32	Value;
}REG_MM853C;


typedef union _REG_MM8540	//TLM_Control_Register
{
	struct
	{
		CBIOS_U32	Reserved12_0	:13;
		CBIOS_U32	REG_INT_USS_EN	:1;
		CBIOS_U32	REG_MULTI_MSI_EN	:1;
		CBIOS_U32	REG_HDA_HI_PRI_EN	:1;
		CBIOS_U32	Reserved_27_16	:12;
		CBIOS_U32	Reserved	:4;
	};
	CBIOS_U32	Value;
}REG_MM8540;


typedef union _REG_MM8544	//Apeture_Table_(APT)_Base_Address_Register
{
	struct
	{
		CBIOS_U32	Reserved_11to0	:12;
		CBIOS_U32	APT_Base_Addr	:18;
		CBIOS_U32	Reserved_31to30	:2;
	};
	CBIOS_U32	Value;
}REG_MM8544;


typedef union _REG_MM8548	//CSP_Interrupt_Status_Register
{
	struct
	{
		CBIOS_U32	MSVD_Int	:1;
		CBIOS_U32	Surface_Fault_Int	:1;
		CBIOS_U32	Page_Fault_Int	:1;
		CBIOS_U32	MXU_Invalid_Address_Fault_Int	:1;
		CBIOS_U32	Hang_Int	:1;
		CBIOS_U32	IGA1_VLine_Int	:1;
		CBIOS_U32	IGA2_VLine_Int	:1;
		CBIOS_U32	IGA1_Vsync_Int	:1;
		CBIOS_U32	IGA2_Vsync_Int	:1;
		CBIOS_U32	Reserved_bit_9	:1;
		CBIOS_U32	Fence_cmd_Int	:1;
		CBIOS_U32	VCP_cmd_Int	:1;
		CBIOS_U32	Dump_cmd_Int	:1;
		CBIOS_U32	Flip_cmd_Int	:1;
		CBIOS_U32	LH_Signal_cmd_Int	:1;
		CBIOS_U32	End_Of_cmd_Int	:1;
		CBIOS_U32	Reserved_16to31	:16;
	};
	CBIOS_U32	Value;
}REG_MM8548;


typedef union _REG_MM854C	//CSP_Interrupt_Enable_Register
{
	struct
	{
		CBIOS_U32	MSVD_INT	:1;
		CBIOS_U32	Surface_Fault_Int	:1;
		CBIOS_U32	Page_Fault_Int	:1;
		CBIOS_U32	MXU_Invalid_Address_Fault_Int	:1;
		CBIOS_U32	Hang_Int	:1;
		CBIOS_U32	IGA1_VLine_Int	:1;
		CBIOS_U32	IGA2_VLine_Int	:1;
		CBIOS_U32	IGA1_Vsync_Int	:1;
		CBIOS_U32	IGA2_Vsync_Int	:1;
		CBIOS_U32	Reserved_bit_9	:1;
		CBIOS_U32	Fence_cmd_Int	:1;
		CBIOS_U32	VCP_cmd_Int	:1;
		CBIOS_U32	Dump_cmd_Int	:1;
		CBIOS_U32	Flip_cmd_Int	:1;
		CBIOS_U32	LH_Signal_cmd_Int	:1;
		CBIOS_U32	End_Of_cmd_Int	:1;
		CBIOS_U32	Reserved	:16;
	};
	CBIOS_U32	Value;
}REG_MM854C;


typedef union _REG_MM8574	//Interrupt_Synchronization_MM8504_Mirror_Register
{
    struct
    {
        CBIOS_U32    VSYNC1_INT                     :1;
        CBIOS_U32    ENGINE_BUSY_INT                :1;
        CBIOS_U32    VSYNC2_INT                     :1;
        CBIOS_U32    DIU_BIU_DVO_INT                :1;
        CBIOS_U32    DP2_INT                        :1;
        CBIOS_U32    ENGINE_IDLE_CNTR_EXPIRE_INT    :1;
        CBIOS_U32    DIU_BIU_CEC_INT                :1;
        CBIOS_U32    DP1_INT                        :1;
        CBIOS_U32    CSP_TIMEOUT_INT                :1;
        CBIOS_U32    Reserved_1                     :1;
        CBIOS_U32    Reserved_2                     :1;
        CBIOS_U32    CORRECTABLE_ERR_INT            :1;
        CBIOS_U32    NON_FATAL_ERR_INT              :1;
        CBIOS_U32    FATAL_ERR_INT                  :1;
        CBIOS_U32    UNSUPPORTED_ERR_INT            :1;
        CBIOS_U32    INVALID_CPURW_INT              :1;
        CBIOS_U32    VPP_TIMEOUT_INT                :1;
        CBIOS_U32    SP_TIMEOUT_INT                 :1;
        CBIOS_U32    EXT_PANEL_DETECT_INT           :1;
        CBIOS_U32    GPIO_INT                       :1;
        CBIOS_U32    GOP0_INT                       :1;
        CBIOS_U32    EXT_PANEL2_DETECT_INT          :1;
        CBIOS_U32    HDCP_INT                       :1;
        CBIOS_U32    CHIP_IDLE_CNTR_EXPIRE_INT      :1;
        CBIOS_U32    PECTL_TIMEOUT_INT              :1;
        CBIOS_U32    DAC_SENSE_INT                  :1;
        CBIOS_U32    HDA_CODEC_INT                  :1;
        CBIOS_U32    VCP_TIMEOUT_INT                :1;
        CBIOS_U32    MSVD_TIMEOUT_INT               :1;
        CBIOS_U32    HDA_AUDIO_INT                  :1;
        CBIOS_U32    VSYNC3_INT                     :1;
        CBIOS_U32    Reserved_3                     :1;
    };
	CBIOS_U32	Value;
}REG_MM8574;


typedef union _REG_MM8578	//Interrupt_Synchronization_MM8548_Mirror_Register
{
	struct
	{
		CBIOS_U32	MSVD_Int	:1;
		CBIOS_U32	Surface_Fault_Int	:1;
		CBIOS_U32	Page_Fault_Int	:1;
		CBIOS_U32	MXU_Invalid_Address_Fault_Int	:1;
		CBIOS_U32	Hang_Int	:1;
		CBIOS_U32	IGA1_VLine_Int	:1;
		CBIOS_U32	IGA2_VLine_Int	:1;
		CBIOS_U32	IGA1_Vsync_Int	:1;
		CBIOS_U32	IGA2_Vsync_Int	:1;
		CBIOS_U32	Reserved_bit_9	:1;
		CBIOS_U32	Fence_cmd_Int	:1;
		CBIOS_U32	VCP_cmd_Int	:1;
		CBIOS_U32	Dump_cmd_Int	:1;
		CBIOS_U32	Flip_cmd_Int	:1;
		CBIOS_U32	LH_Signal_cmd_Int	:1;
		CBIOS_U32	End_Of_cmd_Int	:1;
		CBIOS_U32	Reserved_16to31	:16;
	};
	CBIOS_U32	Value;
}REG_MM8578;


typedef union _REG_MM85B4	//CRBx_C_Shadow_Software_RESET_Register
{
	struct
	{
		CBIOS_U32	CRB4_C_Software_Reset_register	:8;
		CBIOS_U32	CRB5_C_Software_Reset_register	:8;
		CBIOS_U32	CRB6_C_Software_Reset_register	:8;
		CBIOS_U32	CRB7_C_Software_Reset_register	:8;
	};
	CBIOS_U32	Value;
}REG_MM85B4;


typedef union _REG_MM85D0    //EPLL_control_Register
{
    struct
    {
        CBIOS_U32    EPLL_integer_division_setting    :8;
        CBIOS_U32    EPLL_fractional_division_setting    :20;
        CBIOS_U32    EPLL_Clock1_output_division_ratio    :2;
        CBIOS_U32    EPLL_VCO_frequency_range_setting    :1;
        CBIOS_U32    EPLL_enable_Clock1_output    :1;
    };
    CBIOS_U32    Value;
}REG_MM85D0;

typedef union _REG_MM85D0_CHX002    //EPLL_control_Register
{
    struct
    {
        CBIOS_U32    EPLL_M    :8;
        CBIOS_U32    EPLL_N    :20;
        CBIOS_U32    reserved    :2;
        CBIOS_U32    EPLL_Ck2_En    :1;
        CBIOS_U32    EPLL_Ck1_En    :1;
    };
    CBIOS_U32    Value;
}REG_MM85D0_CHX002;


typedef union _REG_MM85D4    //EPLL_control_Register
{
    struct
    {
        CBIOS_U32    EPLL_Power_up_control    :1;
        CBIOS_U32    EPLL_Reset_control    :1;
        CBIOS_U32    Reserved    :30;
    };
    CBIOS_U32    Value;
}REG_MM85D4;


typedef union _REG_MM85D8    //EPLL_control_Register
{
    struct
    {
        CBIOS_U32    EPLL_Charge_Pump_current_selection_setting    :3;
        CBIOS_U32    EPLL_Fractional_function_enable    :1;
        CBIOS_U32    EPLL_Enable_Clock2_output    :1;
        CBIOS_U32    EPLL_Clock2_output_division_ratio    :2;
        CBIOS_U32    Reserved    :25;
    };
    CBIOS_U32    Value;
}REG_MM85D8;

typedef union _REG_MM85D8_CHX002    //EPLL_control_Register
{
    struct
    {
        CBIOS_U32    EPLL_Icpsel    :4;
        CBIOS_U32    EPLL_Fracen    :1;
        CBIOS_U32    EPLL_Ck2_Divn    :3;
        CBIOS_U32    EPLL_Ck1_Divn    :3;
        CBIOS_U32    EPLL_Refdly_Sel    :3;
        CBIOS_U32    EPLL_Fbdly_Sel    :3;
        CBIOS_U32    EPLL_Div_Mode    :1;
        CBIOS_U32    EPLL_Vco_Tune_Abs    :4;
        CBIOS_U32    EPLL_Vco_Tine_Tc    :4;
        CBIOS_U32    Reserved    :6;
    };
    CBIOS_U32    Value;
}REG_MM85D8_CHX002;


typedef union _REG_MM85E0    //IPLL_control_Register
{
    struct
    {
        CBIOS_U32    IPLL_integer_division_setting    :8;
        CBIOS_U32    IPLL_fractional_division_setting    :20;
        CBIOS_U32    IPLL_Clock1_output_division_ratio    :2;
        CBIOS_U32    IPLL_VCO_frequency_range_setting    :1;
        CBIOS_U32    IPLL_enable_Clock1_output    :1;
    };
    CBIOS_U32    Value;
}REG_MM85E0;

typedef union _REG_MM85E0_CHX002    //IPLL_control_Register
{
    struct
    {
        CBIOS_U32    IPLL_M    :8;
        CBIOS_U32    IPLL_N    :20;
        CBIOS_U32    reserved    :2;
        CBIOS_U32    IPLL_Ck2_En    :1;
        CBIOS_U32    IPLL_Ck1_En   :1;
    };
    CBIOS_U32    Value;
}REG_MM85E0_CHX002;


typedef union _REG_MM85E4    //IPLL_control_Register
{
    struct
    {
        CBIOS_U32    IPLL_Power_up_control    :1;
        CBIOS_U32    IPLL_Reset_control    :1;
        CBIOS_U32    Reserved    :30;
    };
    CBIOS_U32    Value;
}REG_MM85E4;


typedef union _REG_MM85E8    //IPLL_control_Register
{
    struct
    {
        CBIOS_U32    IPLL_Charge_Pump_current_selection_setting    :3;
        CBIOS_U32    IPLL_Fractional_function_enable    :1;
        CBIOS_U32    IPLL_Enable_Clock2_output    :1;
        CBIOS_U32    IPLL_Clock2_output_division_ratio    :2;
        CBIOS_U32    Reserved    :25;
    };
    CBIOS_U32    Value;
}REG_MM85E8;

typedef union _REG_MM85E8_CHX002    //IPLL_control_Register
{
    struct
    {
        CBIOS_U32    IPLL_Icpsel    :4;
        CBIOS_U32    IPLL_Fracen    :1;
        CBIOS_U32    IPLL_Ck2_Divn    :3;
        CBIOS_U32    IPLL_Ck1_Divn    :3;
        CBIOS_U32    IPLL_Refdly_Sel    :3;
        CBIOS_U32    IPLL_Fbdly_Sel    :3;
        CBIOS_U32    IPLL_Div_Mode    :1;
        CBIOS_U32    IPLL_Vco_Tune_Abs    :4;
        CBIOS_U32    IPLL_Vco_Tine_Tc    :4;
        CBIOS_U32    Reserved    :6;
    };
    CBIOS_U32    Value;
}REG_MM85E8_CHX002;


typedef union _REG_MM85F0    //VPLL_control_Register
{
    struct
    {
        CBIOS_U32    VPLL_integer_division_setting    :8;
        CBIOS_U32    VPLL_fractional_division_setting    :20;
        CBIOS_U32    VPLL_Clock1_output_division_ratio    :2;
        CBIOS_U32    VPLL_VCO_frequency_range_setting    :1;
        CBIOS_U32    VPLL_enable_Clock1_output    :1;
    };
    CBIOS_U32    Value;
}REG_MM85F0;

typedef union _REG_MM85F0_CHX002    //VPLL_control_Register
{
    struct
    {
        CBIOS_U32    VPLL_M    :8;
        CBIOS_U32    VPLL_N    :20;
        CBIOS_U32    Resered    :2;
        CBIOS_U32    VPLL_Ck2_En    :1;
        CBIOS_U32    VPLL_Ck1_En    :1;
    };
    CBIOS_U32    Value;
}REG_MM85F0_CHX002;


typedef union _REG_MM85F4    //VPLL_control_Register
{
    struct
    {
        CBIOS_U32    VPLL_Power_up_control    :1;
        CBIOS_U32    VPLL_Reset_control    :1;
        CBIOS_U32    Reserved    :30;
    };
    CBIOS_U32    Value;
}REG_MM85F4;


typedef union _REG_MM85F8    //VPLL_control_Register
{
    struct
    {
        CBIOS_U32    VPLL_Charge_Pump_current_selection_setting    :3;
        CBIOS_U32    VPLL_Fractional_function_enable    :1;
        CBIOS_U32    VPLL_Enable_Clock2_output    :1;
        CBIOS_U32    VPLL_Clock2_output_division_ratio    :2;
        CBIOS_U32    Reserved    :25;
    };
    CBIOS_U32    Value;
}REG_MM85F8;

typedef union _REG_MM85F8_CHX002    //VPLL_control_Register
{
    struct
    {
        CBIOS_U32    VPLL_Icpsel    :4;
        CBIOS_U32    VPLL_Fracen    :1;
        CBIOS_U32    VPLL_Ck2_Divn    :3;
        CBIOS_U32    VPLL_Ck1_Divn    :3;
        CBIOS_U32    VPLL_Refdly_Sel    :3;
        CBIOS_U32    VPLL_Fbdly_Sel    :3;
        CBIOS_U32    VPLL_Div_Mode    :1;
        CBIOS_U32    VPLL_Vco_Tune_Abs    :4;
        CBIOS_U32    VPLL_Vco_Tine_Tc    :4;
        CBIOS_U32    Reserved    :6;
    };
    CBIOS_U32    Value;
}REG_MM85F8_CHX002;


