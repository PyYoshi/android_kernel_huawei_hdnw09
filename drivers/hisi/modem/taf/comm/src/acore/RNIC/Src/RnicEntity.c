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
   1 ͷ�ļ�����
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
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_RNIC_ENTITY_C

/******************************************************************************
   2 �ⲿ������������
******************************************************************************/

/******************************************************************************
   3 ˽�ж���
******************************************************************************/

/******************************************************************************
   4 ȫ�ֱ�������
*****************************************************************************/

/******************************************************************************
   5 ����ʵ��
******************************************************************************/

/*****************************************************************************
 �� �� ��  : RNIC_ProcUlDataInPdpActive
 ��������  : ��PDP�����״̬��RNIC��������ʱ�Ĵ������
 �������  : pstSkb     :SKBUF�����׵�ַ
             pstNetCntxt:����������
             ucRabid    :��·���غ�
             enIpType   :IPv4��IPv6
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��13��
    ��    ��   : ����
    �޸�����   : �����ɺ���
  2.��    ��   : 2012��11��23��
    ��    ��   : f00179208
    �޸�����   : DSDA Phase I: RNIC��ʵ��
  3.��    ��   : 2013��6��3��
    ��    ��   : L47619
    �޸�����   : V3R3 Share-PDP��Ŀ�޸�
  4.��    ��   : 2015��5��28��
    ��    ��   : l00198894
    �޸�����   : TSTS
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

    /* ����Modem Id��װRabId */
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

    /* ͳ������������Ϣ */
    pstPriv->stStats.tx_packets++;
    pstPriv->stStats.tx_bytes += ulDataLen;

    /* ͳ���������� */
    pstNetCntxt->stDsFlowStats.ulPeriodSendPktNum++;
    pstNetCntxt->stDsFlowStats.ulTotalSendFluxLow += ulDataLen;

    return;
}

#if (FEATURE_ON == FEATURE_IMS)
/*****************************************************************************
 �� �� ��  : RNIC_ProcVoWifiULData
 ��������  :  ����vowifi����������
              ��ע��! ���������VOWIFI�����������У����Ƕ���������˵��
              IPЭ��ջ�ĳ������������У��պ÷�����
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��11��9��
    ��    ��   : n00269697
    �޸�����   : �����ɺ���

  2.��    ��   : 2016��12��27��
    ��    ��   : A00165503
    �޸�����   : DTS2016121600573: ����VOWIFIר������
*****************************************************************************/
VOS_VOID RNIC_ProcVoWifiULData(
    struct sk_buff                     *pstSkb,
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt
)
{

    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv = VOS_NULL_PTR;

    /* ��IMS��ΪWIFIʱ��RMNET_IMS�������ڵ�����ͨ��RNIC��CDS֮��ĺ˼���Ϣ���� */
    if (VOS_OK == RNIC_SendCdsImsDataReq(pstSkb, pstNetCntxt))
    {
        /* Modified by m00217266 for ˫VoWiFi��Ŀ, 2017-2-27, begin */
#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_DBG_SEND_UL_PKT_NUM(1, pstNetCntxt->enRmNetId);
#else
        RNIC_DBG_SEND_UL_PKT_NUM(1, RNIC_RMNET_ID_IMS1);
#endif
        /* Modified by m00217266 for ˫VoWiFi��Ŀ, 2017-2-27, end */

        /* ͳ������������Ϣ */
        pstPriv   = pstNetCntxt->pstPriv;

        pstPriv->stStats.tx_packets++;
        pstPriv->stStats.tx_bytes += pstSkb->len;

        /* ͳ���������� */
        pstNetCntxt->stDsFlowStats.ulPeriodSendPktNum++;
        pstNetCntxt->stDsFlowStats.ulTotalSendFluxLow += pstSkb->len;
    }

    IMM_ZcFreeAny(pstSkb);

    return;
}
#endif

/*****************************************************************************
 �� �� ��  : RNIC_ProcUlIpv4Data
 ��������  : RNIC����IPV4����������
 �������  : pstSkb     :SKBUF���ݰ��׵�ַ
             pstNetCntxt:����������
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
 1.��    ��   : 2011��12��15��
   ��    ��   : ����
   �޸�����   : �����ɺ���
 2.��    ��   : 2012��8��30��
   ��    ��   : l60609
   �޸�����   : AP������Ŀ����RNIC_TransSkbToImmZC����ͳ��ת��ʧ�ܵĴ���
 3.��    ��   : 2012��11��23��
   ��    ��   : f00179208
   �޸�����   : DSDA Phase I: RNIC��ʵ��

  4.��    ��   : 2016��12��27��
    ��    ��   : A00165503
    �޸�����   : DTS2016121600573: ����VOWIFIר������
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
    /* ��IMS��ΪWIFIʱ��RMNET_IMS�������ڵ�����ͨ��RNIC��CDS֮��ĺ˼���Ϣ���� */
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

    /* ��ȡ����ӳ���RABID */
    ucRabId = RNIC_GET_SPEC_NET_IPV4_RABID(enRmNetId);
    if (RNIC_RAB_ID_INVALID == ucRabId)
    {
        IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
        RNIC_DBG_RAB_ID_ERR_NUM(1, enRmNetId);
        return;
    }

    /* PDP�������������ݵĴ��� */
    RNIC_SendULDataInPdpActive(pstImmZc, pstNetCntxt, ucRabId, ADS_PKT_TYPE_IPV4);

    return;
}

/*****************************************************************************
 �� �� ��  : RNIC_ProcUlIpv6Data
 ��������  : RNIC����IPV6����������
 �������  : pstSkb     : SKBUF���ݰ��׵�ַ
             pstNetCntxt: ����������ָ��
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
 1.��    ��   : 2011��12��15��
   ��    ��   : ����
   �޸�����   : �����ɺ���
 2.��    ��   : 2012��8��30��
   ��    ��   : l60609
   �޸�����   : AP������Ŀ����RNIC_TransSkbToImmZC����ͳ��ת��ʧ�ܵĴ���
 3.��    ��   : 2012��11��23��
   ��    ��   : f00179208
   �޸�����   : DSDA Phase I: RNIC��ʵ��

  4.��    ��   : 2016��12��27��
    ��    ��   : A00165503
    �޸�����   : DTS2016121600573: ����VOWIFIר������
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
    /* ��IMS��ΪWIFIʱ��RMNET_IMS�������ڵ�����ͨ��RNIC��CDS֮��ĺ˼���Ϣ���� */
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

    /* ��ȡ����ӳ���RABID */
    ucRabId = RNIC_GET_SPEC_NET_IPV6_RABID(enRmNetId);
    if (RNIC_RAB_ID_INVALID == ucRabId)
    {
        IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
        RNIC_DBG_RAB_ID_ERR_NUM(1, enRmNetId);
        return;
    }

    /* PDP�������������ݵĴ��� */
    RNIC_SendULDataInPdpActive(pstImmZc, pstNetCntxt, ucRabId, ADS_PKT_TYPE_IPV6);

    return;
}

/*****************************************************************************
 �� �� ��  : RNIC_ProcDemDial
 ��������  : RNIC�����貦��
 �������  : struct sk_buff  *pstSkb,
             pBuf -- �洢���ݵĻ����ID

 �������  : ��
 �� �� ֵ  : OK
             ERROR
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2010��1��19��
    ��    ��   : ����
    �޸�����   : �����ɺ���

  2.��    ��   : 2012��6��6��
    ��    ��   : A00165503
    �޸�����   : DTS2012060502819: �����ʽ�������貦��, ����Ƶ���ϱ��¼�

*****************************************************************************/
VOS_UINT32 RNIC_ProcDemDial(
    struct sk_buff                     *pstSkb
)
{
    RNIC_DIAL_MODE_STRU                *pstDialMode;
    RNIC_TIMER_STATUS_ENUM_UINT8        enTiStatus;
    VOS_UINT32                          ulIpAddr;

    /* ��ȡIP��ַ */
    ulIpAddr = *((VOS_UINT32 *)((pstSkb->data) + RNIC_IP_HEAD_DEST_ADDR_OFFSET));

    /*����ǹ㲥�����򲻷����貦�ţ�ֱ�ӹ��˵�*/
    if (RNIC_IPV4_BROADCAST_ADDR == ulIpAddr)
    {
        RNIC_DBG_UL_RECV_IPV4_BROADCAST_NUM(1, RNIC_RMNET_ID_0);
        return VOS_ERR;
    }

    /* ��ȡ���貦�ŵ�ģʽ�Լ�ʱ���ĵ�ַ */
    pstDialMode = RNIC_GetDialModeAddr();

    /* ��ȡ��ǰ���ű�����ʱ����״̬ */
    enTiStatus  = RNIC_GetTimerStatus(TI_RNIC_DEMAND_DIAL_PROTECT);

    /*Ϊ�˷�ֹ���貦���ϱ�̫�죬����һ�����붨ʱ����*/
    if (RNIC_TIMER_STATUS_STOP == enTiStatus)
    {
        /* ֪ͨӦ�ý��в��Ų��� */
        if (RNIC_ALLOW_EVENT_REPORT == pstDialMode->enEventReportFlag)
        {
            if (VOS_OK == RNIC_SendDialEvent(DEVICE_ID_WAN, RNIC_DAIL_EVENT_UP))
            {
                /* �������ű�����ʱ��  */
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

/* Modified by l60609 for L-C��������Ŀ, 2014-1-14, begin */
#if (FEATURE_ON == FEATURE_CL_INTERWORK)
/*****************************************************************************
 �� �� ��  : RNIC_RcvOutsideModemUlData
 ��������  : CDMAģʽ���յ�TCP/IPЭ��ջ����������
 �������  : struct sk_buff                     *pstSkb
             RNIC_SPEC_CTX_STRU                 *pstNetCntxt
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��1��7��
    ��    ��   : m00217266
    �޸�����   : �����ɺ���

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

    /* ��ȡ��̫֡���� */
    usEthType   = VOS_NTOHS(((RNIC_ETH_HEADER_STRU *)(pstSkb->data))->usEtherType);

    /* IP���������ж� */
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

    /* �Ƿ�Pdn Id */
    if (RNIC_PDN_ID_INVALID == ucPdnId)
    {
        IMM_ZcFreeAny((IMM_ZC_STRU *)pstSkb);
        RNIC_DBG_PDN_ID_ERR_NUM(1, enRmNetId);
        return;
    }

    /* �Ƴ�MACͷ */
    if (VOS_OK != IMM_ZcRemoveMacHead(pstSkb))
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_UL_RMV_MAC_HDR_FAIL_NUM(1, enRmNetId);
        return;
    }

    /* дSDIO */
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
 �� �� ��  : RNIC_RcvInsideModemUlData
 ��������  : ���յ�TCP/IPЭ��ջ3GPP����������
 �������  : struct sk_buff                     *pstSkb
             RNIC_RMNET_ID_ENUM_UINT8            enRmNetId
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��1��7��
    ��    ��   : m00217266
    �޸�����   : �����ɺ���

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

    /* ��ȡ��̫֡���� */
    usEthType   = VOS_NTOHS(((RNIC_ETH_HEADER_STRU *)(pstSkb->data))->usEtherType);

    /* ���ؼ�� */
    if (RNIC_FLOW_CTRL_STATUS_START == RNIC_GET_FLOW_CTRL_STATUS(enRmNetId))
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_FLOW_CTRL_UL_DISCARD_NUM(1, enRmNetId);
        return;
    }

    /* �Ƴ�MACͷ */
    if (VOS_OK != IMM_ZcRemoveMacHead(pstSkb))
    {
        IMM_ZcFreeAny(pstSkb);
        RNIC_DBG_UL_RMV_MAC_HDR_FAIL_NUM(1, enRmNetId);
        pstPriv->stStats.tx_dropped++;
        return;
    }

    /* ֻ������0����Żᴥ�����貦�� */
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

    /* IP���������ж� */
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
/* Modified by l60609 for L-C��������Ŀ, 2014-1-14, end */

/* Modified by l60609 for L-C��������Ŀ, 2014-01-06, Begin */
#if (FEATURE_ON == FEATURE_CL_INTERWORK)
/*****************************************************************************
 �� �� ��  : RNIC_ShowSdioDlData
 ��������  : ��ӡsdioģ�����������
 �������  : pstSkb   :SKBUF�����׵�ַ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
 1.��    ��   : 2014��03��11��
   ��    ��   : m00217266
   �޸�����   : �����ɺ���
*****************************************************************************/
VOS_VOID RNIC_ShowSdioDlData(
    VOS_UINT8                           ucPdnId,
    IMM_ZC_STRU                        *pstImmZc
)
{
    VOS_UINT32                          i;

    /* RNIC �������ݴ�ӡ���أ���ӡȥmacͷ������ */
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
 �� �� ��  : RNIC_RcvSdioDlData
 ��������  : ����SDIO����������
 �������  : VOS_UINT8                           ucPdnId
             IMM_ZC_STRU                        *pstData
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��1��6��
    ��    ��   : m00217266
    �޸�����   : �����ɺ���

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

    /* ��ν�ɲ⣬��ӡ���ݰ����ݺ�pdn id */
    RNIC_ShowSdioDlData(ucPdnId, pstImmZc);

    /* ����PDNid��ȡ��Ӧ��RMNETid */
    ucNetIndex     = RNIC_GET_RM_NET_ID_BY_PDN_ID(ucPdnId);

    /* ��ȡЭ��汾�� */
    ucIpType    = RNIC_GET_IP_VERSION(pstImmZc->data[0]);

    if (RNIC_IPV4_VERSION == ucIpType)
    {
        enPktType = ADS_PKT_TYPE_IPV4;
    }
    else if (RNIC_IPV6_VERSION == ucIpType)
    {
        enPktType = ADS_PKT_TYPE_IPV6;
    }
    else    /* ���ݰ����������֧�����Ͳ�һ�� */
    {
        RNIC_DBG_RECV_DL_ERR_PKT_NUM(1, ucNetIndex);
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_RcvSdioDlData, Ip Type is !", ucIpType);

        /* �ͷ��ڴ� */
        IMM_ZcFreeAny(pstImmZc);

        return RNIC_PKT_TYPE_INVAL;
    }

    ulRet = RNIC_SendDlData(ucNetIndex, pstImmZc, enPktType);
    return ulRet;
}
#endif

/*****************************************************************************
 �� �� ��  : RNIC_RcvAdsDlData
 ��������  : RNIC�յ�ADS��������
 �������  : ucRabid  :��·���غ�
             pstData  :SKBUF���ݰ��׵�ַ
             enPdpType:PDP ����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��2��10��
    ��    ��   : m00217266
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��5��28��
    ��    ��   : l00198894
    �޸�����   : TSTS
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
 �� �� ��  : RNIC_SendDlData
 ��������  : RNIC_SendDlData��������ʱ�Ĵ������
 �������  : enRmNetId  :����id
             pstData    :SKBUF���ݰ��׵�ַ
             enPdpType  :PDP ����
 �������  : ��
 �� �� ֵ  :
 ���ú���  : VOS_UINT32:RNIC_OK, RNIC_ERROR
 ��������  :

 �޸���ʷ     :
  1.��    ��   : 2011��12��06��
    ��    ��   : ����
    �޸�����   : �����ɺ���
  2.��    ��   : 2011��06��08��
    ��    ��   : zhangyizhan 60575
    �޸�����   : DTS2012060708396��ϱ����޸�
  3.��    ��   : 2011��06��09��
    ��    ��   : zhangyizhan 60575
    �޸�����   : DTS2012060902909���������޸ģ�,V3R2/V7R1ͳһ����netif_rx
  4.��    ��   : 2012��6��20��
    ��    ��   : A00165503
    �޸�����   : DTS2012061904440: �����û���ʱ��ͳ��
  5.��    ��   : 2012��6��20��
    ��    ��   : f00179208
    �޸�����   : DTS2012070306267: �����ں˽ӿ�ʧ�ܺ󣬲���Ҫ�ͷ��ڴ�
  6.��    ��   : 2012��11��23��
    ��    ��   : f00179208
    �޸�����   : DSDA Phase I: RNIC��ʵ��
  7.��    ��   : 2014��6��13��
    ��    ��   : A00165503
    �޸�����   : DTS2014052607108: ������й����κ˸�λ����
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

    /* ��ȡ���������� */
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(enRmNetId);

#if (defined(CONFIG_BALONG_SPE))
    if (RNIC_INVALID_SPE_PORT != pstNetCntxt->lSpePort)
    {
        return RNIC_SpeRxData(pstNetCntxt, pstImmZc, enPktType);
    }
#endif

    return RNIC_NetRxDataEx(pstNetCntxt, pstImmZc, enPktType);
}
/* Modified by l60609 for L-C��������Ŀ, 2014-01-06, End */

/*****************************************************************************
 �� �� ��  : RNIC_StartFlowCtrl
 ��������  : ������������
 �������  : ucRmNetId : ����ID
 �������  : ��
 �� �� ֵ  : VOS_OK     - �������سɹ�
             VOS_ERR    - ��������ʧ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
 1.��    ��   : 2012��5��31��
   ��    ��   : A00165503
   �޸�����   : �����ɺ���
 2.��    ��   : 2012��11��23��
   ��    ��   : f00179208
   �޸�����   : DSDA Phase I: RNIC��ʵ��
*****************************************************************************/
unsigned int RNIC_StartFlowCtrl(unsigned char ucRmNetId)
{
    RNIC_SET_FLOW_CTRL_STATUS(RNIC_FLOW_CTRL_STATUS_START, ucRmNetId);
    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : RNIC_StopFlowCtrl
 ��������  : ֹͣ��������
 �������  : ucRmNetId : ����ID
 �������  : ��
 �� �� ֵ  : VOS_OK     - ֹͣ���سɹ�
             VOS_ERR    - ֹͣ����ʧ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
 1.��    ��   : 2012��5��31��
   ��    ��   : A00165503
   �޸�����   : �����ɺ���
 2.��    ��   : 2012��11��23��
   ��    ��   : f00179208
   �޸�����   : DSDA Phase I: RNIC��ʵ��
*****************************************************************************/
unsigned int RNIC_StopFlowCtrl(unsigned char ucRmNetId)
{
    RNIC_SET_FLOW_CTRL_STATUS(RNIC_FLOW_CTRL_STATUS_STOP, ucRmNetId);
    return VOS_OK;
}

/* Added by m00217266 for L-C��������Ŀ, 2014-1-21, begin */
/*****************************************************************************
 �� �� ��  : RNIC_ConfigRmnetStatus
 ��������  : ���õ�ǰ����״̬���ⲿģ��ӿڣ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : unsigned long
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2014��1��21��
    ��    ��   : m00217266
    �޸�����   : �����ɺ���

*****************************************************************************/
unsigned long RNIC_ConfigRmnetStatus(
    RNIC_RMNET_CONFIG_STRU             *pstConfigInfo
)
{
    /*��ά�ɲ⣬���������Ϣ*/
    RNIC_MNTN_SndRmnetConfigInfoMsg(pstConfigInfo);
    RNIC_DBG_CONFIGCHECK_ADD_TOTLA_NUM();

    /* ������� */
    /* �ڲ�modem��Ҫ��� rab id�Ƿ��쳣 */
    if (RNIC_MODEM_TYPE_INSIDE == pstConfigInfo->enModemType)
    {
        if (!RNIC_RAB_ID_IS_VALID(pstConfigInfo->ucRabId))
        {
            RNIC_DBG_CONFIGCHECK_ADD_RABID_ERR_NUM();
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild RAB id !");
            return VOS_ERR;
        }
    }
    /* �ⲿmodem��Ҫ��� pdn id�Ƿ��쳣 */
    else if (RNIC_MODEM_TYPE_OUTSIDE == pstConfigInfo->enModemType)
    {
        if (!RNIC_PDN_ID_IS_VALID(pstConfigInfo->ucPdnId))
        {
            RNIC_DBG_CONFIGCHECK_ADD_PDNID_ERR_NUM();
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild Pdn id !");
            return VOS_ERR;
        }
    }
    /* ��ЧMODEM TYPE */
    else
    {
        RNIC_DBG_CONFIGCHECK_ADD_MODEMTYPE_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild modem type!");
        return VOS_ERR;
    }

    /* �������������쳣ֱ�ӷ���error */
    if (!RNIC_RMNET_STATUS_IS_VALID(pstConfigInfo->enRmnetStatus))
    {
        RNIC_DBG_CONFIGCHECK_ADD_RMNETSTATUS_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild Rmnet Status !");
        return VOS_ERR;
    }

    /* IP���ͷǷ�ֱ�ӷ���error */
    if (!RNIC_IP_TYPE_IS_VALID(pstConfigInfo->enIpType))
    {
        RNIC_DBG_CONFIGCHECK_ADD_IPTYPE_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: invaild IP type !");
        return VOS_ERR;
    }

    /* �����ڲ���Ϣ */
    if (VOS_OK != RNIC_SndRnicRmnetConfigReq(pstConfigInfo))
    {
        RNIC_DBG_CONFIGCHECK_ADD_SND_ERR_NUM();
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ConfigRmnetStatus: send pdp status ind fail !");
        return VOS_ERR;
    }

    RNIC_DBG_CONFIGCHECK_ADD_SUCC_NUM();
    return VOS_OK;
}
/* Added by m00217266 for L-C��������Ŀ, 2014-1-21, end */

/*****************************************************************************
 �� �� ��  : RNIC_CheckNetCardStatus
 ��������  : ���RNIC�����豸��״̬
 �������  : pstNetCntxt --- ����������
 �������  : ��
 �� �� ֵ  : RNIC_RESULT_TYPE_ENUM
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_INT32 RNIC_CheckNetCardStatus(RNIC_SPEC_CTX_STRU *pstNetCntxt)
{
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv    = VOS_NULL_PTR;
    RNIC_RMNET_ID_ENUM_UINT8            enRmNetId  = RNIC_RMNET_ID_BUTT;
    RNIC_RESULT_TYPE_ENUM_INT32         enRet      = RNIC_BUTT;

    pstPriv     = pstNetCntxt->pstPriv;
    enRmNetId   = pstNetCntxt->enRmNetId;

    /* �����豸��� */
    if (VOS_NULL_PTR == pstPriv)
    {
        RNIC_DBG_NETCAED_DL_DISCARD_NUM(1, enRmNetId);
        return RNIC_INVAL;
    }

    /* ����δ�� */
    if (RNIC_NETCARD_STATUS_CLOSED == pstPriv->enStatus)
    {
        RNIC_DBG_DISCARD_DL_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.rx_dropped++;
        return RNIC_OK;
    }

    return enRet;
}

/*****************************************************************************
 �� �� ��  : RNIC_AddMacHead
 ��������  : RNIC���MACͷ
 �������  : enRmNetId   --- rmnet����ID
             pstImmZc    --- ���ݰ�
             enPktType   --- ��������
 �������  : ��
 �� �� ֵ  : RNIC_RESULT_TYPE_ENUM
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

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

    /* ���ݳ��ȳ�����Чֵ(������MACͷ) */
    if ((pstImmZc->len) > RNIC_MAX_PACKET)
    {
        RNIC_DBG_RECV_DL_BIG_PKT_NUM(1, enRmNetId);
        pstNetCntxt->pstPriv->stStats.rx_errors++;
        pstNetCntxt->pstPriv->stStats.rx_length_errors++;
        return RNIC_OK;
    }

    /* ���MAC֡ͷ������ImmZc�ӿڽ�MAC֡ͷ����ImmZc�� */
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
    else   /* ���ݰ����������֧�����Ͳ�һ�� */
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
 �� �� ��  : RNIC_NetIfRx
 ��������  : �����������ݵ�����Э��ջ��ֻ֧��netif_rx��netif_rx_ni�ӿ�
 �������  : pstNetCntxt --- ����������
             pstImmZc    --- ���ݰ�
 �������  : ��
 �� �� ֵ  : VOS_INT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

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

    /* �������з�������ͳ�� */
    RNIC_DBG_SEND_DL_PKT_NUM(1, enRmNetId);

    return RNIC_OK;
}

/*****************************************************************************
 �� �� ��  : RNIC_NetIfRxEx
 ��������  : �����������ݵ�����Э��ջ��֧��NAPI+GRO����
 �������  : pstNetCntxt --- ����������
             pstImmZc    --- ���ݰ�
 �������  : ��
 �� �� ֵ  : VOS_INT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

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
            /* ��������£��������ݰ���Poll���У��ȴ�����Э��ջȡ�ߡ�
             * ����������е�����޳��������ñ��ģ�����ϵͳ�ڴ�ľ�
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
     * NAPI+GROģʽ�£���û��ֱ����Э��ջ�����ǵ�Э��ջ����
     * RNIC_Poll������ȡ���ݰ�����poll��ͳ�����з�������
     */
    if (RNIC_NET_IF_NAPI != RNIC_GET_Net_API())
#endif
    {
        /* �������з�������ͳ�� */
        RNIC_DBG_SEND_DL_PKT_NUM(1, enRmNetId);
    }

    return RNIC_OK;
}

/*****************************************************************************
 �� �� ��  : RNIC_EncapEthHead
 ��������  : RNIC�����̫��ͷ��
 �������  : pstNetCntxt --- ����������
             pstImmZc    --- ���ݰ�
             enPktType   --- ��������
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

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
    /* RNIC�����豸�쳣 */
    if (RNIC_BUTT != ulRet)
    {
        IMM_ZcFreeAny(pstImmZc);

#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_EncapEthHead, RNIC_CheckNetCardStatus fail ret is !", ulRet);
#endif

        return ulRet;
    }

    ulRet = (VOS_UINT32)RNIC_AddMacHead(pstNetCntxt, pstImmZc, enPktType);
    /* ���MACͷ�쳣 */
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
 �� �� ��  : RNIC_NetRxData
 ��������  : ��IPЭ��ջ�ݽ�����
 �������  : pstNetCntxt --- ����������
             pstImmZc    --- ���ݰ�
             enPktType   --- ��������
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : RNIC����NAPI+GRO���ܽ��͸��Ӷ�

*****************************************************************************/
VOS_UINT32 RNIC_NetRxData(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    VOS_UINT32                          ulRet = RNIC_BUTT;

    ulRet = RNIC_EncapEthHead(pstNetCntxt, pstImmZc, enPktType);
    /* ��װ��̫��ͷʧ�� */
    if (RNIC_BUTT != ulRet)
    {
#ifdef CONFIG_VOWIFI_NEW_FRW
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_NetRxData, RNIC_EncapEthHead fail ret is !", ulRet);
#endif

        return ulRet;
    }

    /* ͳ����������������Ϣ */
    pstNetCntxt->pstPriv->stStats.rx_packets++;
    pstNetCntxt->pstPriv->stStats.rx_bytes += pstImmZc->len;

    /* ͳ���յ������������ֽ��������������ϱ� */
    pstNetCntxt->stDsFlowStats.ulTotalRecvFluxLow += pstImmZc->len;

    return (VOS_UINT32)RNIC_NetIfRx(pstNetCntxt, pstImmZc);
}

/*****************************************************************************
 �� �� ��  : RNIC_NetRxDataEx
 ��������  : ��IPЭ��ջ�ݽ����ݣ�֧��NAPI+GRO����
 �������  : pstNetCntxt --- ����������
             pstImmZc    --- ���ݰ�
             enPktType   --- ��������
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : RNIC����NAPI+GRO����

*****************************************************************************/
VOS_UINT32 RNIC_NetRxDataEx(
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType
)
{
    VOS_UINT32                          ulRet = RNIC_BUTT;

    ulRet = RNIC_EncapEthHead(pstNetCntxt, pstImmZc, enPktType);
    /* ��װ��̫��ͷʧ�� */
    if (RNIC_BUTT != ulRet)
    {
        return ulRet;
    }

    /* ͳ����������������Ϣ */
    pstNetCntxt->pstPriv->stStats.rx_packets++;
    pstNetCntxt->pstPriv->stStats.rx_bytes += pstImmZc->len;

    /* ͳ���յ������������ֽ��������������ϱ� */
    pstNetCntxt->stDsFlowStats.ulTotalRecvFluxLow += pstImmZc->len;

    return (VOS_UINT32)RNIC_NetIfRxEx(pstNetCntxt, pstImmZc);
}

/*****************************************************************************
 �� �� ��  : RNIC_ProcessTxDataByModemType
 ��������  : ����MODEM���ʹ���������
 �������  : pstNetCntxt --- ����������
             pstSkb      --- ���ݰ�
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��3��30��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���
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
 �� �� ��  : RNIC_IsSpeMem
 ��������  : ���SPE�ڴ�
 �������  : pstImmZc --- imm memory
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��10��06��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 RNIC_IsSpeMem(IMM_ZC_STRU *pstImmZc)
{
    return (0 != pstImmZc->spe_own);
}

/*****************************************************************************
 �� �� ��  : RNIC_GetMemDma
 ��������  : ��ȡSPE�ڴ�������ַ
 �������  : pstImmZc --- imm memory
 �������  : ��
 �� �� ֵ  : dma_addr_t
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��10��06��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���

*****************************************************************************/
dma_addr_t RNIC_GetMemDma(IMM_ZC_STRU *pstImmZc)
{
    return RNIC_SPE_MEM_CB(pstImmZc)->ulDmaAddr;
}

/*****************************************************************************
 �� �� ��  : RNIC_SpeMemMapRequset
 ��������  : cache invalidate
 �������  : pstImmZc --- imm memory
             ulLen    --- cache size
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��10��06��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���

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
 �� �� ��  : RNIC_SpeMemUnmapRequset
 ��������  : cache invalidate
 �������  : pstImmZc --- imm memory
             ulLen    --- cache size
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��10��06��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID RNIC_SpeMemUnmapRequset(IMM_ZC_STRU *pstImmZc, VOS_UINT32 ulLen)
{
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = RNIC_GetMemDma(pstImmZc);
    dma_unmap_single(VOS_NULL_PTR, ulDmaAddr, ulLen, DMA_FROM_DEVICE);
    return;
}

/*****************************************************************************
 �� �� ��  : RNIC_SpeReadCB
 ��������  : ��SPE�˿ڽ�������
 �������  : VOS_INT32 lPort
             struct sk_buff *pstSkb
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��3��30��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���
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

    /* ��MODEM���ͷַ��������� */
    RNIC_ProcessTxDataByModemType(pstNetCntxt, pstSkb);

    return;
}

/*****************************************************************************
 �� �� ��  : RNIC_SpeRxData
 ��������  : �ݽ����ݸ�SPE
 �������  : RNIC_SPEC_CTX_STRU                 *pstNetCntxt
             IMM_ZC_STRU                        *pstImmZc
             ADS_PKT_TYPE_ENUM_UINT8             enPktType
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��3��30��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���
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

    /* �����豸��� */
    if (VOS_NULL_PTR == pstPriv)
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_NETCAED_DL_DISCARD_NUM(1, enRmNetId);
        return RNIC_INVAL;
    }

    /* ����δ�� */
    if (RNIC_NETCARD_STATUS_CLOSED == pstPriv->enStatus)
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_DISCARD_DL_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.rx_dropped++;
        return RNIC_OK;
    }

    /* ���ݳ��ȳ�����Чֵ(������MACͷ) */
    if ((pstImmZc->len) > RNIC_MAX_PACKET)
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_RECV_DL_BIG_PKT_NUM(1, enRmNetId);
        pstPriv->stStats.rx_errors++;
        pstPriv->stStats.rx_length_errors++;
        return RNIC_OK;
    }

    /* ���MAC֡ͷ������ImmZc�ӿڽ�MAC֡ͷ����ImmZc�� */
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
    else   /* ���ݰ����������֧�����Ͳ�һ�� */
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

    /* ͳ����������������Ϣ */
    pstPriv->stStats.rx_packets++;
    pstPriv->stStats.rx_bytes += pstImmZc->len;

    /* ���ݵݽ��ɹ������������ͷ�, �ݽ�ʧ���ɵ������ͷ� */
    if (MDRV_OK != mdrv_spe_wport_xmit(pstNetCntxt->lSpePort, pstImmZc))
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_DBG_SEND_DL_PKT_FAIL_NUM(1, enRmNetId);
        pstPriv->stStats.rx_dropped++;
        return RNIC_RX_PKT_FAIL;
    }

    /* ͳ���յ��������������� */
    RNIC_DBG_SEND_DL_PKT_NUM(1, enRmNetId);
    pstNetCntxt->stDsFlowStats.ulTotalRecvFluxLow += pstImmZc->len;

    return RNIC_OK;
}

/*****************************************************************************
 �� �� ��  : RNIC_SpeInit
 ��������  :
 �������  : VOS_VOID
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2015��3��30��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���

  2.��    ��   : 2015��12��01��
    ��    ��   : A00165503
    �޸�����   : DTS2015120205686: ע��SPE��PUSH�ص�

  3.��    ��   : 2016��3��30��
    ��    ��   : A00165503
    �޸�����   : DTS2016031107313: SPE��TD��ȿ�����, ��ֹADSԤ������ڴ�ȫ
                 ����ѹ��SPE��, �������ж���

*****************************************************************************/
VOS_VOID RNIC_SpeInit(VOS_VOID)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    spe_wport_attr_t                    stAttr = {0};
    VOS_INT32                           lBypass;
    VOS_INT32                           lPort;
    VOS_UINT32                          ulTdNum;

    /* RMNET0֧��SPE���� */
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(RNIC_RMNET_ID_0);

    /* ��ȡTD������� */
    ulTdNum  = ADS_IPF_GetSpeWPortTdDepth();

    /*
     * ��ȡUSB��������ģʽ
     * 1 --- ����ΪBYPASSģʽ(STICK)
     * 0 --- ����ΪLINUX����
     */
    lBypass = mdrv_spe_usb_eth_is_bypass(0);
    if (0 == lBypass)
    {
        /* ��������豸 */
        if (VOS_NULL_PTR == pstNetCntxt->pstPriv->pstDev)
        {
            RNIC_DEV_ERR_PRINTK("RNIC_SpeInit: pstNetDev is null!");
            return;
        }

        /* ���ö˿ڲ��� */
        RNIC_SET_SPE_PORT_ATTR(&stAttr, pstNetCntxt->pstPriv->pstDev, ulTdNum, 0);

        /* ��SPE�˿� */
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
 �� �� ��  : RNIC_BST_GetModemInfo
 ��������  : RNIC�ṩ��BASTET�Ľӿ�, ���ڻ�ȡMDOEM��Ϣ
 �������  : pstNetDev    --- net device
 �������  : pstModemInfo --- modem info
 �� �� ֵ  : VOS_OK/VOS_ERROR
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��7��28��
    ��    ��   : A00165503
    �޸�����   : �����ɺ���

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
 �� �� ��  : RNIC_NapiSchedule
 ��������  : RNIC��������NAPI Poll List���������ж�
 �������  : ulRmNetId --- RMNET����ID
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID RNIC_NapiSchedule(VOS_UINT32 ulRmNetId)
{
    RNIC_SPEC_CTX_STRU                 *pstNetCntxt = VOS_NULL_PTR;
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv     = VOS_NULL_PTR;

    if (!RNIC_RMNET_IS_VALID(ulRmNetId))
    {
        return;
    }

    /* ��ȡ���������� */
    pstNetCntxt = RNIC_GET_SPEC_NET_CTX(ulRmNetId);

    /* ��ȡ�����豸��Ϣ */
    pstPriv = pstNetCntxt->pstPriv;

    /* ��������豸 */
    if (VOS_NULL_PTR != pstPriv)
    {
        napi_schedule(&pstPriv->stNapi);
    }

    return;
}

/*****************************************************************************
 �� �� ��  : RNIC_Poll
 ��������  : RNIC����NAPI Poll����
 �������  : pstNapi  --- RMNET�����豸��NAPI�ṹ
             ulWeight --- RMNET�����豸һ��poll����������
 �������  : ��
 �� �� ֵ  : VOS_INT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��03��10��
    ��    ��   : l00373346
    �޸�����   : �����ɺ���

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
