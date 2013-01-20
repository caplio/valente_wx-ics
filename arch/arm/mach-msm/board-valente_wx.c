/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/i2c/sx150x.h>
#include <linux/i2c/isl9519.h>
#include <linux/gpio.h>
#include <linux/usb/android_composite.h>
#include <linux/msm_ssbi.h>
#include <linux/pn544.h>
#include <linux/regulator/gpio-regulator.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>
#include <linux/mpu.h>
#include <linux/slimbus/slimbus.h>
#include <linux/bootmem.h>
#include <linux/msm_kgsl.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <linux/cyttsp.h>
#include <linux/atmel_qt602240.h>
#include <linux/input/cy8c_cs.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/qcom_crypto_device.h>
#include <linux/platform_data/qcom_wcnss_device.h>
#include <linux/leds.h>
#include <linux/leds-pm8xxx.h>
#include <linux/msm_tsens.h>
#include <linux/ks8851.h>
#include <linux/proc_fs.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>
#include <asm/hardware/gic.h>
#include <asm/mach/mmc.h>
#include <linux/cm3629.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_spi.h>
#ifdef CONFIG_USB_MSM_OTG_72K
#include <mach/msm_hsusb.h>
#else
#include <linux/usb/msm_hsusb.h>
#endif
#include <mach/htc_usb.h>
#include <mach/usbdiag.h>
#include <mach/socinfo.h>
#include <mach/rpm.h>
#include <mach/gpio.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/dma.h>
#include <mach/msm_dsps.h>
#include <mach/msm_xo.h>
#include <mach/restart.h>
#include <mach/htc_headset_mgr.h>
#include <mach/htc_headset_pmic.h>
#include <mach/cable_detect.h>

#include <linux/akm8975.h>
#include <linux/bma250.h>
#ifdef CONFIG_WCD9310_CODEC
#include <linux/slimbus/slimbus.h>
#include <linux/mfd/wcd9310/core.h>
#include <linux/mfd/wcd9310/pdata.h>
#endif

#ifdef CONFIG_PERFLOCK
#include <mach/perflock.h>
#endif

#ifdef CONFIG_BT
#include <mach/htc_bdaddress.h>
#endif

#include <linux/ion.h>
#include <mach/ion.h>

#include <mach/msm_rtb.h>
#include <mach/msm_cache_dump.h>
#include <mach/scm.h>
#include <linux/fmem.h>

#include "timer.h"
#include "devices.h"
#include "devices-msm8x60.h"
#include "spm.h"
#include "board-valente_wx.h"
#include <mach/pm.h>
#include <mach/cpuidle.h>
#include "rpm_resources.h"
#include "mpm.h"
#include "acpuclock.h"
#include "rpm_log.h"
#include "smd_private.h"
#include "pm-boot.h"
#include <linux/htc_irda.h>

#include <linux/htc_flashlight.h>
#include <mach/board_htc.h>
#include <mach/htc_util.h>
#ifdef CONFIG_HTC_BATT_8960
#include "mach/htc_battery_8960.h"
#include "mach/htc_battery_cell.h"
#include "linux/mfd/pm8xxx/pm8921-charger.h"
#endif

#ifdef CONFIG_TSIF
#include <mach/msm_tsif.h>
#endif

#ifdef CONFIG_FELICA_DD
#include <linux/platform_device.h>
#include <linux/felica.h>
#endif

#ifdef CONFIG_FLASHLIGHT_TPS61310
#ifdef CONFIG_MSM_CAMERA_FLASH
#define LED_ON				1
#define LED_OFF				0

#define HW_VER_ID_VIRT          (MSM_TLMM_BASE + 0x00002054)
static uint32_t msm_rpm_get_swfi_latency(void);

static int flashlight_control(int mode)
{
	int	rc;
	/* Andrew_Cheng Turn off backlight when flash on */
	static int	backlight_off = 0;

	if(mode == FL_MODE_FLASH
	|| mode == FL_MODE_TORCH_LEVEL_1
	||mode == FL_MODE_TORCH_LEVEL_2
	||mode == FL_MODE_FLASH_LEVEL1
	||mode == FL_MODE_FLASH_LEVEL2) {
		led_brightness_switch("lcd-backlight", LED_OFF);
		backlight_off = 1;
	}

	rc = tps61310_flashlight_control(mode);

	if(mode != FL_MODE_FLASH
	&&  mode != FL_MODE_TORCH_LEVEL_1
	&& mode != FL_MODE_TORCH_LEVEL_2
	&& mode != FL_MODE_FLASH_LEVEL1
	&& mode != FL_MODE_FLASH_LEVEL2) {
		if(backlight_off) {
			led_brightness_switch("lcd-backlight", LED_ON);
			backlight_off = 0;
		}
	}

	return rc;
}
#endif

static void config_flashlight_gpios(void)
{
	static uint32_t flashlight_gpio_table[] = {
		GPIO_CFG(32, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		GPIO_CFG(33, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	};

	gpio_tlmm_config(flashlight_gpio_table[0], GPIO_CFG_ENABLE);
	gpio_tlmm_config(flashlight_gpio_table[1], GPIO_CFG_ENABLE);
}

static struct TPS61310_flashlight_platform_data valente_wx_flashlight_data = {
	.gpio_init = config_flashlight_gpios,
	.tps61310_strb0 = 33,
	.tps61310_strb1 = 32,
	.led_count = 1,
	.flash_duration_ms = 600,
};

static struct i2c_board_info i2c_tps61310_flashlight[] = {
	{
		I2C_BOARD_INFO("TPS61310_FLASHLIGHT", 0x66 >> 1),
		.platform_data = &valente_wx_flashlight_data,
	},
};
#endif

static struct platform_device msm_fm_platform_init = {
	.name = "iris_fm",
	.id   = -1,
};

/* TSIF begin */
#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
static struct platform_device nm32x_62x_tsi_device = {
	.name = "nm32x_62x-tsi",
};

#endif /* defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE) */
/* TSIF end   */

struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio			config;
};

struct pm8xxx_mpp_init {
	unsigned			mpp;
	struct pm8xxx_mpp_config_data	config;
};

#define PM8XXX_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

#define PM8XXX_MPP_INIT(_mpp, _type, _level, _control) \
{ \
	.mpp	= PM8921_MPP_PM_TO_SYS(_mpp), \
	.config	= { \
		.type		= PM8XXX_MPP_TYPE_##_type, \
		.level		= _level, \
		.control	= PM8XXX_MPP_##_control, \
	} \
}

#define PM8XXX_GPIO_DISABLE(_gpio) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, 0, 0, 0, PM_GPIO_VIN_S4, \
			 0, 0, 0, 1)

#define PM8XXX_GPIO_OUTPUT(_gpio, _val) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8XXX_GPIO_INPUT(_gpio, _pull) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_IN, PM_GPIO_OUT_BUF_CMOS, 0, \
			_pull, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_NO, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8XXX_GPIO_OUTPUT_FUNC(_gpio, _val, _func) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_S4, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(_gpio, _val, _func) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_BB, \
			PM_GPIO_STRENGTH_HIGH, \
			_func, 0, 0)

#define PM8XXX_GPIO_OUTPUT_VIN_L17_FUNC(_gpio, _val) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_NO, PM_GPIO_VIN_L17, \
			PM_GPIO_STRENGTH_HIGH, \
			PM_GPIO_FUNC_NORMAL, 0, 0)

#define PM8XXX_GPIO_OUTPUT_VIN_L17(_gpio, _val) \
	PM8XXX_GPIO_INIT(_gpio, PM_GPIO_DIR_OUT, PM_GPIO_OUT_BUF_CMOS, _val, \
			PM_GPIO_PULL_UP_30, PM_GPIO_VIN_L17, \
			PM_GPIO_STRENGTH_LOW, \
			PM_GPIO_FUNC_NORMAL, 0, 0)			
			

/* Initial PM8921 GPIO configurations */
static struct pm8xxx_gpio_init pm8921_gpios[] __initdata = {
	PM8XXX_GPIO_INPUT(VALENTE_WX_EARPHONE_DETz, PM_GPIO_PULL_UP_1P5),
	PM8XXX_GPIO_INPUT(VALENTE_WX_AUD_REMO_PRESz, PM_GPIO_PULL_NO),
	PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(VALENTE_WX_GREEN_LED, 1, PM_GPIO_FUNC_NORMAL),
	PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(VALENTE_WX_AMBER_LED, 1, PM_GPIO_FUNC_NORMAL),
};

/* Initial PM8921 MPP configurations */
static struct pm8xxx_mpp_init pm8921_mpps[] __initdata = {
	/* External 5V regulator enable; shared by HDMI and USB_OTG switches. */
	PM8XXX_MPP_INIT(PM8XXX_AMUX_MPP_8, A_INPUT, PM8XXX_MPP_AIN_AMUX_CH8,
								DOUT_CTRL_LOW),
	PM8XXX_MPP_INIT(PM8XXX_AMUX_MPP_3, D_BI_DIR, PM8921_MPP_DIG_LEVEL_S4, BI_PULLUP_10KOHM),
	PM8XXX_MPP_INIT(PM8XXX_AMUX_MPP_4, D_BI_DIR, PM8921_MPP_DIG_LEVEL_L17, BI_PULLUP_10KOHM),
	PM8XXX_MPP_INIT(PM8XXX_AMUX_MPP_11, D_BI_DIR, PM8921_MPP_DIG_LEVEL_S4, BI_PULLUP_10KOHM),
	PM8XXX_MPP_INIT(PM8XXX_AMUX_MPP_12, D_BI_DIR, PM8921_MPP_DIG_LEVEL_L17, BI_PULLUP_10KOHM),
};

static void __init pm8921_gpio_mpp_init(void)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(pm8921_gpios); i++) {
		rc = pm8xxx_gpio_config(pm8921_gpios[i].gpio,
					&pm8921_gpios[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_gpio_config: rc=%d\n", __func__, rc);
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(pm8921_mpps); i++) {
		rc = pm8xxx_mpp_config(pm8921_mpps[i].mpp,
					&pm8921_mpps[i].config);
		if (rc) {
			pr_err("%s: pm8xxx_mpp_config: rc=%d\n", __func__, rc);
			break;
		}
	}
}

#ifdef CONFIG_I2C
#ifdef CONFIG_VIDEO_NMI
#define MSM_8960_GSBI1_QUP_I2C_BUS_ID 1
#endif
#define MSM_8960_GSBI3_QUP_I2C_BUS_ID 3
#define MSM_8960_GSBI4_QUP_I2C_BUS_ID 4
#define MSM_8960_GSBI8_QUP_I2C_BUS_ID 8
#define MSM_8960_GSBI12_QUP_I2C_BUS_ID 12

#ifdef CONFIG_VIDEO_NMI

static struct i2c_board_info nmi625_i2c_info[] = {
	{
		I2C_BOARD_INFO("nmi625", 0x61),
	},
};
#endif /* CONFIG_VIDEO_NMI */

#endif

#define MSM_PMEM_ADSP_SIZE         0x6D00000
#define MSM_PMEM_ADSP2_SIZE        0x700000
#define MSM_PMEM_AUDIO_SIZE        0x2B4000
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#define MSM_LIQUID_PMEM_SIZE 0x4000000 /* 64 Mbytes */

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#define MSM_PMEM_KERNEL_EBI1_SIZE  0x280000
#define MSM_ION_SF_SIZE		MSM_PMEM_SIZE
#define MSM_ION_MM_FW_SIZE	0x200000 /* (2MB) */
#define MSM_ION_MM_SIZE		MSM_PMEM_ADSP_SIZE - MSM_PMEM_ADSP2_SIZE
#define MSM_ION_ROTATOR_SIZE	MSM_PMEM_ADSP2_SIZE
#define MSM_ION_QSECOM_SIZE	0x100000 /* (1MB) */
#define MSM_ION_MFC_SIZE	0x100000  //SZ_8K
#define MSM_ION_HEAP_NUM	8
#define MSM_LIQUID_ION_MM_SIZE (MSM_ION_MM_SIZE + 0x600000)
static unsigned int msm_ion_cp_mm_size = MSM_ION_MM_SIZE;
#else
#define MSM_PMEM_KERNEL_EBI1_SIZE  0x110C000
#define MSM_ION_HEAP_NUM	1
#endif

#ifdef CONFIG_KERNEL_PMEM_EBI_REGION
static unsigned pmem_kernel_ebi1_size = MSM_PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	pmem_kernel_ebi1_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_kernel_ebi1_size", pmem_kernel_ebi1_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
static unsigned pmem_size = MSM_PMEM_SIZE;
static unsigned pmem_param_set = 0;
static int __init pmem_size_setup(char *p)
{
	pmem_size = memparse(p, NULL);
	pmem_param_set = 1;
	return 0;
}
early_param("pmem_size", pmem_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;

static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = {.platform_data = &android_pmem_pdata},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};
#endif

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};
#endif

struct fmem_platform_data fmem_pdata = {
};

static struct memtype_reserve msm8960_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

#if defined(CONFIG_MSM_RTB)
static struct msm_rtb_platform_data msm_rtb_pdata = {
	.size = SZ_1K,
};

static int __init msm_rtb_set_buffer_size(char *p)
{
	int s;

	s = memparse(p, NULL);
	msm_rtb_pdata.size = ALIGN(s, SZ_4K);
	return 0;
}
early_param("msm_rtb_size", msm_rtb_set_buffer_size);


static struct platform_device msm_rtb_device = {
	.name           = "msm_rtb",
	.id             = -1,
	.dev            = {
		.platform_data = &msm_rtb_pdata,
	},
};
#endif

static void __init reserve_rtb_memory(void)
{
#if defined(CONFIG_MSM_RTB)
	msm8960_reserve_table[MEMTYPE_EBI1].size += msm_rtb_pdata.size;
#endif
}

static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_adsp_pdata.size = pmem_adsp_size;

	if (!pmem_param_set && machine_is_msm8960_liquid())
		pmem_size = MSM_LIQUID_PMEM_SIZE;
	android_pmem_pdata.size = pmem_size;
#endif
	android_pmem_audio_pdata.size = MSM_PMEM_AUDIO_SIZE;
#endif
}

static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	msm8960_reserve_table[p->memory_type].size += p->size;
}

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
#endif
	reserve_memory_for(&android_pmem_audio_pdata);
	msm8960_reserve_table[MEMTYPE_EBI1].size += pmem_kernel_ebi1_size;
#endif
}

static void __init reserve_fmem_memory(void)
{
}

static int msm8960_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_cp_heap_pdata cp_mm_ion_pdata = {
	.permission_type = IPT_TYPE_MM_CARVEOUT,
	.align = PAGE_SIZE,
};

static struct ion_cp_heap_pdata cp_mfc_ion_pdata = {
	.permission_type = IPT_TYPE_MFC_SHAREDMEM,
	.align = PAGE_SIZE,
};

static struct ion_co_heap_pdata co_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
};

static struct ion_co_heap_pdata fw_co_ion_pdata = {
	.adjacent_mem_id = ION_CP_MM_HEAP_ID,
	.align = SZ_128K,
};
#endif

/**
 * These heaps are listed in the order they will be allocated. Due to
 * video hardware restrictions and content protection the FW heap has to
 * be allocated adjacent (below) the MM heap and the MFC heap has to be
 * allocated after the MM heap to ensure MFC heap is not more than 256MB
 * away from the base address of the FW heap.
 * However, the order of FW heap and MM heap doesn't matter since these
 * two heaps are taken care of by separate code to ensure they are adjacent
 * to each other.
 * Don't swap the order unless you know what you are doing!
 */
static struct ion_platform_data ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = {
		{
			.id	= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		{
			.id	= ION_CP_MM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MM_HEAP_NAME,
			.size	= MSM_ION_MM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mm_ion_pdata,
		},
		{
			.id	= ION_CP_ROTATOR_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_ROTATOR_HEAP_NAME,
			.size	= MSM_ION_ROTATOR_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mm_ion_pdata,
		},
		{
			.id	= ION_MM_FIRMWARE_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_MM_FIRMWARE_HEAP_NAME,
			.size	= MSM_ION_MM_FW_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &fw_co_ion_pdata,
		},
		{
			.id	= ION_CP_MFC_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MFC_HEAP_NAME,
			.size	= MSM_ION_MFC_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mfc_ion_pdata,
		},
		{
			.id	= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.size	= MSM_ION_SF_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_ion_pdata,
		},
		{
			.id	= ION_IOMMU_HEAP_ID,
			.type	= ION_HEAP_TYPE_IOMMU,
			.name	= ION_IOMMU_HEAP_NAME,
		},
		{
			.id	= ION_QSECOM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_QSECOM_HEAP_NAME,
			.size	= MSM_ION_QSECOM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_ion_pdata,
		},
#endif
	}
};

static struct platform_device ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &ion_pdata },
};
#endif

struct platform_device fmem_device = {
	.name = "fmem",
	.id = 1,
	.dev = { .platform_data = &fmem_pdata },
};

#ifdef CONFIG_TSIF

#define MSM_TSIF0_PHYS       (0x18200000)
#define MSM_TSIF1_PHYS       (0x18201000)
#define MSM_TSIF_SIZE        (0x200)

#define TSIF_0_CLK       GPIO_CFG(VALENTE_WX_TS_CLK, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA)
#define TSIF_0_EN        GPIO_CFG(VALENTE_WX_TS_EN, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA)
#define TSIF_0_DATA      GPIO_CFG(VALENTE_WX_TS_DATA, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA)
#define TSIF_0_SYNC      GPIO_CFG(VALENTE_WX_TS_SYNC, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA)


static const struct msm_gpio tsif0_gpios[] = {
	{ .gpio_cfg = TSIF_0_CLK,  .label =  "tsif_clk", },
	{ .gpio_cfg = TSIF_0_EN,   .label =  "tsif_en", },
	{ .gpio_cfg = TSIF_0_DATA, .label =  "tsif_data", },
        { .gpio_cfg = TSIF_0_SYNC, .label =  "tsif_sync", },
};

static const struct msm_gpio tsif1_gpios[] = {
//	{ .gpio_cfg = TSIF_1_CLK,  .label =  "tsif_clk", },
//	{ .gpio_cfg = TSIF_1_EN,   .label =  "tsif_en", },
//	{ .gpio_cfg = TSIF_1_DATA, .label =  "tsif_data", },
};

static void tsif_release(struct device *dev)
{
}

struct msm_tsif_platform_data tsif1_platform_data = {
	.num_gpios = ARRAY_SIZE(tsif1_gpios),
	.gpios = tsif1_gpios,
	.tsif_pclk = "tsif_pclk",
	.tsif_ref_clk = "tsif_ref_clk",
	.init = 0
};

struct resource tsif1_resources[] = {
	[0] = {
		.flags = IORESOURCE_IRQ,
		.start = TSIF2_IRQ,
		.end   = TSIF2_IRQ,
	},
	[1] = {
		.flags = IORESOURCE_MEM,
		.start = MSM_TSIF1_PHYS,
		.end   = MSM_TSIF1_PHYS + MSM_TSIF_SIZE - 1,
	},
	[2] = {
		.flags = IORESOURCE_DMA,
		.start = DMOV_TSIF_CHAN,
		.end   = DMOV_TSIF_CRCI,
	},
};

struct msm_tsif_platform_data tsif0_platform_data = {
	.num_gpios = ARRAY_SIZE(tsif0_gpios),
	.gpios = tsif0_gpios,
	.tsif_pclk = "tsif_pclk",
	.tsif_ref_clk = "tsif_ref_clk",
	.init = 0
};
struct resource tsif0_resources[] = {
	[0] = {
		.flags = IORESOURCE_IRQ,
		.start = TSIF1_IRQ,
		.end   = TSIF1_IRQ,
	},
	[1] = {
		.flags = IORESOURCE_MEM,
		.start = MSM_TSIF0_PHYS,
		.end   = MSM_TSIF0_PHYS + MSM_TSIF_SIZE - 1,
	},
	[2] = {
		.flags = IORESOURCE_DMA,
		.start = DMOV_TSIF_CHAN,
		.end   = DMOV_TSIF_CRCI,
	},
};

struct platform_device msm_device_tsif[2] = {
	{
		.name          = "msm_tsif",
		.id            = 0,
		.num_resources = ARRAY_SIZE(tsif0_resources),
		.resource      = tsif0_resources,
		.dev = {
			.release       = tsif_release,
			.platform_data = &tsif0_platform_data
		},
	},
	{
		.name          = "msm_tsif",
		.id            = 1,
		.num_resources = ARRAY_SIZE(tsif1_resources),
		.resource      = tsif1_resources,
		.dev = {
			.release       = tsif_release,
			.platform_data = &tsif1_platform_data
		},
	}
};
#endif



static void reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	unsigned int i;

	if (!pmem_param_set && machine_is_msm8960_liquid()) {
		msm_ion_cp_mm_size = MSM_LIQUID_ION_MM_SIZE;
		for (i = 0; i < ion_pdata.nr; i++) {
			if (ion_pdata.heaps[i].id == ION_CP_MM_HEAP_ID) {
				ion_pdata.heaps[i].size = msm_ion_cp_mm_size;
				pr_debug("msm_ion_cp_mm_size 0x%x\n",
					msm_ion_cp_mm_size);
				break;
			}
		}
	}
	msm8960_reserve_table[MEMTYPE_EBI1].size += msm_ion_cp_mm_size;
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_ROTATOR_SIZE;
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_MM_FW_SIZE;
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_SF_SIZE;
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_MFC_SIZE;
	msm8960_reserve_table[MEMTYPE_EBI1].size += MSM_ION_QSECOM_SIZE;
#endif
}

void __init msm8960_mdp_writeback(struct memtype_reserve* reserve_table);
static void __init reserve_mdp_memory(void)
{
	msm8960_mdp_writeback(msm8960_reserve_table);
}

#if defined(CONFIG_MSM_CACHE_DUMP)
static struct msm_cache_dump_platform_data msm_cache_dump_pdata = {
	.l2_size = L2_BUFFER_SIZE,
};

static struct platform_device msm_cache_dump_device = {
	.name		= "msm_cache_dump",
	.id		= -1,
	.dev		= {
		.platform_data = &msm_cache_dump_pdata,
	},
};

#endif

static void reserve_cache_dump_memory(void)
{
#ifdef CONFIG_MSM_CACHE_DUMP
	unsigned int spare;
	unsigned int l1_size;
	unsigned int total;
	int ret;

	ret = scm_call(L1C_SERVICE_ID, L1C_BUFFER_GET_SIZE_COMMAND_ID, &spare,
		sizeof(spare), &l1_size, sizeof(l1_size));

	if (ret)
		/* Fall back to something reasonable here */
		l1_size = L1_BUFFER_SIZE;

	total = l1_size + L2_BUFFER_SIZE;

	msm8960_reserve_table[MEMTYPE_EBI1].size += total;
	msm_cache_dump_pdata.l1_size = l1_size;
#endif
}

static void __init msm8960_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
	reserve_ion_memory();
	reserve_fmem_memory();
	reserve_mdp_memory();
	reserve_rtb_memory();
	reserve_cache_dump_memory();
}

static struct reserve_info msm8960_reserve_info __initdata = {
	.memtype_reserve_table = msm8960_reserve_table,
	.calculate_reserve_sizes = msm8960_calculate_reserve_sizes,
	.paddr_to_memtype = msm8960_paddr_to_memtype,
};

#ifdef MEMORY_HOTPLUG
static int msm8960_memory_bank_size(void)
{
	return 1<<29;
}

static void __init locate_unstable_memory(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_size;
	unsigned long low, high;

	bank_size = msm8960_memory_bank_size();
	low = meminfo.bank[0].start;
	high = mb->start + mb->size;

	/* Check if 32 bit overflow occured */
	if (high < mb->start)
		high = ~0UL;

	low &= ~(bank_size - 1);

	if (high - low <= bank_size)
		return;
	msm8960_reserve_info.low_unstable_address = low + bank_size;
	/* To avoid overflow of u32 compute max_unstable_size
	 * by first subtracting low from mb->start)
	 * */
	msm8960_reserve_info.max_unstable_size = (mb->start - low) +
						mb->size - bank_size;

	msm8960_reserve_info.bank_size = bank_size;
	pr_info("low unstable address %lx max size %lx bank size %lx\n",
		msm8960_reserve_info.low_unstable_address,
		msm8960_reserve_info.max_unstable_size,
		msm8960_reserve_info.bank_size);
}

static void __init place_movable_zone(void)
{
	movable_reserved_start = msm8960_reserve_info.low_unstable_address;
	movable_reserved_size = msm8960_reserve_info.max_unstable_size;
	pr_info("movable zone start %lx size %lx\n",
		movable_reserved_start, movable_reserved_size);
}
#endif

static void __init valente_wx_early_memory(void)
{
	reserve_info = &msm8960_reserve_info;
#ifdef MEMORY_HOTPLUG
	locate_unstable_memory();
	place_movable_zone();
#endif
}

static void __init valente_wx_reserve(void)
{
	msm_reserve();
	fmem_pdata.phys = reserve_memory_for_fmem(fmem_pdata.size);
}
static int msm8960_change_memory_power(u64 start, u64 size,
	int change_type)
{
	return soc_change_memory_power(start, size, change_type);
}

#ifdef CONFIG_CPU_FREQ_GOV_ONDEMAND_2_PHASE
int set_two_phase_freq(int cpufreq);
#endif


#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_PRIM_BUF_SIZE (960 * 544 * 4 * 3) /* 4 bpp x 3 pages */
#else
#define MSM_FB_PRIM_BUF_SIZE (960 * 544 * 4 * 2) /* 4 bpp x 2 pages */
#endif

#ifdef CONFIG_FB_MSM_HDMI_MHL
#include <mach/mhl.h>
#endif


#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
#define MSM_FB_EXT_BUF_SIZE	(1920 * 1088 * 2 * 1) /* 2 bpp x 1 page */
#elif defined(CONFIG_FB_MSM_TVOUT)
#define MSM_FB_EXT_BUF_SIZE (720 * 576 * 2 * 2) /* 2 bpp x 2 pages */
#else
#define MSM_FB_EXT_BUF_SIZE	0
#endif

/* Note: must be multiple of 4096 */
#define MSM_FB_SIZE roundup(MSM_FB_PRIM_BUF_SIZE + MSM_FB_EXT_BUF_SIZE, 4096)

#ifdef CONFIG_FB_MSM_OVERLAY0_WRITEBACK
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE roundup((960 * 544 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY0_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY0_WRITEBACK */

#ifdef CONFIG_FB_MSM_OVERLAY1_WRITEBACK
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE roundup((1920 * 1088 * 3 * 2), 4096)
#else
#define MSM_FB_OVERLAY1_WRITEBACK_SIZE (0)
#endif  /* CONFIG_FB_MSM_OVERLAY1_WRITEBACK */

#define MDP_VSYNC_GPIO 0

#define PANEL_NAME_MAX_LEN	30
#define MIPI_CMD_NOVATEK_QHD_PANEL_NAME	"mipi_cmd_novatek_qhd"
#define MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME	"mipi_video_novatek_qhd"
#define MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME	"mipi_video_toshiba_wsvga"
#define MIPI_VIDEO_CHIMEI_WXGA_PANEL_NAME	"mipi_video_chimei_wxga"
#define MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME	"mipi_video_simulator_vga"
#define MIPI_CMD_RENESAS_FWVGA_PANEL_NAME	"mipi_cmd_renesas_fwvga"
#define HDMI_PANEL_NAME	"hdmi_msm"
#define TVOUT_PANEL_NAME	"tvout_msm"

static struct resource msm_fb_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};

static int msm_fb_detect_panel(const char *name)
{
	if (!strncmp(name, MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
			strnlen(MIPI_VIDEO_TOSHIBA_WSVGA_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

#ifndef CONFIG_FB_MSM_MIPI_PANEL_DETECT
	if (!strncmp(name, MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME,
			strnlen(MIPI_VIDEO_NOVATEK_QHD_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	if (!strncmp(name, MIPI_CMD_NOVATEK_QHD_PANEL_NAME,
			strnlen(MIPI_CMD_NOVATEK_QHD_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	if (!strncmp(name, MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME,
			strnlen(MIPI_VIDEO_SIMULATOR_VGA_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	if (!strncmp(name, MIPI_CMD_RENESAS_FWVGA_PANEL_NAME,
			strnlen(MIPI_CMD_RENESAS_FWVGA_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;
#endif

	if (!strncmp(name, HDMI_PANEL_NAME,
			strnlen(HDMI_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	if (!strncmp(name, TVOUT_PANEL_NAME,
			strnlen(TVOUT_PANEL_NAME,
				PANEL_NAME_MAX_LEN)))
		return 0;

	pr_warning("%s: not supported '%s'", __func__, name);
	return -ENODEV;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources     = ARRAY_SIZE(msm_fb_resources),
	.resource          = msm_fb_resources,
	.dev.platform_data = &msm_fb_pdata,
};

#ifdef CONFIG_MSM_CAMERA_FLASH
static struct msm_camera_sensor_flash_src msm_flash_src = {
	.flash_sr_type = MSM_CAMERA_FLASH_SRC_CURRENT_DRIVER,
	.camera_flash = flashlight_control,
};
#endif


#ifdef CONFIG_RAWCHIP
static int valente_wx_use_ext_1v2(void)
{
		return 1;
}

#define VALENTE_WX_SYS_V_RAW_1V2_EN PM8921_GPIO_PM_TO_SYS(VALENTE_WX_V_RAW_1V2_EN)
#define VALENTE_WX_SYS_V_RAW_1V8_EN PM8921_GPIO_PM_TO_SYS(VALENTE_WX_V_RAW_1V8_EN)

static int valente_wx_rawchip_vreg_on(void)
{
	int rc;
	pr_info("[CAM] %s\n", __func__);
	/* PM8921_GPIO_PM_TO_SYS(VALENTE_WX_V_RAW_1V8_EN) 1800000 */
	rc = gpio_request(VALENTE_WX_SYS_V_RAW_1V8_EN, "V_RAW_1V8_EN");
	if (rc) {
		pr_err("[CAM] rawchip on\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_SYS_V_RAW_1V8_EN, rc);
		goto enable_1v8_fail;
	}
	gpio_direction_output(VALENTE_WX_SYS_V_RAW_1V8_EN, 1);
	gpio_free(VALENTE_WX_SYS_V_RAW_1V8_EN);

	mdelay(5);

	/* PM8921_GPIO_PM_TO_SYS(VALENTE_WX_SYS_V_RAW_1V2_EN) 1200000 */
	rc = gpio_request(VALENTE_WX_SYS_V_RAW_1V2_EN, "V_RAW_1V2_EN");
	if (rc) {
		pr_err("[CAM] rawchip on\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_SYS_V_RAW_1V2_EN, rc);
		goto enable_1v2_fail;
	}
	gpio_direction_output(VALENTE_WX_SYS_V_RAW_1V2_EN, 1);
	gpio_free(VALENTE_WX_SYS_V_RAW_1V2_EN);

	//if (system_rev >= 1) { /* for XB */
		if (valente_wx_use_ext_1v2()) { /* use external 1v2 for HW workaround */
			mdelay(1);

			rc = gpio_request(VALENTE_WX_V_CAM_D1V2_EN, "rawchip");
			pr_info("[CAM]rawchip external 1v2 gpio_request,%d\n", VALENTE_WX_V_CAM_D1V2_EN);
			if (rc < 0) {
				pr_err("GPIO(%d) request failed", VALENTE_WX_V_CAM_D1V2_EN);
				goto enable_ext_1v2_fail;
			}
			gpio_direction_output(VALENTE_WX_V_CAM_D1V2_EN, 1);
			gpio_free(VALENTE_WX_V_CAM_D1V2_EN);
		}
	//}

	return rc;

enable_ext_1v2_fail:
	rc = gpio_request(VALENTE_WX_SYS_V_RAW_1V2_EN, "V_RAW_1V2_EN");
	if (rc)
		pr_err("[CAM] rawchip on\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_SYS_V_RAW_1V2_EN, rc);
	gpio_direction_output(VALENTE_WX_SYS_V_RAW_1V2_EN, 0);
	gpio_free(VALENTE_WX_SYS_V_RAW_1V2_EN);
enable_1v2_fail:
	rc = gpio_request(VALENTE_WX_SYS_V_RAW_1V8_EN, "V_RAW_1V8_EN");
	if (rc)
		pr_err("[CAM] rawchip on\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_SYS_V_RAW_1V8_EN, rc);
	gpio_direction_output(VALENTE_WX_SYS_V_RAW_1V8_EN, 0);
	gpio_free(VALENTE_WX_SYS_V_RAW_1V8_EN);
enable_1v8_fail:
	return rc;
}

static int valente_wx_rawchip_vreg_off(void)
{
	int rc = 0;

	pr_info("[CAM] %s\n", __func__);

	//if (system_rev >= 1) { /* for XB */
		if (valente_wx_use_ext_1v2()) { /* use external 1v2 for HW workaround */
			rc = gpio_request(VALENTE_WX_V_CAM_D1V2_EN, "rawchip");
			if (rc)
				pr_err("[CAM] rawchip off(\
					\"gpio %d\", 1.2V) FAILED %d\n",
					VALENTE_WX_V_CAM_D1V2_EN, rc);
			gpio_direction_output(VALENTE_WX_V_CAM_D1V2_EN, 0);
			gpio_free(VALENTE_WX_V_CAM_D1V2_EN);

			mdelay(1);
		}
	//}

	rc = gpio_request(VALENTE_WX_SYS_V_RAW_1V2_EN, "V_RAW_1V2_EN");
	if (rc)
		pr_err("[CAM] rawchip off(\
			\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_SYS_V_RAW_1V2_EN, rc);
	gpio_direction_output(VALENTE_WX_SYS_V_RAW_1V2_EN, 0);
	gpio_free(VALENTE_WX_SYS_V_RAW_1V2_EN);

	mdelay(5);

	rc = gpio_request(VALENTE_WX_SYS_V_RAW_1V8_EN, "V_RAW_1V8_EN");
	if (rc)
		pr_err("[CAM] rawchip off\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_SYS_V_RAW_1V8_EN, rc);
	gpio_direction_output(VALENTE_WX_SYS_V_RAW_1V8_EN, 0);
	gpio_free(VALENTE_WX_SYS_V_RAW_1V8_EN);

       return rc;
}

static struct msm_camera_rawchip_info msm_rawchip_board_info = {
	.rawchip_reset	= VALENTE_WX_RAW_RSTN,
	.rawchip_intr0	= VALENTE_WX_RAW_INTR0,
	.rawchip_intr1	= VALENTE_WX_RAW_INTR1,
	.rawchip_spi_freq = 27, /* MHz, should be the same to spi max_speed_hz */
	.rawchip_mclk_freq = 24, /* MHz, should be the same as cam csi0 mclk_clk_rate */
	.camera_rawchip_power_on = valente_wx_rawchip_vreg_on,
	.camera_rawchip_power_off = valente_wx_rawchip_vreg_off,
	.rawchip_use_ext_1v2 = valente_wx_use_ext_1v2,
};

static struct platform_device msm_rawchip_device = {
	.name	= "rawchip",
	.dev	= {
		.platform_data = &msm_rawchip_board_info,
	},
};

static struct spi_board_info rawchip_spi_board_info[] __initdata = {
	{
		.modalias               = "spi_rawchip",
		.max_speed_hz           = 27000000,
		.bus_num                = 1,
		.chip_select            = 0,
		.mode                   = SPI_MODE_0,
	},
};
#endif

static uint16_t msm_cam_gpio_tbl[] = {
	VALENTE_WX_CAM_MCLK0, /*CAMIF_MCLK*/
	VALENTE_WX_CAM_MCLK1,
#if 0
	VALENTE_WX_CAM_I2C_DAT, /*CAMIF_I2C_DATA*/
	VALENTE_WX_CAM_I2C_CLK, /*CAMIF_I2C_CLK*/
#endif
	VALENTE_WX_RAW_INTR0,
	VALENTE_WX_RAW_INTR1,
	VALENTE_WX_MCAM_SPI_CLK,
	VALENTE_WX_MCAM_SPI_CS0,
	VALENTE_WX_MCAM_SPI_DI,
	VALENTE_WX_MCAM_SPI_DO,
};

static struct msm_camera_gpio_conf gpio_conf = {
	.cam_gpiomux_conf_tbl = NULL,
	.cam_gpiomux_conf_tbl_size = 0,
	.cam_gpio_tbl = msm_cam_gpio_tbl,
	.cam_gpio_tbl_size = ARRAY_SIZE(msm_cam_gpio_tbl),
};


static struct msm_bus_vectors cam_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_preview_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 96215040,
		.ib  = 378224640,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
};

static struct msm_bus_vectors cam_video_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 342150912,
		.ib  = 1361968128,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 207747072,
		.ib  = 489756672,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 60318720,
		.ib  = 150796800,
	},
};

static struct msm_bus_vectors cam_snapshot_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 147045888,
		.ib  = 588183552,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 263678976,
		.ib  = 659197440,
	},
};

static struct msm_bus_vectors cam_zsl_vectors[] = {
	{
		.src = MSM_BUS_MASTER_VFE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 319044096,
		.ib  = 1271531520,
	},
	{
		.src = MSM_BUS_MASTER_VPE,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 0,
		.ib  = 0,
	},
	{
		.src = MSM_BUS_MASTER_JPEG_ENC,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = 239708160,
		.ib  = 599270400,
	},
};

static struct msm_bus_paths cam_bus_client_config[] = {
	{
		ARRAY_SIZE(cam_init_vectors),
		cam_init_vectors,
	},
	{
		ARRAY_SIZE(cam_preview_vectors),
		cam_preview_vectors,
	},
	{
		ARRAY_SIZE(cam_video_vectors),
		cam_video_vectors,
	},
	{
		ARRAY_SIZE(cam_snapshot_vectors),
		cam_snapshot_vectors,
	},
	{
		ARRAY_SIZE(cam_zsl_vectors),
		cam_zsl_vectors,
	},
};

static struct msm_bus_scale_pdata cam_bus_client_pdata = {
		cam_bus_client_config,
		ARRAY_SIZE(cam_bus_client_config),
		.name = "msm_camera",
};

static struct regulator *reg_8921_l2;
static struct regulator *reg_8921_l9;
static struct regulator *reg_8921_l17;
static struct regulator *reg_8921_lvs6;
static int camera_sensor_power_enable(char *power, unsigned volt, struct regulator **sensor_power)
{
	int rc;
	if (power == NULL)
		return -ENODEV;

	*sensor_power = regulator_get(NULL, power);

	if (IS_ERR(*sensor_power)) {
		pr_err("[CAM] %s: Unable to get %s\n", __func__, power);
		return -ENODEV;
	}
	/*TODO: for the condition that not to set voltage when voltage is setting as 1800000, it's because we should not set voltage for 8921_lvs6 for "lvs" power domain.
	Need to refine to set the condition as: "NOT to set voltage when the regulator domain is 8921_lvsxxx" rather than using voltage as 1800000.*/
	if (volt != 1800000) {
		rc = regulator_set_voltage(*sensor_power, volt, volt);
		if (rc < 0) {
			pr_err("[CAM] %s: unable to set %s voltage to %d rc:%d\n",
					__func__, power, volt, rc);
			regulator_put(*sensor_power);
			*sensor_power = NULL;
			return -ENODEV;
		}
	}

	rc = regulator_enable(*sensor_power);
	if (rc < 0) {
		pr_err("[CAM] %s: Enable regulator %s failed\n", __func__, power);
		regulator_put(*sensor_power);
		*sensor_power = NULL;
		return -ENODEV;
	}

	return rc;
}

static int camera_sensor_power_disable(struct regulator *sensor_power)
{

	int rc;
	if (sensor_power == NULL)
		return -ENODEV;

	if (IS_ERR(sensor_power)) {
		pr_err("[CAM] %s: Invalid requlator ptr\n", __func__);
		return -ENODEV;
	}

	rc = regulator_disable(sensor_power);
	if (rc < 0)
		pr_err("[CAM] %s: disable regulator failed\n", __func__);

	regulator_put(sensor_power);
	sensor_power = NULL;
	return rc;
}

static int valente_wx_csi_vreg_on(void)
{
	pr_info("%s\n", __func__);
	return camera_sensor_power_enable("8921_l2", 1200000, &reg_8921_l2);
}

static int valente_wx_csi_vreg_off(void)
{
	pr_info("%s\n", __func__);
	return camera_sensor_power_disable(reg_8921_l2);
}

struct msm_camera_device_platform_data msm_camera_csi_device_data[] = {
	{
		.ioclk.mclk_clk_rate = 24000000,
		.ioclk.vfe_clk_rate  = 228570000,
		.csid_core = 0,
		.camera_csi_on = valente_wx_csi_vreg_on,
		.camera_csi_off = valente_wx_csi_vreg_off,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
	{
		.ioclk.mclk_clk_rate = 24000000,
		.ioclk.vfe_clk_rate  = 228570000,
		.csid_core = 1,
		.camera_csi_on = valente_wx_csi_vreg_on,
		.camera_csi_off = valente_wx_csi_vreg_off,
		.cam_bus_scale_table = &cam_bus_client_pdata,
	},
};

#ifdef CONFIG_S5K3H2YX
static int valente_wx_s5k3h2yx_vreg_on(void)
{
	int rc;
	pr_info("[CAM] %s\n", __func__);

	/* VCM */
	rc = camera_sensor_power_enable("8921_l17", 2850000, &reg_8921_l17);
	if (rc < 0) {
		pr_err("[CAM] sensor_power_enable\
			(\"8921_l17\", 2.8V) FAILED %d\n", rc);
		goto enable_vcm_fail;
	}
	mdelay(1);

	/* analog */
	rc = camera_sensor_power_enable("8921_l9", 2800000, &reg_8921_l9);
	if (rc < 0) {
		pr_err("[CAM] sensor_power_enable\
			(\"8921_l9\", 2.8V) FAILED %d\n", rc);
		goto enable_analog_fail;
	}
	mdelay(1);

	/* digital */
	rc = gpio_request(VALENTE_WX_V_CAM_D1V2_EN, "CAM_D1V2_EN");
	if (rc) {
		pr_err("[CAM] sensor_power_enable\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_V_CAM_D1V2_EN, rc);
		goto enable_digital_fail;
	}
	gpio_direction_output(VALENTE_WX_V_CAM_D1V2_EN, 1);
	gpio_free(VALENTE_WX_V_CAM_D1V2_EN);
	mdelay(1);

	/* IO */
	rc = camera_sensor_power_enable("8921_lvs6", 1800000, &reg_8921_lvs6);
	if (rc < 0) {
		pr_err("[CAM] sensor_power_enable\
			(\"8921_lvs6\", 1.8V) FAILED %d\n", rc);
		goto enable_io_fail;
	}

	return rc;

enable_io_fail:
	rc = gpio_request(VALENTE_WX_V_CAM_D1V2_EN, "CAM_D1V2_EN");
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_V_CAM_D1V2_EN, rc);
	else {
		gpio_direction_output(VALENTE_WX_V_CAM_D1V2_EN, 0);
		gpio_free(VALENTE_WX_V_CAM_D1V2_EN);
	}
enable_digital_fail:
	camera_sensor_power_disable(reg_8921_l9);
enable_analog_fail:
	camera_sensor_power_disable(reg_8921_l17);
enable_vcm_fail:
	return rc;
}

static int valente_wx_s5k3h2yx_vreg_off(void)
{
	int rc = 0;

	pr_info("[CAM] %s\n", __func__);

	/* analog */
	rc = camera_sensor_power_disable(reg_8921_l9);
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"8921_l9\") FAILED %d\n", rc);
	mdelay(1);

	/* digital */
	rc = gpio_request(VALENTE_WX_V_CAM_D1V2_EN, "CAM_D1V2_EN");
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"gpio %d\", 1.2V) FAILED %d\n",
			VALENTE_WX_V_CAM_D1V2_EN, rc);
	else {
		gpio_direction_output(VALENTE_WX_V_CAM_D1V2_EN, 0);
		gpio_free(VALENTE_WX_V_CAM_D1V2_EN);
	}
	mdelay(1);

	/* IO */
	rc = camera_sensor_power_disable(reg_8921_lvs6);
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"8921_lvs6\") FAILED %d\n", rc);

	mdelay(1);

	/* VCM */
	#if 0
	if (system_rev >= 1) {
		rc = camera_sensor_power_disable(reg_8921_l17);
		if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
				(\"8921_l17\") FAILED %d\n", rc);
	}
	#endif
	return rc;
}

#ifdef CONFIG_S5K3H2YX_ACT
static struct i2c_board_info s5k3h2yx_actuator_i2c_info = {
	I2C_BOARD_INFO("s5k3h2yx_act", 0x11),
};

static struct msm_actuator_info s5k3h2yx_actuator_info = {
	.board_info     = &s5k3h2yx_actuator_i2c_info,
	.bus_id         = MSM_8960_GSBI4_QUP_I2C_BUS_ID,
	.vcm_pwd        = VALENTE_WX_CAM_VCM_PD,
	.vcm_enable     = 1,
};
#endif
#define VALENTE_WX_SYS_CAM_PWDN PM8921_GPIO_PM_TO_SYS(VALENTE_WX_CAM_PWDN)

static struct msm_camera_sensor_platform_info sensor_s5k3h2yx_board_info = {
	.mount_angle = 90,
	.mirror_flip = CAMERA_SENSOR_NONE,
	.sensor_reset_enable = 0,
	.sensor_reset	= 0,
	.sensor_pwd	= VALENTE_WX_SYS_CAM_PWDN,
	.vcm_pwd	= VALENTE_WX_CAM_VCM_PD,
	.vcm_enable	= 1,
};


/* Andrew_Cheng linear led 20111205 MB */
//150 mA FL_MODE_FLASH_LEVEL1
//200 mA FL_MODE_FLASH_LEVEL2
//300 mA FL_MODE_FLASH_LEVEL3
//400 mA FL_MODE_FLASH_LEVEL4
//500 mA FL_MODE_FLASH_LEVEL5
//600 mA FL_MODE_FLASH_LEVEL6
//700 mA FL_MODE_FLASH_LEVEL7
static struct camera_led_est msm_camera_sensor_s5k3h2yx_led_table[] = {
//		{
//		.enable = 0,
//		.led_state = FL_MODE_FLASH_LEVEL1,
//		.current_ma = 150,
//		.lumen_value = 150,
//		.min_step = 50,
//		.max_step = 70
//	},
		{
		.enable = 1,
		.led_state = FL_MODE_FLASH_LEVEL2,
		.current_ma = 200,
		.lumen_value = 250,//245,//240,   //mk0118
		.min_step = 29,//23,  //mk0210
		.max_step = 128
	},
		{
		.enable = 1,
		.led_state = FL_MODE_FLASH_LEVEL3,
		.current_ma = 300,
		.lumen_value = 350,
		.min_step = 27,
		.max_step = 28			
	},
		{
		.enable = 1,
		.led_state = FL_MODE_FLASH_LEVEL4,
		.current_ma = 400,
		.lumen_value = 440,
		.min_step = 25,
		.max_step = 26
	},
//		{
//		.enable = 0,
//		.led_state = FL_MODE_FLASH_LEVEL5,
//		.current_ma = 500,
//		.lumen_value = 500,
//		.min_step = 23,//25,
//		.max_step = 29//26,
//	},
		{
		.enable = 1,
		.led_state = FL_MODE_FLASH_LEVEL6,
		.current_ma = 600,
		.lumen_value = 625,
		.min_step = 23,
		.max_step = 24
	},
//		{
//		.enable = 0,
//		.led_state = FL_MODE_FLASH_LEVEL7,
//		.current_ma = 700,
//		.lumen_value = 700,
//		.min_step = 21,
//		.max_step = 22
//	},
		{
		.enable = 1,
		.led_state = FL_MODE_FLASH,
		.current_ma = 750,
		.lumen_value = 745,//725,   //mk0217  //mk0221
		.min_step = 0,
		.max_step = 22    //mk0210
	},

		{
		.enable = 2,
		.led_state = FL_MODE_FLASH_LEVEL2,
		.current_ma = 200,
		.lumen_value = 250,//245,
		.min_step = 0,
		.max_step = 270
	},
		{
		.enable = 0,
		.led_state = FL_MODE_OFF,
		.current_ma = 0,
		.lumen_value = 0,
		.min_step = 0,
		.max_step = 0
	},
	{
		.enable = 0,
		.led_state = FL_MODE_TORCH,
		.current_ma = 150,
		.lumen_value = 150,
		.min_step = 0,
		.max_step = 0
	},
	{
		.enable = 2,     //mk0210
		.led_state = FL_MODE_FLASH,
		.current_ma = 750,
		.lumen_value = 745,//725,   //mk0217   //mk0221
		.min_step = 271,
		.max_step = 317    //mk0210
	},
	{
		.enable = 0,
		.led_state = FL_MODE_FLASH_LEVEL5,
		.current_ma = 500,
		.lumen_value = 500,
		.min_step = 25,
		.max_step = 26
	},
		{
		.enable = 0,//3,  //mk0210
		.led_state = FL_MODE_FLASH,
		.current_ma = 750,
		.lumen_value = 750,//740,//725,
		.min_step = 271,
		.max_step = 325
	},

	{
		.enable = 0,
		.led_state = FL_MODE_TORCH_LEVEL_2,
		.current_ma = 200,
		.lumen_value = 75,
		.min_step = 0,
		.max_step = 40
	},};

static struct camera_led_info msm_camera_sensor_s5k3h2yx_led_info = {
	.enable = 1,
	.low_limit_led_state = FL_MODE_TORCH,
	.max_led_current_ma = 750,  //mk0210
	.num_led_est_table = ARRAY_SIZE(msm_camera_sensor_s5k3h2yx_led_table),
};

static struct camera_flash_info msm_camera_sensor_s5k3h2yx_flash_info = {
	.led_info = &msm_camera_sensor_s5k3h2yx_led_info,
	.led_est_table = msm_camera_sensor_s5k3h2yx_led_table,
};

static struct camera_flash_cfg msm_camera_sensor_s5k3h2yx_flash_cfg = {
	.low_temp_limit		= 5,
	.low_cap_limit		= 15,
	.flash_info             = &msm_camera_sensor_s5k3h2yx_flash_info,
};
/* Andrew_Cheng linear led 20111205 ME */

static struct msm_camera_sensor_flash_data flash_s5k3h2yx = {
	.flash_type	= MSM_CAMERA_FLASH_LED,
#ifdef CONFIG_MSM_CAMERA_FLASH
	.flash_src	= &msm_flash_src,
#endif

};

static struct msm_camera_sensor_info msm_camera_sensor_s5k3h2yx_data = {
	.sensor_name	= "s5k3h2yx",
	.camera_power_on = valente_wx_s5k3h2yx_vreg_on,
	.camera_power_off = valente_wx_s5k3h2yx_vreg_off,
	.pdata	= &msm_camera_csi_device_data[0],
	.flash_data	= &flash_s5k3h2yx,
	.sensor_platform_info = &sensor_s5k3h2yx_board_info,
	.gpio_conf = &gpio_conf,
	.csi_if	= 1,
	.camera_type = BACK_CAMERA_2D,
#ifdef CONFIG_S5K3H2YX_ACT
	.actuator_info = &s5k3h2yx_actuator_info,
#endif
	.use_rawchip = 1,
	.flash_cfg = &msm_camera_sensor_s5k3h2yx_flash_cfg, /* Andrew_Cheng linear led 20111205 */
};

struct platform_device valente_wx_camera_sensor_s5k3h2yx = {
	.name	= "msm_camera_s5k3h2yx",
	.dev	= {
		.platform_data = &msm_camera_sensor_s5k3h2yx_data,
	},
};
#endif
#ifdef CONFIG_S5K6AAFX
static int valente_wx_s5k6aafx_vreg_on(void)
{
	int rc;
	pr_info("[CAM] %s\n", __func__);
	/* analog */
	rc = camera_sensor_power_enable("8921_l9", 2800000, &reg_8921_l9);
	if (rc < 0) {
		pr_err("[CAM] sensor_power_enable\
			(\"8921_l9\", 2.8V) FAILED %d\n", rc);
		goto enable_analog_fail;
	}

	mdelay(1);
	/* digital */
	rc = gpio_request(VALENTE_WX_V_CAM2IO_1V8_EN, "V_CAM_D1V8");
	if (rc) {
		pr_err("[CAM] sensor_power_enable\
			(\"gpio %d\", 1.8V) FAILED %d\n",
			VALENTE_WX_V_CAM2IO_1V8_EN, rc);
		goto enable_digital_fail;
	}
	gpio_direction_output(VALENTE_WX_V_CAM2IO_1V8_EN, 1);
	gpio_free(VALENTE_WX_V_CAM2IO_1V8_EN);
	mdelay(1);
	/* IO */
	rc = camera_sensor_power_enable("8921_lvs6", 1800000, &reg_8921_lvs6);
	if (rc < 0) {
		pr_err("[CAM] sensor_power_enable\
			(\"8921_lvs6\", 1.8V) FAILED %d\n", rc);
		goto enable_io_fail;
	}
	mdelay(1);
	 /*STANDBY */
	rc = gpio_request(VALENTE_WX_CAM2_STBz, "CAM2_STB#");
	if (rc) {
		pr_err("[CAM] sensor_power_enable\
			(\"gpio %d\") FAILED %d\n",
			VALENTE_WX_CAM2_STBz, rc);
		goto enable_standby_fail;
	}
	gpio_direction_output(VALENTE_WX_CAM2_STBz, 1);
	gpio_free(VALENTE_WX_CAM2_STBz);

	return rc;

enable_standby_fail:
	camera_sensor_power_disable(reg_8921_lvs6);
enable_io_fail:
	rc = gpio_request(VALENTE_WX_V_CAM2IO_1V8_EN, "V_CAM_D1V8");
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"gpio %d\", 1.8V) FAILED %d\n",
			VALENTE_WX_V_CAM2IO_1V8_EN, rc);
	else {
		gpio_direction_output(VALENTE_WX_V_CAM2IO_1V8_EN, 0);
		gpio_free(VALENTE_WX_V_CAM2IO_1V8_EN);
	}
enable_digital_fail:
	camera_sensor_power_disable(reg_8921_l9);
enable_analog_fail:
	return rc;
}

static int valente_wx_s5k6aafx_vreg_off(void)
{
	int rc = 0;

	pr_info("[CAM] %s\n", __func__);
	/* Standby */
	rc = gpio_request(VALENTE_WX_CAM2_STBz, "CAM2_STB#");
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"gpio %d\") FAILED %d\n",
			VALENTE_WX_CAM2_STBz, rc);
	else {
		gpio_direction_output(VALENTE_WX_CAM2_STBz, 0);
		gpio_free(VALENTE_WX_CAM2_STBz);
	}
	mdelay(1);

	/* IO */
	rc = camera_sensor_power_disable(reg_8921_lvs6);
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"8921_lvs6\") FAILED %d\n", rc);

	mdelay(1);

	/* Digital*/
	rc = gpio_request(VALENTE_WX_V_CAM2IO_1V8_EN, "V_CAM_D1V8");
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"gpio %d\") FAILED %d\n",
			VALENTE_WX_V_CAM2IO_1V8_EN, rc);
	else {
		gpio_direction_output(VALENTE_WX_V_CAM2IO_1V8_EN, 0);
		gpio_free(VALENTE_WX_V_CAM2IO_1V8_EN);
	}
	mdelay(1);

	/* Analog */
	rc = camera_sensor_power_disable(reg_8921_l9);
	if (rc < 0)
		pr_err("[CAM] sensor_power_disable\
			(\"8921_l9\") FAILED %d\n", rc);
	return rc;
}

static struct msm_camera_sensor_platform_info sensor_s5k6aafx_board_info = {
	.mount_angle = 270,
	.mirror_flip = CAMERA_SENSOR_NONE,
	.sensor_reset_enable = 0,
	.sensor_reset	= VALENTE_WX_CAM2_RSTz,
	.sensor_pwd	= VALENTE_WX_CAM2_STBz,
	.vcm_pwd	= 0,
	.vcm_enable	= 0,
};

static struct msm_camera_sensor_flash_data flash_s5k6aafx = {
	.flash_type	= MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k6aafx_data = {
	.sensor_name	= "s5k6aafx",
	.sensor_reset	= VALENTE_WX_CAM2_RSTz,
	.sensor_pwd	= VALENTE_WX_CAM2_STBz,
	.vcm_pwd	= 0,
	.vcm_enable	= 0,
	.camera_power_on = valente_wx_s5k6aafx_vreg_on,
	.camera_power_off = valente_wx_s5k6aafx_vreg_off,
	.pdata	= &msm_camera_csi_device_data[1],
	.flash_data	= &flash_s5k6aafx,
	.sensor_platform_info = &sensor_s5k6aafx_board_info,
	.gpio_conf = &gpio_conf,
	.csi_if	= 1,
	.camera_type = FRONT_CAMERA_2D,
	.use_rawchip = 0,
	.mirror_mode = 1,
	.power_down_disable = false, /* true: disable pwd down function */
	.full_size_preview = false, /* true: use full size preview */
};

struct platform_device valente_wx_camera_sensor_s5k6aafx = {
	.name	= "msm_camera_s5k6aafx",
	.dev	= {
		.platform_data = &msm_camera_sensor_s5k6aafx_data,
	},
};
#endif

static void __init msm8960_init_cam(void)
{
	int i;
	struct platform_device *cam_dev[] = {
#ifdef CONFIG_S5K3H2YX
		&valente_wx_camera_sensor_s5k3h2yx,
#endif
#ifdef CONFIG_S5K6AAFX
		&valente_wx_camera_sensor_s5k6aafx,
#endif

	};

	for (i = 0; i < ARRAY_SIZE(cam_dev); i++) {
		struct msm_camera_sensor_info *s_info;
		s_info = cam_dev[i]->dev.platform_data;
		msm_get_cam_resources(s_info);
		platform_device_register(cam_dev[i]);
	}
	platform_device_register(&msm8960_device_csiphy0);
	platform_device_register(&msm8960_device_csiphy1);
	platform_device_register(&msm8960_device_csid0);
	platform_device_register(&msm8960_device_csid1);
	platform_device_register(&msm8960_device_ispif);
	platform_device_register(&msm8960_device_vfe);
	platform_device_register(&msm8960_device_vpe);
}

#ifdef CONFIG_HTC_BATT_8960
static struct htc_battery_platform_data htc_battery_pdev_data = {
	.guage_driver = 0,
	.chg_limit_active_mask = HTC_BATT_CHG_LIMIT_BIT_TALK |
								HTC_BATT_CHG_LIMIT_BIT_NAVI,
	.critical_low_voltage_mv = 3100,
	.critical_alarm_voltage_mv = 3000,
	.overload_vol_thr_mv = 4000,
	.overload_curr_thr_ma = 0,
	/* charger */
	.icharger.name = "pm8921",
	.icharger.get_charging_source = pm8921_get_charging_source,
	.icharger.get_charging_enabled = pm8921_get_charging_enabled,
	.icharger.set_charger_enable = pm8921_charger_enable,
	.icharger.set_pwrsrc_enable = pm8921_pwrsrc_enable,
	.icharger.set_pwrsrc_and_charger_enable =
						pm8921_set_pwrsrc_and_charger_enable,
	.icharger.set_limit_charge_enable = pm8921_limit_charge_enable,
	.icharger.is_ovp = pm8921_is_charger_ovp,
	.icharger.is_batt_temp_fault_disable_chg =
						pm8921_is_batt_temp_fault_disable_chg,
	.icharger.charger_change_notifier_register =
						cable_detect_register_notifier,
	.icharger.dump_all = pm8921_dump_all,
	.icharger.get_attr_text = pm8921_charger_get_attr_text,
	/* gauge */
	.igauge.name = "pm8921",
	.igauge.get_battery_voltage = pm8921_get_batt_voltage,
	.igauge.get_battery_current = pm8921_bms_get_batt_current,
	.igauge.get_battery_temperature = pm8921_get_batt_temperature,
	.igauge.get_battery_id = pm8921_get_batt_id,
	.igauge.get_battery_soc = pm8921_bms_get_batt_soc,
	.igauge.get_battery_cc = pm8921_bms_get_batt_cc,
	.igauge.is_battery_temp_fault = pm8921_is_batt_temperature_fault,
	.igauge.is_battery_full = pm8921_is_batt_full,
	.igauge.get_attr_text = pm8921_gauge_get_attr_text,
	.igauge.register_lower_voltage_alarm_notifier =
						pm8xxx_batt_lower_alarm_register_notifier,
	.igauge.enable_lower_voltage_alarm = pm8xxx_batt_lower_alarm_enable,
	.igauge.set_lower_voltage_alarm_threshold =
						pm8xxx_batt_lower_alarm_threshold_set,
};

static struct platform_device htc_battery_pdev = {
	.name = "htc_battery",
	.id = -1,
	.dev    = {
		.platform_data = &htc_battery_pdev_data,
	},
};
#endif /* CONFIG_HTC_BATT_8960 */

#ifdef CONFIG_MSM_BUS_SCALING

static struct msm_bus_vectors rotator_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors rotator_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1024 * 600 * 4 * 2 * 60),
		.ib  = (1024 * 600 * 4 * 2 * 60 * 1.5),
	},
};

static struct msm_bus_vectors rotator_vga_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (640 * 480 * 2 * 2 * 30),
		.ib  = (640 * 480 * 2 * 2 * 30 * 1.5),
	},
};
static struct msm_bus_vectors rotator_720p_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1280 * 736 * 2 * 2 * 30),
		.ib  = (1280 * 736 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_vectors rotator_1080p_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ROTATOR,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab  = (1920 * 1088 * 2 * 2 * 30),
		.ib  = (1920 * 1088 * 2 * 2 * 30 * 1.5),
	},
};

static struct msm_bus_paths rotator_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(rotator_init_vectors),
		rotator_init_vectors,
	},
	{
		ARRAY_SIZE(rotator_ui_vectors),
		rotator_ui_vectors,
	},
	{
		ARRAY_SIZE(rotator_vga_vectors),
		rotator_vga_vectors,
	},
	{
		ARRAY_SIZE(rotator_720p_vectors),
		rotator_720p_vectors,
	},
	{
		ARRAY_SIZE(rotator_1080p_vectors),
		rotator_1080p_vectors,
	},
};

struct msm_bus_scale_pdata rotator_bus_scale_pdata = {
	rotator_bus_scale_usecases,
	ARRAY_SIZE(rotator_bus_scale_usecases),
	.name = "rotator",
};

static struct msm_bus_vectors mdp_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

static struct msm_bus_vectors mdp_ui_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_vga_vectors[] = {
	/* VGA and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 216000000 * 2,
		.ib = 270000000 * 2,
	},
};

static struct msm_bus_vectors mdp_720p_vectors[] = {
	/* 720p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 230400000 * 2,
		.ib = 288000000 * 2,
	},
};

static struct msm_bus_vectors mdp_1080p_vectors[] = {
	/* 1080p and less video */
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 334080000 * 2,
		.ib = 417600000 * 2,
	},
};

static struct msm_bus_paths mdp_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(mdp_init_vectors),
		mdp_init_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_ui_vectors),
		mdp_ui_vectors,
	},
	{
		ARRAY_SIZE(mdp_vga_vectors),
		mdp_vga_vectors,
	},
	{
		ARRAY_SIZE(mdp_720p_vectors),
		mdp_720p_vectors,
	},
	{
		ARRAY_SIZE(mdp_1080p_vectors),
		mdp_1080p_vectors,
	},
};

static struct msm_bus_scale_pdata mdp_bus_scale_pdata = {
	mdp_bus_scale_usecases,
	ARRAY_SIZE(mdp_bus_scale_usecases),
	.name = "mdp",
};

#endif

int mdp_core_clk_rate_table[] = {
	85330000,
	85330000,
	160000000,
	200000000,
};

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = MDP_VSYNC_GPIO,
	.mdp_core_clk_rate = 85330000,
	.mdp_core_clk_table = mdp_core_clk_rate_table,
	.num_mdp_clk = ARRAY_SIZE(mdp_core_clk_rate_table),
#ifdef CONFIG_MSM_BUS_SCALING
	.mdp_bus_scale_table = &mdp_bus_scale_pdata,
#endif
	.mdp_rev = MDP_REV_42,
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
	.mem_hid = ION_CP_MM_HEAP_ID,
#else
	.mem_hid = MEMTYPE_EBI1,
#endif
};

void __init msm8960_mdp_writeback(struct memtype_reserve* reserve_table)
{
	mdp_pdata.ov0_wb_size = MSM_FB_OVERLAY0_WRITEBACK_SIZE;
	mdp_pdata.ov1_wb_size = MSM_FB_OVERLAY1_WRITEBACK_SIZE;
#if defined(CONFIG_ANDROID_PMEM) && !defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov0_wb_size;
	reserve_table[mdp_pdata.mem_hid].size +=
		mdp_pdata.ov1_wb_size;
#endif
}

static uint32_t headset_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_AUD_1WIRE_DO, 0, GPIO_CFG_OUTPUT,
		 GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_AUD_1WIRE_DI, 0, GPIO_CFG_INPUT,
		 GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

struct pm8xxx_gpio_init headset_rx_xa[] = {
	PM8XXX_GPIO_INIT(VALENTE_WX_PM_AUD_1WIRE_DO, PM_GPIO_DIR_IN,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_S4, PM_GPIO_STRENGTH_NO,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_PM_AUD_1WIRE_O, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_OPEN_DRAIN, 1, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_L17, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_AUD_REMO_PRESz, PM_GPIO_DIR_IN,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,
			 PM_GPIO_VIN_L17, PM_GPIO_STRENGTH_LOW,
			 PM_GPIO_FUNC_PAIRED, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_PM_AUD_1WIRE_DI, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_S4, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_PAIRED, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_AUD_HSMIC_BIAS_2V85_EN, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_L17, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
};

struct pm8xxx_gpio_init headset_rx_xb[] = {
	PM8XXX_GPIO_INIT(VALENTE_WX_PM_AUD_1WIRE_DO, PM_GPIO_DIR_IN,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_S4, PM_GPIO_STRENGTH_NO,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_PM_AUD_1WIRE_O, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_OPEN_DRAIN, 1, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_L17, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_AUD_REMO_PRESz, PM_GPIO_DIR_IN,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_DN,
			 PM_GPIO_VIN_L17, PM_GPIO_STRENGTH_LOW,
			 PM_GPIO_FUNC_PAIRED, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_PM_AUD_1WIRE_DI, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_S4, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_PAIRED, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_AUD_HSMIC_BIAS_2V85_EN, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_L17, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
	PM8XXX_GPIO_INIT(VALENTE_WX_AUD_LEVEL_SHIFTER_EN, PM_GPIO_DIR_OUT,
			 PM_GPIO_OUT_BUF_CMOS, 1, PM_GPIO_PULL_NO,
			 PM_GPIO_VIN_S4, PM_GPIO_STRENGTH_HIGH,
			 PM_GPIO_FUNC_NORMAL, 0, 0),
};

static void headset_init(void)
{
	int i = 0;
	int rc = 0;

	pr_info("[HS_BOARD] (%s) Headset initiation\n", __func__);

	gpio_tlmm_config(headset_gpio_table[0], GPIO_CFG_ENABLE);
	gpio_tlmm_config(headset_gpio_table[1], GPIO_CFG_ENABLE);
	if (system_rev == 0) {
		for (i = 0; i < ARRAY_SIZE(headset_rx_xa); i++) {
			rc = pm8xxx_gpio_config(headset_rx_xa[i].gpio,
						&headset_rx_xa[i].config);
			if (rc)
				pr_info("[HS_BOARD] %s: XA_Config ERROR: GPIO=%u, rc=%d\n",
					__func__, headset_rx_xa[i].gpio, rc);
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(headset_rx_xb); i++) {
			rc = pm8xxx_gpio_config(headset_rx_xb[i].gpio,
						&headset_rx_xb[i].config);
			if (rc)
				pr_info("[HS_BOARD] %s: XB or higher Config ERROR: GPIO=%u, rc=%d\n",
					__func__, headset_rx_xb[i].gpio, rc);
		}
		gpio_set_value(PM8921_GPIO_PM_TO_SYS(VALENTE_WX_AUD_LEVEL_SHIFTER_EN), 1);
	}
}

static void headset_power(int enable)
{
	int gpio = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_AUD_HSMIC_BIAS_2V85_EN);

	if (enable)
		gpio_set_value(gpio, 1);
	else
		gpio_set_value(gpio, 0);
}

/* HTC_HEADSET_PMIC Driver */
static struct htc_headset_pmic_platform_data htc_headset_pmic_data = {
	.driver_flag		= DRIVER_HS_PMIC_ADC,
	.hpin_gpio		= PM8921_GPIO_PM_TO_SYS(VALENTE_WX_EARPHONE_DETz),
	.hpin_irq		= 0,
	.key_gpio		= PM8921_GPIO_PM_TO_SYS(VALENTE_WX_AUD_REMO_PRESz),
	.key_irq		= 0,
	.key_enable_gpio	= 0,
	.adc_mic		= 0,
	.adc_remote		= {0, 57, 58, 147, 148, 339},
	.adc_mpp		= PM8XXX_AMUX_MPP_10,
	.adc_amux		= ADC_MPP_1_AMUX6,
	.hs_controller		= 0,
	.hs_switch		= 0,
};

static struct platform_device htc_headset_pmic = {
	.name	= "HTC_HEADSET_PMIC",
	.id	= -1,
	.dev	= {
		.platform_data	= &htc_headset_pmic_data,
	},
};

/* HTC_HEADSET_MGR Driver */
static struct platform_device *headset_devices[] = {
	&htc_headset_pmic,
	/* Please put the headset detection driver on the last */
};

static struct headset_adc_config htc_headset_mgr_config[] = {
	{
		.type = HEADSET_MIC,
		.adc_max = 1530,
		.adc_min = 1244,
	},
	{
		.type = HEADSET_BEATS,
		.adc_max = 1243,
		.adc_min = 916,
	},
	{
		.type = HEADSET_BEATS_SOLO,
		.adc_max = 915,
		.adc_min = 566,
	},
	{
		.type = HEADSET_MIC, /* No Metrico device */
		.adc_max = 565,
		.adc_min = 255,
	},
	{
		.type = HEADSET_NO_MIC,
		.adc_max = 254,
		.adc_min = 0,
	},
};

static struct htc_headset_mgr_platform_data htc_headset_mgr_data = {
	.driver_flag		= DRIVER_HS_MGR_FLOAT_DET,
	.headset_devices_num	= ARRAY_SIZE(headset_devices),
	.headset_devices	= headset_devices,
	.headset_config_num	= ARRAY_SIZE(htc_headset_mgr_config),
	.headset_config		= htc_headset_mgr_config,
	.headset_init		= headset_init,
	.headset_power		= headset_power,
};

static struct platform_device htc_headset_mgr = {
	.name	= "HTC_HEADSET_MGR",
	.id	= -1,
	.dev	= {
		.platform_data	= &htc_headset_mgr_data,
	},
};

static void headset_device_register(void)
{
	pr_info("[HS_BOARD] (%s) Headset device register\n", __func__);
	platform_device_register(&htc_headset_mgr);
}

static struct atmel_cfg ts_atmel_mferr_cfg_data[] = {
	{.objid = GEN_POWERCONFIG_T7,		.byte = 1,     .value = 255,    .orival = 12},
	{.objid = TOUCH_MULTITOUCHSCREEN_T9,    .byte = 7,     .value = 105,	.orival = 50},
	{.objid = TOUCH_MULTITOUCHSCREEN_T9,    .byte = 13,    .value = 1,	.orival = 15},
	{.objid = TOUCH_MULTITOUCHSCREEN_T9,    .byte = 31,    .value = 25,	.orival = 10},
	{.objid = TOUCH_MULTITOUCHSCREEN_T9,    .byte = 34,    .value = 3,	.orival = 2},
	{.objid = EXTRA_NOISE_SUPPRESSION_T58,  .byte = 0,     .value = 0,	.orival = 100},
	{.objid = EXTRA_NOISE_SUPPRESSION_T58,  .byte = 1,     .value = 0, 	.orival = 20},
	{.objid = EXTRA_NOISE_SUPPRESSION_T58,  .byte = 2,     .value = 0,	.orival = 3},
	{.objid = EXTRA_NOISE_SUPPRESSION_T58,  .byte = 3,     .value = 0,	.orival = 1},
	{.objid = EXTRA_NOISE_SUPPRESSION_T58,  .byte = 4,     .value = 0,	.orival = 50},
	{.objid = SPT_CTECONFIG_T46,            .byte = 3,     .value = 63,	.orival = 16},
	{.objid = PROCG_NOISESUPPRESSION_T48,   .byte = 8,     .value = 18,	.orival = 0},
	{.objid = PROCG_NOISESUPPRESSION_T48,   .byte = 9,     .value = 30,	.orival = 0},
	{.objid = PROCG_NOISESUPPRESSION_T48,   .byte = 11,    .value = 45,	.orival = 20},
};

static struct atmel_cfg ts_atmel_cfm_calb_data[] = {
	{.objid = GEN_ACQUISITIONCONFIG_T8,    .byte = 6,    .value = 255,	.orival = 5},
	{.objid = GEN_ACQUISITIONCONFIG_T8,    .byte = 7,    .value = 1,	.orival = 25},
	{.objid = GEN_ACQUISITIONCONFIG_T8,    .byte = 8,    .value = 0,	.orival = 5},
	{.objid = GEN_ACQUISITIONCONFIG_T8,    .byte = 9,    .value = 0,	.orival = 192},
};

static struct atmel_cfg ts_atmel_cable_cfg_data[] = {
	{.objid = TOUCH_MULTITOUCHSCREEN_T9,    .byte = 8,    .value = 3,	.orival = 2},
};

static int ts_atmel_power(int on)
{
		pr_info("%s: power %d\n", __func__, on);

		gpio_set_value(VALENTE_WX_TP_RSTz , 0);
		msleep(5);
		gpio_set_value(VALENTE_WX_TP_RSTz, 1);
		msleep(40);

		return 0;
}

static struct atmel_i2c_platform_data ts_atmel_data[] = {
	{
		.version = 0x0011,
		.build = 0xAA,
		.abs_x_min = 0,
		.abs_x_max = 539,
		.abs_y_min = 0,
		.abs_y_max = 959,
		.abs_pressure_min = 0,
		.abs_pressure_max = 255,
		.abs_width_min = 0,
		.abs_width_max = 20,
		.gpio_irq = VALENTE_WX_TP_ATTz,
		.power = ts_atmel_power,
		.unlock_attr = 1,
		.report_type = SYN_AND_REPORT_TYPE_B,
		.config_T6 = {0, 0, 0, 0, 0, 0},
		.config_T7 = {50, 12, 25},
		.config_T8 = {30, 0, 5, 5, 0, 0, 5, 25, 5, 192},
		.config_T9 = {131, 0, 0, 19, 11, 0, 32, 50, 2, 1,
				0, 3, 1, 15, 5, 10, 10, 10, 191, 3,
				27, 2, 0, 252, 8, 12, 150, 45, 165, 70,
				14, 10, 0, 0, 2},
		.config_T15 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.config_T18 = {0, 0},
		.config_T19 = {0, 0, 0, 60, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0},
		.config_T23 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0},
		.config_T25 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0},
		.config_T40 = {0, 0, 0, 0, 0},
		.config_T42 = {0, 0, 0, 0, 0, 0, 0, 0},
		.config_T46 = {0, 3, 16, 16, 0, 0, 0, 0, 0},
		.config_T47 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.config_T48 = {3, 0, 66, 0, 0, 0, 0, 0, 0, 0,
				16, 20, 0, 6, 6, 0, 0, 100, 4, 64,
				10, 0, 20, 5, 0, 38, 0, 5, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0},
		.config_T55 = {0, 0, 0, 0},
		.config_T58 = {100, 20, 3, 1, 50, 0, 0, 0, 0, 0,
				0},
		.object_crc       = {0x0F, 0xEB, 0x6B},
		.workaround       = TW_SHIFT,
		.cable_config	  = {	.cnt = ARRAY_SIZE(ts_atmel_cable_cfg_data),
					.cfg = ts_atmel_cable_cfg_data, },
		.mferr_config 	  = { 	.cnt = ARRAY_SIZE(ts_atmel_mferr_cfg_data),
					.cfg = ts_atmel_mferr_cfg_data, },
		.cfm_calb	  = {	.cnt = ARRAY_SIZE(ts_atmel_cfm_calb_data),
					.cfg = ts_atmel_cfm_calb_data, },
#if defined(CONFIG_TOUCH_KEY_FILTER)
		.flt_th	  = 945,
#endif
	},
	{
		.version = 0x0010,
		.build = 0xAA,
		.abs_x_min = 0,
		.abs_x_max = 539,
		.abs_y_min = 0,
		.abs_y_max = 959,
		.abs_pressure_min = 0,
		.abs_pressure_max = 255,
		.abs_width_min = 0,
		.abs_width_max = 20,
		.gpio_irq = VALENTE_WX_TP_ATTz,
		.power = ts_atmel_power,
		.unlock_attr = 1,
		.report_type = SYN_AND_REPORT_TYPE_B,
		.config_T6 = {0, 0, 0, 0, 0, 0},
		.config_T7 = {50, 12, 25},
		.config_T8 = {30, 0, 5, 5, 0, 0, 5, 25, 5, 192},
		.config_T9 = {131, 0, 0, 19, 11, 0, 32, 50, 2, 1,
				0, 3, 1, 15, 5, 10, 10, 10, 191, 3,
				27, 2, 0, 252, 8, 12, 150, 45, 165, 70,
				14, 10, 0, 0, 2},
		.config_T15 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.config_T18 = {0, 0},
		.config_T19 = {0, 0, 0, 60, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0},
		.config_T23 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0},
		.config_T25 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0},
		.config_T40 = {0, 0, 0, 0, 0},
		.config_T42 = {0, 0, 0, 0, 0, 0, 0, 0},
		.config_T46 = {0, 3, 16, 16, 0, 0, 0, 0, 0},
		.config_T47 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.config_T48 = {3, 0, 66, 0, 0, 0, 0, 0, 0, 0,
				16, 20, 0, 6, 6, 0, 0, 100, 4, 64,
				10, 0, 20, 5, 0, 38, 0, 5, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0},
		.config_T55 = {0, 0, 0, 0},
		.config_T58 = {100, 20, 3, 1, 50, 0, 0, 0, 0, 0,
				0},
		.object_crc       = {0x0F, 0xEB, 0x6B},
		.workaround       = TW_SHIFT,
		.cable_config	  = {	.cnt = ARRAY_SIZE(ts_atmel_cable_cfg_data),
					.cfg = ts_atmel_cable_cfg_data, },
		.mferr_config 	  = { 	.cnt = ARRAY_SIZE(ts_atmel_mferr_cfg_data),
					.cfg = ts_atmel_mferr_cfg_data, },
		.cfm_calb	  = {	.cnt = ARRAY_SIZE(ts_atmel_cfm_calb_data),
					.cfg = ts_atmel_cfm_calb_data, },
#if defined(CONFIG_TOUCH_KEY_FILTER)
		.flt_th	  = 945,
#endif
	},
};

struct i2c_board_info msm_i2c_gsbi3_info[] = {
	{
		I2C_BOARD_INFO(ATMEL_MXT224E_NAME, 0x94 >> 1),
		.platform_data = &ts_atmel_data,
		.irq = MSM_GPIO_TO_INT(VALENTE_WX_TP_ATTz)
	},
};

int cy8c_cs_reset(void)
{
	pr_info("[cap]%s Enter\n", __func__);

	gpio_set_value_cansleep(PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PMGPIO_CAP_RST), 1);
	msleep(5);
	gpio_set_value_cansleep(PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PMGPIO_CAP_RST), 0);
	msleep(50);
	return 0;
}

struct cy8c_i2c_cs_platform_data cs_cy8c_data[] = {
	{
		.gpio_irq = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PMGPIO_CAP_SENSOR_INTz),
		.gpio_rst = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PMGPIO_CAP_RST),
		.reset    = cy8c_cs_reset,
		.keycode  = {KEY_BACK, KEY_HOME, KEY_APP_SWITCH},
		.id       = {
			.config = CS_KEY_3,
		},
	},
        {
                .gpio_irq = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PMGPIO_CAP_SENSOR_INTz),
                .gpio_rst = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PMGPIO_CAP_RST),
                .reset    = cy8c_cs_reset,
                .keycode  = {KEY_HOME, KEY_APP_SWITCH, KEY_BACK, 0},
                .id       = {
                        .config = CS_KEY_4,
                },
        },
};

static struct i2c_board_info CAP_SENSOR_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(CYPRESS_CS_NAME, 0x40 >> 1),
		.platform_data = &cs_cy8c_data,
		.irq =  PM8921_GPIO_IRQ(PM8921_IRQ_BASE, VALENTE_WX_PMGPIO_CAP_SENSOR_INTz),
	},
};

static uint32_t usb_ID_PIN_input_table[] = {
	GPIO_CFG(VALENTE_WX_USB_ID1, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static uint32_t usb_ID_PIN_ouput_table[] = {
	GPIO_CFG(VALENTE_WX_USB_ID1, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

#ifdef CONFIG_FELICA_DD
static struct pm8xxx_gpio_init felica_dtyped_pmgpios[] = {
        PM8XXX_GPIO_OUTPUT_VIN_L17_FUNC(VALENTE_WX_FELICA_CEN, 0),
        PM8XXX_GPIO_OUTPUT_VIN_L17_FUNC(VALENTE_WX_FELICA_CEN, 1),
};

static struct pm8xxx_gpio_init felica_dtypecp_pmgpios[] = {
        PM8XXX_GPIO_OUTPUT_VIN_L17_FUNC(VALENTE_WX_FELICA_LOCK, 0),
        PM8XXX_GPIO_OUTPUT_VIN_L17_FUNC(VALENTE_WX_FELICA_LOCK, 1),
};

static void valente_wx_felica_dtype_suspend(void)
{
	pm8xxx_gpio_config(felica_dtyped_pmgpios[1].gpio, &felica_dtyped_pmgpios[1].config);
	pm8xxx_gpio_config(felica_dtypecp_pmgpios[0].gpio, &felica_dtypecp_pmgpios[0].config);

	return;
}

static void valente_wx_felica_suspend(void)
{
	printk(KERN_INFO "[FELICA_DD] %s START\n", __func__);
	valente_wx_felica_dtype_suspend();
	printk(KERN_INFO "[FELICA_DD] %s END\n", __func__);

	return;
}

static void valente_wx_felica_resume(void)
{
	printk(KERN_INFO "[FELICA_DD] %s START\n", __func__);
	printk(KERN_INFO "[FELICA_DD] %s END\n", __func__);

	return;
}

static uint32_t felica_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_FEL_PON, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_FEL_CEN, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_FEL_RFS, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_FEL_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_FEL_CON, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_FEL_RX, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_FEL_TX, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};

static void valente_wx_felica_setup_gpio(void)
{
	printk(KERN_INFO "[FELICA_DD] %s START\n", __func__);
	gpio_tlmm_config(felica_gpio_table[0], GPIO_CFG_ENABLE);
	gpio_tlmm_config(felica_gpio_table[1], GPIO_CFG_ENABLE);
	gpio_tlmm_config(felica_gpio_table[2], GPIO_CFG_ENABLE);
	gpio_tlmm_config(felica_gpio_table[3], GPIO_CFG_ENABLE);
	gpio_tlmm_config(felica_gpio_table[4], GPIO_CFG_ENABLE);
	gpio_tlmm_config(felica_gpio_table[5], GPIO_CFG_ENABLE);
	gpio_tlmm_config(felica_gpio_table[6], GPIO_CFG_ENABLE);
	printk(KERN_INFO "[FELICA_DD] %s END\n", __func__);

	return;
}

static struct felica_platform_data valente_wx_felica_data = {
	.pon_gpio = VALENTE_WX_FEL_PON,
	.cen_dtyp_d = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_FELICA_CEN),
	.cen_dtyp_cp = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_FELICA_LOCK),
	.cen_gpio = VALENTE_WX_FEL_CEN,
	.rfs_gpio = VALENTE_WX_FEL_RFS,
	.int_gpio = VALENTE_WX_FEL_INT,
	.con_gpio = VALENTE_WX_FEL_CON,
	.setup_gpio = valente_wx_felica_setup_gpio,
	.sleep_gpio = valente_wx_felica_suspend,
	.wakeup_gpio = valente_wx_felica_resume,
};

static struct platform_device valente_wx_felica_device = {
	.name = "felica",
	.id = 0,
	.dev		= {
		.platform_data	= &valente_wx_felica_data,
	},
};

int __init valente_wx_init_felica(void)
{
	printk(KERN_INFO "[FELICA_DD] %s\n", __func__);
	return platform_device_register(&valente_wx_felica_device);
}
#endif

void config_valente_wx_usb_id_gpios(bool output)
{
	if (output) {
		gpio_tlmm_config(usb_ID_PIN_ouput_table[0], GPIO_CFG_ENABLE);
		gpio_set_value(VALENTE_WX_USB_ID1, 1);
		pr_info("[CABLE] %s: %d output high\n",  __func__, VALENTE_WX_USB_ID1);
	} else {
		gpio_tlmm_config(usb_ID_PIN_input_table[0], GPIO_CFG_ENABLE);
		pr_info("[CABLE] %s: %d input none pull\n",  __func__, VALENTE_WX_USB_ID1);
	}
}

int64_t valente_wx_get_usbid_adc(void)
{
	struct pm8xxx_adc_chan_result result;
	int err = 0, adc = 0;

	err = pm8xxx_adc_mpp_config_read(PM8XXX_AMUX_MPP_7,
					ADC_MPP_1_AMUX6, &result);
	if (err) {
		pr_info("[CABLE] %s: get adc fail, err %d\n", __func__, err);
		return err;
	}
	pr_info("[CABLE] chan=%d, adc_code=%d, measurement=%lld, \
			physical=%lld\n", result.chan, result.adc_code,
			result.measurement, result.physical);
	adc = result.physical;
	return adc/1000;
}

static void mhl_sii9234_1v2_power(bool enable);
static void valente_wx_usb_dpdn_switch(int path);

static uint32_t usbuart_pin_enable_usb_table[] = {
	GPIO_CFG(VALENTE_WX_MHL_USB_ENz, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
};
static uint32_t usbuart_pin_enable_uart_table[] = {
	GPIO_CFG(VALENTE_WX_MHL_USB_ENz, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};
static uint32_t usbuart_pin_enable_uart_table_revA[] = {
	GPIO_CFG(VALENTE_WX_MHL_USB_ENz, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static void valente_wx_usb_uart_switch(int nvbus)
{
	printk(KERN_INFO "%s: %s, rev=%d\n", __func__, nvbus ? "uart" : "usb", system_rev);
	if(system_rev==0x80)
		gpio_tlmm_config(usbuart_pin_enable_uart_table_revA[0], GPIO_CFG_ENABLE);
	else {
		if(nvbus == 1) { /* vbus gone, pin pull up */
			gpio_tlmm_config(usbuart_pin_enable_uart_table[0], GPIO_CFG_ENABLE);
		} else {	/* vbus present, pin pull low */
			gpio_tlmm_config(usbuart_pin_enable_usb_table[0], GPIO_CFG_ENABLE);
		}
	}
}

static struct cable_detect_platform_data cable_detect_pdata = {
	.detect_type		= CABLE_TYPE_PMIC_ADC,
	.usb_id_pin_gpio	= VALENTE_WX_USB_ID1,
	.get_adc_cb		= valente_wx_get_usbid_adc,
	.config_usb_id_gpios	= config_valente_wx_usb_id_gpios,
	.mhl_reset_gpio		= VALENTE_WX_MHL_RSTz,
#ifdef CONFIG_FB_MSM_HDMI_MHL
	.mhl_1v2_power		= mhl_sii9234_1v2_power,
	.usb_dpdn_switch	= valente_wx_usb_dpdn_switch,
#endif
	.usb_uart_switch = valente_wx_usb_uart_switch,
};

static struct platform_device cable_detect_device = {
	.name   = "cable_detect",
	.id     = -1,
	.dev    = {
		.platform_data = &cable_detect_pdata,
	},
};


#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
static struct resource hdmi_msm_resources[] = {
	{
		.name  = "hdmi_msm_qfprom_addr",
		.start = 0x00700000,
		.end   = 0x007060FF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_hdmi_addr",
		.start = 0x04A00000,
		.end   = 0x04A00FFF,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "hdmi_msm_irq",
		.start = HDMI_IRQ,
		.end   = HDMI_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static int hdmi_enable_5v(int on);
static int hdmi_core_power(int on, int show);
/*static int hdmi_cec_power(int on);*/

static struct msm_hdmi_platform_data hdmi_msm_data = {
	.irq = HDMI_IRQ,
	.enable_5v = hdmi_enable_5v,
	.core_power = hdmi_core_power,
	/*.cec_power = hdmi_cec_power,*/
};

static struct platform_device hdmi_msm_device = {
	.name = "hdmi_msm",
	.id = 0,
	.num_resources = ARRAY_SIZE(hdmi_msm_resources),
	.resource = hdmi_msm_resources,
	.dev.platform_data = &hdmi_msm_data,
};
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */

#ifdef CONFIG_MSM_BUS_SCALING
static struct msm_bus_vectors dtv_bus_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};
static struct msm_bus_vectors dtv_bus_def_vectors[] = {
	{
		.src = MSM_BUS_MASTER_MDP_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 566092800 * 2,
		.ib = 707616000 * 2,
	},
};
static struct msm_bus_paths dtv_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(dtv_bus_init_vectors),
		dtv_bus_init_vectors,
	},
	{
		ARRAY_SIZE(dtv_bus_def_vectors),
		dtv_bus_def_vectors,
	},
};
static struct msm_bus_scale_pdata dtv_bus_scale_pdata = {
	dtv_bus_scale_usecases,
	ARRAY_SIZE(dtv_bus_scale_usecases),
	.name = "dtv",
};

static struct lcdc_platform_data dtv_pdata = {
	.bus_scale_table = &dtv_bus_scale_pdata,
};
#endif

static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);

#ifdef CONFIG_MSM_BUS_SCALING
	msm_fb_register_device("dtv", &dtv_pdata);
#endif
}

#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
static int hdmi_enable_5v(int on)
{
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (on) {
		rc = gpio_request(VALENTE_WX_V_BOOST_5V_EN, "HDMI_BOOST_5V");
		if (rc) {
			pr_err("'%s'(%d) gpio_request failed, rc=%d\n",
				"HDMI_BOOST_5V", VALENTE_WX_V_BOOST_5V_EN, rc);
			goto error;
		}
		gpio_set_value(VALENTE_WX_V_BOOST_5V_EN, 1);
		pr_info("%s(on): success\n", __func__);
	} else {
		gpio_set_value(VALENTE_WX_V_BOOST_5V_EN, 0);
		gpio_free(VALENTE_WX_V_BOOST_5V_EN);
		pr_info("%s(off): success\n", __func__);
	}

	prev_on = on;

	return 0;
error:
	return rc;
}

static int hdmi_core_power(int on, int show)
{
	static struct regulator *reg_8921_l23;
	static int prev_on;
	int rc;

	if (on == prev_on)
		return 0;

	if (!reg_8921_l23) {
		reg_8921_l23 = regulator_get(&hdmi_msm_device.dev, "hdmi_avdd");
		if (IS_ERR(reg_8921_l23)) {
			pr_err("could not get reg_8921_l23, rc = %ld\n",
				PTR_ERR(reg_8921_l23));
			return -ENODEV;
		}
		rc = regulator_set_voltage(reg_8921_l23, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage failed for 8921_l23, rc=%d\n", rc);
			return -EINVAL;
		}
	}
	if (on) {
		rc = regulator_set_optimum_mode(reg_8921_l23, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		rc = regulator_enable(reg_8921_l23);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"hdmi_avdd", rc);
			return rc;
		}

		pr_info("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_8921_l23);
		if (rc) {
			pr_err("disable reg_8921_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_set_optimum_mode(reg_8921_l23, 100);
		if (rc < 0) {
			pr_err("set_optimum_mode l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}
		pr_info("%s(off): success\n", __func__);
	}
	prev_on = on;
	return rc;
}
#endif /* CONFIG_FB_MSM_HDMI_MSM_PANEL */
#define _GET_REGULATOR(var, name) do {				\
	var = regulator_get(NULL, name);			\
	if (IS_ERR(var)) {					\
		pr_err("'%s' regulator not found, rc=%ld\n",	\
			name, IS_ERR(var));			\
		var = NULL;					\
		return -ENODEV;					\
	}							\
} while (0)

uint32_t mhl_usb_switch_ouput_table[] = {
	GPIO_CFG(VALENTE_WX_MHL_USBz_SEL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_4MA),
};


static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("[CAM] %s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}


static uint32_t mhl_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_MHL_RSTz, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_MHL_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};


#ifdef CONFIG_FB_MSM_HDMI_MHL
static void valente_wx_usb_dpdn_switch(int path)
{
	switch (path) {
	case PATH_USB:
	case PATH_MHL:
	{
		int polarity = 1; /* high = mhl */
		int mhl = (path == PATH_MHL);
		config_gpio_table(mhl_usb_switch_ouput_table, ARRAY_SIZE(mhl_usb_switch_ouput_table));
		gpio_set_value(VALENTE_WX_MHL_USBz_SEL, (mhl ^ !polarity) ? 1 : 0);
		pr_info("[CABLE]%s: Set %s path\n", __func__, mhl ? "MHL" : "USB");
		break;
	}
	}

	#ifdef CONFIG_FB_MSM_HDMI_MHL
	sii9234_change_usb_owner((path == PATH_MHL) ? 1 : 0);
	#endif /*CONFIG_FB_MSM_HDMI_MHL*/
}
#endif

#ifdef CONFIG_FB_MSM_HDMI_MHL
static struct regulator *reg_s4;
static struct regulator *reg_l12;
static struct regulator *reg_l11;
static struct regulator *reg_8921_l10;
static struct regulator *reg_8921_s2;

uint32_t msm_hdmi_off_gpio[] = {
	GPIO_CFG(VALENTE_WX_HDMI_DDC_CLK,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_HDMI_DDC_DATA,  0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(VALENTE_WX_HDMI_HPD,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};
uint32_t msm_hdmi_on_gpio[] = {
	GPIO_CFG(VALENTE_WX_HDMI_DDC_CLK,  1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_HDMI_DDC_DATA,  1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_HDMI_HPD,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};


void hdmi_hpd_feature(int enable);

static int mhl_sii9234_all_power(bool enable);
static int mhl_sii9234_power_vote(bool enable)
{
	int rc;

	if (!reg_8921_l10) {
		_GET_REGULATOR(reg_8921_l10, "8921_l10");
		rc = regulator_set_voltage(reg_8921_l10, 3000000, 3000000);
                if (rc) {
                        pr_err("%s: regulator_set_voltage reg_8921_l10 failed rc=%d\n",
                                __func__, rc);
                        return rc;
                }
	}
	if (!reg_8921_s2) {
		_GET_REGULATOR(reg_8921_s2, "8921_s2");
                rc = regulator_set_voltage(reg_8921_s2, 1300000, 1300000);
                if (rc) {
                        pr_err("%s: regulator_set_voltage reg_8921_s2 failed rc=%d\n",
                                __func__, rc);
                        return rc;
                }

	}

	if (enable) {
                if (reg_8921_l10) {
                        rc = regulator_enable(reg_8921_l10);
                        if (rc)
                                pr_warning("'%s' regulator enable failed, rc=%d\n",
                                        "reg_8921_l10", rc);
                }
                if (reg_8921_s2) {
                        rc = regulator_enable(reg_8921_s2);
                        if (rc)
                                pr_warning("'%s' regulator enable failed, rc=%d\n",
                                        "reg_8921_s2", rc);
                }
		pr_info("%s(on): success\n", __func__);
	} else {
		if (reg_8921_l10) {
			rc = regulator_disable(reg_8921_l10);
			if (rc)
				pr_warning("'%s' regulator disable failed, rc=%d\n",
					"reg_8921_l10", rc);
		}
		if (reg_8921_s2) {
			rc = regulator_disable(reg_8921_s2);
			if (rc)
				pr_warning("'%s' regulator disable failed, rc=%d\n",
					"reg_8921_s2", rc);
		}
		pr_info("%s(off): success\n", __func__);
	}
	return 0;
}


static void mhl_sii9234_1v2_power(bool enable)
{
	static bool prev_on;

	if (enable == prev_on)
		return;

	if (enable) {
		config_gpio_table(msm_hdmi_on_gpio, ARRAY_SIZE(msm_hdmi_on_gpio));
		hdmi_hpd_feature(1);
		pr_info("%s(on): success\n", __func__);
	} else {
		config_gpio_table(msm_hdmi_off_gpio, ARRAY_SIZE(msm_hdmi_off_gpio));
		hdmi_hpd_feature(0);
		pr_info("%s(off): success\n", __func__);
	}

	prev_on = enable;
}

static int mhl_sii9234_all_power(bool enable)
{
	static bool prev_on;
	int rc;
	if (enable == prev_on)
		return 0;

	if (!reg_s4)
		_GET_REGULATOR(reg_s4, "8921_s4");
	if (!reg_l12)
		_GET_REGULATOR(reg_l12, "8921_l12");
	if (!reg_l11)
		_GET_REGULATOR(reg_l11, "8921_l11");

	if (enable) {
		rc = regulator_set_voltage(reg_s4, 1800000, 1800000);
		if (rc) {
			pr_err("%s: regulator_set_voltage reg_s4 failed rc=%d\n",
				__func__, rc);
			return rc;
		}
		rc = regulator_set_voltage(reg_l11, 3300000, 3300000);
		if (rc) {
			pr_err("%s: regulator_set_voltage reg_l11 failed rc=%d\n",
				__func__, rc);
			return rc;
		}
		rc = regulator_set_voltage(reg_l12, 1200000, 1200000);
		if (rc) {
			pr_err("%s: regulator_set_voltage reg_l12 failed rc=%d\n",
				__func__, rc);
			return rc;
		}
		rc = regulator_enable(reg_s4);

		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_s4", rc);
			return rc;
		}
		rc = regulator_enable(reg_l11);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_l11", rc);
			return rc;
		}
		rc = regulator_enable(reg_l12);
		if (rc) {
			pr_err("'%s' regulator enable failed, rc=%d\n",
				"reg_l12", rc);
			return rc;
		}
		pr_info("%s(on): success\n", __func__);
	} else {
		rc = regulator_disable(reg_s4);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"reg_s4", rc);
		rc = regulator_disable(reg_l11);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"reg_l11", rc);
		rc = regulator_disable(reg_l12);
		if (rc)
			pr_warning("'%s' regulator disable failed, rc=%d\n",
				"reg_l12", rc);
		pr_info("%s(off): success\n", __func__);
	}

	prev_on = enable;

	return 0;
}

#ifdef CONFIG_FB_MSM_HDMI_MHL_SII9234

static int mhl_sii9234_power(int on)
{
	int rc = 0;

	switch (on) {
	case 0:
		mhl_sii9234_1v2_power(false);
		break;
	case 1:
		mhl_sii9234_all_power(true);
		config_gpio_table(mhl_gpio_table, ARRAY_SIZE(mhl_gpio_table));
		break;
	default:
		pr_warning("%s(%d) got unsupport parameter!!!\n", __func__, on);
		break;
	}
	return rc;
}

static T_MHL_PLATFORM_DATA mhl_sii9234_device_data = {
	.gpio_intr = VALENTE_WX_MHL_INT,
	.gpio_reset = VALENTE_WX_MHL_RSTz,
	.ci2ca = 0,
#ifdef CONFIG_FB_MSM_HDMI_MHL
	.mhl_usb_switch		= valente_wx_usb_dpdn_switch,
	.mhl_1v2_power = mhl_sii9234_1v2_power,
	.enable_5v = hdmi_enable_5v,
	.mhl_power_vote = mhl_sii9234_power_vote,
#ifdef CONFIG_FB_MSM_HDMI_MHL_SUPERDEMO
	.abs_x_min = 941,/* 0 */
	.abs_x_max = 31664,/* 32767 */
	.abs_y_min = 417,/* 0 */
	.abs_y_max = 32053,/* 32767 */
	.abs_pressure_min = 0,
	.abs_pressure_max = 255,
	.abs_width_min = 0,
	.abs_width_max = 20,
#endif
#endif
	.power = mhl_sii9234_power,
};

static struct i2c_board_info msm_i2c_gsbi8_mhl_sii9234_info[] =
{
	{
		I2C_BOARD_INFO(MHL_SII9234_I2C_NAME, 0x72 >> 1),
		.platform_data = &mhl_sii9234_device_data,
		.irq = VALENTE_WX_MHL_INT
	},
};

#endif
#endif

static void __init msm8960_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = MSM_FB_SIZE;
	addr = alloc_bootmem_align(size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));

}
#ifdef CONFIG_WCD9310_CODEC

#define TABLA_INTERRUPT_BASE (NR_MSM_IRQS + NR_GPIO_IRQS + NR_PM8921_IRQS)

/* Micbias setting is based on 8660 CDP/MTP/FLUID requirement
 * 4 micbiases are used to power various analog and digital
 * microphones operating at 1800 mV. Technically, all micbiases
 * can source from single cfilter since all microphones operate
 * at the same voltage level. The arrangement below is to make
 * sure all cfilters are exercised. LDO_H regulator ouput level
 * does not need to be as high as 2.85V. It is choosen for
 * microphone sensitivity purpose.
 */
static struct tabla_pdata tabla_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x10, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(62),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_TABLA_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.amic_settings = {
		.legacy_mode = 0x7F,
		.use_pdata = 0x7F,
	},
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 1800,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	}
};

static struct slim_device msm_slim_tabla = {
	.name = "tabla-slim",
	.e_addr = {0, 1, 0x10, 0, 0x17, 2},
	.dev = {
		.platform_data = &tabla_platform_data,
	},
};
static struct tabla_pdata tabla20_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x60, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(62),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_TABLA_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.amic_settings = {
		.legacy_mode = 0x7F,
		.use_pdata = 0x7F,
	},
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 1800,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	}
};

static struct slim_device msm_slim_tabla20 = {
	.name = "tabla2x-slim",
	.e_addr = {0, 1, 0x60, 0, 0x17, 2},
	.dev = {
		.platform_data = &tabla20_platform_data,
	},
};
#endif

static struct slim_boardinfo msm_slim_devices[] = {
#ifdef CONFIG_WCD9310_CODEC
	{
		.bus_num = 1,
		.slim_slave = &msm_slim_tabla,
	},
	{
		.bus_num = 1,
		.slim_slave = &msm_slim_tabla20,
	},
#endif
	/* add more slimbus slaves as needed */
};

#define MSM_WCNSS_PHYS	0x03000000
#define MSM_WCNSS_SIZE	0x280000

static struct resource resources_wcnss_wlan[] = {
	{
		.start	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.end	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.name	= "wcnss_wlanrx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.end	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.name	= "wcnss_wlantx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_WCNSS_PHYS,
		.end	= MSM_WCNSS_PHYS + MSM_WCNSS_SIZE - 1,
		.name	= "wcnss_mmio",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 84,
		.end	= 88,
		.name	= "wcnss_gpios_5wire",
		.flags	= IORESOURCE_IO,
	},
};

static struct qcom_wcnss_opts qcom_wcnss_pdata = {
	.has_48mhz_xo	= 1,
};

static struct platform_device msm_device_wcnss_wlan = {
	.name		= "wcnss_wlan",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_wcnss_wlan),
	.resource	= resources_wcnss_wlan,
	.dev		= {.platform_data = &qcom_wcnss_pdata},
};

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x18500000

#define QCE_HW_KEY_SUPPORT	0
#define QCE_SHA_HMAC_SUPPORT	1
#define QCE_SHARE_CE_RESOURCE	1
#define QCE_CE_SHARED		0

/* Begin Bus scaling definitions */
static struct msm_bus_vectors crypto_hw_init_vectors[] = {
        {
                .src = MSM_BUS_MASTER_ADM_PORT0,
                .dst = MSM_BUS_SLAVE_EBI_CH0,
                .ab = 0,
                .ib = 0,
        },
        {
                .src = MSM_BUS_MASTER_ADM_PORT1,
                .dst = MSM_BUS_SLAVE_GSBI1_UART,
                .ab = 0,
                .ib = 0,
        },
};

static struct msm_bus_vectors crypto_hw_active_vectors[] = {
        {
                .src = MSM_BUS_MASTER_ADM_PORT0,
                .dst = MSM_BUS_SLAVE_EBI_CH0,
                .ab = 70000000UL,
                .ib = 70000000UL,
        },
        {
                .src = MSM_BUS_MASTER_ADM_PORT1,
                .dst = MSM_BUS_SLAVE_GSBI1_UART,
                .ab = 2480000000UL,
                .ib = 2480000000UL,
        },
};

static struct msm_bus_paths crypto_hw_bus_scale_usecases[] = {
        {
                ARRAY_SIZE(crypto_hw_init_vectors),
                crypto_hw_init_vectors,
        },
        {
                ARRAY_SIZE(crypto_hw_active_vectors),
                crypto_hw_active_vectors,
        },
};

static struct msm_bus_scale_pdata crypto_hw_bus_scale_pdata = {
                crypto_hw_bus_scale_usecases,
                ARRAY_SIZE(crypto_hw_bus_scale_usecases),
                .name = "cryptohw",
};
/* End Bus Scaling Definitions*/

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV_CE_IN_CHAN,
		.end = DMOV_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV_CE_IN_CRCI,
		.end = DMOV_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV_CE_OUT_CRCI,
		.end = DMOV_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = &crypto_hw_bus_scale_pdata,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = &crypto_hw_bus_scale_pdata,
};

static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};
#endif


#define MSM_SHARED_RAM_PHYS 0x80000000

static void pm8xxx_adc_device_register(void)
{
	pr_info("%s: Register PM8921 ADC device\n", __func__);
	headset_device_register();
	/* default pull high*/
	gpio_tlmm_config(usbuart_pin_enable_uart_table[0], GPIO_CFG_ENABLE);
	platform_device_register(&cable_detect_device);
}

static struct pm8xxx_adc_amux pm8xxx_adc_channels_data[] = {
	{"vcoin", CHANNEL_VCOIN, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vbat", CHANNEL_VBAT, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"dcin", CHANNEL_DCIN, CHAN_PATH_SCALING4, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ichg", CHANNEL_ICHG, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"vph_pwr", CHANNEL_VPH_PWR, CHAN_PATH_SCALING2, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"ibat", CHANNEL_IBAT, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"batt_therm", CHANNEL_BATT_THERM, CHAN_PATH_SCALING1, AMUX_RSV2,
		ADC_DECIMATION_TYPE2, ADC_SCALE_BATT_THERM},
	{"batt_id", CHANNEL_BATT_ID, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"usbin", CHANNEL_USBIN, CHAN_PATH_SCALING3, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pmic_therm", CHANNEL_DIE_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PMIC_THERM},
	{"625mv", CHANNEL_625MV, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"125v", CHANNEL_125V, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"chg_temp", CHANNEL_CHG_TEMP, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
	{"pa_therm1", ADC_MPP_1_AMUX8, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PA_THERM},
	{"xo_therm", CHANNEL_MUXOFF, CHAN_PATH_SCALING1, AMUX_RSV0,
		ADC_DECIMATION_TYPE2, ADC_SCALE_XOTHERM},
	{"pa_therm0", ADC_MPP_1_AMUX3, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_PA_THERM},
	{"mpp_amux6", ADC_MPP_1_AMUX6, CHAN_PATH_SCALING1, AMUX_RSV1,
		ADC_DECIMATION_TYPE2, ADC_SCALE_DEFAULT},
};

static struct pm8xxx_adc_properties pm8xxx_adc_data = {
	.adc_vdd_reference	= 1800, /* milli-voltage for this adc */
	.bitresolution		= 15,
	.bipolar                = 0,
};

static const struct pm8xxx_adc_map_pt valente_wx_adcmap_btm_table[] = {
	{-200,	1671},
	{-190,	1663},
	{-180,	1654},
	{-170,	1646},
	{-160,	1636},
	{-150,	1627},
	{-140,	1617},
	{-130,	1606},
	{-120,	1595},
	{-110,	1584},
	{-100,	1572},
	{-90,	1560},
	{-80,	1548},
	{-70,	1534},
	{-60,	1521},
	{-50,	1507},
	{-40,	1492},
	{-30,	1477},
	{-20,	1462},
	{-10,	1446},
	{-0,	1430},
	{10,	1413},
	{20,	1396},
	{30,	1379},
	{40,	1361},
	{50,	1343},
	{60,	1325},
	{70,	1306},
	{80,	1287},
	{90,	1267},
	{100,	1248},
	{110,	1228},
	{120,	1208},
	{130,	1188},
	{140,	1168},
	{150,	1147},
	{160,	1127},
	{170,	1106},
	{180,	1086},
	{190,	1065},
	{200,	1044},
	{210,	1024},
	{220,	1004},
	{230,	983},
	{240,	963},
	{250,	943},
	{260,	923},
	{270,	903},
	{280,	884},
	{290,	864},
	{300,	845},
	{310,	827},
	{320,	808},
	{330,	790},
	{340,	772},
	{350,	755},
	{360,	738},
	{370,	721},
	{380,	704},
	{390,	688},
	{400,	672},
	{410,	657},
	{420,	642},
	{430,	627},
	{440,	613},
	{450,	599},
	{460,	585},
	{470,	572},
	{480,	559},
	{490,	547},
	{500,	535},
	{510,	523},
	{520,	511},
	{530,	500},
	{540,	489},
	{550,	479},
	{560,	469},
	{570,	459},
	{580,	449},
	{590,	440},
	{600,	431},
	{610,	423},
	{620,	414},
	{630,	406},
	{640,	398},
	{650,	390},
	{660,	383},
	{670,	376},
	{680,	369},
	{690,	363},
	{700,	356},
	{710,	350},
	{720,	344},
	{730,	338},
	{740,	333},
	{750,	327},
	{760,	322},
	{770,	317},
	{780,	312},
	{790,	308}
};

static struct pm8xxx_adc_map_table pm8xxx_adcmap_btm_table = {
	.table = valente_wx_adcmap_btm_table,
	.size = ARRAY_SIZE(valente_wx_adcmap_btm_table),
};

static struct pm8xxx_adc_platform_data pm8xxx_adc_pdata = {
	.adc_channel			= pm8xxx_adc_channels_data,
	.adc_num_board_channel		= ARRAY_SIZE(pm8xxx_adc_channels_data),
	.adc_prop			= &pm8xxx_adc_data,
	.adc_mpp_base			= PM8921_MPP_PM_TO_SYS(1),
	.adc_map_btm_table	= &pm8xxx_adcmap_btm_table,
	.pm8xxx_adc_device_register	= pm8xxx_adc_device_register,
};

static void __init valente_wx_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_msm8960_io();

	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

static void __init valente_wx_init_irq(void)
{
	msm_mpm_irq_extn_init();
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
						(void *)MSM_QGIC_CPU_BASE);

	/* Edge trigger PPIs except AVS_SVICINT and AVS_SVICINTSWDONE */
	writel_relaxed(0xFFFFD7FF, MSM_QGIC_DIST_BASE + GIC_DIST_CONFIG + 4);

	writel_relaxed(0x0000FFFF, MSM_QGIC_DIST_BASE + GIC_DIST_ENABLE_SET);
	mb();
}

/* MSM8960 has 5 SDCC controllers */
enum sdcc_controllers {
	SDCC1,
	SDCC2,
	SDCC3,
	SDCC4,
	SDCC5,
	MAX_SDCC_CONTROLLER
};

/* All SDCC controllers require VDD/VCC voltage */
static struct msm_mmc_reg_data mmc_vdd_reg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.name = "sdc_vdd",
		.set_voltage_sup = 1,
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
		.always_on = 1,
		.lpm_sup = 1,
		.lpm_uA = 9000,
		.hpm_uA = 200000, /* 200mA */
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.name = "sdc_vdd",
		.set_voltage_sup = 1,
		.high_vol_level = 2850000,
		.low_vol_level = 2850000,
		.hpm_uA = 600000, /* 600mA */
	}
};

/* Only slots having eMMC card will require VCCQ voltage */
static struct msm_mmc_reg_data mmc_vccq_reg_data[1] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.name = "sdc_vccq",
		.set_voltage_sup = 1,
		.always_on = 1,
		.high_vol_level = 1800000,
		.low_vol_level = 1800000,
		.hpm_uA = 200000, /* 200mA */
	}
};

/* All SDCC controllers may require voting for VDD PAD voltage */
static struct msm_mmc_reg_data mmc_vddp_reg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.name = "sdc_vddp",
		.set_voltage_sup = 1,
		.high_vol_level = 2850000,
		.low_vol_level = 1850000,
		.always_on = 1,
		.lpm_sup = 1,
		/* Max. Active current required is 16 mA */
		.hpm_uA = 16000,
		/*
		 * Sleep current required is ~300 uA. But min. vote can be
		 * in terms of mA (min. 1 mA). So let's vote for 2 mA
		 * during sleep.
		 */
		.lpm_uA = 2000,
	}
};

static struct msm_mmc_slot_reg_data mmc_slot_vreg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC1],
		.vccq_data = &mmc_vccq_reg_data[SDCC1],
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC3],
		.vddp_data = &mmc_vddp_reg_data[SDCC3],
	}
};

/* SDC1 pad data */
static struct msm_mmc_pad_drv sdc1_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_6MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_8MA}
};

static struct msm_mmc_pad_drv sdc1_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

/* SDC3 pad data */
static struct msm_mmc_pad_drv sdc3_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_16MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_14MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_14MA}
};

static struct msm_mmc_pad_drv sdc3_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_DOWN},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_DOWN}
};

struct msm_mmc_pad_pull_data mmc_pad_pull_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_pull_on_cfg,
		.off = sdc1_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_pull_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_pull_on_cfg,
		.off = sdc3_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_pull_on_cfg)
	},
};

struct msm_mmc_pad_drv_data mmc_pad_drv_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_drv_on_cfg,
		.off = sdc1_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_drv_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_drv_on_cfg,
		.off = sdc3_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_drv_on_cfg)
	},
};

struct msm_mmc_pad_data mmc_pad_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pull = &mmc_pad_pull_data[SDCC1],
		.drv = &mmc_pad_drv_data[SDCC1]
	},
	[SDCC3] = {
		.pull = &mmc_pad_pull_data[SDCC3],
		.drv = &mmc_pad_drv_data[SDCC3]
	},
};

struct msm_mmc_pin_data mmc_slot_pin_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pad_data = &mmc_pad_data[SDCC1],
	},
	[SDCC3] = {
		.pad_data = &mmc_pad_data[SDCC3],
	},
};

static unsigned int sdc1_sup_clk_rates[] = {
	400000, 24000000, 48000000, 51200000, 96000000, 102400000
};

static unsigned int sdc3_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000, 192000000
};

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static unsigned int sdc1_slot_type = MMC_TYPE_MMC;
static struct mmc_platform_data msm8960_sdc1_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.sup_clk_table	= sdc1_sup_clk_rates,
	.slot_type		= &sdc1_slot_type,
	.sup_clk_cnt	= ARRAY_SIZE(sdc1_sup_clk_rates),
	.pclk_src_dfab	= 1,
	.nonremovable	= 1,
	.vreg_data	= &mmc_slot_vreg_data[SDCC1],
	.pin_data	= &mmc_slot_pin_data[SDCC1],
	.uhs_caps	= (MMC_CAP_1_8V_DDR | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50)
};
#endif

#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static unsigned int valente_wx_sdslot_type = MMC_TYPE_SD;
static struct mmc_platform_data msm8960_sdc3_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.sup_clk_table	= sdc3_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc3_sup_clk_rates),
	.pclk_src_dfab	= 1,
#ifdef CONFIG_MMC_MSM_SDC3_WP_SUPPORT
	/*.wpswitch_gpio	= PM8921_GPIO_PM_TO_SYS(16),*/
#endif
	.vreg_data	= &mmc_slot_vreg_data[SDCC3],
	.pin_data	= &mmc_slot_pin_data[SDCC3],
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status_gpio	= PM8921_GPIO_PM_TO_SYS(23),
	.status_irq	= PM8921_GPIO_IRQ(PM8921_IRQ_BASE, 23),
	.irq_flags	= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
	.slot_type = &valente_wx_sdslot_type,
#if 0
	.xpc_cap	= 1,
	.uhs_caps	= (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50 |
			MMC_CAP_UHS_SDR104 | MMC_CAP_MAX_CURRENT_600),
#endif
};
#endif

static void __init msm8960_init_mmc(void)
{
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	int ret = 0;
#endif

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	msm8960_sdc1_data.swfi_latency = msm_rpm_get_swfi_latency();
	/* SDC1 : eMMC card connected */
	msm_add_sdcc(1, &msm8960_sdc1_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	/* SDC2 : WiMAX connected */
	ret = valente_wx_init_mmc();
	if (ret != 0)
		printk(KERN_ERR "%s: Unable to initialize MMC (SDCC2)\n", __func__);
#endif
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	msm8960_sdc3_data.swfi_latency = msm_rpm_get_swfi_latency();
	/* SDC3: External card slot */
	msm_add_sdcc(3, &msm8960_sdc3_data);
#endif
}

static void __init msm8960_init_buses(void)
{
#ifdef CONFIG_MSM_BUS_SCALING
	msm_bus_rpm_set_mt_mask();
	msm_bus_8960_apps_fabric_pdata.rpm_enabled = 1;
	msm_bus_8960_sys_fabric_pdata.rpm_enabled = 1;
	msm_bus_8960_mm_fabric_pdata.rpm_enabled = 1;
	msm_bus_apps_fabric.dev.platform_data =
		&msm_bus_8960_apps_fabric_pdata;
	msm_bus_sys_fabric.dev.platform_data = &msm_bus_8960_sys_fabric_pdata;
	msm_bus_mm_fabric.dev.platform_data = &msm_bus_8960_mm_fabric_pdata;
	msm_bus_sys_fpb.dev.platform_data = &msm_bus_8960_sys_fpb_pdata;
	msm_bus_cpss_fpb.dev.platform_data = &msm_bus_8960_cpss_fpb_pdata;
#endif
}
static struct msm_spi_platform_data msm8960_qup_spi_gsbi10_pdata = {
	.max_clock_speed = 27000000,
};


#ifdef CONFIG_USB_MSM_OTG_72K
static struct msm_otg_platform_data msm_otg_pdata;
#else
#define USB_5V_EN		42
static void msm_hsusb_vbus_power(bool on)
{
	int rc;
	static bool vbus_is_on;
	static struct regulator *mvs_otg_switch;
	struct pm_gpio param = {
		.direction	= PM_GPIO_DIR_OUT,
		.output_buffer	= PM_GPIO_OUT_BUF_CMOS,
		.output_value	= 1,
		.pull		= PM_GPIO_PULL_NO,
		.vin_sel	= PM_GPIO_VIN_S4,
		.out_strength	= PM_GPIO_STRENGTH_MED,
		.function	= PM_GPIO_FUNC_NORMAL,
	};

	if (vbus_is_on == on)
		return;

	if (on) {
		mvs_otg_switch = regulator_get(&msm8960_device_otg.dev,
					       "vbus_otg");
		if (IS_ERR(mvs_otg_switch)) {
			pr_err("Unable to get mvs_otg_switch\n");
			return;
		}

		rc = gpio_request(PM8921_GPIO_PM_TO_SYS(USB_5V_EN),
						"usb_5v_en");
		if (rc < 0) {
			pr_err("failed to request usb_5v_en gpio\n");
			goto put_mvs_otg;
		}

		if (regulator_enable(mvs_otg_switch)) {
			pr_err("unable to enable mvs_otg_switch\n");
			goto free_usb_5v_en;
		}

		rc = pm8xxx_gpio_config(PM8921_GPIO_PM_TO_SYS(USB_5V_EN),
				&param);
		if (rc < 0) {
			pr_err("failed to configure usb_5v_en gpio\n");
			goto disable_mvs_otg;
		}
		vbus_is_on = true;
		return;
	}
disable_mvs_otg:
		regulator_disable(mvs_otg_switch);
free_usb_5v_en:
		gpio_free(PM8921_GPIO_PM_TO_SYS(USB_5V_EN));
put_mvs_otg:
		regulator_put(mvs_otg_switch);
		vbus_is_on = false;
}

static int valente_wx_phy_init_seq[] = { 0x5f, 0x81, -1 };

static struct msm_otg_platform_data msm_otg_pdata = {
	.phy_init_seq		= valente_wx_phy_init_seq,
	.mode			= USB_PERIPHERAL,
	.otg_control		= OTG_PMIC_CONTROL,
	.phy_type		= SNPS_28NM_INTEGRATED_PHY,
	.vbus_power		= msm_hsusb_vbus_power,
	.power_budget		= 750,
	.ldo_power_collapse	= true,
};
#endif

#define PID_MAGIC_ID		0x71432909
#define SERIAL_NUM_MAGIC_ID	0x61945374
#define SERIAL_NUMBER_LENGTH	127
#define DLOAD_USB_BASE_ADD	0x2A03F0C8

struct magic_num_struct {
	uint32_t pid;
	uint32_t serial_num;
};

struct dload_struct {
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
	uint16_t	reserved4;
	uint16_t	pid;
	char		serial_number[SERIAL_NUMBER_LENGTH];
	uint16_t	reserved5;
	struct magic_num_struct magic_struct;
};

static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	struct dload_struct __iomem *dload = 0;

	dload = ioremap(DLOAD_USB_BASE_ADD, sizeof(*dload));
	if (!dload) {
		pr_err("%s: cannot remap I/O memory region: %08x\n",
					__func__, DLOAD_USB_BASE_ADD);
		return -ENXIO;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, dload, pid, snum);
	/* update pid */
	dload->magic_struct.pid = PID_MAGIC_ID;
	dload->pid = pid;

	/* update serial number */
	dload->magic_struct.serial_num = 0;
	if (!snum) {
		memset(dload->serial_number, 0, SERIAL_NUMBER_LENGTH);
		goto out;
	}

	dload->magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
	strlcpy(dload->serial_number, snum, SERIAL_NUMBER_LENGTH);
out:
	iounmap(dload);
	return 0;
}

#define SFAB_BW_MBPS(_bw) \
	{ \
		.vectors = (struct msm_bus_vectors[]){ \
			{\
				.src = MSM_BUS_MASTER_MSS, \
				.dst = MSM_BUS_SLAVE_EBI_CH0, \
				.ib = (_bw) * 1000000UL, \
				.ab = (_bw) *  100000UL, \
			}, \
		}, \
		.num_paths = 1, \
	}
#define CLKRGM_BSP_LPXO_FREQ_MHZ (27 * 8)
static struct msm_bus_paths usb_client_sfab_bw_level_tbl[] = {
	[0] =  SFAB_BW_MBPS(CLKRGM_BSP_LPXO_FREQ_MHZ), /* At least  27 MHz on bus. */
	[1] =  SFAB_BW_MBPS(528), /* At least  66.667 MHz on bus. */
	[2] =  SFAB_BW_MBPS(640), /* At least  80 MHz on bus. */
	[3] =  SFAB_BW_MBPS(800), /* At least  100 MHz on bus. */
	[4] =  SFAB_BW_MBPS(1064), /* At least 133 MHz on bus. */
	[5] =  SFAB_BW_MBPS(1312), /* At least 164 MHz on bus. */
};

static struct msm_bus_scale_pdata usb_client_sfab_bus_pdata = {
	.usecase = usb_client_sfab_bw_level_tbl,
	.num_usecases = ARRAY_SIZE(usb_client_sfab_bw_level_tbl),
	.active_only = 1,
	.name = "usb_client_systemclock",
};

static void usb_sfab_lock_control(int lock)
{
	static u32 sfab_bus_client;
	static int initial = 0;
	int ret;

	if (!initial) {
		sfab_bus_client = msm_bus_scale_register_client(&usb_client_sfab_bus_pdata);
		if (!sfab_bus_client) {
			pr_err("[USB:ERR] %s unable to register bus client\n", __func__);
			return;
		}
		pr_info("[USB] %s: register sfab usb client\n", __func__);
		initial = 1;
	}

	/* Update bandwidth if request has changed. This may sleep. */
	if (lock)
		ret = msm_bus_scale_client_update_request(sfab_bus_client, 4);
	else
		ret = msm_bus_scale_client_update_request(sfab_bus_client, 0);

	if (ret)
		pr_err("[USB:ERR] %s  bandwidth request failed (%d)\n", __func__, ret);
}

static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x0BB4,
	.product_id	= 0x0cf2,
	.version	= 0x0100,
	.product_name		= "Android Phone",
	.manufacturer_name	= "HTC",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
	.usb_id_pin_gpio = VALENTE_WX_USB_ID1,
	.usb_rmnet_interface = "smd,bam",
	.sfab_lock = usb_sfab_lock_control,
	.nluns = 2,
	.fserial_init_string = "smd:modem,tty,tty,tty:serial",
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

void valente_wx_add_usb_devices(void)
{
	printk(KERN_INFO "%s rev: %d\n", __func__, system_rev);
	android_usb_pdata.products[0].product_id =
			android_usb_pdata.product_id;


	/* diag bit set */
	if (get_radio_flag() & 0x20000) {
		android_usb_pdata.diag_init = 1;
		android_usb_pdata.modem_init = 1;
		android_usb_pdata.rmnet_init = 1;
	}

	/* add cdrom support in normal mode */
	if (board_mfg_mode() == 0) {
		android_usb_pdata.nluns = 3;
		android_usb_pdata.cdrom_lun = 0x4;
	}

	platform_device_register(&msm8960_device_gadget_peripheral);
	platform_device_register(&android_usb_device);
	printk(KERN_INFO "%s: OTG_PMIC_CONTROL in rev: %d\n",
			__func__, system_rev);
}

static int __init board_serialno_setup(char *serialno)
{
	android_usb_pdata.serial_number = serialno;
	return 1;
}
__setup("androidboot.serialno=", board_serialno_setup);

static uint8_t spm_wfi_cmd_sequence[] __initdata = {
			0x03, 0x0f,
};

static uint8_t spm_power_collapse_without_rpm[] __initdata = {
			0x00, 0x24, 0x54, 0x10,
			0x09, 0x03, 0x01,
			0x10, 0x54, 0x30, 0x0C,
			0x24, 0x30, 0x0f,
};

static uint8_t spm_power_collapse_with_rpm[] __initdata = {
			0x00, 0x24, 0x54, 0x10,
			0x09, 0x07, 0x01, 0x0B,
			0x10, 0x54, 0x30, 0x0C,
			0x24, 0x30, 0x0f,
};

static struct msm_spm_seq_entry msm_spm_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SECURE] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
		.reg_init_values[MSM_SPM_REG_SAW2_VCTL] = 0xB0,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_seq_list),
		.modes = msm_spm_seq_list,
	},
	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SECURE] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
		.reg_init_values[MSM_SPM_REG_SAW2_VCTL] = 0xB0,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_seq_list),
		.modes = msm_spm_seq_list,
	},
};

#ifdef CONFIG_PERFLOCK
static unsigned valente_wx_perf_acpu_table[] = {
	810000000, /* LOWEST */
	918000000, /* LOW */
	1026000000, /* MEDIUM */
	1242000000,/* HIGH */
	1512000000, /* HIGHEST */
};

static struct perflock_platform_data valente_wx_perflock_data = {
	.perf_acpu_table = valente_wx_perf_acpu_table,
	.table_size = ARRAY_SIZE(valente_wx_perf_acpu_table),
};

static unsigned valente_wx_cpufreq_ceiling_acpu_table[] = {
	702000000,
	918000000,
	1026000000,
};

static struct perflock_platform_data valente_wx_cpufreq_ceiling_data = {
	.perf_acpu_table = valente_wx_cpufreq_ceiling_acpu_table,
	.table_size = ARRAY_SIZE(valente_wx_cpufreq_ceiling_acpu_table),
};
#endif

static uint8_t l2_spm_wfi_cmd_sequence[] __initdata = {
			0x00, 0x20, 0x03, 0x20,
			0x00, 0x0f,
};

static uint8_t l2_spm_gdhs_cmd_sequence[] __initdata = {
			0x00, 0x20, 0x34, 0x64,
			0x48, 0x07, 0x48, 0x20,
			0x50, 0x64, 0x04, 0x34,
			0x50, 0x0f,
};
static uint8_t l2_spm_power_off_cmd_sequence[] __initdata = {
			0x00, 0x10, 0x34, 0x64,
			0x48, 0x07, 0x48, 0x10,
			0x50, 0x64, 0x04, 0x34,
			0x50, 0x0F,
};

static struct msm_spm_seq_entry msm_spm_l2_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_L2_MODE_RETENTION,
		.notify_rpm = false,
		.cmd = l2_spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_L2_MODE_GDHS,
		.notify_rpm = true,
		.cmd = l2_spm_gdhs_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_L2_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = l2_spm_power_off_cmd_sequence,
	},
};


static struct msm_spm_platform_data msm_spm_l2_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW_L2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SECURE] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x00A000AE,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A00020,
		.modes = msm_spm_l2_seq_list,
		.num_modes = ARRAY_SIZE(msm_spm_l2_seq_list),
	},
};

#ifdef CONFIG_VIDEO_NMI
static uint32_t gsbi1_gpio_table[] = {
        GPIO_CFG(VALENTE_WX__1SEG_I2C_SDA, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
        GPIO_CFG(VALENTE_WX__1SEG_I2C_SCL, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi1_gpio_table_gpio[] = {
	GPIO_CFG(VALENTE_WX__1SEG_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX__1SEG_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

#endif

static uint32_t gsbi3_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_TP_I2C_SDA, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_TP_I2C_SCL, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi3_gpio_table_gpio[] = {
	GPIO_CFG(VALENTE_WX_TP_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_TP_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

/* CAMERA setting */
static uint32_t gsbi4_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_CAM_I2C_SDA, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_CAM_I2C_SCL, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi4_gpio_table_gpio[] = {
	GPIO_CFG(VALENTE_WX_CAM_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_CAM_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi8_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_MC_I2C_SDA, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_MC_I2C_SCL, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi8_gpio_table_gpio[] = {
	GPIO_CFG(VALENTE_WX_MC_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_MC_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi12_gpio_table[] = {
	GPIO_CFG(VALENTE_WX_SR_I2C_SDA, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_SR_I2C_SCL, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static uint32_t gsbi12_gpio_table_gpio[] = {
	GPIO_CFG(VALENTE_WX_SR_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_SR_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
};

static void gsbi_qup_i2c_gpio_config(int adap_id, int config_type)
{
	printk(KERN_INFO "%s(): adap_id = %d, config_type = %d \n", __func__, adap_id, config_type);
#ifdef CONFIG_VIDEO_NMI
	if ((adap_id == MSM_8960_GSBI1_QUP_I2C_BUS_ID) && (config_type == 1)) {
		gpio_tlmm_config(gsbi1_gpio_table[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi1_gpio_table[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI1_QUP_I2C_BUS_ID) && (config_type == 0)) {
		gpio_tlmm_config(gsbi1_gpio_table_gpio[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi1_gpio_table_gpio[1], GPIO_CFG_ENABLE);
	}
#endif
	if ((adap_id == MSM_8960_GSBI3_QUP_I2C_BUS_ID) && (config_type == 1)) {
		gpio_tlmm_config(gsbi3_gpio_table[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi3_gpio_table[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI3_QUP_I2C_BUS_ID) && (config_type == 0)) {
		gpio_tlmm_config(gsbi3_gpio_table_gpio[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi3_gpio_table_gpio[1], GPIO_CFG_ENABLE);
	}

	/* CAMERA setting */
	if ((adap_id == MSM_8960_GSBI4_QUP_I2C_BUS_ID) && (config_type == 1)) {
		gpio_tlmm_config(gsbi4_gpio_table[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi4_gpio_table[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI4_QUP_I2C_BUS_ID) && (config_type == 0)) {
		gpio_tlmm_config(gsbi4_gpio_table_gpio[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi4_gpio_table_gpio[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI8_QUP_I2C_BUS_ID) && (config_type == 1)) {
		gpio_tlmm_config(gsbi8_gpio_table[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi8_gpio_table[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI8_QUP_I2C_BUS_ID) && (config_type == 0)) {
		gpio_tlmm_config(gsbi8_gpio_table_gpio[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi8_gpio_table_gpio[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI12_QUP_I2C_BUS_ID) && (config_type == 1)) {
		gpio_tlmm_config(gsbi12_gpio_table[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi12_gpio_table[1], GPIO_CFG_ENABLE);
	}

	if ((adap_id == MSM_8960_GSBI12_QUP_I2C_BUS_ID) && (config_type == 0)) {
		gpio_tlmm_config(gsbi12_gpio_table_gpio[0], GPIO_CFG_ENABLE);
		gpio_tlmm_config(gsbi12_gpio_table_gpio[1], GPIO_CFG_ENABLE);
	}
}

#ifdef CONFIG_VIDEO_NMI
static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi1_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};
#endif

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi3_pdata = {
	.clk_freq = 100000,	/* use 100Mhz first, then 400Mhz */
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi4_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi8_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
	.share_uart_flag = 1,	/* check if QUP-I2C and Uart share the gisb */
};

static struct msm_i2c_platform_data msm8960_i2c_qup_gsbi12_pdata = {
	.clk_freq = 400000,
	.src_clk_rate = 24000000,
	.msm_i2c_config_gpio = gsbi_qup_i2c_gpio_config,
	.share_uart_flag = 1,	/* check if QUP-I2C and Uart share the gisb */
};

static struct irda_platform_data msm8960_irda_gsbi12_pdata = {
	.irda_shutdown_gpio = VALENTE_WX_SIR_SD,
 };

static struct mpu3050_platform_data mpu3050_data = {
	.int_config = 0x10,
	.orientation = { 1, 0, 0,
			 0, 1, 0,
			 0, 0, 1 },
	.level_shifter = 0,

	.accel = {
		.get_slave_descr = get_accel_slave_descr,
		.adapt_num = MSM_8960_GSBI12_QUP_I2C_BUS_ID, /* The i2c bus to which the mpu device is connected */
		.bus = EXT_SLAVE_BUS_SECONDARY,
		.address = 0x30 >> 1,
			.orientation = { 1, 0, 0,
					 0, 1, 0,
					 0, 0, 1 },

	},

	.compass = {
		.get_slave_descr = get_compass_slave_descr,
		.adapt_num = MSM_8960_GSBI12_QUP_I2C_BUS_ID, /* The i2c bus to which the mpu device is connected */
		.bus = EXT_SLAVE_BUS_PRIMARY,
		.address = 0x1A >> 1,
			.orientation = {  1, 0, 0,
					  0, 1, 0,
					  0, 0, 1 },
	},
};

static struct i2c_board_info __initdata mpu3050_GSBI12_boardinfo[] = {
	{
		I2C_BOARD_INFO("mpu3050", 0xD0 >> 1),
		.irq = MSM_GPIO_TO_INT(VALENTE_WX_GYRO_INT),
		.platform_data = &mpu3050_data,
	},
};

static struct msm_rpm_platform_data msm_rpm_data = {
	.reg_base_addrs = {
		[MSM_RPM_PAGE_STATUS] = MSM_RPM_BASE,
		[MSM_RPM_PAGE_CTRL] = MSM_RPM_BASE + 0x400,
		[MSM_RPM_PAGE_REQ] = MSM_RPM_BASE + 0x600,
		[MSM_RPM_PAGE_ACK] = MSM_RPM_BASE + 0xa00,
		[MSM_RPM_PAGE_STAT] = MSM_RPM_BASE + 0x5204,
	},

	.irq_ack = RPM_APCC_CPU0_GP_HIGH_IRQ,
	.irq_err = RPM_APCC_CPU0_GP_LOW_IRQ,
	.irq_vmpm = RPM_APCC_CPU0_GP_MEDIUM_IRQ,
	.msm_apps_ipc_rpm_reg = MSM_APCS_GCC_BASE + 0x008,
	.msm_apps_ipc_rpm_val = 4,
};

static struct msm_pm_sleep_status_data msm_pm_slp_sts_data = {
	.base_addr = MSM_ACC0_BASE + 0x08,
	.cpu_offset = MSM_ACC1_BASE - MSM_ACC0_BASE,
	.mask = 1UL << 13,
};

static struct platform_device msm_device_saw_core0 = {
	.name          = "saw-regulator",
	.id            = 0,
	.dev	= {
		.platform_data = &msm_saw_regulator_pdata_s5,
	},
};

static struct platform_device msm_device_saw_core1 = {
	.name          = "saw-regulator",
	.id            = 1,
	.dev	= {
		.platform_data = &msm_saw_regulator_pdata_s6,
	},
};

static struct tsens_platform_data msm_tsens_pdata  = {
		.slope			= {910, 910, 910, 910, 910},
		.tsens_factor		= 1000,
		.hw_type		= MSM_8960,
		.tsens_num_sensor	= 5,
};

#ifdef CONFIG_MSM_FAKE_BATTERY
static struct platform_device fish_battery_device = {
	.name = "fish_battery",
};
#endif

static struct msm_rpm_log_platform_data msm_rpm_log_pdata = {
	.phys_addr_base = 0x0010C000,
	.reg_offsets = {
		[MSM_RPM_LOG_PAGE_INDICES] = 0x00000080,
		[MSM_RPM_LOG_PAGE_BUFFER]  = 0x000000A0,
	},
	.phys_size = SZ_8K,
	.log_len = 4096,		  /* log's buffer length in bytes */
	.log_len_mask = (4096 >> 2) - 1,  /* length mask in units of u32 */
};

static struct platform_device msm_rpm_log_device = {
	.name	= "msm_rpm_log",
	.id	= -1,
	.dev	= {
		.platform_data = &msm_rpm_log_pdata,
	},
};

#define SHARED_IMEM_TZ_BASE 0x2a03f720
static struct resource tzlog_resources[] = {
	{
		.start = SHARED_IMEM_TZ_BASE,
		.end = SHARED_IMEM_TZ_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device msm_device_tz_log = {
	.name		= "tz_log",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(tzlog_resources),
	.resource	= tzlog_resources,
};

static struct platform_device scm_memchk_device = {
	.name		= "scm-memchk",
	.id		= -1,
};

#define MSM_RAM_CONSOLE_BASE   MSM_HTC_RAM_CONSOLE_PHYS
#define MSM_RAM_CONSOLE_SIZE   MSM_HTC_RAM_CONSOLE_SIZE

static struct resource ram_console_resources[] = {
	{
		.start	= MSM_RAM_CONSOLE_BASE,
		.end	= MSM_RAM_CONSOLE_BASE + MSM_RAM_CONSOLE_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device ram_console_device = {
	.name		= "ram_console",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ram_console_resources),
	.resource	= ram_console_resources,
};

static struct platform_device *common_devices[] __initdata = {
	&ram_console_device,
	&msm8960_device_dmov,
	&msm_device_smd,
	&msm8960_device_uart_gsbi8,
	&msm_device_uart_dm6,
	&msm_device_saw_core0,
	&msm_device_saw_core1,
	&msm8960_device_ext_5v_vreg,
	&msm8960_device_ssbi_pmic,
	&msm8960_device_qup_spi_gsbi10,
#ifdef CONFIG_VIDEO_NMI
	&msm8960_device_qup_i2c_gsbi1,
#endif
	&msm8960_device_qup_i2c_gsbi3,
	&msm8960_device_qup_i2c_gsbi4,
	&msm8960_device_qup_i2c_gsbi8,
#ifndef CONFIG_MSM_DSPS
	&msm8960_device_qup_i2c_gsbi12,
#endif
	&msm8960_device_uart_gsbi12,
#ifdef CONFIG_GSBI5_UARTDM
	&msm_device_uart_dm5,
#endif
	&msm_slim_ctrl,
	&msm_device_wcnss_wlan,
#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&msm_device_sps,
#ifdef CONFIG_MSM_FAKE_BATTERY
	&fish_battery_device,
#endif
	&fmem_device,
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	&android_pmem_device,
	&android_pmem_adsp_device,
#endif
	&android_pmem_audio_device,
#endif
	&msm_fb_device,
	&msm_device_vidc,
	&msm_device_bam_dmux,
	&msm_fm_platform_init,

#ifdef CONFIG_HW_RANDOM_MSM
	&msm_device_rng,
#endif
	&msm_rpm_device,
#ifdef CONFIG_ION_MSM
	&ion_dev,
#endif
	&msm_rpm_log_device,
	&msm_rpm_stat_device,
#ifdef CONFIG_MSM_QDSS
	&msm_etb_device,
	&msm_tpiu_device,
	&msm_funnel_device,
	&msm_etm_device,
#endif
	&msm8960_device_watchdog,
#ifdef CONFIG_MSM_RTB
	&msm_rtb_device,
#endif
	&msm8960_device_cache_erp,
#ifdef CONFIG_MSM_CACHE_DUMP
	&msm_cache_dump_device,
#endif
#ifdef CONFIG_HTC_BATT_8960
	&htc_battery_pdev,
#endif

#if defined(CONFIG_TSIF) || defined(CONFIG_TSIF_MODULE)
#ifdef CONFIG_MSM_USE_TSIF1
	&msm_device_tsif[1],
#else
	&msm_device_tsif[0],
#endif /* CONFIG_MSM_USE_TSIF1 */
#endif /* CONFIG_TSIF */

};

static struct platform_device *valente_wx_devices[] __initdata = {
	&msm_8960_q6_lpass,
	&msm_8960_q6_mss_fw,
	&msm_8960_q6_mss_sw,
	&msm_8960_riva,
	&msm_pil_tzapps,
	&msm8960_device_otg,
	&msm_device_hsusb_host,
	&msm_pcm,
	&msm_pcm_routing,
	&msm_cpudai0,
	&msm_cpudai1,
	&msm_cpudai_hdmi_rx,
	&msm_cpudai_bt_rx,
	&msm_cpudai_bt_tx,
	&msm_cpudai_fm_rx,
	&msm_cpudai_fm_tx,
	&msm_cpudai_auxpcm_rx,
	&msm_cpudai_auxpcm_tx,
	&msm_cpu_fe,
	&msm_stub_codec,
	&msm_kgsl_3d0,
#ifdef CONFIG_MSM_KGSL_2D
	&msm_kgsl_2d0,
	&msm_kgsl_2d1,
#endif
#ifdef CONFIG_MSM_GEMINI
	&msm8960_gemini_device,
#endif
#ifdef CONFIG_RAWCHIP
	&msm_rawchip_device,
#endif

	&msm_voice,
	&msm_voip,
	&msm_lpa_pcm,
	&msm_cpudai_afe_01_rx,
	&msm_cpudai_afe_01_tx,
	&msm_cpudai_afe_02_rx,
	&msm_cpudai_afe_02_tx,
	&msm_pcm_afe,
#ifdef CONFIG_FB_MSM_HDMI_MSM_PANEL
	&hdmi_msm_device,
#endif
	&msm_compr_dsp,
	&msm_cpudai_incall_music_rx,
	&msm_cpudai_incall_record_rx,
	&msm_cpudai_incall_record_tx,
	&msm_pcm_hostless,
	&msm_bus_apps_fabric,
	&msm_bus_sys_fabric,
	&msm_bus_mm_fabric,
	&msm_bus_sys_fpb,
	&msm_bus_cpss_fpb,
    &msm_device_tz_log,
	&scm_memchk_device,
};

static void __init msm8960_i2c_init(void)
{
#ifdef CONFIG_VIDEO_NMI
	msm8960_device_qup_i2c_gsbi1.dev.platform_data =
					&msm8960_i2c_qup_gsbi1_pdata;
#endif
	msm8960_device_qup_i2c_gsbi3.dev.platform_data =
					&msm8960_i2c_qup_gsbi3_pdata;

	msm8960_device_qup_i2c_gsbi4.dev.platform_data =
					&msm8960_i2c_qup_gsbi4_pdata;

	msm8960_device_qup_i2c_gsbi8.dev.platform_data =
					&msm8960_i2c_qup_gsbi8_pdata;

	msm8960_device_qup_i2c_gsbi12.dev.platform_data =
					&msm8960_i2c_qup_gsbi12_pdata;
}

static void __init msm8960_gfx_init(void)
{
	uint32_t soc_platform_version = socinfo_get_version();
	if (SOCINFO_VERSION_MAJOR(soc_platform_version) == 1) {
		struct kgsl_device_platform_data *kgsl_3d0_pdata =
				msm_kgsl_3d0.dev.platform_data;
		kgsl_3d0_pdata->pwrlevel[0].gpu_freq = 320000000;
		kgsl_3d0_pdata->pwrlevel[1].gpu_freq = 266667000;
		kgsl_3d0_pdata->nap_allowed = false;
	}
}

#ifdef CONFIG_HTC_BATT_8960
static struct pm8921_charger_batt_param chg_batt_params[] = {
	/* for normal type battery */
	[0] = {
		.max_voltage = 4200,
	},
	/* for HV type battery */
	[1] = {
		.max_voltage = 4340,
	},
};

static struct single_row_lut fcc_temp_id_1 = {
	.x	= {-20, -10, 0, 5, 10, 20, 30, 40},
	.y	= {920, 1380, 1480, 1590, 1660, 1730, 1760, 1760},
	.cols	= 8,
};

static struct single_row_lut fcc_sf_id_1 = {
	.x	= {100, 200, 300, 400, 500},
	.y	= {100, 100, 100, 100, 100},
	.cols	= 5,
};

static struct pc_sf_lut pc_sf_id_1 = {
	.rows		= 10,
	.cols		= 5,
	.cycles		= {100, 200, 300, 400, 500},
	.percent	= {100, 90, 80, 70, 60, 50, 40, 30, 20, 10},
	.sf		= {
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100}
	},
};

static struct pc_temp_ocv_lut  pc_temp_ocv_id_1 = {
	.rows		= 29,
	.cols		= 8,
	.temp		= {-20, -10,  0, 5, 10, 20, 30, 40},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
				50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
				8, 7, 6, 5, 4, 3, 2, 1, 0
	},
	.ocv		= {
			{4278, 4278, 4278, 4278, 4276, 4274, 4273, 4273},
			{4245, 4245, 4245, 4245, 4241, 4237, 4236, 4236},
			{4218, 4218, 4218, 4218, 4214, 4209, 4208, 4208},
			{4168, 4168, 4168, 4168, 4162, 4155, 4153, 4153},
			{4120, 4120, 4120, 4120, 4113, 4104, 4101, 4101},
			{4075, 4075, 4075, 4075, 4066, 4056, 4053, 4053},
			{4032, 4032, 4032, 4032, 4022, 4012, 4008, 4008},
			{3991, 3991, 3991, 3991, 3981, 3972, 3967, 3967},
			{3950, 3950, 3950, 3950, 3940, 3931, 3926, 3926},
			{3902, 3902, 3902, 3902, 3889, 3876, 3870, 3870},
			{3864, 3864, 3864, 3864, 3852, 3843, 3840, 3840},
			{3838, 3838, 3838, 3838, 3828, 3820, 3818, 3818},
			{3817, 3817, 3817, 3817, 3809, 3802, 3800, 3800},
			{3801, 3801, 3801, 3801, 3794, 3788, 3786, 3786},
			{3788, 3788, 3788, 3788, 3783, 3778, 3776, 3776},
			{3779, 3779, 3779, 3779, 3776, 3771, 3761, 3761},
			{3773, 3773, 3773, 3773, 3769, 3754, 3739, 3739},
			{3761, 3761, 3761, 3761, 3746, 3721, 3701, 3701},
			{3735, 3735, 3735, 3735, 3708, 3686, 3681, 3681},
			{3728, 3728, 3728, 3728, 3702, 3680, 3670, 3670},
			{3720, 3720, 3720, 3720, 3695, 3674, 3658, 3658},
			{3713, 3713, 3713, 3713, 3689, 3668, 3646, 3646},
			{3705, 3705, 3705, 3705, 3682, 3661, 3634, 3634},
			{3697, 3697, 3697, 3697, 3675, 3655, 3623, 3623},
			{3691, 3691, 3691, 3691, 3665, 3614, 3579, 3579},
			{3685, 3685, 3685, 3685, 3654, 3573, 3534, 3534},
			{3679, 3679, 3679, 3679, 3643, 3532, 3490, 3490},
			{3672, 3672, 3672, 3672, 3633, 3490, 3445, 3445},
			{3666, 3666, 3666, 3666, 3622, 3448, 3400, 3400}

	},
};

struct pm8921_bms_battery_data  bms_battery_data_id_1 = {
        .fcc                    = 1810,
        .fcc_temp_lut           = &fcc_temp_id_1,
        .fcc_sf_lut             = &fcc_sf_id_1,
        .pc_temp_ocv_lut        = &pc_temp_ocv_id_1,
        .pc_sf_lut              = &pc_sf_id_1,

};

static struct single_row_lut fcc_temp_id_2 = {
	.x	= {-20, -10, 0, 5, 10, 20, 30, 40},
	.y	= {920, 1370, 1420, 1510, 1580, 1650, 1690, 1710},
	.cols	= 8,
};

static struct single_row_lut fcc_sf_id_2 = {
	.x	= {100, 200, 300, 400, 500},
	.y	= {100, 100, 100, 100, 100},
	.cols	= 5,
};

static struct pc_sf_lut pc_sf_id_2 = {
	.rows		= 10,
	.cols		= 5,
	.cycles		= {100, 200, 300, 400, 500},
	.percent	= {100, 90, 80, 70, 60, 50, 40, 30, 20, 10},
	.sf		= {
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100},
			{100, 100, 100, 100, 100}
	},
};

static struct pc_temp_ocv_lut  pc_temp_ocv_id_2 = {
	.rows		= 29,
	.cols		= 8,
	.temp		= {-20, -10,  0, 5, 10, 20, 30, 40},
	.percent	= {100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
				50, 45, 40, 35, 30, 25, 20, 15, 10, 9,
				8, 7, 6, 5, 4, 3, 2, 1, 0
	},
	.ocv		= {
			{4290, 4290, 4290, 4290, 4290, 4290, 4290, 4290},
			{4267, 4267, 4267, 4267, 4265, 4262, 4260, 4257},
			{4216, 4216, 4216, 4216, 4213, 4209, 4205, 4203},
			{4169, 4169, 4169, 4169, 4163, 4158, 4153, 4151},
			{4123, 4123, 4123, 4123, 4116, 4109, 4104, 4101},
			{4080, 4080, 4080, 4080, 4072, 4063, 4057, 4054},
			{4038, 4038, 4038, 4038, 4028, 4019, 4014, 4010},
			{3993, 3993, 3993, 3993, 3987, 3980, 3974, 3971},
			{3948, 3948, 3948, 3948, 3938, 3930, 3922, 3917},
			{3905, 3905, 3905, 3905, 3892, 3880, 3875, 3873},
			{3871, 3871, 3871, 3871, 3860, 3851, 3847, 3846},
			{3846, 3846, 3846, 3846, 3836, 3828, 3824, 3823},
			{3825, 3825, 3825, 3825, 3816, 3809, 3805, 3804},
			{3807, 3807, 3807, 3807, 3799, 3793, 3790, 3786},
			{3792, 3792, 3792, 3792, 3786, 3780, 3772, 3762},
			{3780, 3780, 3780, 3780, 3773, 3762, 3747, 3737},
			{3768, 3768, 3768, 3768, 3754, 3731, 3713, 3700},
			{3744, 3744, 3744, 3744, 3721, 3699, 3692, 3686},
			{3713, 3713, 3713, 3713, 3700, 3691, 3681, 3662},
			{3711, 3711, 3711, 3711, 3698, 3680, 3661, 3639},
			{3708, 3708, 3708, 3708, 3695, 3669, 3641, 3616},
			{3705, 3705, 3705, 3705, 3693, 3657, 3621, 3593},
			{3702, 3702, 3702, 3702, 3691, 3646, 3601, 3570},
			{3699, 3699, 3699, 3699, 3688, 3634, 3580, 3547},
			{3696, 3696, 3696, 3696, 3671, 3600, 3541, 3505},
			{3693, 3693, 3693, 3693, 3654, 3566, 3502, 3463},
			{3689, 3689, 3689, 3689, 3636, 3533, 3463, 3422},
			{3686, 3686, 3686, 3686, 3619, 3499, 3424, 3380},
			{3682, 3682, 3682, 3682, 3601, 3465, 3385, 3360}

	},
};

struct pm8921_bms_battery_data  bms_battery_data_id_2 = {
        .fcc                    = 1810,
        .fcc_temp_lut           = &fcc_temp_id_2,
        .fcc_sf_lut             = &fcc_sf_id_2,
        .pc_temp_ocv_lut        = &pc_temp_ocv_id_2,
        .pc_sf_lut              = &pc_sf_id_2,
};

static struct htc_battery_cell htc_battery_cells[] = {
	[0] = {
		.model_name = "BK07100",
		.capacity = 1810,
		.id = 1,
		.id_raw_min = 73, /* unit:mV (10kohm) */
		.id_raw_max = 204,
		.type = HTC_BATTERY_CELL_TYPE_HV,
		.voltage_max = 4340,
		.voltage_min = 3200,
		.chg_param = &chg_batt_params[1],
		.gauge_param = &bms_battery_data_id_1,
	},
	[1] = {
		.model_name = "BK07100",
		.capacity = 1810,
		.id = 2,
		.id_raw_min = 205, /* unit:mV (22kohm) */
		.id_raw_max = 595,
		.type = HTC_BATTERY_CELL_TYPE_HV,
		.voltage_max = 4340,
		.voltage_min = 3200,
		.chg_param = &chg_batt_params[1],
		.gauge_param = &bms_battery_data_id_2,
	},
	[2] = {
		.model_name = "UNKNOWN",
		.capacity = 1810,
		.id = 255,
		.id_raw_min = INT_MIN,
		.id_raw_max = INT_MAX,
		.type = HTC_BATTERY_CELL_TYPE_NORMAL,
		.voltage_max = 4200,
		.voltage_min = 3200,
		.chg_param = &chg_batt_params[0],
		.gauge_param = NULL,
	},
};
#endif /* CONFIG_HTC_BATT_8960 */

static struct pm8xxx_irq_platform_data pm8xxx_irq_pdata __devinitdata = {
	.irq_base		= PM8921_IRQ_BASE,
	.devirq			= MSM_GPIO_TO_INT(104),
	.irq_trigger_flag	= IRQF_TRIGGER_LOW,
};

static struct pm8xxx_gpio_platform_data pm8xxx_gpio_pdata __devinitdata = {
	.gpio_base	= PM8921_GPIO_PM_TO_SYS(1),
};

static struct pm8xxx_mpp_platform_data pm8xxx_mpp_pdata __devinitdata = {
	.mpp_base	= PM8921_MPP_PM_TO_SYS(1),
};

static struct pm8xxx_rtc_platform_data pm8xxx_rtc_pdata __devinitdata = {
	.rtc_write_enable       = true,
	.rtc_alarm_powerup	= false,
};

static struct pm8xxx_pwrkey_platform_data pm8xxx_pwrkey_pdata = {
	.pull_up		= 1,
	.kpd_trigger_delay_us	= 970,
	.wakeup			= 1,
};

static int pm8921_therm_mitigation[] = {
	1100,
	700,
	600,
	225,
};

static struct pm8921_charger_platform_data pm8921_chg_pdata __devinitdata = {
	.safety_time		= 510,
	.update_time		= 60000,
	.max_voltage		= 4200,
	.min_voltage		= 3200,
	.resume_voltage_delta	= 50,
	.term_current		= 50,
	.cool_temp		= 0,
	.warm_temp		= 48,
	.temp_check_period	= 1,
	.max_bat_chg_current	= 1025,
	.cool_bat_chg_current	= 1025,
	.warm_bat_chg_current	= 1025,
	.cool_bat_voltage	= 4200,
	.warm_bat_voltage	= 4000,
	.mbat_in_gpio		= VALENTE_WX_MBAT_IN,
	.thermal_mitigation	= pm8921_therm_mitigation,
	.thermal_levels		= ARRAY_SIZE(pm8921_therm_mitigation),
	.cold_thr = PM_SMBC_BATT_TEMP_COLD_THR__HIGH,
	.hot_thr = PM_SMBC_BATT_TEMP_HOT_THR__LOW,
};

static struct pm8xxx_misc_platform_data pm8xxx_misc_pdata = {
	.priority		= 0,
};

static struct pm8921_bms_platform_data pm8921_bms_pdata __devinitdata = {
	.r_sense		= 10,
	.i_test			= 0,
	.v_failure		= 3000,
	.calib_delay_ms		= 600000,
	.max_voltage_uv		= 4200 * 1000,
};

static int __init check_dq_setup(char *str)
{
	if (!strcmp(str, "PASS")) {
		pr_info("[BATT] overwrite HV battery config\n");
		pm8921_chg_pdata.max_voltage = 4340;
		pm8921_chg_pdata.cool_bat_voltage = 4340;
		pm8921_bms_pdata.max_voltage_uv = 4340 * 1000;
	} else {
		pr_info("[BATT] use default battery config\n");
		pm8921_chg_pdata.max_voltage = 4200;
		pm8921_chg_pdata.cool_bat_voltage = 4200;
		pm8921_bms_pdata.max_voltage_uv = 4200 * 1000;
	}
	return 1;
}
__setup("androidboot.dq=", check_dq_setup);

static struct pm8xxx_gpio_init green_gpios[] = {
	PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(VALENTE_WX_GREEN_LED, 1, PM_GPIO_FUNC_2),
	PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(VALENTE_WX_GREEN_LED, 1, PM_GPIO_FUNC_NORMAL),
};

static struct pm8xxx_gpio_init amber_gpios[] = {
	PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(VALENTE_WX_AMBER_LED, 1, PM_GPIO_FUNC_2),
	PM8XXX_GPIO_OUTPUT_VIN_BB_FUNC(VALENTE_WX_AMBER_LED, 1, PM_GPIO_FUNC_NORMAL),
};

static void green_gpio_config(bool enable)
{
	if (enable)
		pm8xxx_gpio_config(green_gpios[0].gpio, &green_gpios[0].config);
	else
		pm8xxx_gpio_config(green_gpios[1].gpio, &green_gpios[1].config);
}

static void amber_gpio_config(bool enable)
{
	if (enable)
		pm8xxx_gpio_config(amber_gpios[0].gpio, &amber_gpios[0].config);
	else
		pm8xxx_gpio_config(amber_gpios[1].gpio, &amber_gpios[1].config);
}

static struct pm8xxx_led_configure pm8921_led_info[] = {
	[0] = {
		.name		= "button-backlight",
		.flags		= PM8XXX_ID_LED_0,
		.function_flags = LED_PWM_FUNCTION | LED_BRETH_FUNCTION,
		.period_us 	= USEC_PER_SEC / 1000,
		.start_index 	= 0,
		.duites_size 	= 8,
		.duty_time_ms 	= 64,
		.lut_flag 	= PM_PWM_LUT_RAMP_UP | PM_PWM_LUT_PAUSE_HI_EN,
		.out_current    = 40,
		.duties		= {0, 15, 30, 45, 60, 75, 90, 100,
				100, 90, 75, 60, 45, 30, 15, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0},
	},
	[1] = {
		.name           = "green",
		.flags		= PM8XXX_ID_GPIO24,
		.function_flags = LED_BLINK_FUNCTION,
		.gpio_status_switch = green_gpio_config,
	},
	[2] = {
		.name           = "amber",
		.flags		= PM8XXX_ID_GPIO25,
		.function_flags = LED_BLINK_FUNCTION,
		.gpio_status_switch = amber_gpio_config,
	},
};

static struct pm8xxx_led_platform_data pm8xxx_leds_pdata = {
	.num_leds = ARRAY_SIZE(pm8921_led_info),
	.leds = pm8921_led_info,
};

static struct pm8xxx_ccadc_platform_data pm8xxx_ccadc_pdata = {
	.r_sense		= 10,
};

static struct pm8xxx_vibrator_platform_data pm8xxx_vib_pdata = {
	.initial_vibrate_ms = 0,
	.max_timeout_ms = 15000,
	.level_mV = 3100,
};

static struct pm8921_platform_data pm8921_platform_data __devinitdata = {
	.irq_pdata		= &pm8xxx_irq_pdata,
	.gpio_pdata		= &pm8xxx_gpio_pdata,
	.mpp_pdata		= &pm8xxx_mpp_pdata,
	.rtc_pdata              = &pm8xxx_rtc_pdata,
	.pwrkey_pdata		= &pm8xxx_pwrkey_pdata,
	.misc_pdata		= &pm8xxx_misc_pdata,
	.regulator_pdatas	= msm_pm8921_regulator_pdata,
	.charger_pdata		= &pm8921_chg_pdata,
	.bms_pdata		= &pm8921_bms_pdata,
	.adc_pdata		= &pm8xxx_adc_pdata,
	.leds_pdata		= &pm8xxx_leds_pdata,
	.ccadc_pdata		= &pm8xxx_ccadc_pdata,
	.vibrator_pdata		= &pm8xxx_vib_pdata,
};

static struct msm_ssbi_platform_data msm8960_ssbi_pm8921_pdata __devinitdata = {
	.controller_type = MSM_SBI_CTRL_PMIC_ARBITER,
	.slave	= {
		.name			= "pm8921-core",
		.platform_data		= &pm8921_platform_data,
	},
};


static struct msm_cpuidle_state msm_cstates[] __initdata = {
	{0, 0, "C0", "WFI",
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT},

	{0, 1, "C1", "STANDALONE_POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE},

	{0, 2, "C2", "POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE},

	{1, 0, "C0", "WFI",
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT},

	{1, 1, "C1", "STANDALONE_POWER_COLLAPSE",
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE},
};

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR * 2] = {
	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(0, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},

	[MSM_PM_MODE(1, MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
	},
};

static struct msm_rpmrs_level msm_rpmrs_levels[] __initdata = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		100, 8000, 100000, 1,
	},
#if 0
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		2000, 6000, 60100000, 3000,
	},
#endif
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, GDHS, MAX, ACTIVE),
		false,
		4600, 5000, 60350000, 3500,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, MAX, ACTIVE),
		false,
		6700, 4500, 65350000, 4800,
	},
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		7400, 3500, 66600000, 5150,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, GDHS, MAX, ACTIVE),
		false,
		12100, 2500, 67850000, 5500,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MAX, ACTIVE),
		false,
		14200, 2000, 71850000, 6800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		30100, 500, 75850000, 8800,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, RET_HIGH, RET_LOW),
		false,
		30100, 0, 76350000, 9800,
	},
};

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_TZ,
};

static uint32_t msm_rpm_get_swfi_latency(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_levels); i++) {
		if (msm_rpmrs_levels[i].sleep_mode ==
			MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT)
			return msm_rpmrs_levels[i].latency_us;
	}

	return 0;
}

static struct cm3629_platform_data cm36282_pdata = {
	.model = CAPELLA_CM36282,
	.ps_select = CM3629_PS1_ONLY,
	.intr = PM8921_GPIO_PM_TO_SYS(VALENTE_WX_PROXIMITY_INTz),
	.levels = { 0, 2, 0x60, 0x242, 0x869, 0x14c5, 0x235c, 0x210E, 0x38a6, 65535},
	.golden_adc = 0x17D4,
	.power = NULL,
	.cm3629_slave_address = 0xC0>>1,
	.ps1_thd_set = 0xc,
	.ps1_thd_no_cal = 0xF1,
	.ps1_thd_with_cal = 0xc,
	.ps_calibration_rule = 1,
	.ps_conf1_val = CM3629_PS_DR_1_80 | CM3629_PS_IT_1_6T |
			CM3629_PS1_PERS_1,
	.ps_conf2_val = CM3629_PS_ITB_1 | CM3629_PS_ITR_1 |
			CM3629_PS2_INT_DIS | CM3629_PS1_INT_DIS,
	.ps_conf3_val = CM3629_PS2_PROL_32,
};

static struct i2c_board_info i2c_CM36282_devices[] = {
	{
		I2C_BOARD_INFO(CM3629_I2C_NAME, 0xC0 >> 1),
		.platform_data = &cm36282_pdata,
		.irq =  PM8921_GPIO_IRQ(PM8921_IRQ_BASE, VALENTE_WX_PROXIMITY_INTz),
	},
};
#ifdef CONFIG_I2C
#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_FLUID (1 << 4)
#define I2C_LIQUID (1 << 5)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};

#ifdef CONFIG_MSM_CAMERA
static struct i2c_board_info msm_camera_boardinfo[] __initdata = {
#ifdef CONFIG_S5K3H2YX
	{
		I2C_BOARD_INFO("s5k3h2yx", 0x20 >> 1),
	},
#endif
#ifdef CONFIG_S5K6AAFX
 	{
		I2C_BOARD_INFO("s5k6aafx", 0x5a >> 1), /* COB type */
 	},
#endif
#ifdef CONFIG_S5K4E5YX
	{
		I2C_BOARD_INFO("s5k4e5yx", 0x20), /* HTC To distinguish between 3h2 and 4e5 */
	},
#endif
#ifdef CONFIG_MT9V113
	{
		I2C_BOARD_INFO("mt9v113", 0x3C),
	},
#endif
#ifdef CONFIG_S5K6A1GX
	{
		I2C_BOARD_INFO("s5k6a1gx", 0x6C >> 1),
	},
#endif
#ifdef CONFIG_IMX074
	{
		I2C_BOARD_INFO("imx074", 0x1A),
	},
#endif
#ifdef CONFIG_OV2720
	{
		I2C_BOARD_INFO("ov2720", 0x6C),
	},
#endif
	{
		I2C_BOARD_INFO("qs_mt9p017", 0x6C >> 1),
	},
#ifdef CONFIG_MSM_CAMERA_FLASH_SC628A
	{
		I2C_BOARD_INFO("sc628a", 0x6E),
	},
#endif
};
#endif

static struct i2c_registry msm8960_i2c_devices[] __initdata = {
#ifdef CONFIG_MSM_CAMERA
	{
		I2C_SURF | I2C_FFA | I2C_FLUID | I2C_LIQUID | I2C_RUMI,
		MSM_8960_GSBI4_QUP_I2C_BUS_ID,
		msm_camera_boardinfo,
		ARRAY_SIZE(msm_camera_boardinfo),
	},
#endif
#ifdef CONFIG_FLASHLIGHT_TPS61310
	{
		I2C_SURF | I2C_FFA,
		MSM_8960_GSBI12_QUP_I2C_BUS_ID,
		i2c_tps61310_flashlight,
		ARRAY_SIZE(i2c_tps61310_flashlight),
	},
#endif
#ifdef CONFIG_FB_MSM_HDMI_MHL
#ifdef CONFIG_FB_MSM_HDMI_MHL_SII9234
		{
			I2C_SURF | I2C_FFA,
			MSM_8960_GSBI8_QUP_I2C_BUS_ID,
			msm_i2c_gsbi8_mhl_sii9234_info,
			ARRAY_SIZE(msm_i2c_gsbi8_mhl_sii9234_info),
		},
#endif
#endif
	{
		I2C_SURF | I2C_FFA,
		MSM_8960_GSBI3_QUP_I2C_BUS_ID,
		msm_i2c_gsbi3_info,
		ARRAY_SIZE(msm_i2c_gsbi3_info),
	},
	{
		I2C_SURF | I2C_FFA,
		MSM_8960_GSBI12_QUP_I2C_BUS_ID,
		i2c_CM36282_devices,
		ARRAY_SIZE(i2c_CM36282_devices),
	},
        {
                I2C_SURF | I2C_FFA,
                MSM_8960_GSBI12_QUP_I2C_BUS_ID,
                CAP_SENSOR_i2c_boardinfo,
                ARRAY_SIZE(CAP_SENSOR_i2c_boardinfo),
        },
#ifdef CONFIG_VIDEO_NMI
	{
	    I2C_SURF | I2C_FFA | I2C_FLUID,
	    MSM_8960_GSBI1_QUP_I2C_BUS_ID,
	    nmi625_i2c_info,
	    ARRAY_SIZE(nmi625_i2c_info),
	},
#endif /* CONFIG_VIDEO_NMI */
};
#endif /* CONFIG_I2C */

static void __init register_i2c_devices(void)
{
#ifdef CONFIG_I2C
	u8 mach_mask = 0;
	int i;

	mach_mask = I2C_SURF;

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(msm8960_i2c_devices); ++i) {
		if (msm8960_i2c_devices[i].machs & mach_mask) {
			i2c_register_board_info(msm8960_i2c_devices[i].bus,
						msm8960_i2c_devices[i].info,
						msm8960_i2c_devices[i].len);
		}
	}

	i2c_register_board_info(MSM_8960_GSBI12_QUP_I2C_BUS_ID,
			mpu3050_GSBI12_boardinfo, ARRAY_SIZE(mpu3050_GSBI12_boardinfo));
#endif
}

/*UART -> GSBI8*/
static uint32_t msm_uart_gpio[] = {
	GPIO_CFG(34, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(35, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
};

static void msm_uart_gsbi_gpio_init(void)
{
	gpio_tlmm_config(msm_uart_gpio[0], GPIO_CFG_ENABLE);
	gpio_tlmm_config(msm_uart_gpio[1], GPIO_CFG_ENABLE);
}

/*UART -> GSBI12*/
static uint32_t msm_uart_gsbi12_gpio[] = {
	GPIO_CFG(VALENTE_WX_SIR_TX, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(VALENTE_WX_SIR_RX, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
};
static void msm_uart_gsbi12_gpio_init(void)
{
	gpio_tlmm_config(msm_uart_gsbi12_gpio[0], GPIO_CFG_ENABLE);
	gpio_tlmm_config(msm_uart_gsbi12_gpio[1], GPIO_CFG_ENABLE);
	pr_info("msm_uart_gsbi12_gpio_init() ok!\n");
}

static void __init msm8960_irda_init(void)
{
	msm_uart_gsbi12_gpio_init();
	msm8960_device_uart_gsbi12.dev.platform_data =
					&msm8960_irda_gsbi12_pdata;
}

#ifdef CONFIG_VIDEO_NMI
static uint32_t oneseg_on_gpio_table[] = {
    GPIO_CFG(VALENTE_WX__1SEG_I2C_SDA, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
    GPIO_CFG(VALENTE_WX__1SEG_I2C_SCL, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
    GPIO_CFG(VALENTE_WX__1SEG_INTz, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
    GPIO_CFG(VALENTE_WX_V_1SEG_1V2_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static uint32_t oneseg_off_gpio_table[] = {
    GPIO_CFG(VALENTE_WX__1SEG_I2C_SDA, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
    GPIO_CFG(VALENTE_WX__1SEG_I2C_SCL, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
    GPIO_CFG(VALENTE_WX__1SEG_INTz, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
    GPIO_CFG(VALENTE_WX_V_1SEG_1V2_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static uint32_t oneseg_init_gpio_table[] = {
    GPIO_CFG(VALENTE_WX__1SEG_I2C_SDA, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
    GPIO_CFG(VALENTE_WX__1SEG_I2C_SCL, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
    GPIO_CFG(VALENTE_WX__1SEG_INTz, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
    GPIO_CFG(VALENTE_WX_V_1SEG_1V2_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};

static struct pm8xxx_gpio_init oneseg_reset_pmgpios[] = {
    PM8XXX_GPIO_OUTPUT(VALENTE_WX__1SEG_RSTz, 0),
    PM8XXX_GPIO_OUTPUT(VALENTE_WX__1SEG_RSTz, 1),
};

static struct pm8xxx_gpio_init oneseg_enable_pmgpios[] = {
    PM8XXX_GPIO_OUTPUT(VALENTE_WX__1SEG_EN, 0),
    PM8XXX_GPIO_OUTPUT(VALENTE_WX__1SEG_EN, 1),
};


static struct pm8xxx_gpio_init oneseg_fm_ant_switch_pmgpios[] = {
    PM8XXX_GPIO_OUTPUT_VIN_L17(VALENTE_WX__1SEG_FM_ANT, 0),
    PM8XXX_GPIO_OUTPUT_VIN_L17(VALENTE_WX__1SEG_FM_ANT, 1),
};

static struct pm8xxx_gpio_init oneseg_lna_en_pmgpios[] = {
    PM8XXX_GPIO_OUTPUT_VIN_L17(VALENTE_WX__1SEG_LNA_EN, 0),
    PM8XXX_GPIO_OUTPUT_VIN_L17(VALENTE_WX__1SEG_LNA_EN, 1),
};


static struct pm8xxx_gpio_init oneseg_init_pmgpios[] __initdata = {
    PM8XXX_GPIO_OUTPUT(VALENTE_WX__1SEG_RSTz, 0),
    PM8XXX_GPIO_OUTPUT(VALENTE_WX__1SEG_EN, 0),
};

int oneseg_set_lna(int enable)
{
	printk(KERN_INFO "[1SEG] %s: enable: %d \n", __func__, enable);
    if (enable)
    {
        pm8xxx_gpio_config(oneseg_lna_en_pmgpios[1].gpio, &oneseg_lna_en_pmgpios[1].config);
    }
    else
    {
        pm8xxx_gpio_config(oneseg_lna_en_pmgpios[0].gpio, &oneseg_lna_en_pmgpios[0].config);
    }
    return 0;
}

int oneseg_power(int on)
{
    int i, rc;
    
    if (on) 
    {
        printk(KERN_INFO "[1SEG] %s: on \n", __func__);
        gpio_set_value(VALENTE_WX_V_1SEG_1V2_EN, 1); 
        for (i = 0; i < ARRAY_SIZE(oneseg_on_gpio_table); i++) 
        {
            rc = gpio_tlmm_config(oneseg_on_gpio_table[i], GPIO_CFG_ENABLE);
            if (rc) 
            {
                printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n", __func__, oneseg_on_gpio_table[i], rc);
            }
        }
        
        msleep(10);
        pm8xxx_gpio_config(oneseg_enable_pmgpios[1].gpio, &oneseg_enable_pmgpios[1].config);
        msleep(10);
        pm8xxx_gpio_config(oneseg_reset_pmgpios[0].gpio, &oneseg_reset_pmgpios[0].config);
        msleep(1);
        pm8xxx_gpio_config(oneseg_reset_pmgpios[1].gpio, &oneseg_reset_pmgpios[1].config);
        msleep(10);
        
        //Valentewx XB add
        pm8xxx_gpio_config(oneseg_fm_ant_switch_pmgpios[1].gpio, &oneseg_fm_ant_switch_pmgpios[1].config);
        pm8xxx_gpio_config(oneseg_lna_en_pmgpios[1].gpio, &oneseg_lna_en_pmgpios[1].config);
		
    } 
    else 
    {
        /*Power OFF sequence*/
        printk(KERN_INFO "[1SEG] %s: off \n", __func__);        
        //Valentewx XB add
        pm8xxx_gpio_config(oneseg_lna_en_pmgpios[0].gpio, &oneseg_lna_en_pmgpios[0].config);
        pm8xxx_gpio_config(oneseg_fm_ant_switch_pmgpios[0].gpio, &oneseg_fm_ant_switch_pmgpios[0].config);
        
        pm8xxx_gpio_config(oneseg_enable_pmgpios[0].gpio, &oneseg_enable_pmgpios[0].config);
        msleep(5);
        for (i = 0; i < ARRAY_SIZE(oneseg_off_gpio_table); i++) 
        {
            rc = gpio_tlmm_config(oneseg_off_gpio_table[i], GPIO_CFG_DISABLE);
            if (rc) 
            {
                printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n", __func__, oneseg_off_gpio_table[i], rc);
            }
        }
        msleep(5);
        gpio_set_value(VALENTE_WX_V_1SEG_1V2_EN, 0); 

	}
    return 0;
}
EXPORT_SYMBOL(oneseg_power);

static void valente_wx_init_1seg(void)
{
        int i, rc;

        printk(KERN_INFO "valente_wx: %s\n", __func__);


        for (i = 0; i < ARRAY_SIZE(oneseg_init_pmgpios); i++) {
                rc = pm8xxx_gpio_config(oneseg_init_pmgpios[i].gpio,
                                        &oneseg_init_pmgpios[i].config);
                if (rc)
                        printk("%s: 1seg_init_pmgpios: rc=%d\n", __func__, rc);
        }

        /* Initialized 1SEG pins */
        for (i = 0; i < ARRAY_SIZE(oneseg_init_gpio_table); i++) {
                rc = gpio_tlmm_config(oneseg_init_gpio_table[i], GPIO_CFG_DISABLE);
                if (rc) {
                        printk(KERN_ERR
                                   "%s: gpio_tlmm_config(%#x)=%d\n",
                                   __func__, oneseg_init_gpio_table[i], rc);
                }
        }

	platform_device_register(&nm32x_62x_tsi_device);
}
#endif

static void __init valente_wx_init(void)
{
	u32 hw_ver_id = 0;
	if (meminfo_init(SYS_MEMORY, SZ_256M) < 0)
		pr_err("meminfo_init() failed!\n");

	msm_tsens_early_init(&msm_tsens_pdata);
	BUG_ON(msm_rpm_init(&msm_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(msm_rpmrs_levels,
				ARRAY_SIZE(msm_rpmrs_levels)));
	msm_rpmrs_lpm_init(1, 1, 1, 1);

	pmic_reset_irq = PM8921_IRQ_BASE + PM8921_RESOUT_IRQ;
	regulator_suppress_info_printing();
	if (msm_xo_init())
		pr_err("Failed to initialize XO votes\n");
	platform_device_register(&msm8960_device_rpm_regulator);
	msm_clock_init(&msm8960_clock_init_data);
	msm_otg_pdata.swfi_latency = msm_rpmrs_levels[0].latency_us;
	msm8960_device_otg.dev.platform_data = &msm_otg_pdata;
	valente_wx_gpiomux_init();
	msm8960_device_qup_spi_gsbi10.dev.platform_data =
				&msm8960_qup_spi_gsbi10_pdata;
#ifdef CONFIG_RAWCHIP
	spi_register_board_info(rawchip_spi_board_info,
				ARRAY_SIZE(rawchip_spi_board_info));
#endif

	msm8960_device_ssbi_pmic.dev.platform_data =
				&msm8960_ssbi_pm8921_pdata;
	pm8921_platform_data.num_regulators = msm_pm8921_regulator_pdata_len;
	msm8960_i2c_init();
	msm8960_gfx_init();
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	msm8960_init_buses();
#ifdef CONFIG_BT
	bt_export_bd_address();
#endif
#ifdef CONFIG_HTC_BATT_8960
	htc_battery_cell_init(htc_battery_cells, ARRAY_SIZE(htc_battery_cells));
#endif /* CONFIG_HTC_BATT_8960 */
	platform_add_devices(msm_footswitch_devices,
		msm_num_footswitch_devices);
	platform_device_register(&msm8960_device_ext_l2_vreg);
	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
	msm_uart_gsbi_gpio_init();
	msm8960_irda_init();
	pm8921_gpio_mpp_init();
	platform_add_devices(valente_wx_devices, ARRAY_SIZE(valente_wx_devices));
	msm8960_init_cam();
	msm8960_init_mmc();
	acpuclk_init(&acpuclk_8960_soc_data);
	register_i2c_devices();
	msm_fb_add_devices();
	slim_register_board_info(msm_slim_devices,
		ARRAY_SIZE(msm_slim_devices));
	msm_pm_set_platform_data(msm_pm_data, ARRAY_SIZE(msm_pm_data));
	msm_pm_set_rpm_wakeup_irq(RPM_APCC_CPU0_WAKE_UP_IRQ);
	msm_cpuidle_set_states(msm_cstates, ARRAY_SIZE(msm_cstates),
				msm_pm_data);
	change_memory_power = &msm8960_change_memory_power;
	create_proc_read_entry("emmc", 0, NULL, emmc_partition_read_proc, NULL);
	create_proc_read_entry("dying_processes", 0, NULL, dying_processors_read_proc, NULL);

#ifdef CONFIG_PERFLOCK
	perflock_init(&valente_wx_perflock_data);
	cpufreq_ceiling_init(&valente_wx_cpufreq_ceiling_data);
#endif

#ifdef CONFIG_CPU_FREQ_GOV_ONDEMAND_2_PHASE
	if(!cpu_is_krait_v1())
		set_two_phase_freq(1134000);
#endif

	/*usb driver won't be loaded in MFG 58 station and gift mode*/
	if (!(board_mfg_mode() == 6 || board_mfg_mode() == 7))
		valente_wx_add_usb_devices();

	valente_wx_init_keypad();
#ifdef CONFIG_VIDEO_NMI
	valente_wx_init_1seg();
#endif

#ifdef CONFIG_FELICA_DD
	valente_wx_init_felica();
#endif

	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));
	if (get_kernel_flag() & KERNEL_FLAG_PM_MONITOR) {
		htc_monitor_init();
		htc_pm_monitor_init();
	}
	msm_pm_init_sleep_status_data(&msm_pm_slp_sts_data);
	hw_ver_id = readl(HW_VER_ID_VIRT);
	printk(KERN_INFO "hw_ver_id = %x\n", hw_ver_id);
}

#define PHY_BASE_ADDR1  0x80400000
#define SIZE_ADDR1      (132 * 1024 * 1024)

#define PHY_BASE_ADDR2  0x90000000
#define SIZE_ADDR2      (768 * 1024 * 1024)

static void __init valente_wx_fixup(struct machine_desc *desc, struct tag *tags,
				 char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 2;
	mi->bank[0].start = PHY_BASE_ADDR1;
	mi->bank[0].size = SIZE_ADDR1;
	mi->bank[1].start = PHY_BASE_ADDR2;
	mi->bank[1].size = SIZE_ADDR2;
}


MACHINE_START(VALENTE_WX, "valentewx")
	.fixup = valente_wx_fixup,
	.map_io = valente_wx_map_io,
	.reserve = valente_wx_reserve,
	.init_irq = valente_wx_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = valente_wx_init,
	.init_early = msm8960_allocate_memory_regions,
	.init_very_early = valente_wx_early_memory,
MACHINE_END
