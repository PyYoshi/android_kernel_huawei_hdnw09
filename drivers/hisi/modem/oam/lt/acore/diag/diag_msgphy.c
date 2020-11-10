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

#include <mdrv.h>
#include "diag_mem.h"
#include "diag_msgphy.h"
#include "diag_msgbbp.h"
#include "diag_debug.h"
#include "diag_common.h"
#include "diag_cfg.h"
#include "soc_socp_adapter.h"


#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGDSP_C

#if ((VOS_OS_VER == VOS_RTOSCK) || (VOS_OS_VER == VOS_VXWORKS))

DIAG_LDSP_STATE_ENUM_U32 g_diagDspState = LDSP_NOT_INIT;
VOS_UINT32 g_diagDspSocpPowerOnReq = VOS_FALSE;


/*****************************************************************************
 Function Name   : DIAG_DspInitFinished
 Description     : �ṩ��PS��֪ͨDIAG LDSP��ʼ�����
 Input           : VOS_VOID
 Output          : None
 Return          : VOS_VOID

 History         :

*****************************************************************************/
VOS_VOID DIAG_DspInitFinished(VOS_VOID)
{
    g_diagDspState = LDSP_INITED;
    /* ����δ����,֪ͨLDSPֹͣʹ��SOCP */
    if(!(g_ulDiagCfgInfo & (1 << DIAG_CFG_CONN_BIT)))
    {
        diag_DspNotifySocpState(SOCP_DISABLE);
        g_diagDspState = LDSP_SOCP_DISABLE;
    }
}


/*****************************************************************************
 Function Name   : diag_AgentVoteToSocp
 Description     : DSP ��SOCPͶƱ
 Input           : SOCP_VOTE_TYPE_ENUM_U32 voteType
 Output          : None
 Return          : VOS_VOID

 History         :
    1.w00182550      2012-12-20  Draft Enact

*****************************************************************************/
VOS_VOID diag_DspVoteToSocp(SOCP_VOTE_TYPE_ENUM_U32 voteType)
{
#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)
    DIAG_MSG_SOCP_VOTE_REQ_STRU * voteReq;

    voteReq = (DIAG_MSG_SOCP_VOTE_REQ_STRU *)VOS_AllocMsg(MSP_PID_DIAG_AGENT,(sizeof(DIAG_MSG_SOCP_VOTE_REQ_STRU) - 20));
    if(voteReq == NULL)
    {
        diag_printf("%s: alloc msg fail\n", __FUNCTION__);
        return;
    }
    voteReq->ulReceiverPid = MSP_PID_DIAG_APP_AGENT;
    voteReq->ulSenderPid   = MSP_PID_DIAG_AGENT;/*TODO:��ʱʹ��diag agent��Ϊsenderpid�����޸�Ϊdsp pid*/
    voteReq->ulLength      = sizeof(DIAG_MSG_SOCP_VOTE_REQ_STRU) - 20;
    voteReq->ulMsgId       = DIAG_MSG_PHY_SOCP_VOTE_REQ;
    voteReq->ulVoteId      = SOCP_VOTE_TL_DSP;
    voteReq->ulVoteType    = voteType;

    if(ERR_MSP_SUCCESS != VOS_SendMsg(MSP_PID_DIAG_AGENT, voteReq))
    {
        diag_printf("%s: send msg fail\n", __FUNCTION__);
    }
#endif
}


/*****************************************************************************
 Function Name   : diag_DspSocpVoteCnfMsgProc
 Description     : ͶƱ�ظ���Ϣ����ֻ����LDSP�״γ�ʼ��ʱ�ķ���Ʊ�ظ�
 Input           : MsgBlock * pMsgBlock
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.w00182550      2012-12-20  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_DspSocpVoteCnfMsgProc(MsgBlock * pMsgBlock)
{
#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)
    DIAG_MSG_SOCP_VOTE_WAKE_CNF_STRU * voteCnf;

    voteCnf = (DIAG_MSG_SOCP_VOTE_WAKE_CNF_STRU *)pMsgBlock;
    if(voteCnf->ulVoteType == SOCP_VOTE_FOR_WAKE)
    {
        if(voteCnf->ulVoteRst != ERR_MSP_SUCCESS)
        {
            diag_printf("%s: vote fail", __FUNCTION__);
            return ERR_MSP_FAILURE;
        }
    }
#endif
    return ERR_MSP_SUCCESS;
}



/*****************************************************************************
 Function Name   : DIAG_SocpPowerOn
 Description     : �ṩ��PS��SOCP�ϵ�ӿ�
 Input           : VOS_VOID
 Output          : None
 Return          : VOS_UINT32

 History         :

*****************************************************************************/
VOS_UINT32 DIAG_SocpPowerOn(VOS_VOID)
{

    g_diagDspState = LDSP_INITING;

#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)

    if(g_diagDspSocpPowerOnReq == VOS_TRUE)
    {
        diag_printf("%s: power on req is running\n", __FUNCTION__);
        return ERR_MSP_SUCCESS;
    }

    g_diagDspSocpPowerOnReq = VOS_TRUE;

    if(0 != DRV_SOCP_VOTE_TO_MCORE(SOCP_VOTE_FOR_WAKE))
    {
        diag_printf("%s: socp power on fail\n", __FUNCTION__);
        return ERR_MSP_FAILURE;
    }
#endif
    return ERR_MSP_SUCCESS;
}

/*****************************************************************************
 Function Name   : diag_DspMailboxCb
 Description     :  ���������ע��Ļص���������DSP��DIAG����Ϣʱ����ص��˺���
 Input           : MAILBOX_INT_TYPE_E
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_DspMailboxCb(VOS_UINT32 len)
{
    VOS_UINT32 ret;
    VOS_UINT8* pData = VOS_NULL;

    /* �����ڴ� */
    pData = VOS_MemAlloc(MSP_PID_DIAG_AGENT,DYNAMIC_MEM_PT,len);
    if(VOS_NULL == pData)
    {
        diag_printf("[%s] Mem Alloc failed !\n",__FUNCTION__);
        return;
    }

    /* �������е�����ֱ�Ӷ�ȡ���ڴ��� */
    ret = mdrv_mailbox_read_commsg(EN_MAILBOX_SERVICE_RTT_SYS_CTRL, (VOS_UINT8 *)pData, len, EN_MAILBOX_SLEEP_WAKEUP);
    if(ret)
    {
        diag_printf("[%s] BSP_MailBox_ComMsgRead failed !\n",__FUNCTION__);
        (VOS_VOID)VOS_MemFree(MSP_PID_DIAG_AGENT, pData);
        return;
    }
    
/*
DRV_SOCP_VOTE_TO_MCORE�ӿ����л�ȡ�ź����������Բ������ⲿ�ִ����ܷ��ڶ�����Ļص������
��ʱ���Σ�����������ֲʱ��Ҫ������������ۺϿ��� 
*/
#if 0
#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)
    /* LDSP�״μ��أ���Ҫ��MCOREͶƱ�µ� */
    if(g_diagDspSocpPowerOnReq == VOS_TRUE)
    {
        if(0 != DRV_SOCP_VOTE_TO_MCORE(SOCP_VOTE_FOR_SLEEP))
        {
            diag_printf("%s: socp power on fail\n", __FUNCTION__);
        }
        else
        {
            g_diagDspSocpPowerOnReq = VOS_FALSE;
        }
    }
    else if(g_diagDspState == LDSP_SOCP_DISABLE)
    {
        diag_printf("%s: enter Vote Msg to Ap Ops!\n", __FUNCTION__);
        diag_DspVoteToSocp(SOCP_VOTE_FOR_SLEEP);
    }
#endif
#endif
    (VOS_VOID)VOS_MemFree(MSP_PID_DIAG_AGENT, pData);
}

/*****************************************************************************
 Function Name   : diag_DspNotifySocpState
 Description     : ������Ϣ��DSP,֪ͨDSP��ǰ��������״̬
 Input           : VOS_UINT32 state
 Output          : None
 Return          : VOS_VOID

 History         :

*****************************************************************************/
VOS_VOID diag_DspNotifySocpState(DIAG_SOCP_STATE_ENUM_U32 state)
{
    DIAG_MSG_TO_DSP_STRU pData;
    VOS_UINT32  ret;

    pData.ulMsgId = 0;
    pData.ulOpId  = 0;
    pData.ulReq   = state;

    ret = mdrv_mailbox_write_commsg(EN_MAILBOX_SERVICE_RTT_SYS_CTRL, &pData, sizeof(DIAG_MSG_TO_DSP_STRU), EN_MAILBOX_SLEEP_WAKEUP);
    if(ERR_MSP_SUCCESS != ret)
    {
        diag_printf("%s: send to dsp fail ret=0x%x\n", __FUNCTION__, ret);
        return ;
    }
}


/*****************************************************************************
 Function Name   : diag_DspConnMsgProc
 Description     : �����·����ӡ��Ͽ����������
 Input           : DIAG_CONNECT_CMD_ENUM_U32 connCmd
 Output          : None
 Return          : VOS_VOID

 History         :

*****************************************************************************/
VOS_VOID diag_DspConnMsgProc(DIAG_CONNECT_CMD_ENUM_U32 connCmd)
{
    if(connCmd >= DIAG_CONNECT_CMD_BUTT)
    {
        diag_printf("%s: invalid cmd 0x%x\n", __FUNCTION__, connCmd);
        return;
    }

    /* LDSPδ��ʼ���������ڳ�ʼ�� */
    if(g_diagDspState == LDSP_NOT_INIT || g_diagDspState == LDSP_INITING)
    {
        return;
    }

    /* LDSP�Ѿ���ʼ����������ã����ӡ��Ͽ�������Ҫ֪ͨLDSP SOCP״̬�仯 */
    if(DIAG_CONNECT_CMD == connCmd)
    {
#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)
        /* ֪ͨDSP SOCP���� */
        diag_DspVoteToSocp(SOCP_VOTE_FOR_WAKE);
#endif
        diag_DspNotifySocpState(SOCP_ENABLE);
        g_diagDspState = LDSP_SOCP_ENABLE;
        return;
    }

    if(DIAG_DISCONNECT_CMD == connCmd)
    {
        /* ֪ͨDSP SOCP�����ã��ڵȵ�����ظ���Ͷ�޳�Ʊ */
        diag_DspNotifySocpState(SOCP_DISABLE);
        g_diagDspState = LDSP_SOCP_DISABLE;
        return;
    }
    /* coverity[dead_error_line] */
}

/*****************************************************************************
 Function Name   : diag_DspSendMsg
 Description     : ������Ϣ��DSP
 Input           :
 Output          : None
 Return          : VOS_VOID

 History         :

*****************************************************************************/
VOS_UINT32 diag_DspSendMsg(MAILBOX_SERVICE_TYPE_E serType,VOS_UINT8* pstReq ,VOS_UINT32 ulCmdId,VOS_UINT32 ulDataLen)
{
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    VOS_CHAR* pData = NULL;


    pData = (VOS_VOID*)VOS_MemAlloc(MSP_PID_DIAG_AGENT,DYNAMIC_MEM_PT, ulDataLen+sizeof(ulCmdId));
    if(VOS_NULL == pData)
    {
        return ERR_MSP_MALLOC_FAILUE;
    }

    (VOS_VOID)VOS_MemCpy_s(pData, sizeof(ulCmdId), &ulCmdId, sizeof(ulCmdId));
    (VOS_VOID)VOS_MemCpy_s(pData+sizeof(ulCmdId),ulDataLen,pstReq,ulDataLen);


    ulRet = mdrv_mailbox_write_commsg((MAILBOX_SERVICE_TYPE_E)serType, pData, ulDataLen+sizeof(ulCmdId), EN_MAILBOX_SLEEP_WAKEUP);
    if(ulRet != ERR_MSP_SUCCESS)
    {
        diag_printf("[%s]:BSP_MailBox_ComMsgWrite err! 0x%x\n",__FUNCTION__,ulRet);
    }

    (VOS_VOID)VOS_MemFree(MSP_PID_DIAG_AGENT, pData);

    return ulRet;
}


/*****************************************************************************
 Function Name   : diag_AppTransPhyProc
 Description     : A�˷�������phy��������
 Input           : pMsgBlock    ����������������
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.c64416      2014-11-18  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_AppTransPhyProc(MsgBlock* pMsgBlock)
{
    DIAG_PHY_MSG_A_TRANS_C_STRU *pstInfo;

    pstInfo = (DIAG_PHY_MSG_A_TRANS_C_STRU *)pMsgBlock;

    return diag_DspMsgProc(&pstInfo->stInfo);
}
#endif


#if(VOS_OS_VER == VOS_LINUX)
/*****************************************************************************
 Function Name   : diag_GuDspTransProc
 Description     : GU DSP������ֱ��͸����Ϣ��GUDSP������MSP����ظ�
 Input           : pData    ����������������
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.c00326366      2015-06-14  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_GuDspTransProc(DIAG_FRAME_INFO_STRU *pData)
{
    VOS_UINT32 ulRet                    = ERR_MSP_SUCCESS;
    DIAG_COMM_CNF_STRU stRttCnf         = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo   = {0};
    DIAG_OSA_MSG_STRU *pstMsg           = NULL;
    DIAG_OSA_MSG_STRU *pstVosMsg        = NULL;

    pstMsg = (DIAG_OSA_MSG_STRU *)(pData->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));

    pstVosMsg = (DIAG_OSA_MSG_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, pstMsg->ulLength);

    if (pstVosMsg != NULL)
    {
        pstVosMsg->ulReceiverPid  = pstMsg->ulReceiverPid;

        VOS_MemCpy_s(&pstVosMsg->ulMsgId, pstMsg->ulLength, &pstMsg->ulMsgId, pstMsg->ulLength);

        ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstVosMsg);
        if (ulRet != VOS_OK)
        {
            diag_printf("diag_debug_write VOS_SendMsg failed!\n");
        }
    }

    stRttCnf.ulRet  = ulRet;
    DIAG_MSG_COMMON_PROC(stDiagInfo, stRttCnf, pData);

    stDiagInfo.ulMsgType = pData->stID.pri4b;

    /*����ظ�*/
    ulRet = DIAG_MsgReport(&stDiagInfo, &stRttCnf, sizeof(stRttCnf));

    return ulRet;
}

#endif


/*****************************************************************************
 Function Name   : diag_DspMsgProc
 Description     : DSP������Ϣ����������ӶϿ�
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_UINT32 diag_DspMsgProc(DIAG_FRAME_INFO_STRU *pData)
{
    VOS_UINT32 ulRet                    = ERR_MSP_SUCCESS;
    DIAG_COMM_CNF_STRU stRttCnf         = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo   = {0};

#if(VOS_OS_VER == VOS_LINUX)
    VOS_UINT32 ulLen;
    DIAG_PHY_MSG_A_TRANS_C_STRU *pstInfo;
#endif

    switch(pData->stID.mode4b)
    {
        /* GUDSP��������A��ֱ��͸����GUDSP���� */
#if(VOS_OS_VER == VOS_LINUX)
        case DIAG_MODE_GSM:
        case DIAG_MODE_UMTS:
        case DIAG_MODE_1X:
        case DIAG_MODE_HRPD:
            return diag_GuDspTransProc(pData);

        default:
            break;
#endif

        /* GU����Ϣ�Ѿ���A�˴���C��ֻ����TL������ */
#if ((VOS_OS_VER == VOS_RTOSCK) || (VOS_OS_VER == VOS_VXWORKS))
        /*LDSP*/
        case DIAG_MODE_LTE:
            ulRet = diag_DspSendMsg(EN_MAILBOX_SERVICE_LTE_OM,pData->aucData+sizeof(MSP_DIAG_DATA_REQ_STRU),pData->ulCmdId,pData->ulMsgLen-sizeof(MSP_DIAG_DATA_REQ_STRU));
            break;
        /*TDSP*/
        case DIAG_MODE_TDS:
            ulRet = diag_DspSendMsg(EN_MAILBOX_SERVICE_TDS_OM,pData->aucData+sizeof(MSP_DIAG_DATA_REQ_STRU),pData->ulCmdId,pData->ulMsgLen-sizeof(MSP_DIAG_DATA_REQ_STRU));
            break;
        default:
            diag_printf("DIAG receive error mode message : %d.\n", pData->stID.mode4b);
            return ulRet;
#endif
    }

#if(VOS_OS_VER == VOS_LINUX)
    ulLen = sizeof(DIAG_PHY_MSG_A_TRANS_C_STRU)-VOS_MSG_HEAD_LENGTH + pData->ulMsgLen;
    pstInfo = (DIAG_PHY_MSG_A_TRANS_C_STRU*)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);
    if(VOS_NULL == pstInfo)
    {
        ulRet = ERR_MSP_MALLOC_FAILUE;
        goto DIAG_ERROR;
    }
    pstInfo->ulReceiverPid = MSP_PID_DIAG_AGENT;
    pstInfo->ulSenderPid   = MSP_PID_DIAG_APP_AGENT;
    pstInfo->ulMsgId       = DIAG_MSG_PHY_A_TRANS_C_REQ;
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

    stRttCnf.ulRet = ulRet;
    DIAG_MSG_COMMON_PROC(stDiagInfo, stRttCnf, pData);

    stDiagInfo.ulMsgType = pData->stID.pri4b;
    stDiagInfo.ulMode    = pData->stID.mode4b;

    /*����ظ�*/
    ulRet = DIAG_MsgReport(&stDiagInfo, &stRttCnf, sizeof(stRttCnf));
    return ulRet;
}

/*****************************************************************************
 Function Name   : diag_DspMsgInit
 Description     : MSP dsp���ֳ�ʼ��
 Input           : None
 Output          : None
 Return          : None
 History         :

*****************************************************************************/
VOS_VOID diag_DspMsgInit(VOS_VOID)
{
#if ((VOS_OS_VER == VOS_RTOSCK) || (VOS_OS_VER == VOS_VXWORKS))
    VOS_UINT32 ulRet;
    /*��ʼ��DSP SOCPͨ��*/
#ifdef ENABLE_DIAG_FIX_ADDR
    (VOS_VOID)DRV_SOCP_INIT_LTE_DSP(SOCP_CODER_SRC_LDSP1,DIAG_CORER_SRC_DSP_PADDR,DIAG_CORER_SRC_DSP_LENGTH);
#else
    (VOS_VOID)DRV_SOCP_INIT_LTE_DSP(SOCP_CODER_SRC_LDSP1,LPHY_MAILBOX_HDLC_BUF_BASE_ADDR,LPHY_MAILBOX_HDLC_BUF_SIZE);
#endif
    /* ������ע��ص����� */
    ulRet = mdrv_mailbox_register_rcvnotify(EN_MAILBOX_SERVICE_RTT_SYS_CTRL, diag_DspMailboxCb);
    if(ERR_MSP_SUCCESS != ulRet)
    {
        diag_printf("[%s]:mdrv_mailbox_register_rcvnotify err!\n",__FUNCTION__);
        return ;
    }
    
#if(FEATURE_SOCP_ON_DEMAND == FEATURE_ON)
    /*dsp ��ʼ����ʼͶ�޳�Ʊ*/
    diag_DspVoteToSocp(SOCP_VOTE_FOR_SLEEP);
#endif

#endif

#if (VOS_OS_VER == VOS_LINUX)
    /*ע��message��Ϣ�ص�*/
    DIAG_MsgProcReg(DIAG_MSG_TYPE_PHY, diag_DspMsgProc);
#endif

}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


