/* arch/arm/mach-msm/board-valente_wx-gpio.c
 * Copyright (C) 2011 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#include <mach/gpiomux.h>
#include "board-valente_wx.h"

static struct gpiomux_setting gsbi3 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

#if 0	/* active in cam_settings */
static struct gpiomux_setting gsbi4 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};
#endif

static struct gpiomux_setting gsbi8 = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config valente_wx_gsbi_configs[] __initdata = {
	{
		.gpio      = VALENTE_WX_TP_I2C_SDA,	/* GSBI3 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3,
		},
	},
	{
		.gpio      = VALENTE_WX_TP_I2C_SCL,	/* GSBI3 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi3,
		},
	},
#if 0	/* active in cam_settings */
	{
		.gpio      = VALENTE_WX_CAM_I2C_SDA,	/* GSBI4 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi4,
		},
	},
	{
		.gpio      = VALENTE_WX_CAM_I2C_SCL,	/* GSBI4 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi4,
		},
	},
#endif
	{
		.gpio	   = VALENTE_WX_MC_I2C_SDA,	/* GSBI8 I2C QUP SDA */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi8,
		},
	},
	{
		.gpio	   = VALENTE_WX_MC_I2C_SCL,	/* GSBI8 I2C QUP SCL */
		.settings = {
			[GPIOMUX_SUSPENDED] = &gsbi8,
		},
	},
};

static struct gpiomux_setting cdc_mclk = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config valente_wx_audio_codec_configs[] __initdata = {
	{
		.gpio = VALENTE_WX_AUD_WCD_MCLK_CPU,
		.settings = {
			[GPIOMUX_SUSPENDED] = &cdc_mclk,
		},
	},
};

static struct gpiomux_setting slimbus = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_KEEPER,
};

static struct msm_gpiomux_config valente_wx_slimbus_configs[] __initdata = {
	{
		.gpio	= VALENTE_WX_AUD_WCD_SB_CLK_CPU,		/* slimbus data */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
	{
		.gpio	= VALENTE_WX_AUD_WCD_SB_DATA,		/* slimbus clk */
		.settings = {
			[GPIOMUX_SUSPENDED] = &slimbus,
		},
	},
};

static struct gpiomux_setting wcnss_5wire_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_10MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting wcnss_5wire_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_10MA,
	.pull = GPIOMUX_PULL_DOWN,
};

struct msm_gpiomux_config wcnss_5wire_interface[] = {
	{
		.gpio = VALENTE_WX_WCN_CMD_DATA2,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_WCN_CMD_DATA1,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_WCN_CMD_DATA0,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_WCN_CMD_SET,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_WCN_CMD_CLK,
		.settings = {
			[GPIOMUX_ACTIVE]    = &wcnss_5wire_active_cfg,
			[GPIOMUX_SUSPENDED] = &wcnss_5wire_suspend_cfg,
		},
	},
};

static struct gpiomux_setting cam_settings[11] = {
	{
		.func = GPIOMUX_FUNC_GPIO, /*suspend - I(L) 8MA*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 1 - A FUNC1 8MA*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 2 - O(L) 8MA*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

	{
		.func = GPIOMUX_FUNC_1, /*active 3 - A FUNC1 8MA*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_2, /*active 4 - A FUNC2 8MA*/
		.drv = GPIOMUX_DRV_8MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 5 - I(L) 4MA*/
		.drv = GPIOMUX_DRV_4MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN,
	},

	{
		.func = GPIOMUX_FUNC_2, /*active 6 - A FUNC2 2MA*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 7 - I(NP) 2MA*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_IN,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 8 - I(L) 2MA*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 9 - O(H) 2MA*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_HIGH,
	},

	{
		.func = GPIOMUX_FUNC_GPIO, /*active 10 - O(L) 2MA*/
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_NONE,
		.dir = GPIOMUX_OUT_LOW,
	},

};

static struct msm_gpiomux_config valente_wx_cam_configs[] = {
	{
		.gpio = VALENTE_WX_CAM_MCLK1,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[4],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = VALENTE_WX_CAM_MCLK0,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[1],
			[GPIOMUX_SUSPENDED] = &cam_settings[2],
		},
	},
	{
		.gpio = VALENTE_WX_CAM_I2C_SDA,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = VALENTE_WX_CAM_I2C_SCL,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[3],
			[GPIOMUX_SUSPENDED] = &cam_settings[0],
		},
	},
	{
		.gpio = VALENTE_WX_RAW_INTR0,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[7], /*I(NP) 2MA*/
			[GPIOMUX_SUSPENDED] = &cam_settings[8], /*I(L) 2MA*/
		},
	},
	{
		.gpio = VALENTE_WX_RAW_INTR1,
		.settings = {
			[GPIOMUX_ACTIVE]    = &cam_settings[7], /*I(NP) 2MA*/
			[GPIOMUX_SUSPENDED] = &cam_settings[8], /*I(L) 2MA*/
		},
	},
	/* gpio config for Rawchip SPI - gsbi10 */
	{
		.gpio      = VALENTE_WX_MCAM_SPI_CLK,
		.settings = {
			[GPIOMUX_ACTIVE] = &cam_settings[4], /*A FUNC2 8MA*/
			[GPIOMUX_SUSPENDED] = &cam_settings[2], /*O(L) 8MA*/
		},
	},
	{
		.gpio      = VALENTE_WX_MCAM_SPI_CS0,
		.settings = {
			[GPIOMUX_ACTIVE] = &cam_settings[6], /*A FUNC2 2MA*/
			[GPIOMUX_SUSPENDED] = &cam_settings[10], /* O(L) 2MA*/
		},
	},
	{
		.gpio      = VALENTE_WX_MCAM_SPI_DI,
		.settings = {
			[GPIOMUX_ACTIVE] = &cam_settings[4], /*A FUNC2 8MA*/
			[GPIOMUX_SUSPENDED] = &cam_settings[0], /* I(L) 8MA*/
		},
	},
	{
		.gpio      = VALENTE_WX_MCAM_SPI_DO,
		.settings = {
			[GPIOMUX_ACTIVE] = &cam_settings[4], /*A FUNC2 8MA*/
			[GPIOMUX_SUSPENDED] = &cam_settings[2], /*O(L) 8MA*/
		},
	},
};

#ifdef CONFIG_FELICA_DD
static struct gpiomux_setting felica_settings[] = {
	[0]={
		.func = GPIOMUX_FUNC_GPIO,	/* 0 : O(L) NP 2MA*/
		.drv  = GPIOMUX_DRV_2MA,	/* 2MA */
		.pull = GPIOMUX_PULL_NONE,	/* NP */
		.dir  = GPIOMUX_OUT_LOW,	/* output */
	},
	[1]={
		.func = GPIOMUX_FUNC_GPIO,	/* 1: O(H) NP 2MA*/
		.drv  = GPIOMUX_DRV_2MA,	/* 2MA */
		.pull = GPIOMUX_PULL_NONE,	/* NP */
		.dir  = GPIOMUX_OUT_HIGH,	/* output */
	},
	[2]={
		.func = GPIOMUX_FUNC_GPIO,	/* 2: I(PD) 2MA*/
		.drv  = GPIOMUX_DRV_2MA,	/* 2MA */
		.pull = GPIOMUX_PULL_DOWN,	/* PD */
		.dir  = GPIOMUX_IN, 		/* input */
	},
	[3]={
		.func = GPIOMUX_FUNC_GPIO,	/* 3: I(PU) 2MA*/
		.drv  = GPIOMUX_DRV_2MA,	/* 2MA */
		.pull = GPIOMUX_PULL_UP,	/* PU */
		.dir  = GPIOMUX_IN, 		/* input */
	},
	[4]={
		.func = GPIOMUX_FUNC_GPIO,	/* 4: I(NP) 2MA*/
		.drv  = GPIOMUX_DRV_2MA,	/* 2MA */
		.pull = GPIOMUX_PULL_NONE,	/* NP */
		.dir  = GPIOMUX_IN, 		/* input */
	},
};

static struct msm_gpiomux_config valente_wx_felica_configs[] = {
	{
		.gpio = VALENTE_WX_FEL_PON,  /* PON (GPIO-25), output */
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_settings[1],	/* 1: O(H) NP 2MA*/
			[GPIOMUX_SUSPENDED] = &felica_settings[0],	/* 0 : O(L) NP 2MA*/
		},
	},
	{
		.gpio = VALENTE_WX_FEL_RFS, /* RFS (GPIO-10), input */
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_settings[3],	/* 3: I(PU) 2MA*/
			[GPIOMUX_SUSPENDED] = &felica_settings[3],	/* 3: I(PU) 2MA*/
		},
	},
	{
		.gpio = VALENTE_WX_FEL_INT, /* INT (GPIO-24), input */
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_settings[4],	/* 4: I(NP) 2MA*/
			[GPIOMUX_SUSPENDED] = &felica_settings[4],	/* 4: I(NP) 2MA*/
		},
	},
	{
		.gpio = VALENTE_WX_FEL_CON,  /* CON (GPIO-13), input */
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_settings[2],	/* 2: I(PD) 2MA*/
			[GPIOMUX_SUSPENDED] = &felica_settings[2],	/* 2: I(PD) 2MA*/
		},
	},
	{
		.gpio = VALENTE_WX_FEL_CEN,  /* CEN (GPIO-12), output */
		.settings = {
			[GPIOMUX_ACTIVE]    = &felica_settings[3],	/* 3: I(PU) 2MA*/
			[GPIOMUX_SUSPENDED] = &felica_settings[3],	/* 3: I(PU) 2MA*/
		},
	},
};
#endif	/* #ifdef CONFIG_FELICA_DD */

static struct gpiomux_setting mdp_vsync_active_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct gpiomux_setting mdp_vsync_suspend_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config valente_wx_mdp_vsync_configs[] __initdata = {
	{
		.gpio = VALENTE_WX_LCD_TE,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mdp_vsync_active_cfg,
			[GPIOMUX_SUSPENDED] = &mdp_vsync_suspend_cfg,
		},
	}
};
#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL

static struct gpiomux_setting mhl_active_int_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting mhl_suspend_int_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting mhl_active_rst_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};
static struct gpiomux_setting mhl_suspend_rst_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};


static struct msm_gpiomux_config valente_wx_mhl_configs[] __initdata = {
	{
		.gpio = VALENTE_WX_MHL_INT,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_active_int_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_suspend_int_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_MHL_RSTz,
		.settings = {
			[GPIOMUX_ACTIVE]    = &mhl_active_rst_cfg,
			[GPIOMUX_SUSPENDED] = &mhl_suspend_rst_cfg,
		},
	},
};


static struct gpiomux_setting hdmi_suspend_pu_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_UP,
};
static struct gpiomux_setting hdmi_suspend_np_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi_active_1_cfg = {
	.func = GPIOMUX_FUNC_1,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct gpiomux_setting hdmi_active_2_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir= GPIOMUX_OUT_HIGH,
};


static struct msm_gpiomux_config valente_wx_hdmi_configs[] __initdata = {
	{
		.gpio = VALENTE_WX_HDMI_DDC_CLK,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_np_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_HDMI_DDC_DATA,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_1_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_np_cfg,
		},
	},
	{
		.gpio = VALENTE_WX_HDMI_HPD,
		.settings = {
			[GPIOMUX_ACTIVE]    = &hdmi_active_2_cfg,
			[GPIOMUX_SUSPENDED] = &hdmi_suspend_pu_cfg,
		},
	},
};
#endif
static struct gpiomux_setting usb_id_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct msm_gpiomux_config cable_detect_usbid_config[] __initdata = {
	{
		.gpio = VALENTE_WX_USB_ID1,
		.settings = {
			[GPIOMUX_ACTIVE] = &usb_id_cfg,
			[GPIOMUX_SUSPENDED] = &usb_id_cfg,
		},
	},
};

int __init valente_wx_gpiomux_init(void)
{
	int rc;

	rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc) {
		pr_err(KERN_ERR "msm_gpiomux_init failed %d\n", rc);
		return rc;
	}

	msm_gpiomux_install(valente_wx_cam_configs,
			ARRAY_SIZE(valente_wx_cam_configs));

	msm_gpiomux_install(valente_wx_gsbi_configs,
			ARRAY_SIZE(valente_wx_gsbi_configs));

	msm_gpiomux_install(valente_wx_slimbus_configs,
			ARRAY_SIZE(valente_wx_slimbus_configs));

	msm_gpiomux_install(valente_wx_audio_codec_configs,
			ARRAY_SIZE(valente_wx_audio_codec_configs));
#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
	msm_gpiomux_install(valente_wx_hdmi_configs,
			ARRAY_SIZE(valente_wx_hdmi_configs));
	msm_gpiomux_install(valente_wx_mhl_configs,
			ARRAY_SIZE(valente_wx_mhl_configs));
#endif
	msm_gpiomux_install(valente_wx_mdp_vsync_configs,
			ARRAY_SIZE(valente_wx_mdp_vsync_configs));

	msm_gpiomux_install(wcnss_5wire_interface,
			ARRAY_SIZE(wcnss_5wire_interface));

	msm_gpiomux_install(cable_detect_usbid_config,
			ARRAY_SIZE(cable_detect_usbid_config));

#ifdef CONFIG_FELICA_DD
	msm_gpiomux_install(valente_wx_felica_configs,
			ARRAY_SIZE(valente_wx_felica_configs));
#endif	/* #ifdef CONFIG_FELICA_DD */

	return 0;
}
