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

/************************************************************************
  Copyright    : 2005-2009, Huawei Tech. Co., Ltd.
  File name    : PBApi.c
  Author       : z00100318
  Version      : V200R001
  Date         : 2008-10-28
  Description  : ��C�ļ�������---�ӿ�ģ��ʵ��
  Function List:
  History      :
 ************************************************************************/
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif


#include "TafTypeDef.h"
#include "si_pb.h"
#include "product_config.h"

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define      THIS_FILE_ID     PS_FILE_ID_PBAPI_C

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (defined(DMT)))

/*****************************************************************************
�� �� ��  : SI_PB_GetReceiverPid
��������  : FDN����
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��18��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
VOS_UINT32 SI_PB_GetReceiverPid(MN_CLIENT_ID_T  ClientId, VOS_UINT32 *pulReceiverPid)
{
#if ( FEATURE_MULTI_MODEM == FEATURE_ON )
    MODEM_ID_ENUM_UINT16    enModemID;

    /* ���ýӿڻ�ȡModem ID */
    if(VOS_OK != AT_GetModemIdFromClient(ClientId,&enModemID))
    {
        return VOS_ERR;
    }

    if(MODEM_ID_1 == enModemID)
    {
        *pulReceiverPid = I1_MAPS_PB_PID;
    }
#if (MULTI_MODEM_NUMBER == 3)
    else if (MODEM_ID_2 == enModemID)
    {
        *pulReceiverPid = I2_MAPS_PB_PID;
    }
#endif /* MULTI_MODEM_NUMBER == 3 */
    else
    {
        *pulReceiverPid = I0_MAPS_PB_PID;
    }
#else
    *pulReceiverPid = MAPS_PB_PID;
#endif

    return VOS_OK;
}

/*****************************************************************************
�� �� ��  : SI_PB_Read
��������  : ��ȡ�绰����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��14��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Read(  MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE       Storage,
                            SI_UINT16                Index1,
                            SI_UINT16                Index2)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return TAF_FAILURE;
#else
    SI_PB_READ_REQ_STRU     *pMsg;

    pMsg = (SI_PB_READ_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_READ_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    pMsg->ulReceiverPid = MAPS_PB_PID;
#else
    pMsg->ulReceiverPid = ACPU_PID_PB;
#endif  /*(VOS_WIN32 == VOS_OS_VER)*/

    pMsg->ulMsgName     = SI_PB_READ_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex1      = Index1;
    pMsg->usIndex2      = Index2;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
#endif
}

/*****************************************************************************
�� �� ��  : SI_PB_SRead
��������  : ��ȡ���ϵ绰����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2009��06��05��
  ��    ��  : m00128685
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_SRead(  MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE       Storage,
                            SI_UINT16                Index1,
                            SI_UINT16                Index2)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return TAF_FAILURE;
#else
    SI_PB_READ_REQ_STRU     *pMsg;

    pMsg = (SI_PB_READ_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_READ_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    pMsg->ulReceiverPid = MAPS_PB_PID;
#else
    pMsg->ulReceiverPid = ACPU_PID_PB;
#endif  /*(VOS_WIN32 == VOS_OS_VER)*/

    pMsg->ulMsgName     = SI_PB_SREAD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex1      = Index1;
    pMsg->usIndex2      = Index2;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Read:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
#endif
}

/*****************************************************************************
�� �� ��  : SI_PB_Query
��������  : �绰�������ѯ
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��14��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Query(     MN_CLIENT_ID_T           ClientId,
                                MN_OPERATION_ID_T        OpId)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return TAF_FAILURE;
#else
    SI_PB_QUERY_REQ_STRU    *pMsg;

    pMsg = (SI_PB_QUERY_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_QUERY_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Query:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = MAPS_PB_PID;
    pMsg->ulMsgName     = SI_PB_QUERY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ulStorage     = gstPBCtrlInfo.enPBCurType;

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Query:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
#endif
}

/*****************************************************************************
�� �� ��  : SI_PB_Set
��������  : ���õ�ǰ���뱾ʹ�õĴ洢������
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��14��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Set(    MN_CLIENT_ID_T           ClientId,
                            MN_OPERATION_ID_T        OpId,
                            SI_PB_STORATE_TYPE      Storage)
{
    SI_PB_SET_REQ_STRU  *pMsg;
    VOS_UINT32          ulReceiverPid;

#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    if(SI_PB_STORAGE_FD != Storage)
    {
        PB_ERROR_LOG("SI_PB_Set:Double Modem only support the FDN");

        return TAF_FAILURE;
    }
#endif

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Set:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_SET_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Set:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_SET_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ulStorage     = Storage;

    if(VOS_OK != VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Set:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*****************************************************************************
�� �� ��  : SI_PB_Add
��������  : �绰����׷��һ����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��14��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Add(    MN_CLIENT_ID_T          ClientId,
                            MN_OPERATION_ID_T        OpId,
                             SI_PB_STORATE_TYPE      Storage,
                             SI_PB_RECORD_STRU       *pRecord)
{
    SI_PB_ADD_REP_STRU  *pMsg;
    VOS_UINT32          ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Add:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Add:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    /*lint -e433  �޸���: j00174725; ������: xucheng */
    pMsg = (SI_PB_ADD_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_ADD_REP_STRU) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433  �޸���: j00174725; ������: xucheng */

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_ADD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

#if( FEATURE_MULTI_MODEM == FEATURE_ON )
    pMsg->ulStorage = SI_PB_STORAGE_FD;     /*ֻ�ܹ�����FDN����*/
#else
    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }
#endif

    pRecord->Index = 1;

    PAM_MEM_CPY_S(&pMsg->stRecord, sizeof(SI_PB_RECORD_STRU), pRecord, sizeof(SI_PB_RECORD_STRU));

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}
/*****************************************************************************
�� �� ��  : SI_PB_SAdd
��������  : �绰����׷��һ����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2009��06��05��
  ��    ��  : m00128685
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_SAdd(    MN_CLIENT_ID_T          ClientId,
                            MN_OPERATION_ID_T        OpId,
                             SI_PB_STORATE_TYPE      Storage,
                             SI_PB_RECORD_STRU       *pRecord)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return TAF_FAILURE;
#else
    SI_PB_ADD_REP_STRU  *pMsg;

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Add:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    /*lint -e433  �޸���: j00174725; ������: xucheng */
    pMsg = (SI_PB_ADD_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_ADD_REP_STRU) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433  �޸���: j00174725; ������: xucheng */

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = MAPS_PB_PID;
    pMsg->ulMsgName     = SI_PB_SADD_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    pRecord->Index = 1;

    PAM_MEM_CPY_S(&pMsg->stRecord, sizeof(SI_PB_RECORD_STRU), pRecord, sizeof(SI_PB_RECORD_STRU));

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Add:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
#endif
}

/*****************************************************************************
�� �� ��  : SI_PB_Modify
��������  : �绰����׷��һ����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��14��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Modify(    MN_CLIENT_ID_T          ClientId,
                                MN_OPERATION_ID_T       OpId,
                                SI_PB_STORATE_TYPE      Storage,
                                SI_PB_RECORD_STRU       *pRecord )
{
    SI_PB_MODIFY_REP_STRU  *pMsg;
    VOS_UINT32              ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Modify:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Modify:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    /*lint -e433  �޸���: j00174725; ������: xucheng */
    pMsg = (SI_PB_MODIFY_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_MODIFY_REP_STRU) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433  �޸���: j00174725; ������: xucheng */

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_MODIFY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

#if( FEATURE_MULTI_MODEM == FEATURE_ON )
    pMsg->ulStorage = SI_PB_STORAGE_FD;     /*ֻ�ܹ�����FDN����*/
#else
    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }
#endif

    PAM_MEM_CPY_S(&pMsg->Record, sizeof(SI_PB_RECORD_STRU), pRecord, sizeof(SI_PB_RECORD_STRU));

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*****************************************************************************
�� �� ��  : SI_PB_SModify
��������  : �绰����׷��һ����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2009��06��05��
  ��    ��  : m00128685
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_SModify(    MN_CLIENT_ID_T          ClientId,
                                MN_OPERATION_ID_T       OpId,
                                SI_PB_STORATE_TYPE      Storage,
                                SI_PB_RECORD_STRU       *pRecord )
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return TAF_FAILURE;
#else
    SI_PB_MODIFY_REP_STRU  *pMsg;

    if(VOS_NULL_PTR == pRecord)
    {
        PB_ERROR_LOG("SI_PB_Modify:pRecord is a NULL pointer");

        return TAF_FAILURE;
    }

    /*lint -e433  �޸���: j00174725; ������: xucheng */
    pMsg = (SI_PB_MODIFY_REP_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_MODIFY_REP_STRU) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433  �޸���: j00174725; ������: xucheng */

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = MAPS_PB_PID;
    pMsg->ulMsgName     = SI_PB_SMODIFY_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;

    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }

    PAM_MEM_CPY_S(&pMsg->Record, sizeof(SI_PB_RECORD_STRU), pRecord, sizeof(SI_PB_RECORD_STRU));

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Modify:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
#endif
}

/*****************************************************************************
�� �� ��  : SI_PB_Delete
��������  : �绰����ɾ��һ����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2008��10��14��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Delete(     MN_CLIENT_ID_T             ClientId,
                                MN_OPERATION_ID_T           OpId,
                                SI_PB_STORATE_TYPE          Storage,
                                SI_UINT16                   Index)
{
    SI_PB_DELETE_REQ_STRU  *pMsg;
    VOS_UINT32              ulReceiverPid;

    if (VOS_OK != SI_PB_GetReceiverPid(ClientId, &ulReceiverPid))
    {
        PB_ERROR_LOG("SI_PB_Modify:Get ulReceiverPid Error.");

        return TAF_FAILURE;
    }

    pMsg = (SI_PB_DELETE_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_DELETE_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Delete:VOS_AllocMsg Failed");

        return TAF_FAILURE;
    }

    pMsg->ulReceiverPid = ulReceiverPid;
    pMsg->ulMsgName     = SI_PB_DELETE_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->usIndex       = Index;

#if( FEATURE_MULTI_MODEM == FEATURE_ON )
    pMsg->ulStorage = SI_PB_STORAGE_FD;     /*ֻ�ܹ�����FDN����*/
#else
    if(SI_PB_STORAGE_UNSPECIFIED == Storage)
    {
        pMsg->ulStorage = gstPBCtrlInfo.enPBCurType;
    }
    else
    {
        pMsg->ulStorage = Storage;
    }
#endif

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Delete:VOS_SendMsg Failed");

        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
}

/*****************************************************************************
�� �� ��  : SI_PB_Search
��������  : �绰���в���һ����¼
�������  : ��
�������  : ��
�� �� ֵ  : SI_UINT32 ����ִ�н��
���ú���  : ��
��������  : �ⲿ�ӿ�
History     :
1.��    ��  : 2009��3��12��
  ��    ��  : H59254
  �޸�����  : Create
*****************************************************************************/
SI_UINT32 SI_PB_Search(    MN_CLIENT_ID_T             ClientId,
                                MN_OPERATION_ID_T           OpId,
                                SI_PB_STORATE_TYPE          Storage,
                                SI_UINT8                    ucLength,
                                SI_UINT8                    *pucContent)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return TAF_FAILURE;
#else
    SI_PB_SEARCH_REQ_STRU  *pMsg;

    /*lint -e433  �޸���: j00174725; ������: xucheng */
    pMsg = (SI_PB_SEARCH_REQ_STRU *)VOS_AllocMsg(WUEPS_PID_AT, sizeof(SI_PB_SEARCH_REQ_STRU) - VOS_MSG_HEAD_LENGTH);
    /*lint +e433  �޸���: j00174725; ������: xucheng */

    if (VOS_NULL_PTR == pMsg)
    {
        PB_ERROR_LOG("SI_PB_Search:VOS_AllocMsg Failed");
        return TAF_FAILURE;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    pMsg->ulReceiverPid = MAPS_PB_PID;
#else
    pMsg->ulReceiverPid = ACPU_PID_PB;
#endif  /*(VOS_WIN32 == VOS_OS_VER)*/

    pMsg->ulMsgName     = SI_PB_SEARCH_REQ;
    pMsg->usClient      = ClientId;
    pMsg->ucOpID        = OpId;
    pMsg->ucLength      = ucLength;

    pMsg->ulStorage = ((Storage == SI_PB_STORAGE_UNSPECIFIED)?(gstPBCtrlInfo.enPBCurType):(Storage));

    PAM_MEM_CPY_S(pMsg->aucContent, sizeof(pMsg->aucContent), pucContent, ucLength);

    if(VOS_OK !=  VOS_SendMsg(WUEPS_PID_AT, pMsg))
    {
        PB_ERROR_LOG("SI_PB_Search:VOS_SendMsg Failed");
        return TAF_FAILURE;
    }

    return TAF_SUCCESS;
#endif
}

/*****************************************************************************
�� �� ��  :SI_PB_GetStorateType
��������  :��ȡ�绰����ǰ�洢����
�������  :��
�������  :��
�� �� ֵ  :�绰����ǰ�洢����

�޶���¼  :
1. ��    ��   : 2011��05��17��
   ��    ��   : j00168360
   �޸�����   : Creat [DTS2011042105653]��at+cpbf��ϲ��ҹ���
*****************************************************************************/
VOS_UINT32 SI_PB_GetStorateType(VOS_VOID)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return SI_PB_STORAGE_UNSPECIFIED;   /*���ص�ǰδָ��*/
#else
    return (VOS_UINT32)gstPBCtrlInfo.enPBCurType;
#endif
}

/*****************************************************************************
�� �� ��  :SI_PB_GetSPBFlag
��������  :��ȡ���ϵ绰����־���ɴ˿��жϳ���ǰ�Ƿ�֧�ָ��ϵ绰��
�������  :��
�������  :��
�� �� ֵ  :���ϵ绰��flag

�޶���¼  :
1. ��    ��   : 2011��05��17��
   ��    ��   : j00168360
   �޸�����   : Creat [DTS2011042105653]��at+cpbf��ϲ��ҹ���
*****************************************************************************/
VOS_UINT32 SI_PB_GetSPBFlag(VOS_VOID)
{
#if (( FEATURE_MULTI_MODEM == FEATURE_ON )&&(!defined(DMT)))
    return VOS_FALSE;   /*����״̬�ر�*/
#else
    return (VOS_UINT32)gstPBConfigInfo.ucSPBFlag;
#endif
}


#endif

#if ((OSA_CPU_CCPU == VOS_OSA_CPU)||(defined(DMT)))

/*****************************************************************************
�� �� ��      :
��������  :
�������  :
�������  :
�� �� ֵ      :
���ú���  :
��������  :
�޶���¼  :
1. ��    ��   : 2007��10��15��
    ��    ��   : z00100318
    �޸�����   : Creat

*****************************************************************************/
VOS_UINT32 SI_PB_GetEccNumber(SI_PB_ECC_DATA_STRU *pstEccData)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucPBOffset=0;
    VOS_UINT8                          *ptemp;
    VOS_UINT32                          ulNum;
    VOS_UINT8                           i,j;

    ulResult = SI_PB_LocateRecord(PB_ECC, 1, 1, &ucPBOffset);

    if(VOS_OK != ulResult)     /*��ǰ�绰�������ڻ��߳�ʼ��δ���*/
    {
        PB_ERROR_LOG("SI_PB_GetEccNumber Error: SI_PB_LocateRecord Return Failed");

        pstEccData->bEccExists = SI_PB_CONTENT_INVALID;
        pstEccData->ulReocrdNum= VOS_NULL;

        return VOS_ERR;
    }

    pstEccData->bEccExists = SI_PB_CONTENT_VALID;

    ulNum = ((gastPBContent[ucPBOffset].usTotalNum>USIM_MAX_ECC_RECORDS)?USIM_MAX_ECC_RECORDS:gastPBContent[ucPBOffset].usTotalNum);

    ptemp = gastPBContent[ucPBOffset].pContent;

    for(i=0,j=0; i<ulNum; i++)   /*�������ݽṹ��󳤶�ѭ��*/
    {
        ulResult = SI_PB_CheckEccValidity(ptemp);

        if(VOS_ERR == ulResult)     /*��ǰ��¼������Ч*/
        {
            PB_INFO_LOG("SI_PB_GetEccNumber Info: The Ecc Number is Empty");
        }
        else                                /*ת����ǰ��¼����*/
        {
            PB_INFO_LOG("SI_PB_GetEccNumber Info: The Ecc Number is Not Empty");

            PAM_MEM_CPY_S(pstEccData->astEccRecord[j].aucEccCode, gastPBContent[ucPBOffset].ucNumberLen,
                                    ptemp, gastPBContent[ucPBOffset].ucNumberLen);

            if (USIMM_CARD_USIM == gstPBInitState.enCardType)
            {
                pstEccData->astEccRecord[j].bESC = SI_PB_CONTENT_VALID;

                PAM_MEM_CPY_S(pstEccData->astEccRecord[j].AlphaIdentifier,
                            gastPBContent[ucPBOffset].ucNameLen,
                           &ptemp[gastPBContent[ucPBOffset].ucNumberLen],
                           gastPBContent[ucPBOffset].ucNameLen);

                pstEccData->astEccRecord[j].ucESC = ptemp[gastPBContent[ucPBOffset].ucRecordLen - 1];
            }

            j++;
        }

        ptemp += gastPBContent[ucPBOffset].ucRecordLen;
    }

    pstEccData->ulReocrdNum = j;

    return VOS_OK;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/*****************************************************************************
�� �� ��  : SI_PB_GetXeccNumber
��������  : ��ȡCDMA ECC����
�������  : ��
�������  : pstEccData:ECC��������
�� �� ֵ  : VOS_OK/VOS_ERR
�޶���¼  :
1. ��    ��   : 2015��06��15��
   ��    ��   : h00300778
   �޸�����   : Creat
*****************************************************************************/
VOS_UINT32 SI_PB_GetXeccNumber(SI_PB_ECC_DATA_STRU *pstEccData)
{
    VOS_UINT32                          ulResult;
    VOS_UINT32                          ulNum;
    VOS_UINT8                          *pucTemp;
    VOS_UINT8                           ucPBOffset=0;
    VOS_UINT32                          i;
    VOS_UINT32                          j;

    /* ���������� */
    if (VOS_NULL_PTR == pstEccData)
    {
        PB_ERROR_LOG("SI_PB_GetXeccNumber Error: Para is incorrect.");

        return VOS_ERR;
    }

    PAM_MEM_SET_S(pstEccData, sizeof(SI_PB_ECC_DATA_STRU), 0, sizeof(SI_PB_ECC_DATA_STRU));

    ulResult = SI_PB_LocateRecord(PB_XECC, 1, 1, &ucPBOffset);

    /* ��ǰ�绰�������ڻ��߳�ʼ��δ��� */
    if (VOS_OK != ulResult)
    {
        PB_ERROR_LOG("SI_PB_GetXeccNumber Error: SI_PB_LocateRecord Return Failed");

        pstEccData->bEccExists = SI_PB_CONTENT_INVALID;
        pstEccData->ulReocrdNum= VOS_NULL;

        return VOS_ERR;
    }

    pstEccData->bEccExists = SI_PB_CONTENT_VALID;

    ulNum = ((gastPBContent[ucPBOffset].usTotalNum>USIM_MAX_ECC_RECORDS)?USIM_MAX_ECC_RECORDS:gastPBContent[ucPBOffset].usTotalNum);

    pucTemp = gastPBContent[ucPBOffset].pContent;

    for (i = 0, j = 0; i < ulNum; i++)   /* �������ݽṹ��󳤶�ѭ�� */
    {
        ulResult = SI_PB_CheckEccValidity(pucTemp);

        if (VOS_ERR == ulResult)     /* ��ǰ��¼������Ч */
        {
            PB_INFO_LOG("SI_PB_GetXeccNumber Info: The Ecc Number is Empty");
        }
        else                                /* ת����ǰ��¼���� */
        {
            PB_INFO_LOG("SI_PB_GetXeccNumber Info: The Ecc Number is Not Empty");

            PAM_MEM_CPY_S(pstEccData->astEccRecord[j].aucEccCode, gastPBContent[ucPBOffset].ucNumberLen, pucTemp, gastPBContent[ucPBOffset].ucNumberLen);

            j++;
        }

        pucTemp += gastPBContent[ucPBOffset].ucRecordLen;
    }

    pstEccData->ulReocrdNum = j;

    return VOS_OK;
}
#endif

#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif



