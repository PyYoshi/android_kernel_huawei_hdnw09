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
#ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif

/******************************************************************************
   1 头文件包含
******************************************************************************/
#include "v_typdef.h"
#include "ImmInterface.h"
#include "RnicProcMsg.h"
#include "RnicLog.h"
#include "RnicEntity.h"
#include "RnicDebug.h"
#include "RnicCtx.h"
#include "RnicConfigInterface.h"
#include "product_config.h"
#if (VOS_WIN32 == VOS_OS_VER)
#include <stdio.h>
#endif
#include "mdrv.h"
#include "PppRnicInterface.h"
#include "RnicSndMsg.h"
#include "BastetRnicInterface.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_RNIC_ENTITY_C

/******************************************************************************
   2 外部函数变量声明
******************************************************************************/

/******************************************************************************
   3 私有定义
******************************************************************************/

/******************************************************************************
   4 全局变量定义
*****************************************************************************/

/******************************************************************************
   5 函数实现
******************************************************************************/

/*****************************************************************************
 函 数 名  : RNIC_ProcUlDataInPdpActive
 功能描述  : 在PDP激活的状态，RNIC发送数据时的处理过程
 输入参数  : pstSkb     :SKBUF数据首地址
             pstNetCntxt:网卡上下文
             ucRabid    :链路承载号
             enIpType   :IPv4或IPv6
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年12月13日
    作    者   : 范晶
    修改内容   : 新生成函数
  2.日    期   : 2012年11月23日
    作    者   : f00179208
    修改内容   : DSDA Phase I: RNIC多实例
  3.日    期   : 2013年6月3日
    作    者   : L47619
    修改内容   : V3R3 Share-PDP项目修改
  4.日    期   : 2015年5月28日
    作    者   : l00198894
    修改内容   : TSTS
*****************************************************************************/
VOS_VOID RNIC_SendULDataInPdpActive(
    IMM_ZC_STRU                        *pstImmZc,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    VOS_UINT8                           ucRabId,
    ADS_PKT_TYPE_ENUM_UINT8             enIpType
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv   = VOS_NULL_PTR;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId;
    VOS_UINT32                          ulDataLen = 0;
    VOS_UINT8                           ucSendAdsRabId;



    pstPriv   = pstNetCntxt->pstPriv;
    enRmNetId = pstNetCntxt->enRmNetId;

    /* 根据Modem Id组装RabId */
    if (MODEM_ID_0 == pstNetCntxt->enModemId)
    {
        ucSendAdsRabId = ucRabId;
    }
#if (FEATURE_ON == FEATURE_MULTI_MODEM)
    else if (MODEM_ID_1 == pstNetCntxt->enModemId)
    {
        ucSendAdsRabId = ucRabId | RNIC_RABID_TAKE_MODEM_1_MASK;
    }
#if (MULTI_MODEM_NUMBER == 3)
    else if (MODEM_ID_2 == pstNetCntxt->enModemId)
    {
        ucSendAdsRabId = ucRabId | RNIC_RABID_TAKE_MODEM_2_MASK;
    }
#endif
#endif
    else
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_MODEM_ID_UL_DISCARD_NUM(1, enRmNetId);
        pstPriv->stStats.tx_dropped++;
        return;
    }

    ulDataLen = pstImmZc->len;

    if (VOS_OK != ADS_UL_SendPacketEx(pstImmZc, enIpType, ucSendAdsRabId))
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_SEND_UL_PKT_FAIL_NUM(1, enRmNetId);
        pstPriv->stStats.tx_dropped++;
        return;
    }

   RNIC_DBG_SEND_UL_PKT_NUM(1, enRmNetId);

    /* 统计网卡发送信息 */
    pstPriv->stStats.tx_packets++;
    pstPriv->stStats.tx_bytes += ulDataLen;

    /* 统计上行数据 */
    pstNetCntxt->stDsFlowStats.ulPeriodSendPktNum++;
    pstNetCntxt->stDsFlowStats.ulTotalSendFluxLow += ulDataLen;

    return;
}

#if (FEATURE_ON == FEATURE_IMS)
/*****************************************************************************
 函 数 名  : RNIC_ProcVoWifiULData
 功能描述  :  处理vowifi的上行数据
              请注意! 这个是整个VOWIFI数据流的上行，但是对于网卡来说，
              IP协议栈的出口数据是上行，刚好反过来
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年11月9日
    作    者   : n00269697
    修改内容   : 新生成函数

  2.日    期   : 2016年12月27日
    作    者   : A00165503
    修改内容   : DTS2016121600573: 新增VOWIFI专用网卡
*****************************************************************************/
VOS_VOID RNIC_ProcVoWifiULData(
    struct sk_buff                     *pstSkb,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt
)
{

    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* 当IMS域为WIFI时，RMNET_IMS网卡出口的数据通过RNIC和CDS之间的核间消息传递 */
    if (VOS_OK == RNIC_SendCdsImsDataReq(pstSkb, pstNetCntxt))
    {
        /* Modified by m00217266 for 双VoWiFi项目, 2017-2-27, begin */
#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_DBG_SEND_UL_PKT_NUM(1, pstNetCntxt->enRmNetId);
#else
        RNIC_DBG_SEND_UL_PKT_NUM(1, RNIC_RMNET_ID_IMS1);
#endif
        /* Modified by m00217266 for 双VoWiFi项目, 2017-2-27, end */

        /* 统计网卡发送信息 */
        pstPriv   = pstNetCntxt->pstPriv;

        pstPriv->stStats.tx_packets++;
        pstPriv->stStats.tx_bytes += pstSkb->len;

        /* 统计上行数据 */
        pstNetCntxt->stDsFlowStats.ulPeriodSendPktNum++;
        pstNetCntxt->stDsFlowStats.ulTotalSendFluxLow += pstSkb->len;
    }

    IMM_ZcFreeAny(pstSkb);

    return;
}
#endif

/*****************************************************************************
 函 数 名  : RNIC_ProcUlIpv4Data
 功能描述  : RNIC处理IPV4的上行数据
 输入参数  : pstSkb     :SKBUF数据包首地址
             pstNetCntxt:网卡上下文
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2011年12月15日
   作    者   : 范晶
   修改内容   : 新生成函数
 2.日    期   : 2012年8月30日
   作    者   : l60609
   修改内容   : AP适配项目：在RNIC_TransSkbToImmZC中已统计转换失败的次数
 3.日    期   : 2012年11月23日
   作    者   : f00179208
   修改内容   : DSDA Phase I: RNIC多实例

  4.日    期   : 2016年12月27日
    作    者   : A00165503
    修改内容   : DTS2016121600573: 新增VOWIFI专用网卡
*****************************************************************************/
VOS_VOID RNIC_SendULIpv4Data(
    struct sk_buff                     *pstSkb,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt
)
{
    IMM_ZC_STRU                        *pstImmZc = VOS_NULL_PTR;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId;
    VOS_UINT8                           ucRabId;
#if (FEATURE_ON == FEATURE_IMS)
    VOS_UINT32                          ulNonEmpty = VOS_FALSE;
#endif

    pstImmZc  = (IMM_ZC_STRU *)pstSkb;
    enRmNetId = pstNetCntxt->enRmNetId;

#if (FEATURE_ON == FEATURE_IMS)
    /* 当IMS域为WIFI时，RMNET_IMS网卡出口的数据通过RNIC和CDS之间的核间消息传递 */
#ifdef CONFIG_VOWIFI_NEW_FRW
    if (RNIC_RMNET_R_IS_VALID(pstNetCntxt->enRmNetId))
#else
    if (RNIC_RMNET_ID_IMS1 == pstNetCntxt->enRmNetId)
#endif
    {
        if (0 == IMM_ZcQueueLen(&(pstNetCntxt->stPdpCtx.stImsQue)))
        {
            ulNonEmpty = VOS_TRUE;
        }

        IMM_ZcQueueTail(&(pstNetCntxt->stPdpCtx.stImsQue), pstSkb);

        if (VOS_TRUE == ulNonEmpty)
        {
            RNIC_TrigImsDataProcEvent(pstNetCntxt->enRmNetId);
        }

        return;
    }
#endif

    /* 获取网卡映射的RABID */
    ucRabId = RNIC_GET_SPEC_NET_IPV4_RABID(enRmNetId);
    if (RNIC_RAB_ID_INVALID == ucRabId)
    {
        IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
        RNIC_DBG_RAB_ID_ERR_NUM(1, enRmNetId);
        return;
    }

    /* PDP激活的情况下数据的处理 */
    RNIC_SendULDataInPdpActive(pstImmZc, pstNetCntxt, ucRabId, ADS_PKT_TYPE_IPV4);

    return;
}

/*****************************************************************************
 函 数 名  : RNIC_ProcUlIpv6Data
 功能描述  : RNIC处理IPV6的上行数据
 输入参数  : pstSkb     : SKBUF数据包首地址
             pstNetCntxt: 网卡上下文指针
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2011年12月15日
   作    者   : 范晶
   修改内容   : 新生成函数
 2.日    期   : 2012年8月30日
   作    者   : l60609
   修改内容   : AP适配项目：在RNIC_TransSkbToImmZC中已统计转换失败的次数
 3.日    期   : 2012年11月23日
   作    者   : f00179208
   修改内容   : DSDA Phase I: RNIC多实例

  4.日    期   : 2016年12月27日
    作    者   : A00165503
    修改内容   : DTS2016121600573: 新增VOWIFI专用网卡
*****************************************************************************/
VOS_VOID RNIC_SendULIpv6Data(
    struct sk_buff                     *pstSkb,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt
)
{
    IMM_ZC_STRU                        *pstImmZc = VOS_NULL_PTR;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId;
    VOS_UINT8                           ucRabId;
#if (FEATURE_ON == FEATURE_IMS)
    VOS_UINT32                          ulNonEmpty = VOS_FALSE;
#endif

    pstImmZc  = (IMM_ZC_STRU *)pstSkb;
    enRmNetId = pstNetCntxt->enRmNetId;

#if (FEATURE_ON == FEATURE_IMS)
    /* 当IMS域为WIFI时，RMNET_IMS网卡出口的数据通过RNIC和CDS之间的核间消息传递 */
#ifdef CONFIG_VOWIFI_NEW_FRW
    if (RNIC_RMNET_R_IS_VALID(pstNetCntxt->enRmNetId))
#else
    if (RNIC_RMNET_ID_IMS1 == pstNetCntxt->enRmNetId)
#endif
    {
        if (0 == IMM_ZcQueueLen(&(pstNetCntxt->stPdpCtx.stImsQue)))
        {
            ulNonEmpty = VOS_TRUE;
        }

        IMM_ZcQueueTail(&(pstNetCntxt->stPdpCtx.stImsQue), pstSkb);

        if (VOS_TRUE == ulNonEmpty)
        {
            RNIC_TrigImsDataProcEvent(enRmNetId);
        }

        return;
    }
#endif

    /* 获取网卡映射的RABID */
    ucRabId = RNIC_GET_SPEC_NET_IPV6_RABID(enRmNetId);
    if (RNIC_RAB_ID_INVALID == ucRabId)
    {
        IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
        RNIC_DBG_RAB_ID_ERR_NUM(1, enRmNetId);
        return;
    }

    /* PDP激活的情况下数据的处理 */
    RNIC_SendULDataInPdpActive(pstImmZc, pstNetCntxt, ucRabId, ADS_PKT_TYPE_IPV6);

    return;
}

/*****************************************************************************
 函 数 名  : RNIC_ProcDemDial
 功能描述  : RNIC处理按需拨号
 输入参数  : struct sk_buff  *pstSkb,
             pBuf -- 存储数据的缓存的ID

 输出参数  : 无
 返 回 值  : OK
             ERROR
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2010年1月19日
    作    者   : 范晶
    修改内容   : 新生成函数

  2.日    期   : 2012年6月6日
    作    者   : A00165503
    修改内容   : DTS2012060502819: 灌包方式触发按需拨号, 导致频繁上报事件

*****************************************************************************/
VOS_UINT32 RNIC_ProcDemDial(
    struct sk_buff                     *pstSkb
)
{
    RNIC_DIAL_MODE_STRU                *pstDialMode;
    RNIC_TIMER_STATUS_ENUM_UINT8        enTiStatus;
    VOS_UINT32                          ulIpAddr;

    /* 获取IP地址 */
    ulIpAddr = *((VOS_UINT32 *)((pstSkb->data) + RNIC_IP_HEAD_DEST_ADDR_OFFSET));

    /*如果是广播包，则不发起按需拨号，直接过滤调*/
    if (RNIC_IPV4_BROADCAST_ADDR == ulIpAddr)
    {
        RNIC_DBG_UL_RECV_IPV4_BROADCAST_NUM(1, RNIC_RMNET_ID_0);
        return VOS_ERR;
    }

    /* 获取按需拨号的模式以及时长的地址 */
    pstDialMode = RNIC_GetDialModeAddr();

    /* 获取当前拨号保护定时器的状态 */
    enTiStatus  = RNIC_GetTimerStatus(TI_RNIC_DEMAND_DIAL_PROTECT);

    /*为了防止按需拨号上报太快，启动一个两秒定时器，*/
    if (RNIC_TIMER_STATUS_STOP == enTiStatus)
    {
        /* 通知应用进行拨号操作 */
        if (RNIC_ALLOW_EVENT_REPORT == pstDialMode->enEventReportFlag)
        {
            if (VOS_OK == RNIC_SendDialEvent(DEVICE_ID_WAN, RNIC_DAIL_EVENT_UP))
            {
                /* 启动拨号保护定时器  */
                RNIC_StartTimer(TI_RNIC_DEMAND_DIAL_PROTECT, TI_RNIC_DEMAND_DIAL_PROTECT_LEN);
                RNIC_DBG_SEND_APP_DIALUP_SUCC_NUM(1, RNIC_RMNET_ID_0);
                RNIC_NORMAL_LOG(ACPU_PID_RNIC, "RNIC_ProcDemDial: Send dial event succ.");
            }
            else
            {
                RNIC_DBG_SEND_APP_DIALUP_FAIL_NUM(1, RNIC_RMNET_ID_0);
                RNIC_WARNING_LOG(ACPU_PID_RNIC, "RNIC_ProcDemDial: Send dial event fail.");
            }

            RNIC_MNTN_TraceDialConnEvt();
        }
    }

    return VOS_OK;
}

/* Modified by l60609 for L-C互操作项目, 2014-1-14, begin */
#if (FEATURE_ON == FEATURE_CL_INTERWORK)
/*****************************************************************************
 函 数 名  : RNIC_RcvOutsideModemUlData
 功能描述  : CDMA模式接收到TCP/IP协议栈的上行数据
 输入参数  : struct sk_buff                     *pstSkb
             RNIC_SPEC_CTX_STRU                 *pstNetCntxt
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年1月7日
    作    者   : m00217266
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID RNIC_RcvOutsideModemUlData(
    struct sk_buff                     *pstSkb,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;
    VOS_UINT16                          usEthType;
    VOS_UINT8                           ucPdnId;
    VOS_ULONG                           ulRslt;
    VOS_UINT32                          ulDataLen;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId;

    pstPriv     = pstNetCntxt->pstPriv;
    enRmNetId   = pstNetCntxt->enRmNetId;
    ulDataLen   = pstSkb->len;
    ulRslt      = SDIO_ERR;

    /* 获取以太帧类型 */
    usEthType   = VOS_NTOHS(((RNIC_ETH_HEADER_STRU *)(pstSkb->data))->usEtherType);

    /* IP报文类型判断 */
    switch(usEthType)
    {
        case RNIC_ETH_TYPE_IP:
            ucPdnId = RNIC_GET_SPEC_NET_IPV4_PDNID(enRmNetId);
            break;

        case RNIC_ETH_TYPE_IPV6:
            ucPdnId = RNIC_GET_SPEC_NET_IPV6_PDNID(enRmNetId);
            break;

        default:
            IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
            RNIC_DBG_RECV_UL_ERR_PKT_NUM(1, enRmNetId);
            pstPriv->stStats.tx_dropped++;
            return;
    }

    /* 非法Pdn Id */
    if (RNIC_PDN_ID_INVALID == ucPdnId)
    {
        IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
        RNIC_DBG_PDN_ID_ERR_NUM(1, enRmNetId);
        return;
    }

    /* 移除MAC头 */
    if (VOS_OK != IMM_ZcRemoveMacHead(pstSkb))
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_UL_RMV_MAC_HDR_FAIL_NUM(1, enRmNetId);
        return;
    }

    /* 写SDIO */
    ulRslt = SDIO_UL_SendPacket(pstSkb, ucPdnId);
    if (SDIO_OK != ulRslt)
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_SEND_UL_PKT_FAIL_NUM(1, enRmNetId);
        pstPriv->stStats.tx_dropped++;
    }
    else
    {
        RNIC_DBG_SEND_UL_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.tx_packets++;
        pstPriv->stStats.tx_bytes += ulDataLen;
    }

    return;
}
#endif
/*****************************************************************************
 函 数 名  : RNIC_RcvInsideModemUlData
 功能描述  : 接收到TCP/IP协议栈3GPP的上行数据
 输入参数  : struct sk_buff                     *pstSkb
             RNIC_RMNET_ID_ENUM_UINT8            enRmNetId
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年1月7日
    作    者   : m00217266
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID RNIC_RcvInsideModemUlData(
    struct sk_buff                     *pstSkb,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;
    VOS_UINT16                          usEthType;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId;

    pstPriv     = pstNetCntxt->pstPriv;
    enRmNetId   = pstNetCntxt->enRmNetId;

    /* 获取以太帧类型 */
    usEthType   = VOS_NTOHS(((RNIC_ETH_HEADER_STRU *)(pstSkb->data))->usEtherType);

    /* 流控检查 */
    if (RNIC_FLOW_CTRL_STATUS_START == RNIC_GET_FLOW_CTRL_STATUS(enRmNetId))
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_FLOW_CTRL_UL_DISCARD_NUM(1, enRmNetId);
        return;
    }

    /* 移除MAC头 */
    if (VOS_OK != IMM_ZcRemoveMacHead(pstSkb))
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_UL_RMV_MAC_HDR_FAIL_NUM(1, enRmNetId);
        pstPriv->stStats.tx_dropped++;
        return;
    }

    /* 只在网卡0上面才会触发按需拨号 */
    if ((RNIC_ETH_TYPE_IP == usEthType)
     && (RNIC_DIAL_MODE_DEMAND_DISCONNECT == RNIC_GET_DIAL_MODE())
     && (RNIC_PDP_REG_STATUS_DEACTIVE == RNIC_GET_SPEC_NET_IPV4_REG_STATE(enRmNetId))
     && (RNIC_RMNET_ID_0 == enRmNetId))
    {
        RNIC_SPE_MEM_UNMAP(pstSkb, RNIC_SPE_CACHE_HDR_SIZE);

        if (VOS_ERR == RNIC_ProcDemDial(pstSkb))
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_StartXmit, the data is discared!");
        }

        RNIC_SPE_MEM_MAP(pstSkb, RNIC_SPE_CACHE_HDR_SIZE);

        IMM_ZcFreeAny(pstSkb);
        return;
    }

    /* IP报文类型判断 */
    switch(usEthType)
    {
        case RNIC_ETH_TYPE_IP:
            RNIC_SendULIpv4Data(pstSkb, pstNetCntxt);
            RNIC_DBG_RECV_UL_IPV4_PKT_NUM(1, enRmNetId);
            break;

        case RNIC_ETH_TYPE_IPV6:
            RNIC_SendULIpv6Data(pstSkb, pstNetCntxt);
            RNIC_DBG_RECV_UL_IPV6_PKT_NUM(1, enRmNetId);
            break;

        default:
            IMM_ZcFreeAny(pstSkb);
            RNIC_DBG_RECV_UL_ERR_PKT_NUM(1, enRmNetId);
            break;
    }

    return;
}
/* Modified by l60609 for L-C互操作项目, 2014-1-14, end */

/* Modified by l60609 for L-C互操作项目, 2014-01-06, Begin */
#if (FEATURE_ON == FEATURE_CL_INTERWORK)
/*****************************************************************************
 函 数 名  : RNIC_ShowSdioDlData
 功能描述  : 打印sdio模块过来的数据
 输入参数  : pstSkb   :SKBUF数据首地址
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2014年03月11日
   作    者   : m00217266
   修改内容   : 新生成函数
*****************************************************************************/
VOS_VOID RNIC_ShowSdioDlData(
    VOS_UINT8                           ucPdnId,
    IMM_ZC_STRU                        *pstImmZc
)
{
    VOS_UINT32                          i;

    /* RNIC 上行数据打印开关，打印去mac头的数据 */
    if (VOS_TRUE == g_ulRnicPrintDlDataFlg)
    {
        PS_PRINTF("RNIC_ShowSdioDlData : dl data pdn id is %d. \r\n", ucPdnId);
        PS_PRINTF("RNIC_ShowSdioDlData : dl data len is %d. \r\n", pstImmZc->len);
        PS_PRINTF("RNIC_ShowSdioDlData : dl data content is: \r\n");

        for (i = 0; i < pstImmZc->len; i++)
        {
            if (pstImmZc->data[i] > 0xf)
            {
                PS_PRINTF("%x", pstImmZc->data[i]);
            }
            else
            {
                PS_PRINTF("0%x", pstImmZc->data[i]);
            }
        }
        PS_PRINTF("\r\n");
    }

    return;
}

/*****************************************************************************
 函 数 名  : RNIC_RcvSdioDlData
 功能描述  : 接收SDIO的下行数据
 输入参数  : VOS_UINT8                           ucPdnId
             IMM_ZC_STRU                        *pstData
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年1月6日
    作    者   : m00217266
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 RNIC_RcvSdioDlData(
    VOS_UINT8                           ucPdnId,
    IMM_ZC_STRU                        *pstImmZc
)
{
    VOS_UINT8                           ucNetIndex;
    ADS_PKT_TYPE_ENUM_UINT8             enPktType;
    VOS_UINT8                           ucIpType;
    VOS_UINT32                          ulRet;

    /* 可谓可测，打印数据包内容和pdn id */
    RNIC_ShowSdioDlData(ucPdnId, pstImmZc);

    /* 根据PDNid获取对应的RMNETid */
    ucNetIndex     = RNIC_GET_RM_NET_ID_BY_PDN_ID(ucPdnId);

    /* 获取协议版本号 */
    ucIpType    = RNIC_GET_IP_VERSION(pstImmZc->data[0]);

    if (RNIC_IPV4_VERSION == ucIpType)
    {
        enPktType = ADS_PKT_TYPE_IPV4;
    }
    else if (RNIC_IPV6_VERSION == ucIpType)
    {
        enPktType = ADS_PKT_TYPE_IPV6;
    }
    else    /* 数据包类型与承载支持类型不一致 */
    {
        RNIC_DBG_RECV_DL_ERR_PKT_NUM(1, ucNetIndex);
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_RcvSdioDlData, Ip Type is !", ucIpType);

        /* 释放内存 */
        IMM_ZcFreeAny(pstImmZc);

        return RNIC_PKT_TYPE_INVAL;
    }

    ulRet = RNIC_SendDlData(ucNetIndex, pstImmZc, enPktType);
    return ulRet;
}
#endif

/*****************************************************************************
 函 数 名  : RNIC_RcvAdsDlData
 功能描述  : RNIC收到ADS下行数据
 输入参数  : ucRabid  :链路承载号
             pstData  :SKBUF数据包首地址
             enPdpType:PDP 类型
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年2月10日
    作    者   : m00217266
    修改内容   : 新生成函数

  2.日    期   : 2015年5月28日
    作    者   : l00198894
    修改内容   : TSTS
*****************************************************************************/
VOS_UINT32  RNIC_RcvAdsDlData(
    VOS_UINT8                           ucExRabid,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType,
    VOS_UINT32                          ulExParam
)
{
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucRmNetId;

    ucRmNetId = RNIC_GET_RMNETID_FROM_EXPARAM(ulExParam);

    ulRet = RNIC_SendDlData(ucRmNetId, pstImmZc, enPktType);

    return ulRet;
}

/*****************************************************************************
 函 数 名  : RNIC_SendDlData
 功能描述  : RNIC_SendDlData接收数据时的处理过程
 输入参数  : enRmNetId  :网卡id
             pstData    :SKBUF数据包首地址
             enPdpType  :PDP 类型
 输出参数  : 无
 返 回 值  :
 调用函数  : VOS_UINT32:RNIC_OK, RNIC_ERROR
 被调函数  :

 修改历史     :
  1.日    期   : 2011年12月06日
    作    者   : 范晶
    修改内容   : 新生成函数
  2.日    期   : 2011年06月08日
    作    者   : zhangyizhan 60575
    修改内容   : DTS2012060708396配合北京修改
  3.日    期   : 2011年06月09日
    作    者   : zhangyizhan 60575
    修改内容   : DTS2012060902909性能问题修改，,V3R2/V7R1统一调用netif_rx
  4.日    期   : 2012年6月20日
    作    者   : A00165503
    修改内容   : DTS2012061904440: 增加用户面时延统计
  5.日    期   : 2012年6月20日
    作    者   : f00179208
    修改内容   : DTS2012070306267: 调用内核接口失败后，不需要释放内存
  6.日    期   : 2012年11月23日
    作    者   : f00179208
    修改内容   : DSDA Phase I: RNIC多实例
  7.日    期   : 2014年6月13日
    作    者   : A00165503
    修改内容   : DTS2014052607108: 规避下行灌包插拔核复位问题
*****************************************************************************/
VOS_UINT32 RNIC_SendDlData(
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;

    if (!RNIC_RMNET_IS_VALID(enRmNetId))
    {
        IMM_ZcFreeAny(pstImmZc);
        return RNIC_INVAL;
    }

    /* 获取网卡上下文 */
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(enRmNetId);

#if (defined(CONFIG_BALONG_SPE))
    if (RNIC_INVALID_SPE_PORT != pstNetCntxt->lSpePort)
    {
        return RNIC_SpeRxData(pstNetCntxt, pstImmZc, enPktType);
    }
#endif

    return RNIC_NetRxDataEx(pstNetCntxt, pstImmZc, enPktType);
}
/* Modified by l60609 for L-C互操作项目, 2014-01-06, End */

/*****************************************************************************
 函 数 名  : RNIC_StartFlowCtrl
 功能描述  : 启动上行流控
 输入参数  : ucRmNetId : 网卡ID
 输出参数  : 无
 返 回 值  : VOS_OK     - 启动流控成功
             VOS_ERR    - 启动流控失败
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2012年5月31日
   作    者   : A00165503
   修改内容   : 新生成函数
 2.日    期   : 2012年11月23日
   作    者   : f00179208
   修改内容   : DSDA Phase I: RNIC多实例
*****************************************************************************/
unsigned int RNIC_StartFlowCtrl(unsigned char ucRmNetId)
{
    RNIC_SET_FLOW_CTRL_STATUS(RNIC_FLOW_CTRL_STATUS_START, ucRmNetId);
    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : RNIC_StopFlowCtrl
 功能描述  : 停止上行流控
 输入参数  : ucRmNetId : 网卡ID
 输出参数  : 无
 返 回 值  : VOS_OK     - 停止流控成功
             VOS_ERR    - 停止流控失败
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2012年5月31日
   作    者   : A00165503
   修改内容   : 新生成函数
 2.日    期   : 2012年11月23日
   作    者   : f00179208
   修改内容   : DSDA Phase I: RNIC多实例
*****************************************************************************/
unsigned int RNIC_StopFlowCtrl(unsigned char ucRmNetId)
{
    RNIC_SET_FLOW_CTRL_STATUS(RNIC_FLOW_CTRL_STATUS_STOP, ucRmNetId);
    return VOS_OK;
}

/* Added by m00217266 for L-C互操作项目, 2014-1-21, begin */
/*****************************************************************************
 函 数 名  : RNIC_ConfigRmnetStatus
 功能描述  : 配置当前网卡状态（外部模块接口）
 输入参数  : 无
 输出参数  : 无
 返 回 值  : unsigned long
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年1月21日
    作    者   : m00217266
    修改内容   : 新生成函数

*****************************************************************************/
unsigned long RNIC_ConfigRmnetStatus(
    RNIC_RMNET_CONFIG_STRU             *pstConfigInfo
)
{
    /*可维可测，输出配置信息*/
    RNIC_MNTN_SndRmnetConfigInfoMsg(pstConfigInfo);
    RNIC_DBG_CONFIGCHECK_ADD_TOTLA_NUM();

    /* 参数检查 */
    /* 内部modem需要检查 rab id是否异常 */
    if (RNIC_MODEM_TYPE_INSIDE == pstConfigInfo->enModemType)
    {
        if (!RNIC_RAB_ID_IS_VALID(pstConfigInfo->ucRabId))
        {
            RNIC_DBG_CONFIGCHECK_ADD_RABID_ERR_NUM();
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild RAB id !");
            return VOS_ERR;
        }
    }
    /* 外部modem需要检查 pdn id是否异常 */
    else if (RNIC_MODEM_TYPE_OUTSIDE == pstConfigInfo->enModemType)
    {
        if (!RNIC_PDN_ID_IS_VALID(pstConfigInfo->ucPdnId))
        {
            RNIC_DBG_CONFIGCHECK_ADD_PDNID_ERR_NUM();
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild Pdn id !");
            return VOS_ERR;
        }
    }
    /* 无效MODEM TYPE */
    else
    {
        RNIC_DBG_CONFIGCHECK_ADD_MODEMTYPE_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild modem type!");
        return VOS_ERR;
    }

    /* 网卡操作类型异常直接返回error */
    if (!RNIC_RMNET_STATUS_IS_VALID(pstConfigInfo->enRmnetStatus))
    {
        RNIC_DBG_CONFIGCHECK_ADD_RMNETSTATUS_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild Rmnet Status !");
        return VOS_ERR;
    }

    /* IP类型非法直接返回error */
    if (!RNIC_IP_TYPE_IS_VALID(pstConfigInfo->enIpType))
    {
        RNIC_DBG_CONFIGCHECK_ADD_IPTYPE_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild IP type !");
        return VOS_ERR;
    }

    /* 发送内部消息 */
    if (VOS_OK != RNIC_SndRnicRmnetConfigReq(pstConfigInfo))
    {
        RNIC_DBG_CONFIGCHECK_ADD_SND_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: send pdp status ind fail !");
        return VOS_ERR;
    }

    RNIC_DBG_CONFIGCHECK_ADD_SUCC_NUM();
    return VOS_OK;
}
/* Added by m00217266 for L-C互操作项目, 2014-1-21, end */

/*****************************************************************************
 函 数 名  : RNIC_CheckNetCardStatus
 功能描述  : 检查RNIC网卡设备的状态
 输入参数  : pstNetCntxt --- 网卡上下文
 输出参数  : 无
 返 回 值  : RNIC_RESULT_TYPE_ENUM
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT32 RNIC_CheckNetCardStatus(RNIC_SPEC_CTX_STRU *pstNetCntxt)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv    = VOS_NULL_PTR;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId  = RNIC_RMNET_ID_BUTT;
    RNIC_RESULT_TYPE_ENUM_INT32         enRet      = RNIC_BUTT;

    pstPriv     = pstNetCntxt->pstPriv;
    enRmNetId   = pstNetCntxt->enRmNetId;

    /* 网卡设备检查 */
    if (VOS_NULL_PTR == pstPriv)
    {
        RNIC_DBG_NETCAED_DL_DISCARD_NUM(1, enRmNetId);
        return RNIC_INVAL;
    }

    /* 网卡未打开 */
    if (RNIC_NETCARD_STATUS_CLOSED == pstPriv->enStatus)
    {
        RNIC_DBG_DISCARD_DL_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.rx_dropped++;
        return RNIC_OK;
    }

    return enRet;
}

/*****************************************************************************
 函 数 名  : RNIC_AddMacHead
 功能描述  : RNIC添加MAC头
 输入参数  : enRmNetId   --- rmnet网卡ID
             pstImmZc    --- 数据包
             enPktType   --- 数据类型
 输出参数  : 无
 返 回 值  : RNIC_RESULT_TYPE_ENUM
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT32 RNIC_AddMacHead(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    VOS_UINT8                          *pucAddData = VOS_NULL_PTR;
    RNIC_RESULT_TYPE_ENUM_INT32         enRet      = RNIC_BUTT;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId  = RNIC_RMNET_ID_BUTT;

    enRmNetId   = pstNetCntxt->enRmNetId;

    /* 数据长度超出有效值(不包含MAC头) */
    if ((pstImmZc->len) > RNIC_MAX_PACKET)
    {
        RNIC_DBG_RECV_DL_BIG_PKT_NUM(1, enRmNetId);
        pstNetCntxt->pstPriv->stStats.rx_errors++;
        pstNetCntxt->pstPriv->stStats.rx_length_errors++;
        return RNIC_OK;
    }

    /* 填充MAC帧头，调用ImmZc接口将MAC帧头填入ImmZc中 */
    if (ADS_PKT_TYPE_IPV4 == enPktType)
    {
        pucAddData = (VOS_UINT8*)&g_astRnicManageTbl[enRmNetId].stIpv4Ethhead;
        RNIC_DBG_RECV_DL_IPV4_PKT_NUM(1, enRmNetId);
    }
    else if (ADS_PKT_TYPE_IPV6 == enPktType)
    {
        pucAddData = (VOS_UINT8*)&g_astRnicManageTbl[enRmNetId].stIpv6Ethhead;
        RNIC_DBG_RECV_DL_IPV6_PKT_NUM(1, enRmNetId);
    }
    else   /* 数据包类型与承载支持类型不一致 */
    {
        RNIC_DBG_RECV_DL_ERR_PKT_NUM(1, enRmNetId);
        return RNIC_PKT_TYPE_INVAL;
    }

    if (VOS_OK != IMM_ZcAddMacHead(pstImmZc, pucAddData))
    {
        RNIC_DBG_ADD_DL_MACHEAD_FAIL_NUM(1, enRmNetId);
        return RNIC_ADDMAC_FAIL;
    }

    return enRet;
}

/*****************************************************************************
 函 数 名  : RNIC_NetIfRx
 功能描述  : 发送下行数据到网络协议栈，只支持netif_rx和netif_rx_ni接口
 输入参数  : pstNetCntxt --- 网卡上下文
             pstImmZc    --- 数据包
 输出参数  : 无
 返 回 值  : VOS_INT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT32 RNIC_NetIfRx(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc
)
{
    VOS_INT32                           lNetRxRet  = NET_RX_SUCCESS;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId  = RNIC_RMNET_ID_BUTT;

    enRmNetId   = pstNetCntxt->enRmNetId;

#if (defined(CONFIG_BALONG_SPE))
    lNetRxRet = netif_rx(pstImmZc);
#else
    if (VOS_FALSE == VOS_CheckInterrupt())
    {
        lNetRxRet = netif_rx_ni(pstImmZc);
    }
    else
    {
        lNetRxRet = netif_rx(pstImmZc);
    }
#endif

    if (NET_RX_SUCCESS != lNetRxRet)
    {
        RNIC_DBG_SEND_DL_PKT_FAIL_NUM(1, enRmNetId);
        pstNetCntxt->pstPriv->stStats.rx_dropped++;

#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_NetIfRx, netif_rx fail ret is !", lNetRxRet);
#endif

        return RNIC_RX_PKT_FAIL;
    }

    /* 增加下行发送数据统计 */
    RNIC_DBG_SEND_DL_PKT_NUM(1, enRmNetId);

    return RNIC_OK;
}

/*****************************************************************************
 函 数 名  : RNIC_NetIfRxEx
 功能描述  : 发送下行数据到网络协议栈，支持NAPI+GRO功能
 输入参数  : pstNetCntxt --- 网卡上下文
             pstImmZc    --- 数据包
 输出参数  : 无
 返 回 值  : VOS_INT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT32 RNIC_NetIfRxEx(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc
)
{
    VOS_INT32                           lNetRxRet  = NET_RX_SUCCESS;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId  = RNIC_RMNET_ID_BUTT;

    enRmNetId   = pstNetCntxt->enRmNetId;

#if (defined(CONFIG_BALONG_SPE))
    lNetRxRet = netif_rx(pstImmZc);
#else
    if (VOS_FALSE == VOS_CheckInterrupt())
    {
        lNetRxRet = netif_rx_ni(pstImmZc);
    }
    else
    {
#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
        if (RNIC_NET_IF_NAPI == RNIC_GET_Net_API())
        {
            /* 正常情况下，下行数据包入Poll队列，等待网络协议栈取走。
             * 如果超出队列的最大限长，则丢弃该报文，避免系统内存耗尽
             */
            if (IMM_ZcQueueLen(RNIC_GET_PollBuff_QUE()) < RNIC_POLL_QUEUE_MAX_LEN)
            {
                IMM_ZcQueueTail(RNIC_GET_PollBuff_QUE(), pstImmZc);
            }
            else
            {
                IMM_ZcFreeAny(pstImmZc);
                lNetRxRet = NET_RX_DROP;
            }
        }
        else
#endif
        {
            lNetRxRet = netif_rx(pstImmZc);
        }
    }
#endif

    if (NET_RX_SUCCESS != lNetRxRet)
    {
        RNIC_DBG_SEND_DL_PKT_FAIL_NUM(1, enRmNetId);
        pstNetCntxt->pstPriv->stStats.rx_dropped++;
        return RNIC_RX_PKT_FAIL;
    }

#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
    /*
     * NAPI+GRO模式下，并没有直接送协议栈，而是等协议栈调用
     * RNIC_Poll主动来取数据包，在poll中统计下行发送数据
     */
    if (RNIC_NET_IF_NAPI != RNIC_GET_Net_API())
#endif
    {
        /* 增加下行发送数据统计 */
        RNIC_DBG_SEND_DL_PKT_NUM(1, enRmNetId);
    }

    return RNIC_OK;
}

/*****************************************************************************
 函 数 名  : RNIC_EncapEthHead
 功能描述  : RNIC添加以太网头部
 输入参数  : pstNetCntxt --- 网卡上下文
             pstImmZc    --- 数据包
             enPktType   --- 数据类型
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 RNIC_EncapEthHead(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv    = VOS_NULL_PTR;
#if (defined(CONFIG_BALONG_SPE))
    IMM_ZC_STRU                        *pstImmZc2  = VOS_NULL_PTR;
    VOS_UINT32                          ulCacheLen;
#endif
    VOS_UINT32                          ulRet      = RNIC_BUTT;

    pstPriv     = pstNetCntxt->pstPriv;

#if (defined(CONFIG_BALONG_SPE))
    if (VOS_TRUE == RNIC_IsSpeMem(pstImmZc))
    {
        ulCacheLen = IMM_ZcGetUsedLen(pstImmZc) + RNIC_MAC_HDR_LEN;

        RNIC_SpeMemUnmapRequset(pstImmZc, ulCacheLen);
        pstImmZc2 = skb_copy(pstImmZc, GFP_ATOMIC);
        RNIC_SpeMemMapRequset(pstImmZc, ulCacheLen);

        IMM_ZcFreeAny(pstImmZc);
        if (VOS_NULL_PTR == pstImmZc2)
        {
            return RNIC_NOMEM;
        }

        pstImmZc = pstImmZc2;
    }
#endif

    ulRet = (VOS_UINT32)RNIC_CheckNetCardStatus(pstNetCntxt);
    /* RNIC网卡设备异常 */
    if (RNIC_BUTT != ulRet)
    {
        IMM_ZcFreeAny(pstImmZc);

#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_EncapEthHead, RNIC_CheckNetCardStatus fail ret is !", ulRet);
#endif

        return ulRet;
    }

    ulRet = (VOS_UINT32)RNIC_AddMacHead(pstNetCntxt, pstImmZc, enPktType);
    /* 添加MAC头异常 */
    if (RNIC_BUTT != ulRet)
    {
        IMM_ZcFreeAny(pstImmZc);

#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_EncapEthHead, RNIC_AddMacHead fail ret is !", ulRet);
#endif

        return ulRet;
    }

    pstImmZc->protocol = eth_type_trans(pstImmZc, pstPriv->pstDev);

    return ulRet;
}

/*****************************************************************************
 函 数 名  : RNIC_NetRxData
 功能描述  : 向IP协议栈递交数据
 输入参数  : pstNetCntxt --- 网卡上下文
             pstImmZc    --- 数据包
             enPktType   --- 数据类型
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : RNIC新增NAPI+GRO功能降低复杂度

*****************************************************************************/
VOS_UINT32 RNIC_NetRxData(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    VOS_UINT32                          ulRet = RNIC_BUTT;

    ulRet = RNIC_EncapEthHead(pstNetCntxt, pstImmZc, enPktType);
    /* 封装以太网头失败 */
    if (RNIC_BUTT != ulRet)
    {
#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_NetRxData, RNIC_EncapEthHead fail ret is !", ulRet);
#endif

        return ulRet;
    }

    /* 统计网卡接收数据信息 */
    pstNetCntxt->pstPriv->stStats.rx_packets++;
    pstNetCntxt->pstPriv->stStats.rx_bytes += pstImmZc->len;

    /* 统计收到的下行数据字节数，用于流量上报 */
    pstNetCntxt->stDsFlowStats.ulTotalRecvFluxLow += pstImmZc->len;

    return (VOS_UINT32)RNIC_NetIfRx(pstNetCntxt, pstImmZc);
}

/*****************************************************************************
 函 数 名  : RNIC_NetRxDataEx
 功能描述  : 向IP协议栈递交数据，支持NAPI+GRO功能
 输入参数  : pstNetCntxt --- 网卡上下文
             pstImmZc    --- 数据包
             enPktType   --- 数据类型
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : RNIC新增NAPI+GRO功能

*****************************************************************************/
VOS_UINT32 RNIC_NetRxDataEx(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    VOS_UINT32                          ulRet = RNIC_BUTT;

    ulRet = RNIC_EncapEthHead(pstNetCntxt, pstImmZc, enPktType);
    /* 封装以太网头失败 */
    if (RNIC_BUTT != ulRet)
    {
        return ulRet;
    }

    /* 统计网卡接收数据信息 */
    pstNetCntxt->pstPriv->stStats.rx_packets++;
    pstNetCntxt->pstPriv->stStats.rx_bytes += pstImmZc->len;

    /* 统计收到的下行数据字节数，用于流量上报 */
    pstNetCntxt->stDsFlowStats.ulTotalRecvFluxLow += pstImmZc->len;

    return (VOS_UINT32)RNIC_NetIfRxEx(pstNetCntxt, pstImmZc);
}

/*****************************************************************************
 函 数 名  : RNIC_ProcessTxDataByModemType
 功能描述  : 根据MODEM类型处理发送数据
 输入参数  : pstNetCntxt --- 网卡上下文
             pstSkb      --- 数据包
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年3月30日
    作    者   : A00165503
    修改内容   : 新生成函数
*****************************************************************************/
VOS_VOID RNIC_ProcessTxDataByModemType(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    struct sk_buff                     *pstSkb
)
{
    RNIC_DBG_PRINT_UL_DATA(pstSkb);

    if (RNIC_MODEM_TYPE_INSIDE == pstNetCntxt->enModemType)
    {
        RNIC_RcvInsideModemUlData(pstSkb, pstNetCntxt);
    }
#if (FEATURE_ON == FEATURE_CL_INTERWORK)
    else if (RNIC_MODEM_TYPE_OUTSIDE == pstNetCntxt->enModemType)
    {
        RNIC_RcvOutsideModemUlData(pstSkb, pstNetCntxt);
    }
#endif
    else
    {
        IMM_ZcFreeAny(pstSkb);
    }

    return;
}

#if (defined(CONFIG_BALONG_SPE))
/*****************************************************************************
 函 数 名  : RNIC_IsSpeMem
 功能描述  : 检查SPE内存
 输入参数  : pstImmZc --- imm memory
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月06日
    作    者   : A00165503
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 RNIC_IsSpeMem(IMM_ZC_STRU *pstImmZc)
{
    return (0 != pstImmZc->spe_own);
}

/*****************************************************************************
 函 数 名  : RNIC_GetMemDma
 功能描述  : 获取SPE内存的物理地址
 输入参数  : pstImmZc --- imm memory
 输出参数  : 无
 返 回 值  : dma_addr_t
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月06日
    作    者   : A00165503
    修改内容   : 新生成函数

*****************************************************************************/
dma_addr_t RNIC_GetMemDma(IMM_ZC_STRU *pstImmZc)
{
    return RNIC_SPE_MEM_CB(pstImmZc)->ulDmaAddr;
}

/*****************************************************************************
 函 数 名  : RNIC_SpeMemMapRequset
 功能描述  : cache invalidate
 输入参数  : pstImmZc --- imm memory
             ulLen    --- cache size
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月06日
    作    者   : A00165503
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID RNIC_SpeMemMapRequset(IMM_ZC_STRU *pstImmZc, VOS_UINT32 ulLen)
{
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = RNIC_GetMemDma(pstImmZc);
    pucData   = phys_to_virt(ulDmaAddr);
    dma_map_single(VOS_NULL_PTR, pucData, ulLen, DMA_FROM_DEVICE);
    return;
}

/*****************************************************************************
 函 数 名  : RNIC_SpeMemUnmapRequset
 功能描述  : cache invalidate
 输入参数  : pstImmZc --- imm memory
             ulLen    --- cache size
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月06日
    作    者   : A00165503
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID RNIC_SpeMemUnmapRequset(IMM_ZC_STRU *pstImmZc, VOS_UINT32 ulLen)
{
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = RNIC_GetMemDma(pstImmZc);
    dma_unmap_single(VOS_NULL_PTR, ulDmaAddr, ulLen, DMA_FROM_DEVICE);
    return;
}

/*****************************************************************************
 函 数 名  : RNIC_SpeReadCB
 功能描述  : 从SPE端口接收数据
 输入参数  : VOS_INT32 lPort
             struct sk_buff *pstSkb
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年3月30日
    作    者   : A00165503
    修改内容   : 新生成函数
*****************************************************************************/
VOS_VOID RNIC_SpeReadCB(
    VOS_INT32                           lPort,
    struct sk_buff                     *pstSkb
)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;

    pstNetCntxt = RNIC_GetNetCntxtBySpePort(lPort);
    if (VOS_NULL_PTR == pstNetCntxt)
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_SPE_TX_PORTID_ERR_NUM(1);
        return;
    }

    /* 按MODEM类型分发处理数据 */
    RNIC_ProcessTxDataByModemType(pstNetCntxt, pstSkb);

    return;
}

/*****************************************************************************
 函 数 名  : RNIC_SpeRxData
 功能描述  : 递交数据给SPE
 输入参数  : RNIC_SPEC_CTX_STRU                 *pstNetCntxt
             IMM_ZC_STRU                        *pstImmZc
             ADS_PKT_TYPE_ENUM_UINT8             enPktType
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年3月30日
    作    者   : A00165503
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 RNIC_SpeRxData(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv    = VOS_NULL_PTR;
    VOS_UINT8                          *pucAddData = VOS_NULL_PTR;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId;

    pstPriv     = pstNetCntxt->pstPriv;
    enRmNetId   = pstNetCntxt->enRmNetId;

    /* 网卡设备检查 */
    if (VOS_NULL_PTR == pstPriv)
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_NETCAED_DL_DISCARD_NUM(1, enRmNetId);
        return RNIC_INVAL;
    }

    /* 网卡未打开 */
    if (RNIC_NETCARD_STATUS_CLOSED == pstPriv->enStatus)
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_DISCARD_DL_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.rx_dropped++;
        return RNIC_OK;
    }

    /* 数据长度超出有效值(不包含MAC头) */
    if ((pstImmZc->len) > RNIC_MAX_PACKET)
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_RECV_DL_BIG_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.rx_errors++;
        pstPriv->stStats.rx_length_errors++;
        return RNIC_OK;
    }

    /* 填充MAC帧头，调用ImmZc接口将MAC帧头填入ImmZc中 */
    if (ADS_PKT_TYPE_IPV4 == enPktType)
    {
        pucAddData = (VOS_UINT8 *)&g_astRnicManageTbl[enRmNetId].stIpv4Ethhead;
        mdrv_spe_set_skb_ipv4(pstImmZc);
        RNIC_DBG_RECV_DL_IPV4_PKT_NUM(1, enRmNetId);
    }
    else if (ADS_PKT_TYPE_IPV6 == enPktType)
    {
        pucAddData = (VOS_UINT8 *)&g_astRnicManageTbl[enRmNetId].stIpv6Ethhead;
        mdrv_spe_set_skb_ipv6(pstImmZc);
        RNIC_DBG_RECV_DL_IPV6_PKT_NUM(1, enRmNetId);
    }
    else   /* 数据包类型与承载支持类型不一致 */
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_RECV_DL_ERR_PKT_NUM(1, enRmNetId);
        return RNIC_PKT_TYPE_INVAL;
    }

    if (VOS_TRUE == pstNetCntxt->ulIpfPortFlg)
    {
        IMM_ZcPush(pstImmZc, RNIC_MAC_HDR_LEN);
    }
    else
    {
        if (VOS_OK != IMM_ZcAddMacHead(pstImmZc, pucAddData))
        {
            IMM_ZcFreeAny(pstImmZc);
            RNIC_DBG_ADD_DL_MACHEAD_FAIL_NUM(1, enRmNetId);
            return RNIC_ADDMAC_FAIL;
        }
    }

    /* 统计网卡接收数据信息 */
    pstPriv->stStats.rx_packets++;
    pstPriv->stStats.rx_bytes += pstImmZc->len;

    /* 数据递交成功由驱动负责释放, 递交失败由调用者释放 */
    if (MDRV_OK != mdrv_spe_wport_xmit(pstNetCntxt->lSpePort, pstImmZc))
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_SEND_DL_PKT_FAIL_NUM(1, enRmNetId);
        pstPriv->stStats.rx_dropped++;
        return RNIC_RX_PKT_FAIL;
    }

    /* 统计收到的下行数据流量 */
    RNIC_DBG_SEND_DL_PKT_NUM(1, enRmNetId);
    pstNetCntxt->stDsFlowStats.ulTotalRecvFluxLow += pstImmZc->len;

    return RNIC_OK;
}

/*****************************************************************************
 函 数 名  : RNIC_SpeInit
 功能描述  :
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年3月30日
    作    者   : A00165503
    修改内容   : 新生成函数

  2.日    期   : 2015年12月01日
    作    者   : A00165503
    修改内容   : DTS2015120205686: 注册SPE的PUSH回调

  3.日    期   : 2016年3月30日
    作    者   : A00165503
    修改内容   : DTS2016031107313: SPE的TD深度可配置, 防止ADS预申请的内存全
                 部挤压在SPE中, 导致下行断流

*****************************************************************************/
VOS_VOID RNIC_SpeInit(VOS_VOID)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    spe_wport_attr_t                    stAttr = {0};
    VOS_INT32                           lBypass;
    VOS_INT32                           lPort;
    VOS_UINT32                          ulTdNum;

    /* RMNET0支持SPE加速 */
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(RNIC_RMNET_ID_0);

    /* 获取TD深度配置 */
    ulTdNum  = ADS_IPF_GetSpeWPortTdDepth();

    /*
     * 获取USB虚拟网卡模式
     * 1 --- 网卡为BYPASS模式(STICK)
     * 0 --- 网卡为LINUX网卡
     */
    lBypass = mdrv_spe_usb_eth_is_bypass(0);
    if (0 == lBypass)
    {
        /* 检查网卡设备 */
        if (VOS_NULL_PTR == pstNetCntxt->pstPriv->pstDev)
        {
            RNIC_DEV_ERR_PRINTK("RNIC_SpeInit: pstNetDev is null!");
            return;
        }

        /* 设置端口参数 */
        RNIC_SET_SPE_PORT_ATTR(&stAttr, pstNetCntxt->pstPriv->pstDev, ulTdNum, 0);

        /* 打开SPE端口 */
        lPort = mdrv_spe_wport_open(&stAttr);
        if (lPort > 0)
        {
            pstNetCntxt->lSpePort     = lPort;
            pstNetCntxt->ulIpfPortFlg = VOS_TRUE;

            mdrv_spe_wport_ioctl(pstNetCntxt->lSpePort,
                                 SPE_WPORT_IOCTL_SET_RX_CB,
                                 RNIC_SpeReadCB);

            mdrv_spe_wport_ioctl(pstNetCntxt->lSpePort,
                                 SPE_WPORT_IOCTL_SET_MAC,
                                 (VOS_VOID *)RNIC_GET_ETH_HDR_ADDR(RNIC_RMNET_ID_0));

            mdrv_spe_wport_ioctl(pstNetCntxt->lSpePort,
                                 SPE_WPORT_IOCTL_SET_PUSH_CB,
                                 ADS_IPF_SpeIntWakeupADS);

            mdrv_spe_wport_ioctl(pstNetCntxt->lSpePort,
                                 SPE_WPORT_IOCTL_SET_RECYCLE_CB,
                                 ADS_IPF_RecycleMem);

            ADS_IPF_RegSpeWPort(pstNetCntxt->lSpePort);
        }
        else
        {
            RNIC_DEV_ERR_PRINTK("RNIC_SpeInit: Open spe port failed!");
        }
    }

    return;
}
#endif

/*****************************************************************************
 函 数 名  : RNIC_BST_GetModemInfo
 功能描述  : RNIC提供给BASTET的接口, 用于获取MDOEM信息
 输入参数  : pstNetDev    --- net device
 输出参数  : pstModemInfo --- modem info
 返 回 值  : VOS_OK/VOS_ERROR
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年7月28日
    作    者   : A00165503
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT RNIC_BST_GetModemInfo(
    struct net_device                  *pstNetDev,
    BST_RNIC_MODEM_INFO_STRU           *pstModemInfo
)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;

    if (VOS_NULL_PTR == pstNetDev)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_BST_GetModemInfo: pstNetDev is null.");
        return VOS_ERROR;
    }

    if (VOS_NULL_PTR == pstModemInfo)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_BST_GetModemInfo: pstModemInfo is null.");
        return VOS_ERROR;
    }

    pstPriv = (RNIC_NETCARD_DEV_INFO_STRU *)netdev_priv(pstNetDev);

    pstNetCntxt = RNIC_GetNetCntxtByRmNetId(pstPriv->enRmNetId);
    if (VOS_NULL_PTR == pstNetCntxt)
    {
        RNIC_DEV_ERR_PRINTK("RNIC_BST_GetModemInfo: enRmNetId is invalid.");
        return VOS_ERROR;
    }

    pstModemInfo->enIPv4State = (RNIC_PDP_REG_STATUS_DEACTIVE == pstNetCntxt->stPdpCtx.stIpv4PdpInfo.enRegStatus) ?
                                BST_RNIC_PDP_STATE_INACTIVE : BST_RNIC_PDP_STATE_ACTIVE;
    pstModemInfo->usModemId   = pstNetCntxt->enModemId;
    pstModemInfo->ucRabId     = pstNetCntxt->stPdpCtx.stIpv4PdpInfo.ucRabId;

    return VOS_OK;
}

#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
/*****************************************************************************
 函 数 名  : RNIC_NapiSchedule
 功能描述  : RNIC网卡挂载NAPI Poll List并触发软中断
 输入参数  : ulRmNetId --- RMNET网卡ID
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID RNIC_NapiSchedule(VOS_UINT32 ulRmNetId)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;

    if (!RNIC_RMNET_IS_VALID(ulRmNetId))
    {
        return;
    }

    /* 获取网卡上下文 */
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(ulRmNetId);

    /* 获取网卡设备信息 */
    pstPriv = pstNetCntxt->pstPriv;

    /* 检查网卡设备 */
    if (VOS_NULL_PTR != pstPriv)
    {
        napi_schedule(&pstPriv->stNapi);
    }

    return;
}

/*****************************************************************************
 函 数 名  : RNIC_Poll
 功能描述  : RNIC网卡NAPI Poll函数
 输入参数  : pstNapi  --- RMNET网卡设备的NAPI结构
             ulWeight --- RMNET网卡设备一次poll的最大包个数
 输出参数  : 无
 返 回 值  : VOS_INT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月10日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_INT32 RNIC_Poll(
    struct napi_struct                 *pstNapi,
    VOS_INT32                           lWeight
)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv  = VOS_NULL_PTR;
    IMM_ZC_STRU                        *pstImmZc = VOS_NULL_PTR;
    VOS_INT32                           lRxNum   = 0;

    pstPriv = container_of(pstNapi, RNIC_NETCARD_DEV_INFO_STRU, stNapi);

    for (lRxNum = 0; lRxNum < lWeight; lRxNum++)
    {
        pstImmZc = IMM_ZcDequeueHead(RNIC_GET_PollBuff_QUE());
        if (VOS_NULL_PTR == pstImmZc)
        {
            break;
        }

        napi_gro_receive(&pstPriv->stNapi, pstImmZc);
        RNIC_DBG_SEND_DL_PKT_NUM(1, pstPriv->enRmNetId);
        RNIC_DBG_GRO_IF_RECV_DL_PKT_NUM(1,pstPriv->enRmNetId);
    }

    /* If weight not fully consumed, exit the polling mode */
    if (lRxNum < lWeight)
    {
        napi_complete(pstNapi);
    }

    return lRxNum;
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
