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


#include "diag_common.h"
#include "diag_msgmsp.h"
#include "diag_msgbbp.h"
#include "diag_msgps.h"
#include "diag_cfg.h"
#include "msp_errno.h"
#include "diag_debug.h"
#include "diag_api.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_MSGMSP_C

typedef VOS_UINT32 (*DIAG_MSGMSP_PROC_FUNC)(VOS_UINT8* pstReq);

typedef struct
{
    VOS_UINT32              ulCmdId;
    DIAG_MSGMSP_PROC_FUNC   pfnProc;
}DIAG_MSGMSP_PROC_STRU;


DIAG_MSGMSP_PROC_STRU g_astMsgMsp[] =
{
    {DIAG_CMD_LOG_CAT_PRINT,        diag_PrintCfgProc},
    {DIAG_CMD_LOG_CAT_LAYER,        diag_LayerCfgProc},
    {DIAG_CMD_LOG_CAT_AIR,          diag_AirCfgProc},
    {DIAG_CMD_LOG_CAT_EVENT,        diag_EventCfgProc},
    {DIAG_CMD_LOG_CAT_MSG,          diag_MsgCfgProc},

    {DIAG_CMD_GTR_SET,              diag_GtrProcEntry},
    {DIAG_CMD_GU_GTR_SET,           diag_GuGtrProcEntry},

    {DIAG_CMD_HOST_DISCONNECT,      diag_DisConnProc},

#if (VOS_OS_VER == VOS_LINUX)
    {DIAG_CMD_HOST_CONNECT,         diag_ConnProc},
    {DIAG_CMD_HOST_CONNECT_AGENT,   diag_ConnProc},
    {DIAG_CMD_GET_TIMESTAMP_VALUE,  diag_GetTimeStampInitValue},
    {DIAG_CMD_GET_MODEM_NUM,        diag_GetModemNum},
    {DIAG_CMD_PID_TABLE_MSG,        diag_GetPidTable},
#endif

};

VOS_UINT32 diag_MspMsgProc(DIAG_FRAME_INFO_STRU *pData);

#if (VOS_OS_VER == VOS_LINUX)
/*****************************************************************************
 Function Name   : diag_MspMsgInit
 Description     : MSP��������ʼ���ӿ�

 History         :
    1.c64416         2014-11-18  Draft Enact

*****************************************************************************/
void diag_MspMsgInit(void)
{
    VOS_UINT32 ulRet;

    ulRet = DIAG_MsgProcReg(DIAG_MSG_TYPE_MSP, diag_MspMsgProc);
    if(VOS_OK != ulRet)
    {
        diag_printf("diag_MspMsgInit DIAG_MsgProcReg failed.\n");
    }
}

/*****************************************************************************
 Function Name   : diag_GetModemNum
 Description     : ��ȡmodem num
 Input           : pstReq ����������
 Output          : None
 Return          : VOS_UINT32

    1.c00326366      2012-11-22  Draft Enact
*****************************************************************************/
VOS_UINT32 diag_GetModemNum(VOS_UINT8* pstReq)
{
    VOS_UINT ret = ERR_MSP_SUCCESS;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_CMD_GET_MODEM_NUM_CNF_STRU stModemNum = {0};
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    DIAG_MSG_COMMON_PROC(stDiagInfo, stModemNum, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

#if ( FEATURE_MULTI_MODEM == FEATURE_ON )
#if (MULTI_MODEM_NUMBER == 3)
    stModemNum.ulNum = 3;
#else
    stModemNum.ulNum = 2;
#endif
#else
    stModemNum.ulNum = 1;
#endif

    stModemNum.ulRc  = ERR_MSP_SUCCESS;

    /*�����FW�ظ�*/
    ret = DIAG_MsgReport(&stDiagInfo, &stModemNum, sizeof(stModemNum));

    return (VOS_UINT32)ret;
}


/*****************************************************************************
 Function Name   : diag_GetPidTable
 Description     : ��ȡPID�б�
 Input           : pstReq ����������
 Output          : None
 Return          : VOS_UINT32

    1.c00326366      2016-02-14  Draft Enact
*****************************************************************************/
VOS_UINT32 diag_GetPidTable(VOS_UINT8* pstReq)
{
    VOS_UINT32 i, num, ulRc, len, ret;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_CMD_PID_TABLE_CNF_STRU *pstPidTable = VOS_NULL;
    DIAG_FRAME_INFO_STRU *pstDiagHead = VOS_NULL;
    VOS_UINT32 *pulPid = VOS_NULL;

    num = (VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND) + (VOS_CPU_ID_1_PID_BUTT - VOS_PID_CPU_ID_1_DOPRAEND);

    pulPid = (VOS_UINT32 *)VOS_MemAlloc(DIAG_AGENT_PID, DYNAMIC_MEM_PT, (num * sizeof(VOS_UINT32)));

    num  = 0;
    ulRc = ERR_MSP_SUCCESS;

    if(VOS_NULL == pulPid)
    {
        return ERR_MSP_FAILURE;
    }
    else
    {
        /* ��ȡA��PID table */
        for(i = 0; i < (VOS_CPU_ID_1_PID_BUTT - VOS_PID_CPU_ID_1_DOPRAEND); i++)
        {
            if(VOS_PID_AVAILABLE == VOS_CheckPidValidity(VOS_PID_CPU_ID_1_DOPRAEND + i))
            {
                pulPid[num++] = (VOS_PID_CPU_ID_1_DOPRAEND + i);
            }
        }

        /* ��ȡC��PID table */
        for(i = 0; i < (VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND); i++)
        {
            if(VOS_PID_AVAILABLE == VOS_CheckPidValidity(VOS_PID_CPU_ID_0_DOPRAEND + i))
            {
                pulPid[num++] = (VOS_PID_CPU_ID_0_DOPRAEND + i);
            }
        }
    }

    len = sizeof(DIAG_CMD_PID_TABLE_CNF_STRU) + (num * sizeof(VOS_UINT32));
    pstPidTable = (DIAG_CMD_PID_TABLE_CNF_STRU *)VOS_MemAlloc(DIAG_AGENT_PID, DYNAMIC_MEM_PT, len);
    if(VOS_NULL == pstPidTable)
    {
        VOS_MemFree(DIAG_AGENT_PID, pulPid);
        return ERR_MSP_FAILURE;
    }

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

    DIAG_MSG_COMMON_PROC(stDiagInfo, (*pstPidTable), pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_MSP;

    pstPidTable->ulRc       = ulRc;
    pstPidTable->ulPidNum   = num;

    VOS_MemCpy_s(pstPidTable->aulPid, (VOS_UINT32)(num*sizeof(VOS_UINT32)), pulPid, (VOS_UINT32)(num*sizeof(VOS_UINT32)));

    ret = DIAG_MsgReport(&stDiagInfo, pstPidTable, len);

    VOS_MemFree(DIAG_AGENT_PID, pulPid);
    VOS_MemFree(DIAG_AGENT_PID, pstPidTable);

    return (VOS_UINT32)ret;
}

#endif

#if ((VOS_OS_VER == VOS_RTOSCK) || (VOS_OS_VER == VOS_VXWORKS))
/*****************************************************************************
 Function Name   : diag_AppTransMspProc
 Description     : A�˷���������������
 Input           : pMsgBlock    ����������������
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.c64416      2014-11-18  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_AppTransMspProc(MsgBlock* pMsgBlock)
{
    DIAG_MSG_A_TRANS_C_STRU *pstInfo;

    pstInfo = (DIAG_MSG_A_TRANS_C_STRU *)pMsgBlock;

    return diag_MspMsgProc(&pstInfo->stInfo);
}

DIAG_GTR_DATA_RCV_PFN g_pfnDiagGtrCallBack = VOS_NULL;

/*****************************************************************************
 Function Name   : DIAG_GtrRcvCallBackReg
 Description     : GTR�ص�ע��
 Input           :DIAG_GTR_DATA_RCV_PFN pfnGtrRcv
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.w00182550      2012-12-26  Draft Enact

*****************************************************************************/
VOS_VOID DIAG_GtrRcvCallBackReg(DIAG_GTR_DATA_RCV_PFN pfnGtrRcv)
{
    g_pfnDiagGtrCallBack = pfnGtrRcv;
}

#endif


/*****************************************************************************
 Function Name   : diag_GuGtrProcEntry
 Description     : GU��RTT���������ֻ��Ҫ͸��������Ҫ�ظ�
                    1. ԭֵ͸�������ı�senderpid
                    2. ������GUЭ��ջ��������Ҫ��C��ȥ������Ϣ������Ϣ
                       (senderpid������A�˵�PID������������Ϣ��ʧ��)
                    3. ����Ҫ�ظ���GTRͨ�������Ϣ�Ĺ��������жϳɹ�ʧ��
                    4. GU��RTT���Բ��������汾��RTT�汾(�汾��һ)

 History         :
    1.c00326366      2015-9-6  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_GuGtrProcEntry(VOS_UINT8* pstReq)
{
    VOS_UINT32 ulRet                    = ERR_MSP_SUCCESS;
    DIAG_FRAME_INFO_STRU *pstDiagHead   = NULL;

#if(VOS_OS_VER == VOS_LINUX)
    VOS_UINT32 ulLen;
    DIAG_MSG_A_TRANS_C_STRU *pstInfo;
#else
    DIAG_OSA_MSG_STRU *pstMsg           = NULL;
    DIAG_OSA_MSG_STRU *pstVosMsg        = NULL;
#endif

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

#if(VOS_OS_VER == VOS_LINUX)
    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ulRet);
    return ulRet;
#else

    /* coverity[Event self_assign] */
    pstMsg = pstMsg;
    /* coverity[self_assign] */
    pstVosMsg = pstVosMsg;


    pstMsg = (DIAG_OSA_MSG_STRU *)(pstDiagHead->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));

    pstVosMsg = (DIAG_OSA_MSG_STRU *)VOS_AllocMsg(pstMsg->ulSenderPid, pstMsg->ulLength);

    if (pstVosMsg != NULL)
    {
        pstVosMsg->ulReceiverPid  = pstMsg->ulReceiverPid;

        (VOS_VOID)VOS_MemCpy_s(&pstVosMsg->ulMsgId, pstMsg->ulLength, &pstMsg->ulMsgId, pstMsg->ulLength);

        ulRet = VOS_SendMsg(pstMsg->ulSenderPid, pstVosMsg);
        if (ulRet != VOS_OK)
        {
            diag_printf("diag_GuGtrProcEntry VOS_SendMsg failed!\n");
        }
    }

    return ulRet;
#endif

#if(VOS_OS_VER == VOS_LINUX)
DIAG_ERROR:
    /* ����Ҫ�ظ�ʧ�� */
    return ERR_MSP_FAILURE;
#endif
}


/*****************************************************************************
 Function Name   : diag_GtrProcEntry
 Description     : GTR�����ӿ����
                    �˹���ֻ�ڻ�ƬǰRTT����ʱʹ�ã��ᵥ������RTT�����汾
                    ��PS�Ĵ�׮�ӿ�ע�ᵽMSP��MSP�ڴӹ��߲���յ�GTR����ʱ�ص�PS
                    �Ĵ�׮�ӿڣ�PSͨ���������Ϣת����DSP
                    PS���յ�DSP�Ļظ��󣬵���(DIAG_TransReport)�ӿڰ�Ӧ������
                    �������߲�
                    GTR�������-->HIDS-->MSP-->PS-->DSP-->PS-->MSP-->HIDS-->GTR

 History         :
    1.w00182550      2012-12-26  Draft Enact
    2.c64416         2014-11-18  �����µ���ϼܹ�

*****************************************************************************/
VOS_UINT32 diag_GtrProcEntry(VOS_UINT8* pstReq)
{
    DIAG_CMD_GTR_SET_CNF_STRU stGtrCnf = {0};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};
    DIAG_FRAME_INFO_STRU *pstDiagHead = NULL;

#if(VOS_OS_VER == VOS_LINUX)
    VOS_UINT32 ulLen;
    DIAG_MSG_A_TRANS_C_STRU *pstInfo;
#else
    DIAG_CMD_GTR_SET_REQ_STRU* pstGtrReq = NULL;
#endif

    pstDiagHead = (DIAG_FRAME_INFO_STRU*)(pstReq);

#if(VOS_OS_VER == VOS_LINUX)
    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
    return ret;
    
DIAG_ERROR:
#else

    pstGtrReq = (DIAG_CMD_GTR_SET_REQ_STRU*)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    if (pstGtrReq->ulGtrDtaSize <= DIAG_CMD_DATA_MAX_LEN)
    {
        if (g_pfnDiagGtrCallBack != NULL)
        {
            ret = g_pfnDiagGtrCallBack(pstGtrReq->ulGtrDtaSize, pstGtrReq->aucDta);
            
            DIAG_MSG_COMMON_PROC(stDiagInfo, stGtrCnf, pstDiagHead);
            
            stGtrCnf.ulRc = ret;
            
            return DIAG_MsgReport(&stDiagInfo, &stGtrCnf, sizeof(stGtrCnf));
        }
    }

#endif

    DIAG_MSG_COMMON_PROC(stDiagInfo, stGtrCnf, pstDiagHead);

    stDiagInfo.ulMsgType    = DIAG_MSG_TYPE_MSP;

    stGtrCnf.ulRc = ERR_HIDS_CORE_ERROR;

    ret = DIAG_MsgReport(&stDiagInfo, &stGtrCnf, sizeof(stGtrCnf));

    return ret;
}


/*****************************************************************************
 Function Name   : diag_MspMsgProc
 Description     : MSP����������
 Input           : pData    ����������������
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.c64416      2014-11-18  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_MspMsgProc(DIAG_FRAME_INFO_STRU *pData)
{
    VOS_UINT32 ret = ERR_MSP_FAILURE;
    VOS_UINT32 i;

    if(NULL == pData)
    {
        return ret;
    }

#if(VOS_OS_VER == VOS_LINUX)
    diag_PTR(EN_DIAG_PTR_MSGMSP_IN);
#endif

    if(DIAG_MSG_TYPE_MSP != pData->stID.pri4b)
    {
        return ret;
    }

    for(i = 0; i < sizeof(g_astMsgMsp)/sizeof(DIAG_MSGMSP_PROC_STRU); i++)
    {
        if(pData->ulCmdId == g_astMsgMsp[i].ulCmdId)
        {
            return g_astMsgMsp[i].pfnProc((VOS_UINT8*)pData);
        }
    }

    (VOS_VOID)diag_FailedCmdCnf(pData, ERR_MSP_DIAG_INVALID_CMD);

    return ERR_MSP_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


