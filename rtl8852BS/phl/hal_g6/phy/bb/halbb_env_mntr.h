/******************************************************************************
 *
 * Copyright(c) 2007 - 2020  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/
#ifndef __HALBB_ENV_MNTR_H__
#define __HALBB_ENV_MNTR_H__

/*--------------------------[Define] ---------------------------------------*/
#define ENV_MNTR_FAIL_BYTE	0xff
#define ENV_MNTR_FAIL_WORD	0xffff
#define ENV_MNTR_FAIL_DWORD	0xffffffff
#define	MAX_ENV_MNTR_TIME	8	/*second*/
#define	MS_TO_4US_RATIO		250
/*NHM*/
#define	RSSI_2_NHM_TH(rssi)	((rssi) << 1) /*NHM_threshold = u(8,1)*/
#define	NHM_TH_2_RSSI(th)	(th >> 1)
#define	NHM_PWR_OFST		20
#define NHM_NOISE_F_TH		60	/*60/2 = 30 = -80 dBm*/
#define NHM_WA_TH		109	/*109 = -1 dBm*/
#define NHM_WA_PWR		26 /*26 - 110 = -84dBm, only for 52A*/
/*FAHM*/
#define	RSSI_2_FAHM_TH(rssi)	((rssi) << 1) /*FAHM_threshold = u(8,1)*/
#define	FAHM_TH_2_RSSI(th)	(th >> 1)
#define	FAHM_PWR_OFST		20
#define FAHM_WA_TH		109	/*109 = -1 dBm*/

/*--------------------------[Enum]------------------------------------------*/
enum ccx_unit {
	CCX_04_US		= 0,	/*4us*/
	CCX_08_US		= 1,	/*8us*/
	CCX_16_US		= 2,	/*16us*/
	CCX_32_US		= 3	/*32us*/
};

enum ccx_func_sel {
	NHM_SEL			= BIT(0),
	CLM_SEL			= BIT(1),
	FAHM_SEL		= BIT(2),
	IFS_CLM_SEL		= BIT(3),
	EDCCA_CLM_SEL		= BIT(4),
};

enum ccx_edcca_opt_bw_idx {
	CCX_EDCCA_BW20_0	= 0,	/*seg0:SC=4*/
	CCX_EDCCA_BW20_1	= 1,	/*seg0:SC=2*/
	CCX_EDCCA_BW20_2	= 2,	/*seg0:SC=1*/
	CCX_EDCCA_BW20_3	= 3,	/*seg0:SC=3*/
	CCX_EDCCA_BW20_4	= 4,	/*seg1:SC=4*/
	CCX_EDCCA_BW20_5	= 5,	/*seg1:SC=2*/
	CCX_EDCCA_BW20_6	= 6,	/*seg1:SC=1*/
	CCX_EDCCA_BW20_7	= 7	/*seg2:SC=3*/
};

/*--------------------------[Structure]-------------------------------------*/

struct bb_env_mntr_cr_info {
	u32 ccx_en;
	u32 ccx_en_m;
	u32 ccx_trig_opt;
	u32 ccx_trig_opt_m;
	u32 ccx_trig;
	u32 ccx_trig_m;
	u32 ccx_edcca_opt;
	u32 ccx_edcca_opt_m;
	u32 clm_unit_idx;
	u32 clm_unit_idx_m;
	u32 clm_opt;
	u32 clm_opt_m;
	u32 clm_period;
	u32 clm_period_m;
	u32 clm_dbg_sel;
	u32 clm_dbg_sel_m;
	u32 clm_cnt;
	u32 clm_cnt_m;
	u32 clm_rdy;
	u32 clm_rdy_m;
	u32 edcca_clm_period;
	u32 edcca_clm_period_m;
	u32 edcca_clm_unit_idx;
	u32 edcca_clm_unit_idx_m;
	u32 edcca_clm_cnt;
	u32 edcca_clm_cnt_m;
	u32 edcca_clm_rdy;
	u32 edcca_clm_rdy_m;
	u32 nhm_en;
	u32 nhm_en_m;
	u32 nhm_method_sel;
	u32 nhm_method_sel_m;
	u32 nhm_period;
	u32 nhm_period_m;
	u32 nhm_unit_idx;
	u32 nhm_unit_idx_m;
	u32 nhm_inclu_cca;
	u32 nhm_inclu_cca_m;
	u32 nhm_th0;
	u32 nhm_th0_m;
	u32 nhm_th1;
	u32 nhm_th1_m;
	u32 nhm_th2;
	u32 nhm_th2_m;
	u32 nhm_th3;
	u32 nhm_th3_m;
	u32 nhm_th4;
	u32 nhm_th4_m;
	u32 nhm_th5;
	u32 nhm_th5_m;
	u32 nhm_th6;
	u32 nhm_th6_m;
	u32 nhm_th7;
	u32 nhm_th7_m;
	u32 nhm_th8;
	u32 nhm_th8_m;
	u32 nhm_th9;
	u32 nhm_th9_m;
	u32 nhm_th10;
	u32 nhm_th10_m;
	u32 nhm_cnt0;
	u32 nhm_cnt0_m;
	u32 nhm_cnt1;
	u32 nhm_cnt1_m;
	u32 nhm_cnt2;
	u32 nhm_cnt2_m;
	u32 nhm_cnt3;
	u32 nhm_cnt3_m;
	u32 nhm_cnt4;
	u32 nhm_cnt4_m;
	u32 nhm_cnt5;
	u32 nhm_cnt5_m;
	u32 nhm_cnt6;
	u32 nhm_cnt6_m;
	u32 nhm_cnt7;
	u32 nhm_cnt7_m;
	u32 nhm_cnt8;
	u32 nhm_cnt8_m;
	u32 nhm_cnt9;
	u32 nhm_cnt9_m;
	u32 nhm_cnt10;
	u32 nhm_cnt10_m;
	u32 nhm_cnt11;
	u32 nhm_cnt11_m;
	u32 nhm_cca_cnt;
	u32 nhm_cca_cnt_m;
	u32 nhm_tx_cnt;
	u32 nhm_tx_cnt_m;
	u32 nhm_idle_cnt;
	u32 nhm_idle_cnt_m;
	u32 nhm_rdy;
	u32 nhm_rdy_m;
	u32 fahm_ofdm_en;
	u32 fahm_ofdm_en_m;
	u32 fahm_cck_en;
	u32 fahm_cck_en_m;
	u32 fahm_numer_opt;
	u32 fahm_numer_opt_m;
	u32 fahm_denom_opt;
	u32 fahm_denom_opt_m;
	u32 fahm_dis_count_each_mpdu;
	u32 fahm_dis_count_each_mpdu_m;
	u32 fahm_period;
	u32 fahm_period_m;
	u32 fahm_unit_idx;
	u32 fahm_unit_idx_m;
	u32 fahm_method_sel;
	u32 fahm_method_sel_m;
	u32 fahm_th0;
	u32 fahm_th0_m;
	u32 fahm_th1;
	u32 fahm_th1_m;
	u32 fahm_th2;
	u32 fahm_th2_m;
	u32 fahm_th3;
	u32 fahm_th3_m;
	u32 fahm_th4;
	u32 fahm_th4_m;
	u32 fahm_th5;
	u32 fahm_th5_m;
	u32 fahm_th6;
	u32 fahm_th6_m;
	u32 fahm_th7;
	u32 fahm_th7_m;
	u32 fahm_th8;
	u32 fahm_th8_m;
	u32 fahm_th9;
	u32 fahm_th9_m;
	u32 fahm_th10;
	u32 fahm_th10_m;
	u32 fahm_cnt0;
	u32 fahm_cnt0_m;
	u32 fahm_cnt1;
	u32 fahm_cnt1_m;
	u32 fahm_cnt2;
	u32 fahm_cnt2_m;
	u32 fahm_cnt3;
	u32 fahm_cnt3_m;
	u32 fahm_cnt4;
	u32 fahm_cnt4_m;
	u32 fahm_cnt5;
	u32 fahm_cnt5_m;
	u32 fahm_cnt6;
	u32 fahm_cnt6_m;
	u32 fahm_cnt7;
	u32 fahm_cnt7_m;
	u32 fahm_cnt8;
	u32 fahm_cnt8_m;
	u32 fahm_cnt9;
	u32 fahm_cnt9_m;
	u32 fahm_cnt10;
	u32 fahm_cnt10_m;
	u32 fahm_cnt11;
	u32 fahm_cnt11_m;
	u32 fahm_denom_cnt;
	u32 fahm_denom_cnt_m;
	u32 fahm_rdy;
	u32 fahm_rdy_m;
	u32 ifs_clm_en;
	u32 ifs_clm_en_m;
	u32 ifs_clm_clr;
	u32 ifs_clm_clr_m;
	u32 ifs_clm_period;
	u32 ifs_clm_period_m;
	u32 ifs_clm_unit_idx;
	u32 ifs_clm_unit_idx_m;
	u32 ifs_t1_en;
	u32 ifs_t1_en_m;
	u32 ifs_t2_en;
	u32 ifs_t2_en_m;
	u32 ifs_t3_en;
	u32 ifs_t3_en_m;
	u32 ifs_t4_en;
	u32 ifs_t4_en_m;
	u32 ifs_t1_th_l;
	u32 ifs_t1_th_l_m;
	u32 ifs_t2_th_l;
	u32 ifs_t2_th_l_m;
	u32 ifs_t3_th_l;
	u32 ifs_t3_th_l_m;
	u32 ifs_t4_th_l;
	u32 ifs_t4_th_l_m;
	u32 ifs_t1_th_h;
	u32 ifs_t1_th_h_m;
	u32 ifs_t2_th_h;
	u32 ifs_t2_th_h_m;
	u32 ifs_t3_th_h;
	u32 ifs_t3_th_h_m;
	u32 ifs_t4_th_h;
	u32 ifs_t4_th_h_m;
	u32 ifs_clm_tx_cnt;
	u32 ifs_clm_tx_cnt_m;
	u32 ifs_clm_edcca_exclu_cca;
	u32 ifs_clm_edcca_exclu_cca_m;
	u32 ifs_clm_cckcca_exclu_fa;
	u32 ifs_clm_cckcca_exclu_fa_m;
	u32 ifs_clm_ofdmcca_exclu_fa;
	u32 ifs_clm_ofdmcca_exclu_fa_m;
	u32 ifs_clm_cck_fa;
	u32 ifs_clm_cck_fa_m;
	u32 ifs_clm_ofdm_fa;
	u32 ifs_clm_ofdm_fa_m;
	u32 ifs_clm_t1_his;
	u32 ifs_clm_t1_his_m;
	u32 ifs_clm_t2_his;
	u32 ifs_clm_t2_his_m;
	u32 ifs_clm_t3_his;
	u32 ifs_clm_t3_his_m;
	u32 ifs_clm_t4_his;
	u32 ifs_clm_t4_his_m;
	u32 ifs_clm_t1_avg;
	u32 ifs_clm_t1_avg_m;
	u32 ifs_clm_t2_avg;
	u32 ifs_clm_t2_avg_m;
	u32 ifs_clm_t3_avg;
	u32 ifs_clm_t3_avg_m;
	u32 ifs_clm_t4_avg;
	u32 ifs_clm_t4_avg_m;
	u32 ifs_clm_t1_cca;
	u32 ifs_clm_t1_cca_m;
	u32 ifs_clm_t2_cca;
	u32 ifs_clm_t2_cca_m;
	u32 ifs_clm_t3_cca;
	u32 ifs_clm_t3_cca_m;
	u32 ifs_clm_t4_cca;
	u32 ifs_clm_t4_cca_m;
	u32 ifs_total_cnt;
	u32 ifs_total_cnt_m;
	u32 ifs_clm_rdy;
	u32 ifs_clm_rdy_m;
};

struct bb_env_mntr_info {
	struct bb_env_mntr_cr_info	bb_env_mntr_cr_i;
	/*sw ctrl*/
	u32				ccx_trigger_time;
	u64				start_time;
	u8				ccx_rpt_stamp;
	u8				ccx_watchdog_result;
	bool				ccx_ongoing;
	u8				ccx_rac_lv;
	bool				ccx_manual_ctrl;
	u8				ccx_pre_rssi;
	u16				clm_mntr_time; /*0~2097 ms*/
	enum clm_application		clm_app;
	u16				nhm_mntr_time; /*0~2097 ms*/
	enum nhm_application		nhm_app;
	u16				ifs_clm_mntr_time; /*0~2097 ms*/
	enum ifs_clm_application	ifs_clm_app;
	u16				fahm_mntr_time; /*0~2097 ms*/
	enum fahm_application		fahm_app;
	u16				edcca_clm_mntr_time; /*0~2097 ms*/
	enum edcca_clm_application	edcca_clm_app;
	/*hw ctrl*/
	u16				ccx_period;
	u8				ccx_unit_idx; /*0/1/2/3 : 4/8/16/32 us*/
	enum ccx_edcca_opt_bw_idx	ccx_edcca_opt_bw_idx;
	enum clm_opt_input		clm_input_opt;
	enum nhm_option_cca_all		nhm_include_cca;
	u8				nhm_th[NHM_TH_NUM];
	u16				ifs_clm_th_l[IFS_CLM_NUM];
	u16				ifs_clm_th_h[IFS_CLM_NUM];
	u8				fahm_numer_opt;
	u8				fahm_denom_opt;
	u8				fahm_th[FAHM_TH_NUM];
	/*hw report*/
	u16			clm_result; /*sample cnt*/
	u16			nhm_result[NHM_RPT_NUM]; /*sample cnt*/
	u16			nhm_tx_cnt; /*sample cnt*/
	u16			nhm_cca_cnt; /*sample cnt*/
	u16			nhm_idle_cnt; /*sample cnt*/
	u16			ifs_clm_tx; /*sample cnt*/
	u16			ifs_clm_edcca_excl_cca; /*sample cnt*/
	u16			ifs_clm_ofdmfa; /*sample cnt*/
	u16			ifs_clm_ofdmcca_excl_fa; /*sample cnt*/
	u16			ifs_clm_cckfa; /*sample cnt*/
	u16			ifs_clm_cckcca_excl_fa; /*sample cnt*/
	u16			ifs_clm_total_ifs; /*cnt*/
	u16			ifs_clm_his[IFS_CLM_NUM]; /*cnt*/
	u16			ifs_clm_avg[IFS_CLM_NUM]; /*sample cnt*/
	u16			ifs_clm_cca[IFS_CLM_NUM]; /*sample cnt*/
	u16			fahm_result[FAHM_RPT_NUM]; /*sample cnt*/
	u16			fahm_denom_result; /*sample cnt*/
	u16			edcca_clm_result; /*sample cnt*/
	/*sw report*/
	u8			clm_ratio; /*percent*/
	u16			nhm_sw_result[NHM_RPT_NUM]; /*sample cnt*/
	u8			nhm_wgt[NHM_RPT_NUM]; /*dBm+110*/
	u8			nhm_rpt[NHM_RPT_NUM]; /*percent*/
	u8			nhm_tx_ratio; /*percent*/
	u8			nhm_cca_ratio; /*percent*/
	u8			nhm_idle_ratio; /*percent*/
	u8			nhm_ratio; /*percent*/
	u16			nhm_result_sum; /*sample cnt*/
	u8			nhm_pwr; /*dBm+110*/
	u8			ifs_clm_tx_ratio; /*percent*/
	u8			ifs_clm_edcca_excl_cca_ratio; /*percent*/
	u8			ifs_clm_cck_fa_ratio; /*percent*/
	u8			ifs_clm_ofdm_fa_ratio; /*percent*/
	u8			ifs_clm_cck_cca_excl_fa_ratio; /*percent*/
	u8			ifs_clm_ofdm_cca_excl_fa_ratio; /*percent*/
	u16			ifs_clm_cck_fa_permil; /*permil*/
	u16			ifs_clm_ofdm_fa_permil; /*permil*/
	u32			ifs_clm_ifs_avg[IFS_CLM_NUM]; /*us*/
	u32			ifs_clm_cca_avg[IFS_CLM_NUM]; /*us*/
	u16			fahm_sw_result[FAHM_RPT_NUM]; /*sample cnt*/
	u8			fahm_wgt[FAHM_RPT_NUM]; /*dBm+110*/
	u8			fahm_rpt[FAHM_RPT_NUM]; /*percent*/
	u16			fahm_result_sum; /*sample cnt*/
	u8			fahm_ratio; /*percent*/
	u8			fahm_denom_ratio; /*percent*/
	u8			fahm_pwr; /*dBm+110*/
	u8			edcca_clm_ratio; /*percent*/
	u8			edcca_noise_bg; /*dBm+110, only for 52A*/
	struct env_mntr_rpt	env_mntr_rpt_bg;
	struct ccx_para_info	ccx_para_info_bg;
	u16			idle_pwr_physts; /*RSSI u(16,3) Idle time pwr from physts*/
};

/*--------------------------[Prptotype]-------------------------------------*/
struct bb_info;

u32 halbb_ccx_idx_cnt_2_us(struct bb_info *bb, u16 idx_cnt);

#ifdef CLM_SUPPORT
void halbb_clm_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		   char *output, u32 *_out_len);
#endif
#ifdef NHM_SUPPORT
void halbb_nhm_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		   char *output, u32 *_out_len);
#endif
#ifdef IFS_CLM_SUPPORT
void halbb_ifs_clm_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		       char *output, u32 *_out_len);
#endif
#ifdef FAHM_SUPPORT
void halbb_fahm_dbg(struct bb_info *bb, char input[][16], u32 *_used,
		    char *output, u32 *_out_len);
#endif
#ifdef EDCCA_CLM_SUPPORT
void halbb_edcca_clm_dbg(struct bb_info *bb, char input[][16], u32 *_used,
			 char *output, u32 *_out_len);
#endif
void halbb_env_mntr_log(struct bb_info *bb, u32 dbg_comp);
void halbb_idle_time_pwr_physts(struct bb_info *bb, struct physts_rxd *desc, bool is_cck_rate);
void halbb_env_mntr(struct bb_info *bb);
void halbb_env_mntr_init(struct bb_info *bb);
void halbb_env_mntr_dbg(struct bb_info *bb, char input[][16], u32 *_used,
			char *output, u32 *_out_len);
void halbb_cr_cfg_env_mntr_init(struct bb_info *bb);
#endif
