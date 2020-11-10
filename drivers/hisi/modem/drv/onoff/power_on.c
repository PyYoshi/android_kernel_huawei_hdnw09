/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/rtc.h>

#include <product_config.h>
#include <mdrv_sysboot.h>
#include <mdrv_chg.h>
#include <bsp_icc.h>
#include <bsp_onoff.h>
#include "power_exchange.h"
#include "mdrv_chg.h"



int his_boot_is_modem_crash(void)
{
    c_power_st_e status = power_on_c_status_get();

    if (status < POWER_MSP_OK )
        return 1;
    else
        return 0;
}

#ifdef CONFIG_BALONG_ONOFF
#define POWER_ON_MONITOR_TIMEROUT       (60*1000)
#define EXCH_POWER_LOG_PATH     "/modem_log/poweron.txt"
#define POWER_SYS_PRIV_RO  (S_IRUSR | S_IRGRP)
#define POWER_SYS_PRIV_WR  (S_IWUSR | S_IRUGO)
extern BATT_LEVEL_E chg_get_batt_level(void);

typedef struct
{
    DRV_START_MODE_E mode;
    DRV_START_REASON reason;
    struct rtc_time         time;
    struct softtimer_list   on_timer;
}power_on_ctrl_s;


static power_on_ctrl_s power_item_info = {0}; //lint !e64

static ssize_t his_boot_show_property(struct device *dev,
                            struct device_attribute *attr, char *buf);

#define POWER_ON_ATTR(_name, _mode)					\
{									            \
	.attr = { .name = #_name, .mode = _mode },					\
	.show = his_boot_show_property,          \
}

/* ���ó�ֻ�� */
static struct device_attribute power_on_attrs[] = {
	POWER_ON_ATTR(power_status, POWER_SYS_PRIV_RO),
	POWER_ON_ATTR(power_reason, POWER_SYS_PRIV_RO),
	POWER_ON_ATTR(power_time, POWER_SYS_PRIV_RO),
};

static struct attribute *
his_boot_attributes[ARRAY_SIZE(power_on_attrs) + 1]; //lint !e30 !e84

static const struct attribute_group his_boot_group = {
	.attrs = his_boot_attributes,
};

/*****************************************************************************
 �� �� ��  : power_on_init_attrs
 ��������  : init attributes
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static void power_on_init_attrs(void)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(power_on_attrs); i++) //lint !e30 !e84
		his_boot_attributes[i] = &power_on_attrs[i].attr;
}

/*****************************************************************************
 �� �� ��  : power_on_time_get
 ��������  : ��¼����ʱ��.
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static void power_on_time_get( void )
{
    struct rtc_time tm;
    struct timespec ts;

    getnstimeofday(&ts);

    rtc_time_to_tm((unsigned long)ts.tv_sec, &tm);

    power_item_info.time = tm;

    pr_dbg("system power on time:%4d-%02d-%02d %02d:%02d:%02d\n",tm.tm_year, tm.tm_mon, \
            tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/*****************************************************************************
 �� �� ��  : power_on_reason_get
 ��������  : ��ȡ����ԭ��
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static void power_on_reason_get( void )
{
    DRV_START_REASON reason = DRV_START_REASON_BUTT;
    power_on_start_reason sram_data = POWER_ON_START_REASON_BUTT;

    sram_data = power_on_start_reason_get();
    switch(sram_data)
    {
    case POWER_ON_START_REASON_CHARGE:
        reason = DRV_START_REASON_CHARGE;
        break;
    case POWER_ON_START_REASON_POWER_KEY:
        reason = DRV_START_REASON_POWER_KEY;
        break;
    case POWER_ON_START_REASON_WARM_RESET:
        reason = DRV_START_REASON_WARM_RESET;
        break;
    default:
        reason = DRV_START_REASON_BUTT;
        break;
    }

    power_item_info.reason = reason;
}

/*****************************************************************************
 �� �� ��  : power_on_mode_get
 ��������  : ��ȡ����ģʽ
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static void power_on_mode_get( void )
{
    DRV_START_MODE_E mode = DRV_START_MODE_NORMAL;
    DRV_RUN_MODE boot_mode = get_run_mode();
    DRV_CHARGING_MODE pwd_flag = get_pd_charge_flag();

    if((RUN_MODE_NORMAL == boot_mode)){
        mode = DRV_START_MODE_NORMAL;
    }
    else if((RUN_MODE_RECOVERY == boot_mode) && ((POWER_DOWN_CHARGING_MODE == pwd_flag)))
    {
        mode = DRV_START_MODE_CHARGING;
    }
    else if((RUN_MODE_RECOVERY == boot_mode) && ((NORMAL_CHARGING_MODE == pwd_flag)))
    {
        mode = DRV_START_MODE_UPDATE;
    }

    power_item_info.mode = mode;
}

#ifdef CONFIG_BOOT_PROTECT
/*****************************************************************************
 �� �� ��  : power_off_timeout_isr
 ��������  : ������ʱ���������C��״̬
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static int power_on_timeout_isr( void *arg)
{
    c_power_st_e status = POWER_OS_OK;
    DRV_RUN_MODE run_mode = get_run_mode();

    pr_dbg("power_on_timeout_isr, system had been powered one minute ago\n ");

    if(RUN_MODE_NORMAL == run_mode)
    {
        /* ��ȡC������״̬ */
        status = power_on_c_status_get();

        if (his_boot_is_modem_crash())
        {
           pr_dbg("c core power on fail!\n ");
           /* ����recoveryģʽ */
           bsp_drv_power_reboot();

           /* coverity[no_escape] */
           for(;;) ;
        }
    }
    else if(RUN_MODE_RECOVERY == run_mode){
        /* for test */
       #if 0
           bsp_drv_power_reboot();
           /* �ȴ�ϵͳ���� */
           while(1);
       #endif
    }

    pr_dbg("system power on ok!\n ");

    /* clear warm boot cnt */
    power_on_wdt_cnt_set();

    (void)bsp_softtimer_free(&power_item_info.on_timer);

    return 0;
}

/*****************************************************************************
 �� �� ��  : power_on_timer_init
 ��������  : ��������timer,���ڿ�����һ��ʱ����ϵͳ�Ƿ���������
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
void power_on_timer_init( void )
{
	s32 ret = 0;

	power_item_info.on_timer.func = (softtimer_func)power_on_timeout_isr;

	power_item_info.on_timer.para = (void*)0;
	power_item_info.on_timer.timeout = POWER_ON_MONITOR_TIMEROUT;
	power_item_info.on_timer.wake_type = SOFTTIMER_WAKE;
	power_item_info.on_timer.unit_type = TYPE_MS;
	ret = bsp_softtimer_create(&power_item_info.on_timer);
    if (ret)
    {
        pr_dbg("create softtimer failed \n");
    }

    bsp_softtimer_add(&power_item_info.on_timer);
}
#endif

/*****************************************************************************
 �� �� ��  : his_boot_show_property
 ��������  : show power on status/reason/time item property.
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static ssize_t his_boot_show_property(struct device *dev,
                            struct device_attribute *attr, char *buf)
{
    struct rtc_time tm = power_item_info.time;

	static char *power_status[] = {
		"Power Down Charging", "Normal", "Update", "Unkown"
	};

	static char *power_reason[] = {
		"Charger", "Power Key", "Warm Reset", "Unknown",
	};

	const ptrdiff_t off = attr - power_on_attrs;

	if (off == POWER_ON_PROP_STATUS)
		return snprintf(buf, SZ_1K, "%s\n", power_status[power_item_info.mode]);
	else if (off == POWER_ON_PROP_REASON)
		return snprintf(buf, SZ_1K, "%s\n", power_reason[power_item_info.reason]);
	else if (off == POWER_ON_PROP_TIME){
		return snprintf(buf, SZ_1K, "%4d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year, tm.tm_mon, \
        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    /* coverity[secure_coding] */
	return snprintf(buf, SZ_1K, "%s\n", "error item");
}
#endif

/*****************************************************************************
 �� �� ��  : bsp_start_mode_get
 ��������  : ���ڻ�ȡ����ģʽ
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
int bsp_start_mode_get(void)
{
#ifdef CONFIG_BALONG_ONOFF
    power_on_mode_get();
    return power_item_info.mode;
#else
    return DRV_START_MODE_NORMAL;
#endif
}


/*****************************************************************************
 �� �� ��  : bsp_power_icc_send_state
 ��������  : C�˺˼�ͨ�ź���
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static void bsp_power_icc_send_state(void)
{
    int ret;
    int mode;
    u32 icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

    mode = bsp_start_mode_get();

    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8*)&mode, (u32)sizeof(mode));
    if (ret != (int)sizeof(mode))
    {
        pr_dbg("send len(%x) != expected len(%lu)\n", ret, (unsigned long)sizeof(mode));
    }
}

/*****************************************************************************
 �� �� ��  : bsp_power_ctrl_read_cb
 ��������  : C�˺˼�ص�����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static s32 bsp_power_ctrl_read_cb(void)
{
    int rt = 0;
    int read_len;
    stCtrlMsg msg;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

	read_len = bsp_icc_read(channel_id, (u8*)&msg, (u32)sizeof(stCtrlMsg));
	if(read_len != (int)sizeof(stCtrlMsg))
	{
		pr_dbg("read len(%x) != expected len(%lu)\n", read_len, (unsigned long)sizeof(stCtrlMsg));
		return -1;
	}

	pr_dbg("[onoff]bsp_power_ctrl_read_cb is called, msg: 0x%x\n", msg.pwr_type);

    switch(msg.pwr_type)
    {
    case E_POWER_ON_MODE_GET:
        bsp_power_icc_send_state();
        break;
    case E_POWER_SHUT_DOWN:
        drv_shut_down(msg.reason);
        break;
    case E_POWER_POWER_OFF:
        bsp_drv_power_off();
        break;
    case E_POWER_POWER_REBOOT:
        bsp_drv_power_reboot();
        break;
    default:
        pr_dbg("invalid ctrl by ccore\n");
        break;
    }

    return rt;
}

/*****************************************************************************
 �� �� ��  : his_boot_probe
 ��������  : power on
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :
*****************************************************************************/
static int __init his_boot_probe(struct platform_device *pdev)
{
    int rt;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

#ifdef CONFIG_BALONG_ONOFF
    /* get early param */

    /* ���reboot flag */
    power_on_reboot_flag_set(POWER_OFF_REASON_INVALID);

#if 0
    /* normalģʽ�� clear misc partition */
    if(NORMAL_CHARGING_MODE == get_pd_charge_flag()){
        rt = clear_misc_message();
        if(rt < 0){
            pr_dbg("clear misc message failed.\n");
            return rt;
        }
    }
#endif

    /* get power on reason */
    power_on_reason_get();

    /* get power time */
    power_on_time_get();

    /* sysfs create */
    power_on_init_attrs();

    rt = (int)sysfs_create_group(&pdev->dev.kobj, &his_boot_group);
    if (rt < 0)
    {
        pr_dbg("create sys filesystem node failed.\n");
        return rt;
    }
#endif
#ifdef CONFIG_BOOT_PROTECT
    /* ��������timer */
    power_on_timer_init();
#endif
    rt = bsp_icc_event_register(channel_id, (read_cb_func)bsp_power_ctrl_read_cb, NULL, NULL, NULL);
    if(rt != 0){
        pr_dbg("icc event register failed.\n");
    }

    return rt;
}

static struct platform_device his_boot_dev = {
    .name = "his_boot",
    .id = 0,
    .dev = {
    .init_name = "his_boot",
    },/*lint !e785*/
};/*lint !e785*/

static struct platform_driver his_boot_drv = {
    .probe      = his_boot_probe,
	.driver		= {
		.name	= "his_boot",
		.owner	= THIS_MODULE,/*lint !e64*/
	},/*lint !e785*/
};/*lint !e785*/

static int __init his_boot_init(void)
{
    int ret;

    pr_dbg(KERN_DEBUG "his_boot_init.\n");

    ret = platform_device_register(&his_boot_dev);
    if(ret)
    {
        pr_dbg("register his_boot device failed.\n");
        return ret;
    }

    ret = platform_driver_register(&his_boot_drv);/*lint !e64*/
    if(ret)
    {
        pr_dbg("register his_boot driver failed.\n");
        platform_device_unregister(&his_boot_dev);
    }

    return ret;
}

late_initcall(his_boot_init);

