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



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include <product_config.h>
#include <mdrv.h>
#include <diag_mem.h>
#include "diag_msgbbp.h"
#include "diag_msgphy.h"
#include "diag_debug.h"
#include "msp_diag_comm.h"
#include "soc_socp_adapter.h"

#if(VOS_OS_VER == VOS_LINUX)
#include "diag_acore_common.h"

#ifdef CFG_BSP_FEATURE_PHONE_TYPE
#include "adrv.h"
#endif
#if(FEATURE_SOCP_MEM_RESERVED == FEATURE_ON)
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_fdt.h>
#endif
#endif

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGBBP_C


#if(VOS_OS_VER == VOS_LINUX)
DIAG_TRANS_HEADER_STRU g_stBbpTransHead = {{VOS_NULL, VOS_NULL}, 0};
DIAG_BBP_DS_ADDR_INFO_STRU g_stBbpDsAddrInfo={DIAG_BBP_DS_ENABLE,DDR_SOCP_SIZE,DDR_SOCP_ADDR};
DIAG_BBP_DS_ADDR_INFO_STRU g_stBbpXdataDsAddrInfo={DIAG_BBP_XDATA_DS_DISABLE ,0,0};

#if(FEATURE_SOCP_MEM_RESERVED == FEATURE_ON)

static int  bbpds_probe(struct platform_device *pdev)
{
    s32 ret;
    //struct device_node *dev;
    struct device *pdevice = &(pdev->dev);

    ret=of_reserved_mem_device_init(pdevice);
    if(ret<0)
    {
         printk("modem_mem_device_init fail!/n" );
    }
     printk("modem_mem_device_init success!/n" );
    return 0;
        
}
static const struct of_device_id bbpds_dev_of_match[] = {
        {.compatible = "hisilion,modem_xmode_region"},
        {},
};
/*lint -save -e785 -e64*/
static struct platform_driver bbpds_driver = {
        .driver = {
                   .name = "modem_xmode_region",
                   .owner = THIS_MODULE,
                   .of_match_table = bbpds_dev_of_match,
        },
        .probe = bbpds_probe,
                   
};
/*lint -restore +e785 +e64*/
#endif

#endif

#if ((VOS_OS_VER == VOS_VXWORKS)||(VOS_OS_VER == VOS_RTOSCK))

#define SOCP_DSSTOP_TIMEOUT    118000   /* socp数采通道打开时间默认为 1min58s */
HTIMER g_SocpDsStopTimer;
VOS_UINT32 g_SocpDsChanTimerFlag = 0;   /* socp ds chan start timer flag */

#endif

DIAG_BBP_PROC_FUN_STRU g_DiagBbpFunc[] = {
    {diag_DrxSampleGetChnSizeProc        ,DIAG_CMD_DRX_SAMPLE_CHNSIZE_REQ       ,0},
#if ((VOS_OS_VER == VOS_VXWORKS)||(VOS_OS_VER == VOS_RTOSCK))
    {diag_DrxSampleGenProc               ,DIAG_CMD_DRX_SAMPLE_GEN_REQ           ,0},
    {diag_DrxSampleGetAddrProc           ,DIAG_CMD_DRX_SAMPLE_ADDR_REQ          ,0},
    {diag_DrxSampleGetVersionProc        ,DIAG_CMD_DRX_SAMPLE_GET_VERSION_REQ   ,0},
    {diag_DrxSampleAbleChnProc           ,DIAG_CMD_DRX_SAMPLE_ABLE_CHN_REQ      ,0},
#endif
};

#if ((VOS_OS_VER == VOS_VXWORKS)||(VOS_OS_VER == VOS_RTOSCK))

#ifdef ENABLE_DIAG_FIX_ADDR
DIAG_DRX_SAMPLE_CHNSIZE_S g_DiagBbpChanSize[] = {
    {DRX_SAMPLE_BBP_DMA_LOG0_CHNSIZE,   SOCP_CODER_SRC_BBP_LOG, BBP_LOG0_MEM_ADDR,  BBP_LOG0_MEM_SIZE},
#ifndef CONFIG_VER_3_TLBBP_DS_BOSTON_AND_LATER
    /* BBP 数采通道调整 version 3.0 (Boston及以后的版本不再使用此通道) */
    {DRX_SAMPLE_BBP_DMA_LOG1_CHNSIZE,   SOCP_CODER_SRC_BBP_BUS, BBP_LOG1_MEM_ADDR,  BBP_LOG1_MEM_SIZE},
#endif
    {DRX_SAMPLE_BBP_DMA_DATA_CHNSIZE,   SOCP_CODER_SRC_BBP_DS,  0,  0},
    {DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE,  0,                      0,  0},
};

#else
DIAG_DRX_SAMPLE_CHNSIZE_S g_DiagBbpChanSize[] = {
    {DRX_SAMPLE_BBP_DMA_LOG0_CHNSIZE,   SOCP_CODER_SRC_BBP_LOG, 0,  BBP_LOG0_MEM_SIZE},

#ifndef CONFIG_VER_3_TLBBP_DS_BOSTON_AND_LATER
    /* BBP 数采通道调整 version 3.0 (Boston及以后的版本不再使用此通道) */
    {DRX_SAMPLE_BBP_DMA_LOG1_CHNSIZE,   SOCP_CODER_SRC_BBP_BUS, 0,  BBP_LOG1_MEM_SIZE},
#endif
    {DRX_SAMPLE_BBP_DMA_DATA_CHNSIZE,   SOCP_CODER_SRC_BBP_DS,  0,  0},
    {DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE,  0,                      0,  0},
};
#endif



VOS_UINT32 diag_DrxSampleGenProc(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_CMD_DRX_SAMPLE_REG_WR_REQ_STRU *psDrxSample = NULL;
    DIAG_CMD_DRX_SAMPLE_REG_WR_CNF_STRU stCnfDrxSample = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    VOS_UINT32 ulDataLen = 0;

    psDrxSample = (DIAG_CMD_DRX_SAMPLE_REG_WR_REQ_STRU*)((VOS_UINT8*)(pData->aucData)+sizeof(MSP_DIAG_DATA_REQ_STRU));

    ulDataLen = pData->ulMsgLen - sizeof(MSP_DIAG_DATA_REQ_STRU);

    stCnfDrxSample.ulRet = mdrv_mailbox_write_commsg(EN_MAILBOX_SERVICE_LTE_HS_DIAG, psDrxSample, ulDataLen, EN_MAILBOX_SLEEP_WAKEUP);
    if(stCnfDrxSample.ulRet != VOS_OK)
    {
        diag_printf("%s BSP_MailBox_ComMsgWrite failed  0x%x!\n",__FUNCTION__,stCnfDrxSample.ulRet);
        stCnfDrxSample.ulRet = ERR_MSP_FAILURE;
    }

    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, sizeof(DIAG_CMD_DRX_SAMPLE_REG_WR_CNF_STRU));

    return ulRet;

}
/*****************************************************************************
 Function Name   : diag_DrxSampleGetAddrProc
 Description     : 低功耗数采获取基地址
 Input           :VOS_UINT8* pstReq
                VOS_UINT32 ulCmdSn
 Output          : None
 Return          : VOS_UINT32

 History         :

*****************************************************************************/
VOS_UINT32 diag_DrxSampleGetAddrProc(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_CMD_DRX_SAMPLE_GET_ADDR_REQ_STRU *psDrxSample = NULL;
    DIAG_CMD_DRX_SAMPLE_GET_ADDR_CNF_STRU stCnfDrxSample = {0};
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    VOS_UINT32 ulAddrType     = 0;

    psDrxSample = (DIAG_CMD_DRX_SAMPLE_GET_ADDR_REQ_STRU*)((VOS_UINT8*)(pData->aucData)+sizeof(MSP_DIAG_DATA_REQ_STRU));


    DIAG_DEBUG_SDM_FUN(EN_DIAG_DEBUG_BBP_SAMPLE,psDrxSample->eDiagDrxSampleAddr,0,0);

    ulAddrType = psDrxSample->eDiagDrxSampleAddr;

    stCnfDrxSample.ulDrxSampleType = ulAddrType;
    /* 获取对应的寄存器地址*/
    switch (ulAddrType)
    {
        case DRX_SAMPLE_BBP_DMA_BASE_ADDR:
            stCnfDrxSample.ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_BBPDMA);
            /* add code here */
            break;
        case DRX_SAMPLE_BBP_DBG_BASE_ADDR:
            stCnfDrxSample.ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_BBPDBG);
            /* add code here */
            break;
        case DRX_SAMPLE_BBP_SRC_BASE_ADDR:
            stCnfDrxSample.ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_BBPSRC);
            /* add code here */
            break;
        case DRX_SAMPLE_POW_ONOFF_CLK_BASE_ADDR:
            stCnfDrxSample.ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SYSCTRL);
            /* add code here */
            break;
        case DRX_SAMPLE_SOCP_BASE_ADDR:
            stCnfDrxSample.ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SOCP);
            /* add code here */
            break;
        default:
            break;
    }

    stCnfDrxSample.ulRet = VOS_OK;

    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, sizeof(DIAG_CMD_DRX_SAMPLE_GET_ADDR_CNF_STRU));

    return ulRet;
}

DIAG_CMD_DRX_SAMPLE_BASE_ADDR_STRU g_DiagDrxSampleAddr[DDR_SAMPLE_BASE_ADDR_MAX];

VOS_VOID Diag_GetDrxSampleAddr(VOS_VOID)
{
   g_DiagDrxSampleAddr[DRX_SAMPLE_BBP_DMA_BASE_ADDR].ulDrxSampleType = DRX_SAMPLE_BBP_DMA_BASE_ADDR;
   g_DiagDrxSampleAddr[DRX_SAMPLE_BBP_DMA_BASE_ADDR].ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_BBPDMA);
   
   g_DiagDrxSampleAddr[DRX_SAMPLE_BBP_DBG_BASE_ADDR].ulDrxSampleType = DRX_SAMPLE_BBP_DBG_BASE_ADDR;
   g_DiagDrxSampleAddr[DRX_SAMPLE_BBP_DBG_BASE_ADDR].ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_BBPDBG);
   
   g_DiagDrxSampleAddr[DRX_SAMPLE_BBP_SRC_BASE_ADDR].ulDrxSampleType = DRX_SAMPLE_BBP_SRC_BASE_ADDR;
   g_DiagDrxSampleAddr[DRX_SAMPLE_BBP_SRC_BASE_ADDR].ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_BBPSRC);
   
   g_DiagDrxSampleAddr[DRX_SAMPLE_POW_ONOFF_CLK_BASE_ADDR].ulDrxSampleType = DRX_SAMPLE_POW_ONOFF_CLK_BASE_ADDR;
   g_DiagDrxSampleAddr[DRX_SAMPLE_POW_ONOFF_CLK_BASE_ADDR].ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SYSCTRL);
   
   g_DiagDrxSampleAddr[DRX_SAMPLE_SOCP_BASE_ADDR].ulDrxSampleType = DRX_SAMPLE_SOCP_BASE_ADDR;
   g_DiagDrxSampleAddr[DRX_SAMPLE_SOCP_BASE_ADDR].ulDrxSampleAddr = (VOS_UINT32)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SOCP);
}

/*****************************************************************************
 Function Name   : diag_DrxSampleGetVersionProc
 Description     : 低功耗数采获取版本信息
 Input           :
 Output          : None
 Return          : VOS_UINT32

 History         :

*****************************************************************************/
VOS_UINT32 diag_DrxSampleGetVersionProc(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_CMD_DRX_SAMPLE_GET_VERSION_CNF_STRU stCnfDrxSample;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    VOS_UINT32 ulRet;

    (VOS_VOID)VOS_MemSet_s(&stCnfDrxSample, sizeof(stCnfDrxSample), 0, sizeof(stCnfDrxSample));

    (VOS_VOID)VOS_MemCpy_s((stCnfDrxSample.ulProductName), DIAG_PRODUCT_VERSION_LENGTH-1, PRODUCT_NAME, VOS_StrNLen(PRODUCT_NAME,(DIAG_PRODUCT_VERSION_LENGTH-1)));

    (VOS_VOID)VOS_MemCpy_s((stCnfDrxSample.ulSolutiongName), DIAG_PRODUCT_VERSION_LENGTH-1, PRODUCT_CFG_CHIP_SOLUTION_NAME, VOS_StrNLen(PRODUCT_CFG_CHIP_SOLUTION_NAME,(DIAG_PRODUCT_VERSION_LENGTH-1)));

    /*打包回复给FW*/
    stCnfDrxSample.ulRet = ERR_MSP_SUCCESS;
    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, sizeof(DIAG_CMD_DRX_SAMPLE_GET_VERSION_CNF_STRU));

    return ulRet;

}

/*****************************************************************************
 Function Name   : diag_DrxSampleAbleChnProc
 Description     : 数采处理
 Input           :VOS_UINT8* pstReq
                VOS_UINT32 ulCmdSn
 Output          : None
 Return          : VOS_UINT32

 History         :

*****************************************************************************/
VOS_UINT32 diag_DrxSampleAbleChnProc(DIAG_FRAME_INFO_STRU *pData)
{
    DIAG_CMD_DRX_SAMPLE_ABLE_CHN_REQ_STRU *psDrxSample = NULL;
    DIAG_CMD_DRX_SAMPLE_ABLE_CHN_CNF_STRU stCnfDrxSample = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;

    VOS_UINT32 ulChnAbleType = 0;
    diag_printf("%s: enable data start !\n",__FUNCTION__);
    psDrxSample = (DIAG_CMD_DRX_SAMPLE_ABLE_CHN_REQ_STRU*)((VOS_UINT8*)(pData->aucData)+sizeof(MSP_DIAG_DATA_REQ_STRU));

    /* 调用socp 通道使能函数打开通道或者关闭通道*/
    ulChnAbleType = psDrxSample->eDiagDrxSampleAbleChn;
    switch (ulChnAbleType)
    {
        case DRX_SAMPLE_SOCP_CHN_ENABLE:
            {   
                diag_printf("%s: enable socp BBP DS channel!\n",__FUNCTION__);
                diag_SocpDsStart();
            break;
            }
        case DRX_SAMPLE_SOCP_CHN_DISABLE:
            {
                diag_printf("%s: disable socp BBP DS channel!\n",__FUNCTION__);
                if(1 == g_SocpDsChanTimerFlag)
                {
                    diag_SocpDsStop(SOCP_STOP_CMD);                   
                }
            break;
            }
/*
#ifdef DIAG_BBP_WRPTR_OFFSET
        case DRX_SAMPLE_SOCP_BUS_CHN_ENABLE:
            {   
                diag_printf("%s: enable socp BBP BUS channel!\n",__FUNCTION__);
                

                mdrv_socp_modify_write_read_ptr(SOCP_CODER_SRC_BBP_LOG);               
               
                if(VOS_OK != mdrv_socp_start(SOCP_CODER_SRC_BBP_LOG))
                {
                    diag_printf("%s: enable BBP BUS channel failed!\n",__FUNCTION__);
                }
            break;
            }
        case DRX_SAMPLE_SOCP_BUS_CHN_DISABLE:
            {
                diag_printf("%s: disable socp BBP BUS channel!\n",__FUNCTION__);
                if(VOS_OK != mdrv_socp_stop(SOCP_CODER_SRC_BBP_LOG))
                {
                    diag_printf("%s: disable BBP BUS channel failed!\n",__FUNCTION__);
                }
            break;
            }
#endif
*/
        default:
            break;
    }

    stCnfDrxSample.ulRet = ERR_MSP_SUCCESS;

    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, sizeof(DIAG_CMD_DRX_SAMPLE_ABLE_CHN_CNF_STRU));

    return ulRet;
}

/*****************************************************************************
 Function Name   : diag_AppTransBbpProc
 Description     : A核发过来的BBP诊断命令处理
 Input           : pMsgBlock    诊断命令请求的内容
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.c64416      2014-11-18  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_AppTransBbpProc(MsgBlock* pMsgBlock)
{
    DIAG_BBP_MSG_A_TRANS_C_STRU *pstInfo;

    pstInfo = (DIAG_BBP_MSG_A_TRANS_C_STRU *)pMsgBlock;

    return diag_BbpMsgProc(&pstInfo->stInfo);
}

VOS_VOID diag_BbpInitSocpChan(VOS_VOID)
{
    int i = 0;
    int num = 0;

    num = (int)(sizeof(g_DiagBbpChanSize)/sizeof(g_DiagBbpChanSize[0]));

    for(i=0;i< num;i++)
    {
        /*数采通道需要单独初始化*/
        if((g_DiagBbpChanSize[i].ulChanEnum!= DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE) &&(g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_DS))
        {
            (VOS_VOID)DRV_SOCP_INIT_LTE_BBP_LOG(g_DiagBbpChanSize[i].ulChan,\
                g_DiagBbpChanSize[i].ulAddr,g_DiagBbpChanSize[i].ulSize);
        }
    }
}

VOS_BOOL ulChanEnableStat = FALSE;
VOS_VOID diag_BbpEnableSocpChan(VOS_VOID)
{
    int i = 0;
    int num = 0;
    if(TRUE == ulChanEnableStat)
    {
        return ;
    }

    num = (int)(sizeof(g_DiagBbpChanSize)/sizeof(g_DiagBbpChanSize[0]));

    for(i = 0;i <num;i++)
    {
/*        
#ifdef DIAG_BBP_WRPTR_OFFSET
        if((g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_DS) && (g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_LOG))
        {
            DRV_SOCP_ENABLE_LTE_BBP_DSP(g_DiagBbpChanSize[i].ulChan);
        }
#else

        if(g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_DS)
        {
            DRV_SOCP_ENABLE_LTE_BBP_DSP(g_DiagBbpChanSize[i].ulChan);
        }    
#endif
*/
        /*数采通道使能需要由dsp模块完成*/
        if(g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_DS)
        {
            DRV_SOCP_ENABLE_LTE_BBP_DSP(g_DiagBbpChanSize[i].ulChan);
        }
    }

    ulChanEnableStat = TRUE;
}

VOS_VOID diag_BbpShowDebugInfo(VOS_VOID)
{
    VOS_UINT32 i = 0;

    for(i=0;i<sizeof(g_DiagBbpFunc)/sizeof(DIAG_BBP_PROC_FUN_STRU);i++)
    {
        diag_printf("Command ID :0x%x ,Ops Num %d\n",g_DiagBbpFunc[i].ulCmdId,g_DiagBbpFunc[i].ulReserve);
    }
}
#endif

#if ((VOS_OS_VER == VOS_VXWORKS)||(VOS_OS_VER == VOS_RTOSCK))
/*****************************************************************************
 Function Name   : diag_SocpDsStart
 Description     : SOCP 数采通道使能接口
 Input           : VOS_VOID
 Output          : None
 Return          : VOS_VOID

 History         :
    1.w00393099     2017-08-31

*****************************************************************************/
VOS_VOID diag_SocpDsStart(VOS_VOID)
{
    VOS_UINT32 ulRet;

#ifdef DIAG_BBP_WRPTR_OFFSET                
    mdrv_socp_update_bbp_ptr(SOCP_CODER_SRC_BBP_DS);               
#endif 

    if(g_SocpDsChanTimerFlag == 1)   // 避免ModemLogCat上没有finish，而且定时器没有超时就进行下一次export
    {
       ulRet = VOS_StopRelTimer(&g_SocpDsStopTimer); 
       if(VOS_OK != ulRet)
       {
            diag_printf("%s: VOS_StopRelTimer failed!\n",__FUNCTION__);        
       }
    }

    ulRet = (VOS_UINT32)mdrv_socp_start(SOCP_CODER_SRC_BBP_DS);
    if(VOS_OK != ulRet)
    {
        diag_printf("%s: enable BBP DS channel failed!\n",__FUNCTION__);
    }
    else
    {                    
        /* 定时2min, 超时后自动关闭socp数采通道, 规避socp芯片数采通道备份恢复的bug */
        ulRet = VOS_StartRelTimer(&g_SocpDsStopTimer, MSP_PID_DIAG_AGENT, SOCP_DSSTOP_TIMEOUT, SOCP_DSSTOP_TIMER_NAME,
                    SOCP_DSSTOP_TIMER_PARA, VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_0);
        if(VOS_OK != ulRet)
        {
            diag_printf("%s: VOS_StartRelTimer failed!\n",__FUNCTION__);
        }
        else
        {
            g_SocpDsChanTimerFlag = 1;
        }

    }    
}

/*****************************************************************************
 Function Name   : diag_SocpDsStop
 Description     : SOCP 数采通道停止接口
 Input           : ulType:
                        SOCP_STOP_CMD:收到工具下发finish命令停止socp数采通道
                        SOCP_STOP_TIMER:SOCP数采通道定时器超时停止
 Output          : None
 Return          : VOS_VOID

 History         :
    1.w00393099     2017-08-31

*****************************************************************************/
VOS_VOID diag_SocpDsStop(VOS_UINT32 ulType)
{    
    VOS_UINT32 ulRet;

    g_SocpDsChanTimerFlag = 0;

    if(SOCP_STOP_CMD == ulType)
    {
        ulRet = VOS_StopRelTimer(&g_SocpDsStopTimer);
        if(VOS_OK != ulRet)
        {
            diag_printf("%s: VOS_StopRelTimer failed!\n",__FUNCTION__);
        }
    }    

    (VOS_VOID)mdrv_socp_stop(SOCP_CODER_SRC_BBP_DS);    
}

// VOS_UINT32 g_GetChanSizeCmdCnt = (VOS_UINT32)(0);   // 系统上电后获得工具侧下发通道信息命令的次数

/*****************************************************************************
 Function Name   : Diag_Send_ChanSizeAndAddr_To_Phy
 Description     : MSP将采数通道信息和基地址打包发给PHY,包括TL-PHY、GU-PHY和UPHY
 Input           : VOS_VOID
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_UINT32 Diag_BbpSendDsinfo2phy(VOS_VOID)
{
    VOS_UINT8* pData;
    DIAG_CMD_DRX_SAMPLE_BASE_ADDR_STRU  *pBaseAddr;
    DIAG_PHY_BBPDS_CHN_INFO_STRU    *pChanInfo;
    VOS_UINT32 i = 0;
    VOS_UINT32 ulChanNum = sizeof(g_DiagBbpChanSize)/sizeof(g_DiagBbpChanSize[0]);
    VOS_UINT32 ulInfoSize;
    // VOS_UINT32 ulMsgTotalSize;
    VOS_UINT32 ret_tl;
    VOS_UINT32 ret_gu;

    ulInfoSize      = sizeof(DIAG_PHY_DSINFO_STRU) + ulChanNum*sizeof(DIAG_PHY_BBPDS_CHN_INFO_STRU);
    // ulMsgTotalSize  = ulInfoSize + sizeof(DIAG_TLPHY_DSINFO_STRU);
    
    pData = VOS_MemAlloc(MSP_PID_DIAG_AGENT, DYNAMIC_MEM_PT, ulInfoSize);
    if(NULL == pData)
    {
        diag_printf("%s(%d) pData VOS_MemAlloc failed.\n",__FUNCTION__, __LINE__);
        return ERR_MSP_MALLOC_FAILUE;
    }

    /*set baseaddr info*/
    pBaseAddr = &((DIAG_PHY_DSINFO_STRU*)pData)->AddrInfo[0];
    VOS_MemCpy_s(pBaseAddr,sizeof(g_DiagDrxSampleAddr), g_DiagDrxSampleAddr,sizeof(g_DiagDrxSampleAddr));

    /*set chan ddr info*/
    pChanInfo = &((DIAG_PHY_DSINFO_STRU*)pData)->ChanInfo[0];
    ((DIAG_PHY_DSINFO_STRU*)pData)->ulChanNum = ulChanNum;
    for(i=0;i<ulChanNum;i++)
    {
        (pChanInfo+i)->ulChanEnum   = g_DiagBbpChanSize[i].ulChanEnum;
        (pChanInfo+i)->ulAddr       = g_DiagBbpChanSize[i].ulAddr;
        (pChanInfo+i)->ulSize       = g_DiagBbpChanSize[i].ulSize;            
    }
    
    ret_tl = diag_DspSendMsg(EN_MAILBOX_SERVICE_LTE_OM, pData, MSG_ID_DIAG2TLPHY_CHAN_ADDR_INFO, ulInfoSize);
    if(ERR_MSP_SUCCESS != ret_tl)
    {
        diag_printf("%s(%d) diag send msg to TL-PHY failed.\n",__FUNCTION__, __LINE__);
        diag_printf("The err number of TL-PHY send is %d.\n", ret_tl);
    }
    else
    {
        diag_printf("diag send msg to TL-PHY success.\n");  
    }
        
    ret_gu = diag_SendMsg(MSP_PID_DIAG_AGENT, DSP_PID_STARTUP, MSG_ID_DIAG2GUPHY_CHAN_ADDR_INFO,
        pData,ulInfoSize);
    if(ERR_MSP_SUCCESS != ret_gu)
    {
        diag_printf("%s(%d) diag send msg to GU-PHY failed.\n", __FUNCTION__, __LINE__);
        diag_printf("The err number of GU-PHY send is %d.\n", ret_gu);
    }
    else
    {
        diag_printf("diag send msg to GU-PHY success.\n"); 
    }

    VOS_MemFree(MSP_PID_DIAG_AGENT, pData);

    if((ERR_MSP_SUCCESS != ret_tl) && (ERR_MSP_SUCCESS != ret_gu))
    {
        return ERR_MSP_DIAG_SAMPLE_START_FAIL;
    }
    else if(ERR_MSP_SUCCESS != ret_tl)
    {                       
        return ERR_MSP_DIAG_TL_SEND_MSG_FAIL;
    }
    else if(ERR_MSP_SUCCESS != ret_gu)
    {
        return ERR_MSP_DIAG_GUC_SEND_MSG_FAIL;
    }
    else
    {
        return ERR_MSP_SUCCESS;     
    }   
}
#endif

/*****************************************************************************
 Function Name     : diag_DrxSampleGetChnSizeProc
 Description     : 低功耗数采获取通道大小
 Input             :VOS_UINT8* pstReq
                VOS_UINT32 ulCmdSn
 Output          : None
 Return          : VOS_UINT32

 History         :

*****************************************************************************/
VOS_UINT32 diag_DrxSampleGetChnSizeProc(DIAG_FRAME_INFO_STRU *pData)
{
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_BBP_DS_ADDR_INFO_STRU  *psDrxDsInfo = NULL;
    VOS_UINT32 ulRet ;
    DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_CNF_STRU stCnfDrxSample = {0};
    
    
#if (VOS_OS_VER == VOS_LINUX)
    VOS_UINT32 ulLen;
    DIAG_BBP_MSG_A_TRANS_C_STRU *pstInfo;
    DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU *psCdmaSample;
    VOS_UINT32 ulAddrType = 0;
    /*lint -save -e826*/
    psCdmaSample = (DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU*)((VOS_UINT8*)(pData->aucData)+sizeof(MSP_DIAG_DATA_REQ_STRU));
    
    /*AP在发送给CP命令时，需要把数采地址空间信息一起发送过去*/
    ulLen = (VOS_UINT32)sizeof(DIAG_BBP_MSG_A_TRANS_C_STRU)-VOS_MSG_HEAD_LENGTH + pData->ulMsgLen+(VOS_UINT32)sizeof(DIAG_BBP_DS_ADDR_INFO_STRU);
    
    pstInfo = (DIAG_BBP_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);
    if(VOS_NULL == pstInfo)
    {
       // ulRet = ERR_MSP_MALLOC_FAILUE;
        goto DIAG_ERROR;
    }
    pstInfo->ulReceiverPid  = MSP_PID_DIAG_AGENT;
    pstInfo->ulSenderPid    = MSP_PID_DIAG_APP_AGENT;
    pstInfo->ulMsgId        = DIAG_MSG_BBP_A_TRANS_C_REQ;
    
    ulLen = sizeof(DIAG_FRAME_INFO_STRU)+pData->ulMsgLen;
    (VOS_VOID)VOS_MemCpy_s(&pstInfo->stInfo, ulLen, pData, ulLen);
    
    psDrxDsInfo = (DIAG_BBP_DS_ADDR_INFO_STRU*)((VOS_UINT8*)pstInfo+sizeof(DIAG_BBP_MSG_A_TRANS_C_STRU)+pData->ulMsgLen);
    ulAddrType  = psCdmaSample->eDiagDrxSampleChnSize;
    if(DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE ==ulAddrType)
    {
        psDrxDsInfo->ulAddr     = g_stBbpXdataDsAddrInfo.ulAddr;
        psDrxDsInfo->ulSize     = g_stBbpXdataDsAddrInfo.ulSize;
        psDrxDsInfo->ulenable   = g_stBbpXdataDsAddrInfo.ulenable;
   #ifdef CFG_BSP_FEATURE_PHONE_TYPE
        atfd_hisi_service_access_register_smc( ACCESS_REGISTER_FN_MAIN_ID, (VOS_UINT64)g_stBbpXdataDsAddrInfo.ulAddr, 
            (VOS_UINT64)g_stBbpXdataDsAddrInfo.ulSize, ACCESS_REGISTER_FN_SUB_ID_DDR_MODEM_SEC);
   #endif
    }
    else
    {
        psDrxDsInfo->ulAddr     = g_stBbpDsAddrInfo.ulAddr;
        psDrxDsInfo->ulSize     = g_stBbpDsAddrInfo.ulSize;
        psDrxDsInfo->ulenable   = g_stBbpDsAddrInfo.ulenable;
    }

    ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo);
    if(ulRet)
    {
        goto DIAG_ERROR;
    }

    return ulRet;
DIAG_ERROR:
#endif

#if ((VOS_OS_VER == VOS_VXWORKS)||(VOS_OS_VER == VOS_RTOSCK))
    DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU *psDrxSample = NULL;
    VOS_UINT32 ulAddrType = 0;
    int num = 0;
    int i = 0;
    ulRet = ERR_MSP_SUCCESS;
    psDrxSample = (DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_REQ_STRU*)((VOS_UINT8*)(pData->aucData)+sizeof(MSP_DIAG_DATA_REQ_STRU));
    psDrxDsInfo = (DIAG_BBP_DS_ADDR_INFO_STRU*)((VOS_UINT8*)pData+sizeof(DIAG_FRAME_INFO_STRU)+pData->ulMsgLen);

    DIAG_DEBUG_SDM_FUN(EN_DIAG_DEBUG_BBP_SAMPLE,psDrxSample->eDiagDrxSampleChnSize,0,0);

    ulAddrType  = psDrxSample->eDiagDrxSampleChnSize;
    num = (int)(sizeof(g_DiagBbpChanSize)/sizeof(g_DiagBbpChanSize[0]));

    /* 如果工具侧下发不支持的通道，ulChnAddr和ulChnSize 返回0 */
    (VOS_VOID)VOS_MemSet_s(&stCnfDrxSample,sizeof(stCnfDrxSample),0,sizeof(stCnfDrxSample));

    for(i = 0; i < num; i++)
    {
            
        if(ulAddrType == g_DiagBbpChanSize[i].ulChanEnum)
        {
            if(DRX_SAMPLE_BBP_CDMA_DATA_CHNSIZE == g_DiagBbpChanSize[i].ulChanEnum)
            {
                g_DiagBbpChanSize[i].ulAddr = (VOS_UINT32)psDrxDsInfo->ulAddr;
                g_DiagBbpChanSize[i].ulSize = psDrxDsInfo->ulSize;
                
                stCnfDrxSample.ulChnAddr    = (VOS_UINT32) psDrxDsInfo->ulAddr;
                stCnfDrxSample.ulChnSize    =  psDrxDsInfo->ulSize;
                
                diag_printf("BBP chan base addr :0x%x, base size: 0x%x; SOCP base addr :0x%x, base size: 0x%x.\n", 
                    g_DiagBbpChanSize[i].ulAddr, g_DiagBbpChanSize[i].ulSize, DDR_SOCP_ADDR, DDR_SOCP_SIZE);
                
                /*数据发送到tl跟gu phy*/
                ulRet = Diag_BbpSendDsinfo2phy();
                if(ERR_MSP_SUCCESS == ulRet)
                {
                    diag_printf("%s(%d) Diag Bbp send Dsinfo to phy success.\n",__FUNCTION__, __LINE__);
                }
            }
            else if(g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_DS)
            {
                stCnfDrxSample.ulChnAddr = g_DiagBbpChanSize[i].ulAddr;
                stCnfDrxSample.ulChnSize = g_DiagBbpChanSize[i].ulSize;
            }
            else
            {
                if((DIAG_BBP_DS_ENABLE == psDrxDsInfo->ulenable)&&(!g_DiagBbpChanSize[i].ulAddr))
                {
                    g_DiagBbpChanSize[i].ulAddr = (VOS_UINT32)psDrxDsInfo->ulAddr;
                    g_DiagBbpChanSize[i].ulSize = psDrxDsInfo->ulSize;
                    (VOS_VOID)DRV_SOCP_INIT_LTE_BBP_DS(g_DiagBbpChanSize[i].ulChan,\
                        g_DiagBbpChanSize[i].ulAddr,g_DiagBbpChanSize[i].ulSize);
                }
                
                stCnfDrxSample.ulChnAddr = g_DiagBbpChanSize[i].ulAddr;
                stCnfDrxSample.ulChnSize = g_DiagBbpChanSize[i].ulSize;

                
                
                diag_printf("BBP chan base addr :0x%x, base size: 0x%x; SOCP base addr :0x%x, base size: 0x%x.\n", 
                    g_DiagBbpChanSize[i].ulAddr, g_DiagBbpChanSize[i].ulSize, DDR_SOCP_ADDR, DDR_SOCP_SIZE);
            }
        }
    }
    stCnfDrxSample.ulChnType = ulAddrType;
    if(ulRet == ERR_MSP_SUCCESS)   // 新版本中如果命令执行成功，stCnfDrxSample.ulRet 返回1
    {
        stCnfDrxSample.ulRet = 1;    
    }
    else                           // 新版本中如果命令执行失败，stCnfDrxSample.ulRet 返回错误码
    {
        stCnfDrxSample.ulRet = ulRet;
    }
#endif

/*                        
 #if (VOS_OS_VER == VOS_LINUX) // 新版本中如果命令执行失败，stCnfDrxSample.ulRet 返回错误码  
    stCnfDrxSample.ulRet = ERR_MSP_DIAG_SAMPLE_START_FAIL;
 #endif   
*/  
    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, (VOS_UINT32)sizeof(DIAG_CMD_DRX_SAMPLE_GET_CHNSIZE_CNF_STRU));
     /*lint -restore +e826*/


         
     return ulRet;
}


/*****************************************************************************
 Function Name   : diag_BbpMsgProc
 Description     : MSP bbp部分消息处理函数
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_UINT32 diag_BbpMsgProc(DIAG_FRAME_INFO_STRU *pData)
{
    VOS_UINT32 ulRet = ERR_MSP_INVALID_PARAMETER ;
    DIAG_CMD_DRX_SAMPLE_COMM_CNF_STRU stCnfDrxSample;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    VOS_UINT32 i ;
#if (VOS_OS_VER == VOS_LINUX)
    VOS_UINT32 ulLen;
    DIAG_BBP_MSG_A_TRANS_C_STRU *pstInfo;
#endif
    if(DIAG_MSG_TYPE_BBP != pData->stID.pri4b)
    {
        diag_printf("%s Rcv Error Msg Id 0x%x\n",__FUNCTION__,pData->ulCmdId);
        return ulRet;
    }

    for(i = 0; i< sizeof(g_DiagBbpFunc)/sizeof(g_DiagBbpFunc[0]);i++)
    {
        if(g_DiagBbpFunc[i].ulCmdId == pData->ulCmdId)
        {
            g_DiagBbpFunc[i].ulReserve ++;
            ulRet = g_DiagBbpFunc[i].pFunc(pData);
            return ulRet;
        }
    }

#if (VOS_OS_VER == VOS_LINUX)

    /* GU的BBP命令采用透传处理机制 */
    if((DIAG_MODE_GSM == pData->stID.mode4b) || (DIAG_MODE_UMTS == pData->stID.mode4b))
    {
        return diag_TransReqProcEntry(pData, &g_stBbpTransHead);
    }

    if((DIAG_MODE_1X == pData->stID.mode4b) || (DIAG_MODE_HRPD == pData->stID.mode4b))
    {
        ulRet = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    /*AP在发送给CP命令时，需要把数采地址空间信息一起发送过去*/
    ulLen = (VOS_UINT32)sizeof(DIAG_BBP_MSG_A_TRANS_C_STRU)-VOS_MSG_HEAD_LENGTH + pData->ulMsgLen;
    pstInfo = (DIAG_BBP_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);
    if(VOS_NULL == pstInfo)
    {
        ulRet = ERR_MSP_MALLOC_FAILUE;
        goto DIAG_ERROR;
    }
    pstInfo->ulReceiverPid = MSP_PID_DIAG_AGENT;
    pstInfo->ulSenderPid   = MSP_PID_DIAG_APP_AGENT;
    pstInfo->ulMsgId       = DIAG_MSG_BBP_A_TRANS_C_REQ;
    ulLen = sizeof(DIAG_FRAME_INFO_STRU)+pData->ulMsgLen;
    (VOS_VOID)VOS_MemCpy_s(&pstInfo->stInfo, ulLen, pData, ulLen);

    ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo);
    if(ulRet)
    {
        goto DIAG_ERROR;
    }

    return ulRet;

DIAG_ERROR:
#endif

    stCnfDrxSample.ulRet = ulRet;
    DIAG_MSG_COMMON_PROC(stDiagInfo,stCnfDrxSample,pData);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_BBP;

    ulRet = DIAG_MsgReport(&stDiagInfo,&stCnfDrxSample, (VOS_UINT32)sizeof(DIAG_CMD_DRX_SAMPLE_ABLE_CHN_CNF_STRU));

    return ulRet;
}
/*lint -save -e423 */
/*****************************************************************************
 Function Name   : diag_BbpMsgInit
 Description     : MSP bbp部分初始化
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_BbpMsgInit(VOS_VOID)
{
#if ((VOS_OS_VER == VOS_VXWORKS)||(VOS_OS_VER == VOS_RTOSCK))
    int i = 0;
    int num=0;
    VOS_UINT_PTR ulRealPtr = 0;
    VOS_VOID* ulVirtAddr = VOS_NULL;

    num = (int)sizeof(g_DiagBbpChanSize)/sizeof(g_DiagBbpChanSize[0]);

    for(i=0;i<num;i++)
    {
        if((!g_DiagBbpChanSize[i].ulAddr) && (g_DiagBbpChanSize[i].ulSize) && (g_DiagBbpChanSize[i].ulChan != SOCP_CODER_SRC_BBP_DS))
        {
            ulVirtAddr= VOS_UnCacheMemAlloc(g_DiagBbpChanSize[i].ulSize,&ulRealPtr);
            if(VOS_NULL != ulVirtAddr)
            {   
                g_DiagBbpChanSize[i].ulAddr = ulRealPtr;
                ulVirtAddr = VOS_NULL;  
                ulRealPtr  = 0;
               
            }
        }
    }
/*lint -restore  */    
    diag_BbpInitSocpChan();

    Diag_GetDrxSampleAddr();

    // g_GetChanSizeCmdCnt = 0;  // 每次上电后工具侧下发BBP数采命令次数清0
#else

    VOS_UINT32 ulRet;

    /* 创建节点保护信号量, Diag Trans Bbp */
    ulRet = VOS_SmBCreate("DTB", 1, VOS_SEMA4_FIFO,&g_stBbpTransHead.TransSem);
    if(VOS_OK != ulRet)
    {
        diag_printf("diag_BbpMsgInit VOS_SmBCreate failed.\n");
    }

    /* 初始化请求链表 */
    blist_head_init(&g_stBbpTransHead.TransHead);

    /*注册message消息回调*/
    DIAG_MsgProcReg(DIAG_MSG_TYPE_BBP,diag_BbpMsgProc); 
    printk(KERN_ERR"diag modem:modem_reserver define !");

#if(FEATURE_SOCP_MEM_RESERVED == FEATURE_ON)
    printk(KERN_ERR"diag modem:modem_reserver define!\n");

    ulRet= platform_driver_register(&bbpds_driver);
    if(VOS_OK != ulRet)
    {
        diag_printf("diag_BbpMsgInit bbpds_driver failed.\n");
    }
#endif
#endif

    return; 
}
#if(VOS_OS_VER == VOS_LINUX)
extern unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
/*****************************************************************************
* 函 数 名  : socp_logbuffer_sizeparse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的BUFFER大小参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
static int __init diag_BbpDrxDdrEnable(char *pucChar)
{
    u32      flag;

    flag = (u32)simple_strtoul(pucChar, NULL, 0);
    if(flag)
    {
        #ifdef ENABLE_DIAG_FIX_ADDR
        g_stBbpDsAddrInfo.ulenable  = DIAG_BBP_DS_ENABLE;
        g_stBbpDsAddrInfo.ulAddr    = BBP_DS_MEM_ADDR;
        g_stBbpDsAddrInfo.ulSize    = BBP_DS_MEM_SIZE;
        #else
        g_stBbpDsAddrInfo.ulenable  = DIAG_BBP_DS_ENABLE;
        g_stBbpDsAddrInfo.ulAddr    = DDR_SOCP_ADDR;
        g_stBbpDsAddrInfo.ulSize    = DDR_SOCP_SIZE;
        #endif
        printk(KERN_ERR"[%s] enable!\n",__FUNCTION__);
        printk(KERN_ERR"base addr :0x%x,base size: 0x%x\n",DDR_SOCP_ADDR,DDR_SOCP_SIZE);
    }
    else
    {
        g_stBbpDsAddrInfo.ulenable  = 0;
        g_stBbpDsAddrInfo.ulAddr    = 0;
        g_stBbpDsAddrInfo.ulSize    = 0;
        printk(KERN_ERR"[%s] not enable!\n",__FUNCTION__);
    }
    return 0;
}

early_param("modem_socp_enable",diag_BbpDrxDdrEnable);

#if(FEATURE_SOCP_MEM_RESERVED == FEATURE_ON)

/*****************************************************************************
* 函 数 名  : modem_cdma_bbpds_reserve_area
*
* 功能描述  : 在代码编译阶段将动态预留内存申请出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
static int modem_cdma_bbpds_reserve_area(struct reserved_mem *rmem)
{
    const char* heapname;
    char *status ;
    int namesize = 0;
    
    printk(KERN_ERR"diag modem: modem_cdma_bbpds_reserve_area!\n" );
    
    status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
    if (status && (strncmp(status, "ok", strlen("ok")) != 0))
    {
         printk(KERN_ERR"status is not ok [%s]!\n",status);
         return 0;
    }
        

    heapname = of_get_flat_dt_prop(rmem->fdt_node, "region-name", &namesize);
    if (!heapname || (namesize <= 0)) {
        printk(KERN_ERR"%s %s %d, no 'region-name' property namesize=%d\n",
        __FILE__, __FUNCTION__,__LINE__, namesize);

        return 0;
    }
    
    g_stBbpXdataDsAddrInfo.ulenable         = DIAG_BBP_XDATA_DS_ENABLE; 
    g_stBbpXdataDsAddrInfo.ulSize           = (VOS_UINT32)rmem->size;
    g_stBbpXdataDsAddrInfo.ulAddr           = rmem->base;
/*
#ifdef (DIAG_BBP_WRPTR_OFFSET)
    g_DiagBbpChanSize[DRX_SAMPLE_BBP_DMA_LOG0_CHNSIZE].ulAddr = (VOS_UINT32)rmem->base;
    g_DiagBbpChanSize[DRX_SAMPLE_BBP_DMA_LOG0_CHNSIZE].size = rmem->size;            
#endif
*/
    
    printk(KERN_ERR"[%s]:diag modem: kernel reserved buffer is useful, base 0x%llx, size is 0x%llx\n",
                 __FUNCTION__, rmem->base, rmem->size );
    return 0;
}

/*lint -e611 -esym(611,*)*/
RESERVEDMEM_OF_DECLARE(modem_cdma_bbpds_region, "modem_xmode_region", modem_cdma_bbpds_reserve_area);
/*lint -e611 +esym(611,*)*/
#endif
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


