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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "AtCmdMiscProc.h"
#include "AtSndMsg.h"
#include "ATCmdProc.h"
#include "dms_core.h"
#include "AtDataProc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_MISC_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/

/*****************************************************************************
 函 数 名  : AT_SetActiveModem
 功能描述  : 命令^ACTIVEMODEM设置处理函数
             命令格式:AT^ACTIVEMODEM=<enable>
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年09月21日
    作    者   : l00198894
    修改内容   : DSDS单/双卡模式

*****************************************************************************/
VOS_UINT32 AT_SetActiveModem(VOS_UINT8 ucIndex)
{
    TAF_NV_DSDS_ACTIVE_MODEM_MODE_STRU  stMode;
    AT_MTA_MODEM_CAP_UPDATE_REQ_STRU    stAtMtaModemCapUpdate;

    TAF_MEM_SET_S(&stAtMtaModemCapUpdate, (VOS_UINT32)sizeof(AT_MTA_MODEM_CAP_UPDATE_REQ_STRU), 0x00, (VOS_UINT32)sizeof(AT_MTA_MODEM_CAP_UPDATE_REQ_STRU));

    /* 参数个数检查 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    TAF_MEM_SET_S(&stMode, sizeof(stMode), 0x00, sizeof(stMode));
    stMode.enActiveModem = (TAF_NV_ACTIVE_MODEM_MODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 写NV, 返回AT_OK */
    if (NV_OK != NV_WriteEx(MODEM_ID_0, en_NV_Item_DSDS_Active_Modem_Mode, &stMode, sizeof(stMode)))
    {
        AT_ERR_LOG("AT_SetActiveModem(): en_NV_Item_DSDS_Active_Modem_Mode NV Write Fail!");
        return AT_ERROR;
    }

    /* 更新了NV，通知AT->MTA->RRM，进行底层平台能力更新 */
    /* AT发送至MTA的消息结构赋值 */
    stAtMtaModemCapUpdate.enModemCapUpdateType = AT_MTA_MODEM_CAP_UPDATE_TYPE_ACTIVE_MODEM;

    /* 发送消息给C核处理 */
    if (AT_SUCCESS != AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                             0,
                                             ID_AT_MTA_MODEM_CAP_UPDATE_REQ,
                                             &stAtMtaModemCapUpdate,
                                             (VOS_UINT32)sizeof(AT_MTA_MODEM_CAP_UPDATE_REQ_STRU),
                                             I0_UEPS_PID_MTA))
    {
        AT_WARN_LOG("AT_SetActiveModem(): Snd MTA Req Failed!");

        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MODEM_CAP_UPDATE_SET;

    return AT_WAIT_ASYNC_RETURN;
}


#if(FEATURE_ON == FEATURE_LTE)
#if(FEATURE_ON == FEATURE_LTE_MBMS)
/*****************************************************************************
 函 数 名  : AT_SetMBMSServiceOptPara
 功能描述  : ^MBMSCMD="MBMS_SERVICE_ENABLER",<param1>
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetMBMSServiceOptPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_SERVICE_OPTION_SET_REQ_STRU         stMBMSServiceOption;
    VOS_UINT32                                      ulRst;

    TAF_MEM_SET_S(&stMBMSServiceOption, sizeof(stMBMSServiceOption), 0x00, sizeof(AT_MTA_MBMS_SERVICE_OPTION_SET_REQ_STRU));

    /* 参数为空 */
    if(1 != gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = atAuc2ul(gastAtParaList[1].aucPara,
                     (VOS_UINT16)gastAtParaList[1].usParaLen,
                     &gastAtParaList[1].ulParaValue);

    if(AT_SUCCESS != ulRst)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[1].ulParaValue)
    {
        case 0:
            stMBMSServiceOption.enCfg = AT_MTA_CFG_DISABLE;
            break;

        case 1:
            stMBMSServiceOption.enCfg = AT_MTA_CFG_ENABLE;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息到C核, 设置使能或者去使能MBMS服务特性 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_SERVICE_OPTION_SET_REQ,
                                   &stMBMSServiceOption,
                                   sizeof(stMBMSServiceOption),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetMBMSServicePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_SERVICE_OPTION_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_SetMBMSServiceStatePara
 功能描述  : ^MBMSCMD="ACTIVATE",<param1>,<param2>
             ^MBMSCMD="DEACTIVATE",<param1>,<param2>
             ^MBMSCMD="DEACTIVATE_ALL"
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetMBMSServiceStatePara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_SERVICE_STATE_SET_REQ_STRU          stMBMSServiceState;
    VOS_UINT32                                      ulRst;

    TAF_MEM_SET_S(&stMBMSServiceState, sizeof(stMBMSServiceState), 0x00, sizeof(AT_MTA_MBMS_SERVICE_STATE_SET_REQ_STRU));

    switch(gastAtParaList[0].ulParaValue)
    {
        case AT_MBMS_ACTIVATE_TYPE:
            stMBMSServiceState.enStateSet   = AT_MTA_MBMS_SERVICE_STATE_SET_ACTIVE;
            break;

        case AT_MBMS_DEACTIVATE_TYPE:
            stMBMSServiceState.enStateSet   = AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE;
            break;

        case AT_MBMS_DEACTIVATE_ALL_TYPE:
            stMBMSServiceState.enStateSet   = AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE_ALL;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    if(AT_MTA_MBMS_SERVICE_STATE_SET_DEACTIVE_ALL != stMBMSServiceState.enStateSet)
    {
        /* 参数为空 */
        if(0 == gastAtParaList[1].usParaLen || 0 == gastAtParaList[2].usParaLen)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        ulRst = atAuc2ul(gastAtParaList[1].aucPara,
                         (VOS_UINT16)gastAtParaList[1].usParaLen,
                         &gastAtParaList[1].ulParaValue);

        if(AT_SUCCESS != ulRst)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 参数长度不正确 */
        if(((AT_MBMS_TMGI_MAX_LENGTH - 1 ) != gastAtParaList[2].usParaLen)
        && (AT_MBMS_TMGI_MAX_LENGTH != gastAtParaList[2].usParaLen))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <AreaID> */
        stMBMSServiceState.ulAreaId                 = gastAtParaList[1].ulParaValue;

        /* <TMGI>:MBMS Service ID */
        if(AT_FAILURE == At_Auc2ul(gastAtParaList[2].aucPara, AT_MBMS_SERVICE_ID_LENGTH, &stMBMSServiceState.stTMGI.ulMbmsSerId))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <TMGI>:Mcc */
        if(AT_FAILURE == At_String2Hex(&gastAtParaList[2].aucPara[AT_MBMS_SERVICE_ID_LENGTH], AT_MBMS_MCC_LENGTH, &stMBMSServiceState.stTMGI.stPlmnId.ulMcc))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <TMGI>:Mnc */
        if(AT_FAILURE == At_String2Hex(&gastAtParaList[2].aucPara[AT_MBMS_SERVICE_ID_AND_MCC_LENGTH], gastAtParaList[2].usParaLen - AT_MBMS_SERVICE_ID_AND_MCC_LENGTH, &stMBMSServiceState.stTMGI.stPlmnId.ulMnc))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ( (AT_MBMS_TMGI_MAX_LENGTH - 1 ) == gastAtParaList[2].usParaLen)
        {
            stMBMSServiceState.stTMGI.stPlmnId.ulMnc |= 0x0F00;
        }
    }

    /* 发送跨核消息到C核, 设置使能或者去使能MBMS服务特性 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_SERVICE_STATE_SET_REQ,
                                   &stMBMSServiceState,
                                   sizeof(stMBMSServiceState),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetMBMSServiceStatePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_SERVICE_STATE_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_SetMBMSPreferencePara
 功能描述  : ^MBMSCMD="MBMS_PREFERENCE",<param1>
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetMBMSPreferencePara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_PREFERENCE_SET_REQ_STRU             stMBMSCastMode;
    VOS_UINT32                                      ulRst;

    TAF_MEM_SET_S(&stMBMSCastMode, sizeof(stMBMSCastMode), 0x00, sizeof(AT_MTA_MBMS_PREFERENCE_SET_REQ_STRU));

    /* 参数为空 */
    if(1 != gastAtParaList[1].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ulRst = atAuc2ul(gastAtParaList[1].aucPara, (VOS_UINT16)gastAtParaList[1].usParaLen, &gastAtParaList[1].ulParaValue);

    if(AT_SUCCESS != ulRst)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[1].ulParaValue)
    {
        case 0:
            stMBMSCastMode.enCastMode   = AT_MTA_MBMS_CAST_MODE_UNICAST;
            break;

        case 1:
            stMBMSCastMode.enCastMode   = AT_MTA_MBMS_CAST_MODE_MULTICAST;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息到C核, 设置MBMS广播模式 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_PREFERENCE_SET_REQ,
                                   &stMBMSCastMode,
                                   sizeof(stMBMSCastMode),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetMBMSPreferencePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_PREFERENCE_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_SetMBMSCMDPara
 功能描述  : ^MBMSCMD
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetMBMSCMDPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 3)
    {
        return AT_TOO_MANY_PARA;
    }

    /* 参数为空 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<cmd>*/
    switch(gastAtParaList[0].ulParaValue)
    {
        /* <cmd> equal "MBMS_SERVICE_ENABLER" */
        case AT_MBMS_SERVICE_ENABLER_TYPE:
            ulRst = AT_SetMBMSServiceOptPara(ucIndex);
            break;
        /* <cmd> equal "ACTIVATE", "DEACTIVATE" or "DEACTIVATE_ALL" */
        case AT_MBMS_ACTIVATE_TYPE:
        case AT_MBMS_DEACTIVATE_TYPE:
        case AT_MBMS_DEACTIVATE_ALL_TYPE:
            ulRst = AT_SetMBMSServiceStatePara(ucIndex);
            break;
        /* <cmd> equal "MBMS_PREFERENCE" */
        case AT_MBMS_PREFERENCE_TYPE:
            ulRst = AT_SetMBMSPreferencePara(ucIndex);
            break;
        /* <cmd> equal "SIB16_GET_NETWORK_TIME" */
        case AT_MBMS_SIB16_GET_NETWORK_TIME_TYPE:
            ulRst = AT_QryMBMSSib16NetworkTimePara(ucIndex);
            break;
        /* <cmd> equal "BSSI_SIGNAL_LEVEL" */
        case AT_MBMS_BSSI_SIGNAL_LEVEL_TYPE:
            ulRst = AT_QryMBMSBssiSignalLevelPara(ucIndex);
            break;
        /* <cmd> equal "NETWORK_INFORMATION" */
        case AT_MBMS_NETWORK_INFORMATION_TYPE:
            ulRst = AT_QryMBMSNetworkInfoPara(ucIndex);
            break;
        /* <cmd> equal "MODEM_STATUS" */
        case AT_MBMS_MODEM_STATUS_TYPE:
            ulRst = AT_QryMBMSModemStatusPara(ucIndex);
            break;

        default:
            ulRst = AT_CME_INCORRECT_PARAMETERS;
            break;
    }

    return ulRst;
}

/*****************************************************************************
 函 数 名  : AT_SetMBMSEVPara
 功能描述  : ^MBMSEV
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetMBMSEVPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ_STRU        stMBMSUnsolicitedCfg;
    VOS_UINT32                                      ulRst;

    TAF_MEM_SET_S(&stMBMSUnsolicitedCfg, sizeof(stMBMSUnsolicitedCfg), 0x00, sizeof(AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[0].ulParaValue)
    {
        case 0:
            stMBMSUnsolicitedCfg.enCfg  = AT_MTA_CFG_DISABLE;
            break;

        case 1:
            stMBMSUnsolicitedCfg.enCfg  = AT_MTA_CFG_ENABLE;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息到C核, 设置MBMS主动上报配置 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_MBMS_UNSOLICITED_CFG_SET_REQ,
                                   &stMBMSUnsolicitedCfg,
                                   sizeof(stMBMSUnsolicitedCfg),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetMBMSEVPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_UNSOLICITED_CFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_SetMBMSInterestListPara
 功能描述  : ^MBMSINTERESTLIST
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetMBMSInterestListPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MBMS_INTERESTLIST_SET_REQ_STRU           stMBMSInterestList;
    VOS_UINT32                                      ulRst;
    VOS_UINT8                                       interestNum;

    TAF_MEM_SET_S(&stMBMSInterestList, sizeof(stMBMSInterestList), 0x00, sizeof(AT_MTA_MBMS_INTERESTLIST_SET_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (gucAtParaIndex != 6)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if(0 == gastAtParaList[5].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[5].ulParaValue)
    {
        case 0:
            stMBMSInterestList.enMbmsPriority    = AT_MTA_MBMS_PRIORITY_UNICAST;
            break;

        case 1:
            stMBMSInterestList.enMbmsPriority    = AT_MTA_MBMS_PRIORITY_MBMS;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 频点列表赋值 */
    for(interestNum = 0; interestNum < AT_MTA_INTEREST_FREQ_MAX_NUM; interestNum++)
    {
        if (0 != gastAtParaList[interestNum].usParaLen)
        {
            ulRst = atAuc2ul(gastAtParaList[interestNum].aucPara,
                             (VOS_UINT16)gastAtParaList[interestNum].usParaLen,
                             &gastAtParaList[interestNum].ulParaValue);

            if(AT_SUCCESS != ulRst)
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            stMBMSInterestList.aulFreqList[interestNum]   =   gastAtParaList[interestNum].ulParaValue;
        }
        else
        {
            /* 默认值为无效值0xFFFFFFFF */
            stMBMSInterestList.aulFreqList[interestNum]   =   0xFFFFFFFF;
        }
    }

    /* 发送跨核消息到C核, 设置MBMS主动上报配置 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_INTEREST_LIST_SET_REQ,
                                   &stMBMSInterestList,
                                   sizeof(stMBMSInterestList),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetMBMSInterestListPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_INTERESTLIST_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryMBMSSib16NetworkTimePara
 功能描述  : ^MBMSCMD="SIB16_GET_NETWORK_TIME"
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryMBMSSib16NetworkTimePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* 发送跨核消息到C核, 查询SIB16网络时间请求 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_SIB16_NETWORK_TIME_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_QryMBMSSib16NetworkTimePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryMBMSBssiSignalLevelPara
 功能描述  : ^MBMSCMD="BSSI_SIGNAL_LEVEL"
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryMBMSBssiSignalLevelPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* 发送跨核消息到C核, 查询BSSI信号强度请求 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_BSSI_SIGNAL_LEVEL_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_QryMBMSBssiSignalLevelPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryMBMSNetworkInfoPara
 功能描述  : ^MBMSCMD="NETWORK_INFORMATION"
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryMBMSNetworkInfoPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* 发送跨核消息到C核, 查询网络信息请求 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_NETWORK_INFO_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_QryMBMSNetworkInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_NETWORK_INFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryMBMSModemStatusPara
 功能描述  : ^MBMSCMD="MODEM_STATUS"
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryMBMSModemStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    /* 发送跨核消息到C核, 查询eMBMS功能状态请求 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_EMBMS_STATUS_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_QryMBMSModemStatusPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EMBMS_STATUS_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryMBMSCmdPara
 功能描述  : ^MBMSCMD查询命令处理函数
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年05月22日
    作    者   : w00316404
    修改内容   :新增查询函数
*****************************************************************************/
VOS_UINT32 AT_QryMBMSCmdPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* AT 给MTA 发送查询请求消息 */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MBMS_AVL_SERVICE_LIST_QRY_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_QryMBMSCmdPara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MBMS_AVL_SERVICE_LIST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : At_TestMBMSCMDPara
 功能描述  : ^MBMSCMD=?
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年05月22日
    作    者   : w00316404
    修改内容   :新增函数
*****************************************************************************/

VOS_UINT32 At_TestMBMSCMDPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength = 0;

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: (\"MBMS_SERVICE_ENABLER\",\"ACTIVATE\",\"DEACTIVATE\",\"DEACTIVATE_ALL\",\"MBMS_PREFERENCE\",\"SIB16_GET_NETWORK_TIME\",\"BSSI_SIGNAL_LEVEL\",\"NETWORK_INFORMATION\",\"MODEM_STATUS\")",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}


/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSServiceOptSetCnf
 功能描述  : 收到MTA设置MBMS服务特性的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSServiceOptSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceOptSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceOptSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_SERVICE_OPTION_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceOptSetCnf : Current Option is not AT_CMD_MBMS_SERVICE_OPTION_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSServiceStateSetCnf
 功能描述  : 收到MTA设置MBMS服务状态的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSServiceStateSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceStateSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceStateSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_SERVICE_STATE_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceStateSetCnf : Current Option is not AT_CMD_MBMS_SERVICE_STATE_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSPreferenceSetCnf
 功能描述  : 收到MTA设置MBMS广播模式的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSPreferenceSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSPreferenceSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSPreferenceSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_PREFERENCE_SET  != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSPreferenceSetCnf : Current Option is not AT_CMD_MBMS_PREFERENCE_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSSib16NetworkTimeQryCnf
 功能描述  : 收到MTA查询SIB16网络时间的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSSib16NetworkTimeQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_SIB16_NETWORK_TIME_QRY_CNF_STRU    *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT64                                     *pullUTC;
    VOS_UINT8                                       aucUTC[AT_MBMS_UTC_MAX_LENGTH + 1];
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_SIB16_NETWORK_TIME_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;
    TAF_MEM_SET_S(aucUTC, sizeof(aucUTC), 0x00, sizeof(aucUTC));

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSSib16NetworkTimeQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSSib16NetworkTimeQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSSib16NetworkTimeQryCnf : Current Option is not AT_CMD_MBMS_SIB16_NETWORK_TIME_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        pullUTC = (VOS_UINT64 *)pstMtaCnf->aulUTC;
        VOS_sprintf_s((VOS_CHAR *)aucUTC, AT_MBMS_UTC_MAX_LENGTH+1, "%llu", *pullUTC);
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %s",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    aucUTC);
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSBssiSignalLevelQryCnf
 功能描述  : 收到MTA查询BSSI信号强度的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSBssiSignalLevelQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_BSSI_SIGNAL_LEVEL_QRY_CNF_STRU     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_BSSI_SIGNAL_LEVEL_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSBssiSignalLevelQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSBssiSignalLevelQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSBssiSignalLevelQryCnf : Current Option is not AT_CMD_MBMS_BSSI_SIGNAL_LEVEL_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (VOS_INT32)pstMtaCnf->ucBSSILevel);
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSNetworkInfoQryCnf
 功能描述  : 收到MTA查询网络信息的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSNetworkInfoQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_NETWORK_INFO_QRY_CNF_STRU          *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_NETWORK_INFO_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSNetworkInfoQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSNetworkInfoQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_NETWORK_INFO_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSNetworkInfoQryCnf : Current Option is not AT_CMD_MBMS_NETWORK_INFO_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (VOS_INT32)pstMtaCnf->ulCellId);
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSModemStatusQryCnf
 功能描述  : 收到MTA查询EMBMS功能状态的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSModemStatusQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_EMBMS_STATUS_QRY_CNF_STRU               *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_EMBMS_STATUS_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSModemStatusQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSModemStatusQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EMBMS_STATUS_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSModemStatusQryCnf : Current Option is not AT_CMD_EMBMS_STATUS_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    (VOS_INT32)pstMtaCnf->enStatus);
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSEVSetCnf
 功能描述  : 收到MTA设置MBMS主动上报配置的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSEVSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSEVSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSEVSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_UNSOLICITED_CFG_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSEVSetCnf : Current Option is not AT_CMD_MBMS_UNSOLICITED_CFG_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSServiceEventInd
 功能描述  : AT收到MTA ID_MTA_AT_MBMS_SERVICE_EVENT_IND消息处理函数
 输入参数  : VOS_VOID *pstMsg - 消息指针
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSServiceEventInd(VOS_VOID *pstMsg)
{
    AT_MTA_MSG_STRU                        *pstRcvMsg      = VOS_NULL_PTR;
    MTA_AT_MBMS_SERVICE_EVENT_IND_STRU     *pstMtaAtInd    = VOS_NULL_PTR;
    VOS_UINT8                               ucIndex;
    VOS_UINT16                              usLength;

    /* 初始化 */
    pstRcvMsg           = (AT_MTA_MSG_STRU *)pstMsg;
    pstMtaAtInd         = (MTA_AT_MBMS_SERVICE_EVENT_IND_STRU *)pstRcvMsg->aucContent;
    ucIndex             = 0;
    usLength            = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSServiceEventInd : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s%s %d%s",
                                       gaucAtCrLf,
                                       gastAtStringTab[AT_STRING_MBMSEV].pucText,
                                       pstMtaAtInd->enEvent,
                                       gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, usLength);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSInterestListSetCnf
 功能描述  : 收到MTA设置Interest List的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSInterestListSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSInterestListSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSInterestListSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_INTERESTLIST_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSInterestListSetCnf : Current Option is not AT_CMD_MBMS_INTERESTLIST_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_ReportMBMSCmdQryCnf
 功能描述  : 打印MBMS可用服务列表信息
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID AT_ReportMBMSCmdQryCnf(
    MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU      *pstMtaCnf,
    VOS_UINT8                                       ucIndex
)
{
    VOS_UINT32                          ulListNum;
    VOS_UINT16                          usLength;
    VOS_UINT8                           aucServiceID[AT_MBMS_SERVICE_ID_LENGTH + 1];

    usLength = 0;

    for (ulListNum = 0; ulListNum < pstMtaCnf->ulAvlServiceNum; ulListNum++)
    {
        /* 获得MBMS Service ID字符串形式 */
        TAF_MEM_SET_S(aucServiceID, sizeof(aucServiceID), 0x00, sizeof(aucServiceID));
        At_ul2Auc(pstMtaCnf->astAvlServices[ulListNum].stTMGI.ulMbmsSerId, AT_MBMS_SERVICE_ID_LENGTH, aucServiceID);

        /* ^MBMSCMD: <AreaID>,<TMGI>:MBMS Service ID */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: %d,%s",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          (VOS_INT32)pstMtaCnf->astAvlServices[ulListNum].ulAreaId,
                                          aucServiceID);

        /* <TMGI>:Mcc */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%x%x%x",
                                          (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMcc & 0x0f00) >> 8,
                                          (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMcc & 0xf0) >> 4,
                                          (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMcc & 0x0f));
        /* <TMGI>:Mnc */
        if (0x0f00 == (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f00))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%x%x",
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0xf0) >> 4,
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f));
        }
        else
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%x%x%x",
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f00) >> 8,
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0xf0) >> 4,
                                              (pstMtaCnf->astAvlServices[ulListNum].stTMGI.stPlmnId.ulMnc & 0x0f));
        }

        if (VOS_TRUE == pstMtaCnf->astAvlServices[ulListNum].bitOpSessionId)
        {
            /* <SessionID> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              ",%d",
                                              (VOS_INT32)pstMtaCnf->astAvlServices[ulListNum].ulSessionId);
        }

        if (ulListNum != (pstMtaCnf->ulAvlServiceNum - 1))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                              "%s",
                                              gaucAtCrLf);
        }
    }

    gstAtSendData.usBufLen = usLength;
    return;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMBMSCmdQryCnf
 功能描述  : 收到MTA查询MBMS可用服务列表信息的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMBMSCmdQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                                *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU      *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                      ulResult;
    VOS_UINT8                                       ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSCmdQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMBMSCmdQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_MBMS_AVL_SERVICE_LIST_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMBMSCmdQryCnf : Current Option is not AT_CMD_MBMS_AVL_SERVICE_LIST_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        AT_ReportMBMSCmdQryCnf(pstMtaCnf, ucIndex);
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif

/*****************************************************************************
 函 数 名  : AT_SetLteLowPowerPara
 功能描述  : ^LTELOWPOWER
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetLteLowPowerPara(VOS_UINT8 ucIndex)
{
    AT_MTA_LOW_POWER_CONSUMPTION_SET_REQ_STRU       stPowerConsumption;
    VOS_UINT32                                      ulRst;

    TAF_MEM_SET_S(&stPowerConsumption, sizeof(stPowerConsumption), 0x00, sizeof(AT_MTA_LOW_POWER_CONSUMPTION_SET_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    switch(gastAtParaList[0].ulParaValue)
    {
        case 0:
            stPowerConsumption.enLteLowPowerFlg = AT_MTA_LTE_LOW_POWER_NORMAL;
            break;

        case 1:
            stPowerConsumption.enLteLowPowerFlg = AT_MTA_LTE_LOW_POWER_LOW;
            break;

        default:
            return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息到C核, 设置低功耗 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_LOW_POWER_SET_REQ,
                                   &stPowerConsumption,
                                   sizeof(stPowerConsumption),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetLteLowPowerPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_GetIsmCoexParaValue
 功能描述  : GetIsmCoexPara
 输入参数  : pucBegain,ppEnd
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_INT32 AT_GetIsmCoexParaValue(VOS_UINT8 *pucBegain, VOS_UINT8 **ppEnd)
{
    VOS_UINT32 ulTotal      = 0;
    VOS_INT32  lRstTotal    = 0;
    VOS_UINT32 ulRst;
    VOS_UINT8 *pucEnd;
    VOS_UINT32 ulFlag       = 0;

    pucEnd = pucBegain;

    while((' ' != *pucEnd) && ('\0' != *pucEnd))
    {
        pucEnd++;
    }

    if('-' == *pucBegain)
    {
        ulFlag = 1;
        pucBegain++;
    }

    ulRst = atAuc2ul(pucBegain, (VOS_UINT16)(pucEnd - pucBegain), &ulTotal);

    if(AT_SUCCESS != ulRst)
    {
        lRstTotal = AT_COEX_INVALID;
    }
    else
    {
        *ppEnd      = (pucEnd + 1);
        lRstTotal   = (VOS_INT32)(ulFlag ? (0 - ulTotal):ulTotal);
    }

    return lRstTotal;
}

/*****************************************************************************
 函 数 名  : AT_CheckIsmCoexParaValue
 功能描述  : 检查^ISMCOEX参数的有效性
 输入参数  : usVal,ulParaNum
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_CheckIsmCoexParaValue(VOS_INT32 ulVal, VOS_UINT32 ulParaNum)
{
    VOS_UINT32                          ulRst = AT_SUCCESS;

    switch(ulParaNum)
    {
        case AT_COEX_PARA_COEX_ENABLE:
            if((AT_COEX_PARA_COEX_ENABLE_MIN > ulVal)
            || (AT_COEX_PARA_COEX_ENABLE_MAX < ulVal))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_TX_BEGIN:
            if((AT_COEX_PARA_TX_BEGIN_MIN > ulVal)
            || (AT_COEX_PARA_TX_BEGIN_MAX < ulVal))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_TX_END:
            if((AT_COEX_PARA_TX_END_MIN > ulVal)
            || (AT_COEX_PARA_TX_END_MAX < ulVal))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_TX_POWER:
            if((AT_COEX_PARA_TX_POWER_MIN > ulVal)
            || (AT_COEX_PARA_TX_POWER_MAX < ulVal))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_RX_BEGIN:
            if((AT_COEX_PARA_RX_BEGIN_MIN > ulVal)
            || (AT_COEX_PARA_RX_BEGIN_MAX < ulVal))
            {
                ulRst = AT_FAILURE;
            }
            break;
        case AT_COEX_PARA_RX_END:
            if((AT_COEX_PARA_RX_END_MIN > ulVal)
            || (AT_COEX_PARA_RX_END_MAX < ulVal))
            {
                ulRst = AT_FAILURE;
            }
            break;
        default:
            ulRst = AT_FAILURE;
            break;

    }

    return ulRst;
}

/*****************************************************************************
 函 数 名  : AT_SetL4AIsmCoexParaValue
 功能描述  : 填充发往L4A的消息参数
 输入参数  : stIsmCoex
 输出参数  : pstReqToL4A
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_VOID AT_SetL4AIsmCoexParaValue(AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU stIsmCoex, L4A_ISMCOEX_REQ_STRU *pstReqToL4A, VOS_UINT8 ucIndex)
{
    VOS_UINT32                          i;

    pstReqToL4A->stCtrl.ulClientId = gastAtClientTab[ucIndex].usClientId;;
    pstReqToL4A->stCtrl.ulOpId     = 0;
    pstReqToL4A->stCtrl.ulPid      = WUEPS_PID_AT;

    for(i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++)
    {
        pstReqToL4A->astCoex[i].ulFlag      = (VOS_UINT32)stIsmCoex.astCoexPara[i].enCfg;
        pstReqToL4A->astCoex[i].ulTXBegin   = (VOS_UINT32)stIsmCoex.astCoexPara[i].usTxBegin;
        pstReqToL4A->astCoex[i].ulTXEnd     = (VOS_UINT32)stIsmCoex.astCoexPara[i].usTxEnd;
        pstReqToL4A->astCoex[i].lTXPower    = (VOS_INT32)stIsmCoex.astCoexPara[i].sTxPower;
        pstReqToL4A->astCoex[i].ulRXBegin   = (VOS_UINT32)stIsmCoex.astCoexPara[i].usRxBegin;
        pstReqToL4A->astCoex[i].ulRXEnd     = (VOS_UINT32)stIsmCoex.astCoexPara[i].usRxEnd;
    }

    return;
}

/*****************************************************************************
 函 数 名  : AT_SetIsmCoexPara
 功能描述  : ^ISMCOEX
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetIsmCoexPara(VOS_UINT8 ucIndex)
{
    AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU               stIsmCoex;
    L4A_ISMCOEX_REQ_STRU                            stReqToL4A = {0};
    VOS_UINT32                                      ulRst,ulRet;
    VOS_UINT32                                      i, j;
    VOS_INT32                                       ret;
    VOS_UINT16                                     *pusVal;                     /* 将要存储的值指针 */
    VOS_UINT8                                      *pucCur;                     /* 解析字符串时的当前指针 */
    VOS_UINT8                                      *pucPara;                    /* 参数字符串头指针 */

    TAF_MEM_SET_S(&stIsmCoex, sizeof(stIsmCoex), 0x00, sizeof(AT_MTA_LTE_WIFI_COEX_SET_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数 */
    if (AT_MTA_ISMCOEX_BANDWIDTH_NUM != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }


    for(i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++)
    {
        pucCur = gastAtParaList[i].aucPara;
        stIsmCoex.astCoexPara[i].enCoexBWType = (AT_MTA_COEX_BW_TYPE_ENUM_UINT16)i;
        pusVal = &(stIsmCoex.astCoexPara[i].enCfg);

        for(j = 0; j < sizeof(AT_MTA_COEX_PARA_STRU)/sizeof(VOS_UINT16) - 2; j++)
        {
            pucPara = pucCur;
            ret = AT_GetIsmCoexParaValue(pucPara, &pucCur);

            if(AT_COEX_INVALID == ret)
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }

            if (AT_FAILURE == AT_CheckIsmCoexParaValue(ret, j))
            {
                return AT_CME_INCORRECT_PARAMETERS;
            }

           *pusVal = (VOS_UINT16)ret;
            pusVal++;
        }
    }

    stIsmCoex.usCoexParaNum     = AT_MTA_ISMCOEX_BANDWIDTH_NUM;
    stIsmCoex.usCoexParaSize    = sizeof(stIsmCoex.astCoexPara);

    AT_SetL4AIsmCoexParaValue(stIsmCoex, &stReqToL4A, ucIndex);

    /* 发送消息到L4A */
    ulRet = atSendL4aDataMsg(gastAtClientTab[ucIndex].usClientId,
                             I0_MSP_L4_L4A_PID,
                             ID_MSG_L4A_ISMCOEXSET_REQ,
                             (VOS_VOID*)(&stReqToL4A),
                             sizeof(stReqToL4A));

    /* 发送跨核消息到C核 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_WIFI_COEX_SET_REQ,
                                   &stIsmCoex,
                                   sizeof(stIsmCoex),
                                   I0_UEPS_PID_MTA);
    if (TAF_SUCCESS != ulRet)
    {
        AT_WARN_LOG("AT_SetIsmCoexPara: atSendDataMsg fail.");
    }

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetIsmCoexPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_WIFI_COEX_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryIsmCoexPara
 功能描述  : ^ISMCOEX查询命令处理函数,查询
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年05月22日
    作    者   : w00316404
    修改内容   :新增查询函数
*****************************************************************************/
VOS_UINT32 AT_QryIsmCoexPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                                      ulRst;

    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 发送跨核消息到C核, 查询ISMCOEX列表请求 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_LTE_WIFI_COEX_QRY_REQ,
                                   VOS_NULL_PTR,
                                   0,
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_QryIsmCoexPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_LTE_WIFI_COEX_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaLteLowPowerSetCnf
 功能描述  : 收到MTA设置低功耗的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaLteLowPowerSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstMtaCnf       = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstMtaCnf           = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLteLowPowerSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaLteLowPowerSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaLteLowPowerSetCnf : Current Option is not AT_CMD_LTE_LOW_POWER_CONSUMPTION_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstMtaCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaIsmCoexSetCnf
 功能描述  : 收到MTA设置命令 ^ISMCOEX的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaIsmCoexSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_LTE_WIFI_COEX_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexSetCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (VOS_OK != pstCnf->enResult)
    {
        ulResult = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvL4AIsmCoexSetCnf
 功能描述  : 收到L4A设置命令 ^ISMCOEX的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvL4AIsmCoexSetCnf(
    VOS_VOID                           *pMsg
)
{
    return VOS_OK;
}


/*****************************************************************************
 函 数 名  : AT_RcvMtaIsmCoexQryCnf
 功能描述  : 收到MTA查询命令 ^ISMCOEX的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : w00316404
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaIsmCoexQryCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_LTE_WIFI_COEX_QRY_CNF_STRU          *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT32                                  i;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_LTE_WIFI_COEX_QRY_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexQryCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexQryCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_LTE_WIFI_COEX_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaIsmCoexQryCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    (TAF_CHAR *)pgucAtSndCodeAddr,
                                                    "%s:",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    for(i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++)
    {
        gstAtSendData.usBufLen += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                        (VOS_CHAR *)pgucAtSndCodeAddr,
                                                        (VOS_CHAR*)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                                        " %d %d %d %d %d %d,",
                                                        pstCnf->astCoexPara[i].enCfg,
                                                        pstCnf->astCoexPara[i].usTxBegin,
                                                        pstCnf->astCoexPara[i].usTxEnd,
                                                        pstCnf->astCoexPara[i].sTxPower,
                                                        pstCnf->astCoexPara[i].usRxBegin,
                                                        pstCnf->astCoexPara[i].usRxEnd);
    }

    gstAtSendData.usBufLen--;

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
#endif


/*****************************************************************************
 函 数 名  : AT_SetLogEnablePara
 功能描述  : ^LOGENABLE
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月21日
    作    者   : z00301431
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetLogEnablePara(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT设置1表示允许抓取modemlog，设置0表示不允许抓取modemlog */
    if (1 == gastAtParaList[0].ulParaValue)
    {
        /* 设置为FALSE表示允许抓MODEM LOG */
        DMS_SET_PRINT_MODEM_LOG_TYPE(VOS_FALSE);
    }
    else
    {
        /* 设置为TRUE表示不允许抓MODEM LOG */
        DMS_SET_PRINT_MODEM_LOG_TYPE(VOS_TRUE);
    }

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : AT_QryLogEnable
 功能描述  : ^LOGENABLE
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月21日
    作    者   : z00301431
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryLogEnable(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;
    VOS_UINT32                          ulEnableFlag;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usLength                            = 0;

    if (VOS_FALSE == DMS_GET_PRINT_MODEM_LOG_TYPE())
    {
        /* DMS当前允许抓MODEM LOG，返回enable为TRUE */
        ulEnableFlag = VOS_TRUE;
    }
    else
    {
        /* DMS当前不允许抓MODEM LOG，返回enable为FALSE */
        ulEnableFlag = VOS_FALSE;
    }

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulEnableFlag);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


#if ((FEATURE_ON == FEATURE_SC_DATA_STRUCT_EXTERN) || (FEATURE_ON == FEATURE_BOSTON_AFTER_FEATURE))
/*****************************************************************************
 函 数 名  : AT_StopSimlockDataWriteTimer
 功能描述  : 停止AT_SIMLOCKWRITEEX_TIMER
 输入参数  : VOS_UINT8  ucIndex

 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月06日
    作    者   : q00380176
    修改内容   : 新增
*****************************************************************************/
VOS_VOID AT_StopSimlockDataWriteTimer(VOS_UINT8  ucIndex)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimlockWriteExCtx;
    VOS_UINT32                          ulTimerName;
    VOS_UINT32                          ulTempIndex;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    ulTempIndex  = (VOS_UINT32)ucIndex;
    ulTimerName  = AT_SIMLOCKWRITEEX_TIMER;
    ulTimerName |= AT_INTERNAL_PROCESS_TYPE;
    ulTimerName |= (ulTempIndex<<12);

    if (VOS_NULL_PTR != pstSimlockWriteExCtx)
    {
        (VOS_VOID)AT_StopRelTimer(ulTimerName, &(pstSimlockWriteExCtx->hSimLockWriteExProtectTimer));
    }

    return;
}
/*****************************************************************************
 函 数 名  : AT_ProcSimlockWriteExData
 功能描述  :
 输入参数  : AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara ^SIMLOCKDATAWRITEEX设置命令参数信息

 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月06日
    作    者   : q00380176
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32  AT_ProcSimlockWriteExData(
    VOS_UINT8                          *pucSimLockData,
    VOS_UINT16                          usParaLen
)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX *pstSimlockWriteExCtx;
    VOS_UINT8                          *pTempData                  = VOS_NULL_PTR;
    VOS_UINT16                          usTotalLen;

    if ((VOS_NULL_PTR == pucSimLockData)
     || (0 == usParaLen))
    {
        AT_ERR_LOG("AT_ProcSimlockWriteExData: NULL Pointer");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    /* 当前是一次新的设置过程，收到的是第一条AT命令 */
    if (VOS_NULL_PTR == pstSimlockWriteExCtx->pucData)
    {
        pstSimlockWriteExCtx->pucData = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, (VOS_UINT32)usParaLen);

        /* 分配内存失败，直接返回 */
        if (VOS_NULL_PTR == pstSimlockWriteExCtx->pucData)
        {
            AT_ERR_LOG("AT_ProcSimlockWriteExData: first data, Alloc mem fail");

            return AT_CME_MEMORY_FAILURE;
        }

        TAF_MEM_CPY_S(pstSimlockWriteExCtx->pucData, usParaLen, pucSimLockData, usParaLen);

        pstSimlockWriteExCtx->usSimlockDataLen = usParaLen;
    }
    else
    {
        /* 当前不是收到第一条AT命令，需要拼接码流 */
        usTotalLen = usParaLen + pstSimlockWriteExCtx->usSimlockDataLen;

        pTempData  = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, usTotalLen);

        /* 分配内存失败，直接返回 */
        if (VOS_NULL_PTR == pTempData)
        {
            AT_ERR_LOG("AT_ProcSimlockWriteExData: Non-first data, Alloc mem fail");

            return AT_CME_MEMORY_FAILURE;
        }

        TAF_MEM_CPY_S(pTempData, usTotalLen, pstSimlockWriteExCtx->pucData, pstSimlockWriteExCtx->usSimlockDataLen);
        TAF_MEM_CPY_S((pTempData + pstSimlockWriteExCtx->usSimlockDataLen), usTotalLen, pucSimLockData, usParaLen);
        PS_MEM_FREE(WUEPS_PID_AT, pstSimlockWriteExCtx->pucData);

        pstSimlockWriteExCtx->usSimlockDataLen = usTotalLen;
        pstSimlockWriteExCtx->pucData          = pTempData;
    }

    return AT_SUCCESS;


}
/*****************************************************************************
 函 数 名  : AT_SaveSimlockDataIntoCtx
 功能描述  : 保存输入的参数到全局变量
 输入参数  : VOS_UINT8 ucIndex
             AT_SIMLOCK_WRITE_EX_PARA_STRU *stSimlockWriteExPara

 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月06日
    作    者   : q00380176
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_SaveSimlockDataIntoCtx(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara,
    VOS_UINT8                      ucIndex,
    VOS_UINT8                      ucNetWorkFlg)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX                     *pstSimlockWriteExCtx;
    VOS_UINT8                                              *pucSimLockData;
    VOS_UINT8                                              *pucHmac;
    VOS_UINT32                                              ulResult;
    VOS_UINT32                                              ulLayer;
    VOS_UINT32                                              ulTotal;
    VOS_UINT32                                              ulCurIndex;
    VOS_UINT16                                              usSimLockDataLen;
    VOS_UINT16                                              usHmacLen;
    VOS_UINT8                                               ucParaNum;

    /* 变量初始化 */
    ulLayer           = pstSimlockWriteExPara->ulLayer;
    ulTotal           = pstSimlockWriteExPara->ulTotal;
    ulCurIndex        = pstSimlockWriteExPara->ulIndex;
    usSimLockDataLen  = pstSimlockWriteExPara->usSimLockDataLen;
    usHmacLen         = pstSimlockWriteExPara->usHmacLen;
    ucParaNum         = pstSimlockWriteExPara->ucParaNum;
    pucSimLockData    = pstSimlockWriteExPara->pucSimLockData;
    pucHmac           = pstSimlockWriteExPara->pucHmac;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    /* 当前不再设置过程中，第一次收到此命令 */
    if (VOS_FALSE == pstSimlockWriteExCtx->ucSettingFlag)
    {
        if (1 != ulCurIndex)
        {
            AT_WARN_LOG1("AT_SaveSimlockDataIntoCtx: Invalid ulCurrIndex", pstSimlockWriteExPara->ulIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 将字符串参数转换为码流，并保存 */
        ulResult = AT_ProcSimlockWriteExData(pucSimLockData, usSimLockDataLen);

        if (AT_SUCCESS != ulResult)
        {
            AT_WARN_LOG1("AT_SaveSimlockDataIntoCtx: AT_ProcSimlockWriteExData fail %d", ulResult);

            return ulResult;
        }

        pstSimlockWriteExCtx->ucNetWorkFlg  = ucNetWorkFlg;
        pstSimlockWriteExCtx->ucClientId    = ucIndex;
        pstSimlockWriteExCtx->ucTotalNum    = (VOS_UINT8)ulTotal;
        pstSimlockWriteExCtx->ucCurIdx      = (VOS_UINT8)ulCurIndex;
        pstSimlockWriteExCtx->ucLayer       = (VOS_UINT8)ulLayer;
        pstSimlockWriteExCtx->ucSettingFlag = VOS_TRUE;
    }
    else
    {
        /* 必须同一个用户下发 */
        if (ucNetWorkFlg != pstSimlockWriteExCtx->ucNetWorkFlg)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: ucNetWorkFlg error, PreNetWorkFlg %d, CurNetWorkFlg %d", ucNetWorkFlg, pstSimlockWriteExCtx->ucNetWorkFlg);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 必须在同一个通道下发命令 */
        if (ucIndex != pstSimlockWriteExCtx->ucClientId)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: port error, ucIndex %d, ucClientId %d", ucIndex, pstSimlockWriteExCtx->ucClientId);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 当前已经在设置中，当前下发的Layer与之前之前下发的Layer不同 */
        if ((VOS_UINT8)ulLayer != pstSimlockWriteExCtx->ucLayer)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: Layer %d wrong, %d", ulLayer, pstSimlockWriteExCtx->ucLayer);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 当前已经在设置中，当前下发的total与之前之前下发的total不同 */
        if ((VOS_UINT8)ulTotal != pstSimlockWriteExCtx->ucTotalNum)
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: total %d wrong, %d", ulTotal, pstSimlockWriteExCtx->ucTotalNum);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 当前下发的Index不是之前下发Index+1 */
        if ((VOS_UINT8)ulCurIndex != (pstSimlockWriteExCtx->ucCurIdx + 1))
        {
            AT_WARN_LOG2("AT_SaveSimlockDataIntoCtx: CurIndex %d wrong, %d", ulCurIndex, pstSimlockWriteExCtx->ucCurIdx);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 将字符串参数转换为码流 */
        ulResult = AT_ProcSimlockWriteExData(pucSimLockData, usSimLockDataLen);

        if (AT_SUCCESS != ulResult)
        {
            AT_WARN_LOG1("AT_SaveSimlockDataIntoCtx: AT_ProcSimlockWriteExData fail %d", ulResult);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return ulResult;
        }

        /* 更新CurrIndex */
        pstSimlockWriteExCtx->ucCurIdx      = (VOS_UINT8)ulCurIndex;
    }

    /* 如果参数个数为5，将第5个参数copy到全局变量，如果之前有输入HMAC，覆盖之前的输入 */
    if ((5 == ucParaNum) && (AT_SET_SIMLOCK_DATA_HMAC_LEN == usHmacLen))
    {
        TAF_MEM_CPY_S(pstSimlockWriteExCtx->aucHmac, AT_SET_SIMLOCK_DATA_HMAC_LEN, pucHmac, usHmacLen);
        pstSimlockWriteExCtx->ucHmacLen = (VOS_UINT8)usHmacLen;
    }

    return AT_OK;
}
/*****************************************************************************
 函 数 名  : AT_CheckSimlockDataWriteExPara
 功能描述  : 检查^SIMLOCKDATAWRITEEX设置命令参数的有效性
 输入参数  : AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara

 输出参数  : 无
 返 回 值  :  VOS_OK  有效
              VOS_ERR 无效
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月06日
    作    者   : q00380176
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_CheckSimlockDataWriteExPara(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara
)
{

    /* 规定的layer取值为0,1,2,3,4,255 */
    if ((4 < pstSimlockWriteExPara->ulLayer) && (255 != pstSimlockWriteExPara->ulLayer))
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: invalid layer value:", pstSimlockWriteExPara->ulLayer);
        return VOS_ERR;
    }

    /* 规定的ulIndex取值为1-255 */
    if ((0 == pstSimlockWriteExPara->ulIndex) || (255 < pstSimlockWriteExPara->ulIndex))
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: invalid ulIndex value:", pstSimlockWriteExPara->ulIndex);
        return VOS_ERR;
    }

    /* 规定的ulTotal取值为1-255 */
    if ((0 == pstSimlockWriteExPara->ulTotal) || (255 < pstSimlockWriteExPara->ulTotal))
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: invalid ulTotal value:", pstSimlockWriteExPara->ulTotal);
        return VOS_ERR;
    }

    /* Index要小于total */
    if (pstSimlockWriteExPara->ulIndex > pstSimlockWriteExPara->ulTotal)
    {
        AT_WARN_LOG2("AT_CheckSimlockDataWriteExPara: Index bigger than total", pstSimlockWriteExPara->ulIndex, pstSimlockWriteExPara->ulTotal);

        return VOS_ERR;
    }

    /* 规定一次写入的simlockdata数据不大于1400个字符 */
    if (AT_SIMLOCKDATA_PER_WRITE_MAX_LEN < pstSimlockWriteExPara->usSimLockDataLen)
    {
        AT_WARN_LOG1("AT_CheckSimlockDataWriteExPara: SimLockData is too long:", pstSimlockWriteExPara->usSimLockDataLen);
        return VOS_ERR;
    }

    return VOS_OK;

}

/*****************************************************************************
 函 数 名  : AT_SetSimlockDataWriteExPara
 功能描述  : ^SIMLOCKDATAWRITEEX设置命令处理函数
 输入参数  : VOS_UINT8 ucIndex
             AT_SIMLOCK_WRITE_EX_PARA_STRU *stSimlockWriteExPara

 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月06日
    作    者   : q00380176
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_SetSimlockDataWriteExPara(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara,
    VOS_UINT8                      ucIndex,
    VOS_UINT8                      ucNetWorkFlg
)
{
    AT_SIMLOCKDATAWRITEEX_CMD_PROC_CTX                     *pstSimlockWriteExCtx;
    DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU                  *pstSimlockWriteExSetReq;
    VOS_UINT32                                              ulTimerName;
    VOS_UINT32                                              ulTempIndex;
    VOS_UINT32                                              ulResult;
    VOS_UINT32                                              ulLength;
    VOS_UINT16                                              usHmacLen;

    pstSimlockWriteExCtx = AT_GetSimLockWriteExCmdCtxAddr();

    if (VOS_OK != AT_CheckSimlockDataWriteExPara(pstSimlockWriteExPara))
    {
        if (VOS_FALSE == pstSimlockWriteExCtx->ucSettingFlag)
        {
            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);
        }

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将参数保存到全局变量  */
    ulResult = AT_SaveSimlockDataIntoCtx(pstSimlockWriteExPara, ucIndex, ucNetWorkFlg);

    if (AT_OK != ulResult)
    {
        return ulResult;
    }

    /* 如果还未收齐数据，则启动定时器，回复OK */
    if (pstSimlockWriteExCtx->ucCurIdx < pstSimlockWriteExCtx->ucTotalNum)
    {
        /* 停止上一周期的定时器，重启定时器 */
        AT_StopSimlockDataWriteTimer(ucIndex);

        ulTempIndex  = (VOS_UINT32)ucIndex;
        ulTimerName  = AT_SIMLOCKWRITEEX_TIMER;
        ulTimerName |= AT_INTERNAL_PROCESS_TYPE;
        ulTimerName |= (ulTempIndex<<12);

        (VOS_VOID)AT_StartRelTimer(&(pstSimlockWriteExCtx->hSimLockWriteExProtectTimer),
                                   AT_SIMLOCK_WRITE_EX_PROTECT_TIMER_LEN,
                                   ulTimerName,
                                   0, VOS_RELTIMER_NOLOOP);

        return AT_OK;
    }
    else
    {
        /* 已经收齐了数据，将Simlock_Data转换码流 */
        ulResult = At_AsciiNum2HexString(pstSimlockWriteExCtx->pucData, &(pstSimlockWriteExCtx->usSimlockDataLen));
        if (AT_SUCCESS != ulResult)
        {
            AT_WARN_LOG2("AT_SetSimlockDataWriteExPara: At_AsciiNum2HexString fail ulResult: %d ulParaLen: %d",
                         ulResult,
                         pstSimlockWriteExCtx->usSimlockDataLen);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 已经收齐了数据，将HMAC转换码流 */
        usHmacLen = pstSimlockWriteExCtx->ucHmacLen;
        ulResult = At_AsciiNum2HexString(pstSimlockWriteExCtx->aucHmac, &usHmacLen);

        pstSimlockWriteExCtx->ucHmacLen = (VOS_UINT8)usHmacLen;

        if ((AT_SUCCESS != ulResult)
         || (DRV_AGENT_HMAC_DATA_LEN != pstSimlockWriteExCtx->ucHmacLen))
        {
            AT_WARN_LOG2("AT_SetSimlockDataWriteExPara: At_AsciiNum2HexString fail ulResult: %d ulParaLen: %d",
                         ulResult,
                         pstSimlockWriteExCtx->ucHmacLen);

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        ulLength = sizeof(DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU) + pstSimlockWriteExCtx->usSimlockDataLen - 4;
        pstSimlockWriteExSetReq = (DRV_AGENT_SIMLOCKWRITEEX_SET_REQ_STRU *)PS_MEM_ALLOC(WUEPS_PID_AT, ulLength);

        if (VOS_NULL_PTR == pstSimlockWriteExSetReq)
        {
            AT_WARN_LOG("AT_SetSimlockDataWriteExPara: alloc mem fail.");

            AT_ClearSimLockWriteExCtx();
            AT_StopSimlockDataWriteTimer(ucIndex);

            return AT_CME_MEMORY_FAILURE;
        }

        pstSimlockWriteExSetReq->ulHmacLen = pstSimlockWriteExCtx->ucHmacLen;
        TAF_MEM_CPY_S(pstSimlockWriteExSetReq->aucHmac,
                      sizeof(pstSimlockWriteExSetReq->aucHmac),
                      pstSimlockWriteExCtx->aucHmac,
                      pstSimlockWriteExCtx->ucHmacLen);

        pstSimlockWriteExSetReq->ulSimlockDataLen = pstSimlockWriteExCtx->usSimlockDataLen;
        TAF_MEM_CPY_S(pstSimlockWriteExSetReq->aucSimlockData,
                      pstSimlockWriteExCtx->usSimlockDataLen,
                      pstSimlockWriteExCtx->pucData,
                      pstSimlockWriteExCtx->usSimlockDataLen);

        /* 记录总共写入的次数，既最后一次写入时的index */
        pstSimlockWriteExSetReq->ulTotal = pstSimlockWriteExCtx->ucTotalNum;

        /* 记录是否是网络下发的标识 */
        pstSimlockWriteExSetReq->ucNetWorkFlg = pstSimlockWriteExCtx->ucNetWorkFlg;
        pstSimlockWriteExSetReq->ucLayer      = pstSimlockWriteExCtx->ucLayer;

        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        /* 转换成功, 发送跨核消息到C核, 设置产线公钥 */
        ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                          gastAtClientTab[ucIndex].opId,
                                          DRV_AGENT_SIMLOCKWRITEEX_SET_REQ,
                                          pstSimlockWriteExSetReq,
                                          ulLength,
                                          I0_WUEPS_PID_DRV_AGENT);

        PS_MEM_FREE(WUEPS_PID_AT, pstSimlockWriteExSetReq);

        if (TAF_SUCCESS != ulResult)
        {
            AT_WARN_LOG("AT_SetSimlockDataWriteExPara: AT_FillAndSndAppReqMsg fail.");

            return AT_ERROR;
        }

        /* 由于SIMLOCKDATAWRITEEX特殊处理，需要手动启动定时器*/
        if (AT_SUCCESS != At_StartTimer(AT_SET_PARA_TIME, ucIndex))
        {
            AT_WARN_LOG("AT_SetSimlockDataWriteExPara: At_StartTimer fail.");

            return AT_ERROR;
        }

        g_stParseContext[ucIndex].ucClientStatus = AT_FW_CLIENT_STATUS_PEND;

        /* 设置AT模块实体的状态为等待异步返回 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SIMLOCKDATAWRITEEX_SET;

        return AT_WAIT_ASYNC_RETURN;
    }
}

/*****************************************************************************
 函 数 名  : AT_GetSimlockDataWriteExParaValue
 功能描述  : 解析AT^SIMLOCKDATAWRITEEX命令的参数值
 输入参数  : ucIndex --- 用户索引
             pucData --- 输入的字符串
             pusLen --- 字符串长度
 输出参数  : 无
 返 回 值  : AT_SUCCESS 解析参数值正确,处理完毕
             AT_FAILURE 解析参数值错误
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月01日
    作    者   : q00380176
    修改内容   : 网络下发的 AP-Modem锁网锁卡项目新增函数
*****************************************************************************/
VOS_UINT32 AT_ParseSimlockDataWriteExParaValue(
    VOS_UINT8                          *pucData,
    AT_SIMLOCK_WRITE_EX_PARA_STRU      *pstSimlockWriteExPara,
    VOS_UINT16                          usPos,
    VOS_UINT16                          usLen
)
{
    VOS_UINT16                          ausCommPos[4] = {0};
    VOS_UINT32                          ulFirstParaVal;
    VOS_UINT32                          ulSecParaVal;
    VOS_UINT32                          ulThirdParaVal;
    VOS_UINT16                          usLoop;
    VOS_UINT16                          usFirstParaLen;
    VOS_UINT16                          usSecondParaLen;
    VOS_UINT16                          usThirdParaLen;
    VOS_UINT16                          usFourthParaLen;
    VOS_UINT16                          usFifthParaLen;
    VOS_UINT16                          usCommaCnt;
    VOS_UINT8                           ucParaNum;

    usCommaCnt          = 0;
    usFourthParaLen     = 0;
    usFifthParaLen      = 0;
    ulFirstParaVal      = 0;
    ulSecParaVal        = 0;
    ulThirdParaVal      = 0;
    ucParaNum           = 0;

    /* 获取命令中的逗号位置和个数 */
    for ( usLoop = usPos; usLoop < usLen; usLoop++ )
    {
        if (',' == *(pucData + usLoop))
        {
            /* 记录下逗号的位置 */
            if (4 > usCommaCnt)
            {
                ausCommPos[usCommaCnt] = usLoop + 1;
            }
            usCommaCnt++;
        }
    }

    /* 若逗号个数大于4，则AT命令结果返回失败 */
    if ((4 != usCommaCnt) && (3 != usCommaCnt))
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: Num of Para is  Invalid!");
        return VOS_ERR;
    }

    /* 计算参数的长度 */
    usFirstParaLen  = (ausCommPos[0] - usPos) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);
    usSecondParaLen = (ausCommPos[1] - ausCommPos[0]) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);
    usThirdParaLen  = (ausCommPos[2] - ausCommPos[1]) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);

    /* 如果逗号的个数为3，那么参数的个数为4；如果逗号的个数为4，那么参数的个数为5 */
    if (3== usCommaCnt)
    {
        usFourthParaLen = usLen - ausCommPos[2];
        ucParaNum       = 4;
    }
    else
    {
        usFourthParaLen = (ausCommPos[3] - ausCommPos[2]) - (VOS_UINT16)VOS_StrNLen(",", AT_CONST_NUM_2);
        usFifthParaLen  = usLen - ausCommPos[3];
        ucParaNum       = 5;
    }

    /* 获取第一个参数值 */
    if (AT_FAILURE == atAuc2ul(pucData + usPos, usFirstParaLen, &ulFirstParaVal))
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: ulFirstParaVal value invalid");
        return VOS_ERR;
    }

    /* 获取第二个参数值 */
    if (AT_FAILURE == atAuc2ul(pucData + ausCommPos[0], usSecondParaLen, &ulSecParaVal))
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: ulSecParaVal value invalid");
        return VOS_ERR;
    }

    /* 获取第三个参数值 */
    if (AT_FAILURE == atAuc2ul(pucData + ausCommPos[1], usThirdParaLen, &ulThirdParaVal))
    {
        AT_WARN_LOG("AT_ParseSimlockDataWriteExParaValue: ulThirdParaVal value invalid");
        return VOS_ERR;
    }

    pstSimlockWriteExPara->ucParaNum        = ucParaNum;
    pstSimlockWriteExPara->ulLayer          = ulFirstParaVal;
    pstSimlockWriteExPara->ulIndex          = ulSecParaVal;
    pstSimlockWriteExPara->ulTotal          = ulThirdParaVal;
    pstSimlockWriteExPara->usSimLockDataLen = usFourthParaLen;
    pstSimlockWriteExPara->pucSimLockData   = pucData + ausCommPos[2];

    /* 如果参数个数等于5 */
    if (5 == ucParaNum)
    {
        pstSimlockWriteExPara->usHmacLen    = usFifthParaLen;
        pstSimlockWriteExPara->pucHmac      = pucData + ausCommPos[3];
    }

    return VOS_OK;
}
#if (FEATURE_ON == FEATURE_SC_NETWORK_UPDATE)
/*****************************************************************************
 函 数 名  : AT_HandleSimLockNWDataWriteCmd
 功能描述  : 处理网络下发的AT^SIMLOCKNWDATAWRITE命令的特殊函数(因为该命令的第4个参数长度超过
             解析器处理上限，需要进行特殊处理)
 输入参数  : ucIndex --- 用户索引
             pucData --- 输入的字符串
             pusLen --- 字符串长度
 输出参数  : 无
 返 回 值  : AT_SUCCESS 是^SIMLOCKNWDATAWRITE命令,处理完毕
             AT_FAILURE 不是^SIMLOCKNWDATAWRITE命令
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月01日
    作    者   : q00380176
    修改内容   : 网络下发的 AP-Modem锁网锁卡项目新增函数
*****************************************************************************/
VOS_UINT32 AT_HandleSimLockNWDataWriteCmd(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucDataPara;
    AT_PARSE_CMD_NAME_TYPE_STRU         stAtCmdName;
    AT_SIMLOCK_WRITE_EX_PARA_STRU       stSimlockWriteExPara;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usCmdlen;
    VOS_UINT16                          usPos;
    VOS_UINT16                          usLength;
    VOS_INT8                            cRet;

    /* 只能是APPVCOM端口19下发的命令 */
    if ((AT_APP_USER != gastAtClientTab[ucIndex].UserType)
      ||(APP_VCOM_DEV_INDEX_19 != gastAtClientTab[ucIndex].ucPortNo))
    {
        return AT_FAILURE;
    }

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stAtCmdName, sizeof(stAtCmdName), 0x00, sizeof(stAtCmdName));
    usCmdlen             = (VOS_UINT16)VOS_StrNLen("AT^SIMLOCKNWDATAWRITE=", AT_CONST_NUM_23);

    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usCmdlen);
    if (VOS_NULL_PTR == pucDataPara)
    {
        AT_ERR_LOG("AT_HandleSimLockNWDataWriteCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    /*拷贝命令名，供后续比较使用*/
    TAF_MEM_CPY_S(pucDataPara, usCmdlen, pucData, usCmdlen);

    /* AT命令头字符转大写 */
    At_UpString(pucDataPara, usCmdlen);

    /* 待处理的字符串头部不是"AT^SIMLOCKDATAWRITEEX="直接返回AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^SIMLOCKNWDATAWRITE=", usCmdlen);
    if (0 != cRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(ucIndex, (VOS_UINT8*)"^SIMLOCKNWDATAWRITE", AT_EXTEND_CMD_TYPE);

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    usPos = (VOS_UINT16)VOS_StrNLen("AT", AT_CONST_NUM_3);

    stAtCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrNLen("^SIMLOCKNWDATAWRITE", AT_CONST_NUM_23);
    TAF_MEM_CPY_S(stAtCmdName.aucCmdName,
                  sizeof(stAtCmdName.aucCmdName),
                  (pucData + usPos),
                  stAtCmdName.usCmdNameLen);
    stAtCmdName.aucCmdName[stAtCmdName.usCmdNameLen] = '\0';
    usPos += stAtCmdName.usCmdNameLen;

    usPos += (VOS_UINT16)VOS_StrNLen("=", AT_CONST_NUM_2);

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stSimlockWriteExPara, sizeof(stSimlockWriteExPara), 0x00, sizeof(stSimlockWriteExPara));
    stSimlockWriteExPara.pucSimLockData   = VOS_NULL_PTR;
    stSimlockWriteExPara.pucHmac          = VOS_NULL_PTR;

    if (VOS_OK != AT_ParseSimlockDataWriteExParaValue(pucData, &stSimlockWriteExPara,usPos, usLen))
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_CME_INCORRECT_PARAMETERS);
        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        return AT_SUCCESS;
    }

     /* 设置命令类型，操作类型和参数个数 */
    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    gucAtCmdFmtType = AT_EXTEND_CMD_TYPE;

    ulResult = AT_SetSimlockDataWriteExPara(&stSimlockWriteExPara, ucIndex, VOS_TRUE);

    /* 添加打印 ^SIMLOCKNWDATAWRITE:<index>操作 */
    usLength = 0;

    if (AT_WAIT_ASYNC_RETURN != ulResult)
    {
        if (AT_OK == ulResult)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s:%d",
                                               "^SIMLOCKNWDATAWRITE",
                                               stSimlockWriteExPara.ulIndex);
        }

        gstAtSendData.usBufLen  = usLength;
        At_FormatResultData(ucIndex, ulResult);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
    return AT_SUCCESS;
}
#endif
/*****************************************************************************
 函 数 名  : AT_HandleSimLockDataWriteExCmd
 功能描述  : 处理AT^SIMLOCKDATAWRITEEX命令的特殊函数(因为该命令的第4个参数长度超过
             解析器处理上限，需要进行特殊处理)
 输入参数  : ucIndex --- 用户索引
             pucData --- 输入的字符串
             pusLen --- 字符串长度
 输出参数  : 无
 返 回 值  : AT_SUCCESS 是^SIMLOCKDATAWRITEEX命令,处理完毕
             AT_FAILURE 不是^SIMLOCKDATAWRITEEX命令
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月01日
    作    者   : q00380176
    修改内容   : 网络下发的 AP-Modem锁网锁卡项目新增函数
*****************************************************************************/
VOS_UINT32 AT_HandleSimLockDataWriteExCmd(
    VOS_UINT8                           ucIndex,
    VOS_UINT8                          *pucData,
    VOS_UINT16                          usLen
)
{
    VOS_UINT8                          *pucDataPara;
    AT_PARSE_CMD_NAME_TYPE_STRU         stAtCmdName;
    AT_SIMLOCK_WRITE_EX_PARA_STRU       stSimlockWriteExPara;
    VOS_UINT32                          ulResult;
    VOS_UINT16                          usCmdlen;
    VOS_UINT16                          usPos;
    VOS_UINT16                          usLength;
    VOS_INT8                            cRet;

    TAF_MEM_SET_S(&stAtCmdName, sizeof(stAtCmdName), 0x00, sizeof(stAtCmdName));

    /* 局部变量初始化 */
    usCmdlen             = (VOS_UINT16)VOS_StrNLen("AT^SIMLOCKDATAWRITEEX=", AT_CONST_NUM_23);

    /* 通道检查 */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    {
        return AT_FAILURE;
    }

    /* 长度不满足设置命令的最小长度 AT^SIMLOCKDATAWRITEEX=1,1,1,  等于号后6个字符 */
    if ((usCmdlen + 6) > usLen)
    {
        return AT_FAILURE;
    }

    pucDataPara = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, usCmdlen);
    if (VOS_NULL_PTR == pucDataPara)
    {
        AT_ERR_LOG("AT_HandleSimLockDataWriteExCmd: pucDataPara Memory malloc failed!");
        return AT_FAILURE;
    }

    /*拷贝命令名，供后续比较使用*/
    TAF_MEM_CPY_S(pucDataPara, usCmdlen, pucData, usCmdlen);

    /* AT命令头字符转大写 */
    At_UpString(pucDataPara, usCmdlen);

    /* 待处理的字符串头部不是"AT^SIMLOCKDATAWRITEEX="直接返回AT_FAILURE */
    cRet = VOS_StrNiCmp((VOS_CHAR *)pucDataPara, "AT^SIMLOCKDATAWRITEEX=", usCmdlen);
    if (0 != cRet)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        return AT_FAILURE;
    }

    AT_SaveCmdElementInfo(ucIndex, (VOS_UINT8*)"^SIMLOCKDATAWRITEEX", AT_EXTEND_CMD_TYPE);

    /* 获取命令(不包含命令前缀AT)名称及长度 */
    usPos = (VOS_UINT16)VOS_StrNLen("AT", AT_CONST_NUM_3);

    stAtCmdName.usCmdNameLen = (VOS_UINT16)VOS_StrNLen("^SIMLOCKDATAWRITEEX", AT_CONST_NUM_23);
    TAF_MEM_CPY_S(stAtCmdName.aucCmdName,
                  sizeof(stAtCmdName.aucCmdName),
                  (pucData + usPos),
                  stAtCmdName.usCmdNameLen);
    stAtCmdName.aucCmdName[stAtCmdName.usCmdNameLen] = '\0';
    usPos += stAtCmdName.usCmdNameLen;

    usPos += (VOS_UINT16)VOS_StrNLen("=", AT_CONST_NUM_2);

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stSimlockWriteExPara, sizeof(stSimlockWriteExPara), 0x00, sizeof(stSimlockWriteExPara));
    stSimlockWriteExPara.pucSimLockData   = VOS_NULL_PTR;
    stSimlockWriteExPara.pucHmac          = VOS_NULL_PTR;

    if (VOS_OK != AT_ParseSimlockDataWriteExParaValue(pucData, &stSimlockWriteExPara,usPos, usLen))
    {
        PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
        At_FormatResultData(ucIndex, AT_CME_INCORRECT_PARAMETERS);
        AT_ClearSimLockWriteExCtx();
        AT_StopSimlockDataWriteTimer(ucIndex);

        gstAtSendData.usBufLen  = 0;
        At_FormatResultData(ucIndex, AT_ERROR);

        return AT_SUCCESS;
    }

     /* 设置命令类型，操作类型和参数个数 */
    g_stATParseCmd.ucCmdOptType = AT_CMD_OPT_SET_PARA_CMD;
    gucAtCmdFmtType = AT_EXTEND_CMD_TYPE;

#if (VOS_OS_VER == VOS_LINUX)
    printk(KERN_ERR "\n AT_HandleSimLockDataWriteExCmd enter \n");
#endif

    ulResult = AT_SetSimlockDataWriteExPara(&stSimlockWriteExPara, ucIndex, VOS_FALSE);

    /* 添加打印 ^SIMLOCKDATAWRITEEX:<index>操作 */
    usLength = 0;

    if (AT_WAIT_ASYNC_RETURN != ulResult)
    {
        if (AT_OK == ulResult)
        {
#if (VOS_OS_VER == VOS_LINUX)
            printk(KERN_ERR "\n AT_HandleSimLockDataWriteExCmd return OK \n");
#endif

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s:%d",
                                               "^SIMLOCKDATAWRITEEX",
                                               stSimlockWriteExPara.ulIndex);
        }

        gstAtSendData.usBufLen  = usLength;
        At_FormatResultData(ucIndex, ulResult);
    }

    PS_MEM_FREE(WUEPS_PID_AT, pucDataPara);
    return AT_SUCCESS;
}

/*****************************************************************************
 函 数 名  : AT_RcvDrvAgentSimlockWriteExSetCnf
 功能描述  : ^SIMLOCKDATAWRITE命令设置回复处理函数
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年04月10日
    作    者   : q00380176
    修改内容   : AP-Modem锁网锁卡项目新增函数

*****************************************************************************/
VOS_UINT32 AT_RcvDrvAgentSimlockWriteExSetCnf(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                         *pRcvMsg;
    DRV_AGENT_SIMLOCKWRITEEX_SET_CNF_STRU      *pstEvent;
    VOS_UINT32                                  ulResult;
    VOS_UINT16                                  usLength;
    VOS_UINT8                                   ucIndex;

#if (VOS_OS_VER == VOS_LINUX)
    printk(KERN_ERR "\n AT_RcvDrvAgentSimlockWriteExSetCnf enter\n");
#endif

    /* 初始化消息变量 */
    pRcvMsg         = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent        = (DRV_AGENT_SIMLOCKWRITEEX_SET_CNF_STRU *)pRcvMsg->aucContent;
    ucIndex         = 0;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockWriteExSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockWriteExSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_SIMLOCKWRITEEX_SET */
    if ( AT_CMD_SIMLOCKDATAWRITEEX_SET != gastAtClientTab[ucIndex].CmdCurrentOpt )
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockWriteExSetCnf: CmdCurrentOpt ERR.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    usLength = 0;

    /* 判断查询操作是否成功 */
    if ( DRV_AGENT_PERSONALIZATION_NO_ERROR == pstEvent->enResult )
    {
        /* 输出设置结果 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           "%s:%d",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           pstEvent->ulTotal);
        ulResult    = AT_OK;
    }
    else
    {
        /* 异常情况, 转换错误码 */
        ulResult    = AT_PERSONALIZATION_ERR_BEGIN + pstEvent->enResult;
    }

    gstAtSendData.usBufLen  = usLength;

    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}
/*****************************************************************************
 函 数 名  : AT_SimLockDataReadExPara
 功能描述  : ^SIMLOCKDATAREADEX设置命令处理函数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年03月06日
    作    者   : q00380176
    修改内容   : AP-Modem锁网锁卡项目新增函数

*****************************************************************************/
VOS_UINT32 AT_SimLockDataReadExPara(VOS_UINT8 ucIndex)
{
    DRV_AGENT_SIMLOCKDATAREADEX_READ_REQ_STRU stSimLockDataReadExReq;
    VOS_UINT32                                ulResult;

#if (VOS_OS_VER == VOS_LINUX)
    printk(KERN_ERR "\n AT_SimLockDataReadExPara enter\n");
#endif

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stSimLockDataReadExReq, sizeof(stSimLockDataReadExReq), 0x00, sizeof(stSimLockDataReadExReq));

    /* 通道检查 */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: It Is not Ap Port.");
        return AT_ERROR;
    }

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: ucCmdOptType is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数错误 */
    if (2 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: gucAtParaIndex ERR.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数的长度不能为0 */
    if (0 == gastAtParaList[0].usParaLen)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: gastAtParaList[0].usParaLen err.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSimLockDataReadExReq.ucLayer = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stSimLockDataReadExReq.ucIndex = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      DRV_AGENT_SIMLOCKDATAREADEX_READ_REQ,
                                      &stSimLockDataReadExReq,
                                      (VOS_UINT32)sizeof(stSimLockDataReadExReq),
                                      I0_WUEPS_PID_DRV_AGENT);
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SimLockDataReadExPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SIMLOCKDATAREADEX_READ_SET;

    return AT_WAIT_ASYNC_RETURN;

}

/*****************************************************************************
 函 数 名  : AT_RcvDrvAgentSimlockDataReadExReadCnf
 功能描述  : ^SIMLOCKDATAREADEX命令read回复处理函数
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年04月10日
    作    者   : q00380176
    修改内容   : AP-Modem锁网锁卡项目新增函数

*****************************************************************************/
VOS_UINT32 AT_RcvDrvAgentSimlockDataReadExReadCnf(VOS_VOID *pMsg)
{
    DRV_AGENT_MSG_STRU                         *pRcvMsg;
    DRV_AGENT_SIMLOCKDATAREADEX_READ_CNF_STRU  *pstEvent;
    VOS_UINT32                                  ulResult;
    VOS_UINT32                                  ulLoop;
    VOS_UINT16                                  usLength;
    VOS_UINT8                                   ucIndex;

#if (VOS_OS_VER == VOS_LINUX)
    printk(KERN_ERR "\n AT_RcvDrvAgentSimlockDataReadExReadCnf enter\n");
#endif

    /* 初始化消息变量 */
    usLength = 0;
    ucIndex  = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    pRcvMsg  = (DRV_AGENT_MSG_STRU *)pMsg;
    pstEvent = (DRV_AGENT_SIMLOCKDATAREADEX_READ_CNF_STRU *)pRcvMsg->aucContent;

    ulResult = pstEvent->ulResult;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstEvent->stAtAppCtrl.usClientId,&ucIndex) )
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_SIMLOCKDATAREADEX_READ_SET */
    if ( AT_CMD_SIMLOCKDATAREADEX_READ_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: CmdCurrentOpt ERR.");
        return VOS_ERR;
    }

    /* 判断查询操作是否成功 */
    if ( DRV_AGENT_PERSONALIZATION_NO_ERROR == pstEvent->ulResult)
    {
        /* 输出设置结果 */
        ulResult    = AT_OK;

        /* 添加<layer>,<index>,<total>打印 */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                            "%s:%d,%d,%d,",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            pstEvent->ucLayer,
                                            pstEvent->ucIndex,
                                            pstEvent->ucTotal);

        /* 添加<simlock_data>打印 */
        for (ulLoop = 0; ulLoop < pstEvent->ulDataLen; ulLoop++)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (VOS_CHAR *)pgucAtSndCodeAddr,
                                              (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%02x",
                                               pstEvent->aucData[ulLoop]);
        }
    }
    else
    {
        /* 异常情况, 转换错误码 */
        ulResult    = AT_PERSONALIZATION_ERR_BEGIN + pstEvent->ulResult;
    }

    gstAtSendData.usBufLen = usLength;

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;

}
#endif

/*****************************************************************************
 函 数 名  : AT_SetActPdpStubPara
 功能描述  : ^ACTPDPSTUB
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年11月04日
    作    者   : z00301431
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetActPdpStubPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucFlag;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if((0 == gastAtParaList[0].usParaLen)
    || (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取设置的标志 */
    ucFlag = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* 根据MODEM ID调用不同的桩函数 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        AT_SetPcuiPsCallFlag(ucFlag, AT_CLIENT_TAB_APP_INDEX);
    }
    else if (1 == gastAtParaList[0].ulParaValue)
    {
        AT_SetCtrlPsCallFlag(ucFlag, AT_CLIENT_TAB_APP_INDEX);
    }
    else if (2 == gastAtParaList[0].ulParaValue)
    {
    
   AT_SetPcui2PsCallFlag(ucFlag, AT_CLIENT_TAB_APP_INDEX);
    }
    else
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : AT_SetNVCHKPara
 功能描述  : AT_CMD_NVCHK
 输入参数  : ucIndex --- 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX  --- ATC返回码
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月19日
    作    者   : x00316382
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetNVCHKPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8           ucLoopIndex;

    /* 参数检查 */
    if ( AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if ( 1 != gucAtParaIndex )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 未带参数判断 */
    if(0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果是全部检查，则循环检查全部CRC */
    if ( 0 == gastAtParaList[0].ulParaValue)
    {
        for( ucLoopIndex = 0; ucLoopIndex < 3; ucLoopIndex++ )
        {
            if ( 0 != mdrv_nv_check_factorynv( ucLoopIndex ) )
            {
                return AT_ERROR;
            }
        }

        return AT_OK;
    }

    /* 如果返回0，为正常，非0为异常 */
    if ( 0 == mdrv_nv_check_factorynv( gastAtParaList[0].ulParaValue - 1 ))
    {
        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }

}



/*****************************************************************************
 函 数 名  : AT_RcvMtaAfcClkInfo
 功能描述  : 处理来自mta模块AFC_INFO消息
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : C00299064
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaAfcClkInfoCnf(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                                   ucIndex;
    AT_MTA_MSG_STRU                            *pstMtaMsg    = VOS_NULL_PTR;
    MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF_STRU    *pstAfcCnf    = VOS_NULL_PTR;
    VOS_UINT32                                  ulRet;

    pstMtaMsg = (AT_MTA_MSG_STRU*)pMsg;

    pstAfcCnf = (MTA_AT_QRY_AFC_CLK_FREQ_XOCOEF_CNF_STRU*)pstMtaMsg->aucContent;

    /* 初始化消息变量 */
    ucIndex             = 0;
    ulRet               = AT_OK;

     /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaAfcClkInfoCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaAfcClkInfoCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_AFCCLKINFO_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaAfcClkInfoCnf : Current Option is not AT_CMD_AFCCLKINFO_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR != pstAfcCnf->enResult)
    {
        ulRet = AT_ERROR;
    }
    else
    {
        gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucAtSndCodeAddr,
                                                    "%s: %u,%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    pstAfcCnf->enStatus,
                                                    pstAfcCnf->lDeviation,
                                                    pstAfcCnf->sCoeffStartTemp,
                                                    pstAfcCnf->sCoeffEndTemp,
                                                    pstAfcCnf->aulCoeffMantissa[0],
                                                    pstAfcCnf->aulCoeffMantissa[1],
                                                    pstAfcCnf->aulCoeffMantissa[2],
                                                    pstAfcCnf->aulCoeffMantissa[3],
                                                    pstAfcCnf->ausCoeffExponent[0],
                                                    pstAfcCnf->ausCoeffExponent[1],
                                                    pstAfcCnf->ausCoeffExponent[2],
                                                    pstAfcCnf->ausCoeffExponent[3]);
    }

    /* 输出结果 */
    At_FormatResultData(ucIndex, ulRet);

    return VOS_OK;

}



/*****************************************************************************
 函 数 名  : AT_SetSecureStatePara
 功能描述  : 命令AT^SECURESTATE设置Secure State
 输入参数  : ucIndex    -- AT通道索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : h00360002
    修改内容   : 新增函数
*****************************************************************************/
VOS_UINT32 AT_SetSecureStatePara(VOS_UINT8 ucIndex)
{
    VOS_INT                                 iRst;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数合法性检查 */
    if ( (1 != gucAtParaIndex)
      || (0 == gastAtParaList[0].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 调用底软提供的接口实现设置操作 */
    iRst = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_SET_SECURESTATE,
                            (VOS_INT)gastAtParaList[0].ulParaValue,
                            VOS_NULL_PTR,
                            0);

    /* 根据底软接口返回的结果返回设置结果 */
    /* 底软返回1时表明重复设置,返回0时设置成功,其他情况设置失败 */
    if (AT_EFUSE_REPEAT == iRst)
    {
        return AT_ERROR;
    }
    else if (AT_EFUSE_OK == iRst)
    {
        return AT_OK;
    }
    else
    {
        AT_WARN_LOG("AT_SetSecureStatePara : Set Secure state req failed.");
    }

    return AT_CME_UNKNOWN;
}

/*****************************************************************************
 函 数 名  : AT_SetKcePara
 功能描述  : 命令AT^KCE设置128bit的key值用于image加密
 输入参数  : ucIndex    -- AT通道索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月25日
    作    者   : h00360002
    修改内容   : 新增函数
*****************************************************************************/
VOS_UINT32 AT_SetKcePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                              ulResult;
    VOS_INT                                 iRst;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
       return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数合法性检查 */
    if ( (1 != gucAtParaIndex)
      || (AT_KCE_PARA_LEN != gastAtParaList[0].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* Ascii转字节流 */
    ulResult = At_AsciiNum2HexString(gastAtParaList[0].aucPara, &gastAtParaList[0].usParaLen);

    if ( (AT_SUCCESS != ulResult)
      || (AT_DRV_KCE_LEN != gastAtParaList[0].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 调用底软提供的接口实现设置操作 */
    iRst = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_SET_KCE,
                            0,
                            gastAtParaList[0].aucPara,
                            (VOS_INT)gastAtParaList[0].usParaLen);

    /* 根据底软接口返回的结果返回设置结果 */
    /* 底软返回1时表明重复设置,返回0时设置成功,其他情况设置失败 */
    if (AT_EFUSE_REPEAT == iRst)
    {
        return AT_ERROR;
    }
    else if (AT_EFUSE_OK == iRst)
    {
        return AT_OK;
    }
    else
    {
        AT_WARN_LOG("AT_SetSecureStatePara : Set KCE req failed.");
    }

    return AT_CME_UNKNOWN;
}

/*****************************************************************************
 函 数 名  : At_QrySecureStatePara
 功能描述  : 命令AT^SECURESTATE查询Secure State
 输入参数  : ucIndex    -- AT通道索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月24日
    作    者   : h00360002
    修改内容   : 新增函数
*****************************************************************************/
VOS_UINT32 AT_QrySecureStatePara(VOS_UINT8 ucIndex)
{
    VOS_INT                             iResult;
    VOS_UINT16                          usLength;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usLength = 0;

    /* 调用底软提供的借口查询 */
    iResult = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_GET_SECURESTATE,
                               0,
                               VOS_NULL_PTR,
                               0);

    /* 处理异常查询结果 */
    /* 查询失败 */
    if (AT_SECURE_STATE_NOT_SET > iResult)
    {
        return AT_ERROR;
    }

    /* 查询结果异常 */
    if (AT_SECURE_STATE_RMA < iResult)
    {
        return AT_CME_UNKNOWN;
    }

    /* 打印结果 */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: %d",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                     iResult);

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}

/*****************************************************************************
 函 数 名  : AT_QrySocidPara
 功能描述  : 命令AT^SOCID查询SOCID
 输入参数  : ucIndex    -- AT通道索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月29日
    作    者   : h00360002
    修改内容   : 新增函数
*****************************************************************************/
VOS_UINT32 AT_QrySocidPara(VOS_UINT8 ucIndex)
{
    VOS_INT                                 iResult;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucSocid[AT_DRV_SOCID_LEN];
    VOS_UINT32                              i;

    /* 局部变量初始化 */
    TAF_MEM_SET_S(aucSocid, sizeof(aucSocid), 0x00, AT_DRV_SOCID_LEN);
    usLength = 0;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }



    /* 调用底软提供的借口查询 */

    iResult = mdrv_efuse_ioctl(MDRV_EFUSE_IOCTL_CMD_GET_SOCID,
                               0,
                               aucSocid,
                               AT_DRV_SOCID_LEN);

    /* 处理异常查询结果 */
    if (AT_EFUSE_OK != iResult)
    {
        return AT_ERROR;
    }

    /* 输出结果 */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: ",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* 用16进制输出码流 */
    for (i = 0; i < AT_DRV_SOCID_LEN; i++)
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%02X",
                                          aucSocid[i]);
    }

    gstAtSendData.usBufLen = usLength;
    return AT_OK;
}


/*****************************************************************************
 函 数 名  : AT_SetPdmCtrlPara
 功能描述  : AT_CMD_PDMCTRL
 输入参数  : ucIndex --- 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX  --- ATC返回码
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年10月19日
    作    者   : x00316382
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetPdmCtrlPara(VOS_UINT8 ucIndex)
{
    AT_HPA_PDM_CTRL_REQ_STRU                *pstMsg;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if ( 4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    pstMsg   = (AT_HPA_PDM_CTRL_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR( sizeof(AT_HPA_PDM_CTRL_REQ_STRU) );

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetPdmCtrlPara: alloc msg fail!");
        return AT_ERROR;
    }

    /* 填写消息头 */
    AT_CFG_MSG_HDR( pstMsg, DSP_PID_WPHY, ID_AT_HPA_PDM_CTRL_REQ );

    pstMsg->usMsgID                             = ID_AT_HPA_PDM_CTRL_REQ;
    pstMsg->usRsv                               = 0;
    pstMsg->usPdmRegValue                       = ( VOS_UINT16 )gastAtParaList[0].ulParaValue;
    pstMsg->usPaVbias                           = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usPaVbias2                          = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usPaVbias3                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetPdmCtrlPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt      = AT_CMD_PDM_CTRL;                /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}

/*****************************************************************************
 函 数 名  : AT_SetCtzuPara
 功能描述  : +CTZU
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX  - ATC返回码
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年1月8日
    作    者   : z00301431
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetCtzuPara(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if(0 == gastAtParaList[0].usParaLen)
    {
        g_ulCtzuFlag = 0;
    }
    else
    {
        g_ulCtzuFlag = gastAtParaList[0].ulParaValue;
    }

    return AT_OK;
}

/*****************************************************************************
 函 数 名  : AT_QryCtzuPara
 功能描述  : 查询CTZU
 输入参数  : 无
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年1月8日
    作    者   : z00301431
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_QryCtzuPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                           usLength;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     (VOS_CHAR *)pgucAtSndCodeAddr,
                                     "%s: %d",
                                     g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                     g_ulCtzuFlag);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
/*****************************************************************************
 函 数 名  : AT_SetEvdoSysEvent
 功能描述  : 设置EVOD SYS EVENT,命令格式^DOSYSEVENT=<>
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : AT_OK - 成功
             AT_DEVICE_OTHER_ERROR或 AT_DATA_UNLOCK_ERROR - 失败
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月30日
    作    者   : z00316370
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetEvdoSysEvent(VOS_UINT8 ucIndex)
{
    AT_MTA_EVDO_SYS_EVENT_SET_REQ_STRU  stSysEvent;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stSysEvent, sizeof(stSysEvent), 0x00, sizeof(AT_MTA_EVDO_SYS_EVENT_SET_REQ_STRU));

    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不为1或者字符串长度大于10, 4294967295 = 0xffffffff*/
    if ((1 != gucAtParaIndex)
     || (10 < gastAtParaList[0].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSysEvent.ulDoSysEvent = gastAtParaList[0].ulParaValue;

    ulRslt = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_EVDO_SYS_EVENT_SET_REQ,
                                    &stSysEvent,
                                    sizeof(stSysEvent),
                                    I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRslt)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EVDO_SYS_EVENT_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetEvdoSysEvent: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

}

/*****************************************************************************
 函 数 名  : AT_SetDoSigMask
 功能描述  : 设置EVOD SIG MASK,命令格式^DOSIGMASK=<>
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : AT_OK - 成功
             AT_DEVICE_OTHER_ERROR或 AT_DATA_UNLOCK_ERROR - 失败
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月30日
    作    者   : z00316370
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetDoSigMask(VOS_UINT8 ucIndex)
{
    AT_MTA_EVDO_SIG_MASK_SET_REQ_STRU   stSigMask;
    VOS_UINT32                          ulRslt;

    TAF_MEM_SET_S(&stSigMask, sizeof(stSigMask), 0x00, sizeof(AT_MTA_EVDO_SIG_MASK_SET_REQ_STRU));

    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不为1或者字符串长度大于10, 4294967295 = 0xffffffff*/
    if ((1 != gucAtParaIndex)
     || (10 < gastAtParaList[0].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stSigMask.ulDoSigMask = gastAtParaList[0].ulParaValue;

    ulRslt = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    ID_AT_MTA_EVDO_SIG_MASK_SET_REQ,
                                    &stSigMask,
                                    sizeof(stSigMask),
                                    I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRslt)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_EVDO_SIG_MASK_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetDoSigMask: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

}

/*****************************************************************************
 函 数 名  : AT_RcvMtaEvdoSysEventSetCnf
 功能描述  : 收到MTA设置SYS EVENT的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月30日
    作    者   : Z00316370
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_RcvMtaEvdoSysEventSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSysEventSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSysEventSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EVDO_SYS_EVENT_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSysEventSetCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化AT^MEID命令返回 */
    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaEvdoSigMaskSetCnf
 功能描述  : 收到MTA设置SIG MASK的回复
 输入参数  : pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年12月30日
    作    者   : Z00316370
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_RcvMtaEvdoSigMaskSetCnf(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                            *pRcvMsg         = VOS_NULL_PTR;
    MTA_AT_RESULT_CNF_STRU                     *pstCnf          = VOS_NULL_PTR;
    VOS_UINT32                                  ulResult;
    VOS_UINT8                                   ucIndex;

    /* 初始化 */
    pRcvMsg             = (AT_MTA_MSG_STRU *)pMsg;
    pstCnf              = (MTA_AT_RESULT_CNF_STRU *)pRcvMsg->aucContent;
    ulResult            = AT_OK;
    ucIndex             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigMaskSetCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigMaskSetCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_EVDO_SIG_MASK_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigMaskSetCnf : Current Option is not AT_CMD_LTE_WIFI_COEX_QRY.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化AT^MEID命令返回 */
    gstAtSendData.usBufLen = 0;

    if (pstCnf->enResult != MTA_AT_RESULT_NO_ERROR)
    {
        ulResult = AT_ERROR;
    }
    else
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;

}

/*****************************************************************************
 函 数 名  : AT_RcvMtaEvdoRevARLinkInfoInd
 功能描述  : AT模块收到MTA主动上报的RevA Rlink信息
 输入参数  : pstMsg -- 消息内容
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年01月03日
    作    者   : z00316370
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_RcvMtaEvdoRevARLinkInfoInd(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                                               ucIndex;
    AT_MTA_MSG_STRU                                        *pstMtaMsg       = VOS_NULL_PTR;
    MTA_AT_EVDO_REVA_RLINK_INFO_IND_STRU                   *pstRlinkInfo    = VOS_NULL_PTR;
    VOS_UINT8                                              *pucData;
    VOS_UINT32                                              ulDataLen;
    VOS_UINT32                                              ulRslt;

    /* 初始化消息变量 */
    ucIndex             = 0;
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstRlinkInfo        = (MTA_AT_EVDO_REVA_RLINK_INFO_IND_STRU*)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    ulDataLen               = pstRlinkInfo->ulParaLen * sizeof(VOS_UINT8) * 2 + 3;
    pucData                 = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, ulDataLen);

    if (VOS_NULL_PTR == pucData)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd(): mem alloc Fail!");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(pucData, ulDataLen, 0x00, ulDataLen);
    pucData[ulDataLen - 1]  = '\0';

    ulRslt = AT_HexToAsciiString(pstRlinkInfo->aucContent, &pucData[1], (TAF_UINT16)pstRlinkInfo->ulParaLen);

    pucData[0]              = '"';
    pucData[ulDataLen - 2]  = '"';

    if (ulRslt != AT_OK)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd: WARNING: Hex to Ascii trans fail!");

        PS_MEM_FREE(WUEPS_PID_AT, pucData);

        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DOREVARLINK: %d,%s%s",
                                                    gaucAtCrLf,
                                                    pstRlinkInfo->ulParaLen * 2,
                                                    pucData,
                                                    gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    PS_MEM_FREE(WUEPS_PID_AT, pucData);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaEvdoSigExEventInd
 功能描述  : AT模块收到MTA主动上报的Sig Ex Event信息
 输入参数  : pstMsg -- 消息内容
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年01月03日
    作    者   : z00316370
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_RcvMtaEvdoSigExEventInd(
    VOS_VOID                           *pMsg
)
{
    VOS_UINT8                           ucIndex;
    AT_MTA_MSG_STRU                    *pstMtaMsg       = VOS_NULL_PTR;
    MTA_AT_EVDO_SIG_EXEVENT_IND_STRU   *pstSigExEvent   = VOS_NULL_PTR;
    VOS_UINT8                          *pucData;
    VOS_UINT32                          ulDataLen;
    VOS_UINT32                          ulRslt;

    /* 初始化消息变量 */
    ucIndex             = 0;
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstSigExEvent       = (MTA_AT_EVDO_SIG_EXEVENT_IND_STRU*)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if ( AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex) )
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    ulDataLen               = pstSigExEvent->ulParaLen * sizeof(VOS_UINT8) * 2 + 3;
    pucData                 = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, ulDataLen);

    if (VOS_NULL_PTR == pucData)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd(): mem alloc Fail!");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(pucData, ulDataLen, 0x00, ulDataLen);
    pucData[ulDataLen - 1]  = '\0';

    ulRslt = AT_HexToAsciiString(pstSigExEvent->aucContent, &pucData[1], (TAF_UINT16)pstSigExEvent->ulParaLen);

    pucData[0]              = '"';
    pucData[ulDataLen - 2]  = '"';

    if (ulRslt != AT_OK)
    {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd: WARNING: Hex to Ascii trans fail!");

        PS_MEM_FREE(WUEPS_PID_AT, pucData);

        return VOS_ERR;
    }

    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s^DOSIGEXEVENT: %d,%s%s",
                                                    gaucAtCrLf,
                                                    pstSigExEvent->ulParaLen * 2,
                                                    pucData,
                                                    gaucAtCrLf);

    PS_MEM_FREE(WUEPS_PID_AT, pucData);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

#endif


/*****************************************************************************
 函 数 名  : AT_SetFratIgnitionPara
 功能描述  : ^FratIgnition设置命令处理函数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : c00318887
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetFratIgnitionPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulRst;
    AT_MTA_FRAT_IGNITION_ENUM_UNIT8     enFratIgnitionSta;

    /* 参数检查 */
    if ( AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数目检查 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ucFratIgnitionSta 取值错误 */
    if ((gastAtParaList[0].ulParaValue >= AT_MTA_FRAT_IGNITION_STATT_BUTT)
     || (0 == gastAtParaList[0].usParaLen))
    {
        AT_WARN_LOG1("AT_SetFratIgnitionPara: para err", gastAtParaList[0].ulParaValue);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    enFratIgnitionSta = (AT_MTA_FRAT_IGNITION_ENUM_UNIT8)gastAtParaList[0].ulParaValue;

    /* 发送消息 ID_AT_MTA_FRAT_IGNITION_SET_REQ 给MTA处理，该消息带参数(VOS_UINT8)gastAtParaList[0].ulParaValue */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_FRAT_IGNITION_SET_REQ,
                                   (VOS_VOID *)&enFratIgnitionSta,
                                   sizeof(enFratIgnitionSta),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FRATIGNITION_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetFratIgnitionPara: send ReqMsg fail");
        return AT_ERROR;
    }
}

/*****************************************************************************
 函 数 名  : AT_TimeParaYTDCheck
 功能描述  : ^SETTIME设置命令输入参数的检查函数
 输入参数  : AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年6月12日
    作    者   : LWX331495
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_TimeParaYTDCheck(AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];

    /* 参数个数或者VER长度不正确 */
    if (AT_MODEM_YTD_LEN != gastAtParaList[0].usParaLen)
    {
       AT_ERR_LOG("AT_TimeParaYTDCheck: length of YTD parameter is error.");
       return VOS_ERR;
    }

    /* 按照格式 YYYY/MM/DD 解析年月日，并判断格式、范围 */
    if ((gastAtParaList[0].aucPara[4] != '/')
     || (gastAtParaList[0].aucPara[7] != '/'))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The date formats parameter is error.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[0].aucPara, AT_MODEM_YEAR_LEN);
    pstAtMtaModemTime->lYear = (VOS_INT32)AT_AtoI(aucBuffer);

    if ((pstAtMtaModemTime->lYear > AT_MODEM_YEAR_MAX)
     || (pstAtMtaModemTime->lYear < AT_MODEM_YEAR_MIN))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The parameter of year is out of range");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[5], AT_MODEM_MONTH_LEN);
    pstAtMtaModemTime->lMonth = (VOS_INT32)AT_AtoI(aucBuffer);

    if ((pstAtMtaModemTime->lMonth > AT_MODEM_MONTH_MAX)
     || (pstAtMtaModemTime->lMonth < AT_MODEM_MONTH_MIN))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The parameter of month is out of range");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[0].aucPara[8], AT_MODEM_DATE_LEN);
    pstAtMtaModemTime->lDay = (VOS_INT32)AT_AtoI(aucBuffer);

    if ((pstAtMtaModemTime->lDay > AT_MODEM_DAY_MAX)
     || (pstAtMtaModemTime->lDay < AT_MODEM_DAY_MIN))
    {
        AT_ERR_LOG("AT_TimeParaYTDCheck: The parameter of day is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_TimeParaTimeCheck
 功能描述  : ^SETTIME设置命令输入参数的检查函数
 输入参数  : AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年6月12日
    作    者   : LWX331495
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_TimeParaTimeCheck(AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];

    if (AT_MODEM_TIME_LEN != gastAtParaList[1].usParaLen)
    {
       AT_ERR_LOG("AT_TimeParaTimeCheck: length of time parameter is error.");
       return VOS_ERR;
    }

    /* 按照格式 HH:MM:SS 解析时间，并判断格式、范围 */
    if ((gastAtParaList[1].aucPara[2] != ':')
     || (gastAtParaList[1].aucPara[5] != ':'))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The time formats parameter is error.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[1].aucPara, AT_MODEM_HOUR_LEN);
    pstAtMtaModemTime->lHour = (VOS_INT32)AT_AtoI(aucBuffer);

    if ((pstAtMtaModemTime->lHour > AT_MODEM_HOUR_MAX)
     || (pstAtMtaModemTime->lHour < AT_MODEM_HOUR_MIN))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The parameter of hour is out of range");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[1].aucPara[3], AT_MODEM_MIN_LEN);
    pstAtMtaModemTime->lMin = (VOS_INT32)AT_AtoI(aucBuffer);

    if ((pstAtMtaModemTime->lMin > AT_MODEM_MIN_MAX)
     || (pstAtMtaModemTime->lMin < AT_MODEM_MIN_MIN))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The parameter of min is out of range");
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), &gastAtParaList[1].aucPara[6], AT_MODEM_SEC_LEN);
    pstAtMtaModemTime->lSec = (VOS_INT32)AT_AtoI(aucBuffer);

    if ((pstAtMtaModemTime->lSec > AT_MODEM_SEC_MAX)
     || (pstAtMtaModemTime->lSec < AT_MODEM_SEC_MIN))
    {
        AT_ERR_LOG("AT_TimeParaTimeCheck: The parameter of second is out of range");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_TimeParaZoneCheck
 功能描述  : ^SETTIME设置命令输入参数的检查函数
 输入参数  : AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年6月12日
    作    者   : LWX331495
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_TimeParaZoneCheck(AT_MTA_MODEM_TIME_STRU *pstAtMtaModemTime)
{
    VOS_UINT8                           aucBuffer[AT_GET_MODEM_TIME_BUFF_LEN];

    if ( (0                          == gastAtParaList[2].usParaLen)
      || (AT_GET_MODEM_TIME_BUFF_LEN <= gastAtParaList[2].usParaLen) )
    {
        AT_ERR_LOG1("AT_TimeParaZoneCheck: length of zone parameter is wrong.", gastAtParaList[2].usParaLen);
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), 0x00, (VOS_SIZE_T)sizeof(aucBuffer));
    TAF_MEM_CPY_S(aucBuffer, (VOS_SIZE_T)sizeof(aucBuffer), gastAtParaList[2].aucPara, gastAtParaList[2].usParaLen);

    /* 时区范围是[-12, 12] */
    if (VOS_ERR == AT_AtoInt(aucBuffer, &pstAtMtaModemTime->lZone))
    {
        return VOS_ERR;
    }

    if ((pstAtMtaModemTime->lZone > AT_MODEM_ZONE_MAX)
    ||  (pstAtMtaModemTime->lZone < AT_MODEM_ZONE_MIN))
    {
        AT_ERR_LOG("AT_TimeParaZoneCheck: The parameter of zone is out of range.");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_SetModemTimePara
 功能描述  : ^SETTIME设置命令处理函数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年6月12日
    作    者   : LWX331495
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetModemTimePara(VOS_UINT8 ucIndex)
{
    AT_MTA_MODEM_TIME_STRU              stAtMtaModemTime;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if ( AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数目检查 */
    if (gucAtParaIndex != 3)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数格式检查 */
    TAF_MEM_SET_S(&stAtMtaModemTime, (VOS_SIZE_T)sizeof(stAtMtaModemTime), 0x00, (VOS_SIZE_T)sizeof(stAtMtaModemTime));

    /* 检查年月日 */
    if (VOS_ERR == AT_TimeParaYTDCheck(&stAtMtaModemTime))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查时间*/
    if (VOS_ERR == AT_TimeParaTimeCheck(&stAtMtaModemTime))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查时区 */
    if (VOS_ERR == AT_TimeParaZoneCheck(&stAtMtaModemTime))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息 ID_AT_MTA_MODEM_TIME_SET_REQ 给MTA处理，该消息带参数 stAtMtaModemTime */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_MODEM_TIME_SET_REQ,
                                   (VOS_VOID *)&stAtMtaModemTime,
                                   (VOS_SIZE_T)sizeof(stAtMtaModemTime),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MODEM_TIME_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        AT_WARN_LOG("AT_SetModemTimePara: send ReqMsg fail");
        return AT_ERROR;
    }
}

/*****************************************************************************
 函 数 名  : AT_SetPhyComCfg
 功能描述  : ^PHYCOMCFG 处理AT命令设置PHY通用配置
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年01月24日
    作    者   : xwx377961
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetPhyComCfgPara(VOS_UINT8 ucIndex)
{
    AT_MTA_PHY_COM_CFG_SET_REQ_STRU     stPhyComCfg;
    VOS_UINT32                          ulResult;

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stPhyComCfg, (VOS_SIZE_T)sizeof(stPhyComCfg), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_PHY_COM_CFG_SET_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetPhyComCfg : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 判断参数数量 */
    if ((gucAtParaIndex < 3) || (gucAtParaIndex > 5))
    {
        /* 参数数量错误 */
        AT_WARN_LOG("AT_SetPhyComCfg : Current Number wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 消息赋值 */
    stPhyComCfg.usCmdType               = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stPhyComCfg.usRatBitmap             = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stPhyComCfg.ulPara1                 = gastAtParaList[2].ulParaValue;
    
    if (4 == gucAtParaIndex)
    {
        stPhyComCfg.ulPara2             = gastAtParaList[3].ulParaValue;
    }  
    else
    {
        stPhyComCfg.ulPara2             = gastAtParaList[3].ulParaValue;
        stPhyComCfg.ulPara3             = gastAtParaList[4].ulParaValue;
    }

    /* 发送消息给MTA */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_PHY_COM_CFG_SET_REQ,
                                      &stPhyComCfg,
                                      (VOS_SIZE_T)sizeof(stPhyComCfg),
                                      I0_UEPS_PID_MTA);

    /* 发送失败 */
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetPhyComCfg: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PHY_COM_CFG_SET;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaPhyComCfgSetCnf
 功能描述  : 处理MTA回复At的phy通用配置设置结果
 输入参数  : VOS_VOID *pMsg 返回的消息指针
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年01月24日
    作    者   : xwx377961
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaPhyComCfgSetCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg       = VOS_NULL_PTR;
    MTA_AT_PHY_COM_CFG_SET_CNF_STRU    *pstPhyComCfgCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /*初始化局部变量*/
    pstRcvMsg                           = (AT_MTA_MSG_STRU *)pMsg;
    pstPhyComCfgCnf                     = (MTA_AT_PHY_COM_CFG_SET_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex                             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaPhyComCfgSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaPhyComCfgSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_PHY_COM_CFG_SET */
    if (AT_CMD_PHY_COM_CFG_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaPhyComCfgSetCnf: NOT CURRENT CMD OPTION!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    gstAtSendData.usBufLen = 0;

    /* 格式化上报命令 */
    if (MTA_AT_RESULT_NO_ERROR != pstPhyComCfgCnf->enResult)
    {
        /* 命令结果 *AT_ERROR */
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        /* 命令结果 *AT_OK */
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_SetRxTestModePara
 功能描述  : ^RXTESTMODE
 输入参数  : ucIndex - 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年07月07日
    作    者   : l00373346
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 AT_SetRxTestModePara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_RXTESTMODE_REQ_STRU      stRxTestModeCfg;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stRxTestModeCfg, (VOS_SIZE_T)sizeof(stRxTestModeCfg), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_SET_RXTESTMODE_REQ_STRU));

    /* 参数检查 */
    if ( AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType )
    {
        AT_WARN_LOG("AT_SetRxTestModePara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if ( 1 != gucAtParaIndex )
    {
        AT_WARN_LOG("AT_SetRxTestModePara : The number of input parameters is error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if( 0 == gastAtParaList[0].usParaLen )
    {
        AT_WARN_LOG("AT_SetRxTestModePara : The number of input parameters is zero.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    stRxTestModeCfg.enCfg  = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 发送跨核消息到C核, 设置侦听测试模式 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_RX_TEST_MODE_SET_REQ,
                                   &stRxTestModeCfg,
                                   (VOS_SIZE_T)sizeof(stRxTestModeCfg),
                                   I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetRxTestModePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_RXTESTMODE_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaSetRxTestModeCnf
 功能描述  : AT模块收到MTA回复的设置侦听测试模式消息的处理函数
 输入参数  : VOS_VOID *pMsg - 收到MTA的CNF消息
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年07月07日
    作    者   : l00373346
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaSetRxTestModeCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg;
    MTA_AT_SET_RXTESTMODE_CNF_STRU     *pstSetCnf;
    VOS_UINT8                           ucIndex;
    VOS_UINT32                          ulResult;

    /* 初始化 */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_RXTESTMODE_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetRxTestModeCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetRxTestModeCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_RXTESTMODE_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetRxTestModeCnf : Current Option is not AT_CMD_RXTESTMODE_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR == pstSetCnf->enResult)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_SetMipiWrParaEx
 功能描述  : 处理at^MIPIWREX的AT命令
 输入参数  : VOS_UINT8                           ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年01月16日
    作    者   : xwx377961
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetMipiWrParaEx(VOS_UINT8 ucIndex)
{
    AT_MTA_MIPI_WREX_REQ_STRU           stMipiWrEx;
    VOS_UINT32                          ulResult;

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stMipiWrEx, (VOS_SIZE_T)sizeof(stMipiWrEx), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_MIPI_WREX_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数量不对 */
    if (6 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx : Current Number wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* extend_flag 为 0 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        /* byte_cnt 如果不为1 或者 reg_addr 不在0-31之间,数值大于0XFF,直接返回错误 */
        if ((1 != gastAtParaList[4].ulParaValue)
         || (gastAtParaList[3].ulParaValue > 31)
         || (gastAtParaList[5].ulParaValue > 0xFF))
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 0, byte_cnt or reg_addr or value wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stMipiWrEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
        stMipiWrEx.ulMipiId             = gastAtParaList[1].ulParaValue;
        stMipiWrEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
        stMipiWrEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
        stMipiWrEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
        stMipiWrEx.ulValue              = gastAtParaList[5].ulParaValue;
    }
    /* extend_flag 为 1 */
    else if (1 == gastAtParaList[0].ulParaValue)
    {
        /* reg_addr 不在0-255之间, 直接返回错误 */
        if ((1 == gastAtParaList[4].ulParaValue)
         && (gastAtParaList[5].ulParaValue > 0xFF))
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 1, byte_cnt is 1, value wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if ((2 == gastAtParaList[4].ulParaValue)
         && (gastAtParaList[5].ulParaValue > 0xFFFF))
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 1, byte_cnt is 2, value wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (gastAtParaList[3].ulParaValue > 255)
        {
            AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag is 1, reg_addr wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stMipiWrEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
        stMipiWrEx.ulMipiId             = gastAtParaList[1].ulParaValue;
        stMipiWrEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
        stMipiWrEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
        stMipiWrEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
        stMipiWrEx.ulValue              = gastAtParaList[5].ulParaValue;
    }
    else
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx : extend_flag wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息给MTA */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MIPI_WREX_REQ,
                                      &stMipiWrEx,
                                      (VOS_SIZE_T)sizeof(stMipiWrEx),
                                      I0_UEPS_PID_MTA);

    /* 发送失败 */
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetMipiWrParaEx: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPI_WREX;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMiPiWrEXCnf
 功能描述  : 处理MTA回复At的MiPi写入结果
 输入参数  : VOS_VOID *pMsg 返回的消息指针
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年01月16日
    作    者   : xwx377961
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMipiWrEXCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg      = VOS_NULL_PTR;
    MTA_AT_MIPI_WREX_CNF_STRU          *pstMipiWrEXCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* 初始化局部变量 */
    pstRcvMsg                           = (AT_MTA_MSG_STRU *)pMsg;
    pstMipiWrEXCnf                      = (MTA_AT_MIPI_WREX_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex                             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMiPiWrEXCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMiPiWrEXCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_MIPI_WREX */
    if (AT_CMD_MIPI_WREX != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMiPiWrEXCnf: NOT CURRENT CMD OPTION!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化上报命令 */
    if (MTA_AT_RESULT_NO_ERROR != pstMipiWrEXCnf->enResult)
    {
        /* 命令结果 *AT_ERROR */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        /* 命令结果 *AT_OK */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_SetMipiRdParaEx
 功能描述  : 处理at^MIPIRDEX的AT命令
 输入参数  : VOS_UINT8                           ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年01月16日
    作    者   : xwx377961
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetMipiRdParaEx(VOS_UINT8 ucIndex)
{
    AT_MTA_MIPI_RDEX_REQ_STRU           stMipiRdEx;
    VOS_UINT32                          ulResult;

    /* 局部变量初始化 */
    TAF_MEM_SET_S(&stMipiRdEx, (VOS_SIZE_T)sizeof(stMipiRdEx), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_MIPI_RDEX_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数量不对 */
    if (5 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx : Current Numbers Wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* extend_flag 为 0 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        /* byte_cnt 如果不为1 或者 reg_addr 不在0-31之间, 直接返回错误 */
        if ((1 != gastAtParaList[4].ulParaValue)
         || (gastAtParaList[3].ulParaValue > 31))
        {
            AT_WARN_LOG("AT_SetMipiRdParaEx : extend_flag is 0, byte_cnt or reg_addr wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stMipiRdEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
        stMipiRdEx.ulMipiId             = gastAtParaList[1].ulParaValue;
        stMipiRdEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
        stMipiRdEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
        stMipiRdEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
    }
    /* extend_flag 为 1 */
    else if (1 == gastAtParaList[0].ulParaValue)
    {
        /* reg_addr 不在0-255之间, 直接返回错误 */
        if (gastAtParaList[3].ulParaValue > 255)
        {
            AT_WARN_LOG("AT_SetMipiRdParaEx : extend_flag is 1, reg_addr wrong.");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stMipiRdEx.ulExtendFlag         = gastAtParaList[0].ulParaValue;
        stMipiRdEx.ulMipiId             = gastAtParaList[1].ulParaValue;
        stMipiRdEx.ulSlaveId            = gastAtParaList[2].ulParaValue;
        stMipiRdEx.ulRegAddr            = gastAtParaList[3].ulParaValue;
        stMipiRdEx.ulByteCnt            = gastAtParaList[4].ulParaValue;
    }
    else
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx : extend_flag wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息给MTA */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MIPI_RDEX_REQ,
                                      &stMipiRdEx,
                                      (VOS_SIZE_T)sizeof(stMipiRdEx),
                                      I0_UEPS_PID_MTA);

    /* 发送失败 */
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetMipiRdParaEx: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 发送成功，设置当前操作模式 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPI_RDEX;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 等待异步处理时间返回 */
    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaMiPiWrEXCnf
 功能描述  : 处理MTA回复At的MiPi读取结果
 输入参数  : VOS_VOID *pMsg 返回的消息指针
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年01月16日
    作    者   : xwx377961
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaMipiRdEXCnf(VOS_VOID *pMsg)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg      = VOS_NULL_PTR;
    MTA_AT_MIPI_RDEX_CNF_STRU          *pstMiPiRdEXCnf = VOS_NULL_PTR;
    VOS_UINT16                          ulLength;
    VOS_UINT8                           ucIndex;

    /* 初始化局部变量 */
    pstRcvMsg                           = (AT_MTA_MSG_STRU *)pMsg;
    pstMiPiRdEXCnf                      = (MTA_AT_MIPI_RDEX_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex                             = 0;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMiPiRdEXCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaMiPiRdEXCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_MIPI_WREX */
    if (AT_CMD_MIPI_RDEX != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaMiPiRdEXCnf: NOT CURRENT CMD OPTION!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化上报命令 */
    if (MTA_AT_RESULT_NO_ERROR != pstMiPiRdEXCnf->enResult)
    {
        /* 命令结果 *AT_ERROR */
        gstAtSendData.usBufLen = 0;
        At_FormatResultData(ucIndex, AT_ERROR);
    }
    else
    {
        /* 命令结果 *AT_OK */
        ulLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          "%s:%d",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                          pstMiPiRdEXCnf->ulValue);
        gstAtSendData.usBufLen = ulLength;
        At_FormatResultData(ucIndex, AT_OK);
    }

    return VOS_OK;
}

/* Added by c00380008 for WIFI 共天线&VOLTE视频调速, 2016-08-22, begin */
/*****************************************************************************
 函 数 名  : AT_SetCrrconnPara
 功能描述  : ^CRRCONN=<enable>,设置^CRRCONN的命令参数
 输入参数  : ucIndex --- 端口索引
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年8月22日
    作    者   : c00380008
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetCrrconnPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_CRRCONN_REQ_STRU         stSetCrrconn;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stSetCrrconn, sizeof(stSetCrrconn), 0x00, sizeof(AT_MTA_SET_CRRCONN_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数赋值 */
    stSetCrrconn.enEnable = (AT_MTA_CFG_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 发送跨核消息到C核，设置CRRCONN主动上报设置 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_CRRCONN_SET_REQ,
                                   &stSetCrrconn,
                                   sizeof(stSetCrrconn),
                                   I0_UEPS_PID_MTA);
    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetCrrconnPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CRRCONN_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_QryCrrconnPara
 功能描述  : ^CRRCONN?
 输入参数  : ucIndex --- 端口索引
 输出参数  : 无
 返 回 值  : AT_XXX
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年8月22日
    作    者   : c00380008
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_QryCrrconnPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;

    if(AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* AT 给MTA 发送查询请求消息 */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_CRRCONN_QRY_REQ,
                                      VOS_NULL_PTR,
                                      0,
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_QryCrrconnPara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CRRCONN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaSetCrrconnCnf
 功能描述  : AT模块收到MTA回复的消息处理函数
 输入参数  : pstMsg -- 消息内容
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年08月22日
    作    者   : c00380008
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_RcvMtaSetCrrconnCnf(
    VOS_VOID                        *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg;
    MTA_AT_SET_CRRCONN_CNF_STRU        *pstSetCnf;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* 初始化 */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_CRRCONN_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetCrrconnCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetCrrconnCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_CRRCONN_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetCrrconnCnf : Current Option is not AT_CMD_CRRCONN_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR == pstSetCnf->enResult)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaQryCrrconnCnf
 功能描述  : AT模块处理查询后返回的内容
 输入参数  : pstMsg -- 消息内容
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年08月23日
    作    者   : c00380008
    修改内容   : 新增
*****************************************************************************/
VOS_UINT32 AT_RcvMtaQryCrrconnCnf(
    VOS_VOID                        *pMsg
)
{
    /* 定义局部变量 */
    AT_MTA_MSG_STRU                  *pstMtaMsg         = VOS_NULL_PTR;
    MTA_AT_QRY_CRRCONN_CNF_STRU      *pstQryCrrconnCnf  = VOS_NULL_PTR;
    VOS_UINT32                        ulResult;
    VOS_UINT8                         ucIndex;

    /* 初始化消息变量 */
    pstMtaMsg           = (AT_MTA_MSG_STRU*)pMsg;
    pstQryCrrconnCnf    = (MTA_AT_QRY_CRRCONN_CNF_STRU*)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryCrrconnCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaQryCrrconnCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CRRCONN_QRY */
    if (AT_CMD_CRRCONN_QRY != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaQryCrrconnCnf: WARNING:Not AT_CMD_CRRCONN_QRY!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 判断查询操作是否成功 */
    if (MTA_AT_RESULT_NO_ERROR == pstQryCrrconnCnf->enResult)
    {
        ulResult                = AT_OK;
        gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s: %d,%d,%d,%d",
                                            g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            pstQryCrrconnCnf->enEnable,
                                            pstQryCrrconnCnf->ucStatus0,
                                            pstQryCrrconnCnf->ucStatus1,
                                            pstQryCrrconnCnf->ucStatus2);
    }
    else
    {
        ulResult                = AT_ERROR;
        gstAtSendData.usBufLen  = 0;
    }

    /* 调用AT_FormatResultData发送命令结果 */
    At_FormatResultData(ucIndex, ulResult);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaCrrconnStatusInd
 功能描述  : At模块收到MTA模块上报消息处理函数
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年08月23日
    作    者   : c00380008
    修改内容   : 新增函数
*****************************************************************************/
VOS_UINT32 AT_RcvMtaCrrconnStatusInd(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_CRRCONN_STATUS_IND_STRU     *pstCrrconnStatusInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* 初始化消息变量 */
    ucIndex                = 0;
    pstMtaMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstCrrconnStatusInd    = (MTA_AT_CRRCONN_STATUS_IND_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaCrrconnStatusInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s^CRRCONN: %d,%d,%d%s",
                                            gaucAtCrLf,
                                            pstCrrconnStatusInd->ucStatus0,
                                            pstCrrconnStatusInd->ucStatus1,
                                            pstCrrconnStatusInd->ucStatus2,
                                            gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_SetVtrlqualrptPara
 功能描述  : 设置^VTRLQUALRPT命令参数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年8月30日
    作    者   : c00380008
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_SetVtrlqualrptPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_VTRLQUALRPT_REQ_STRU     stSetVtrlqualrpt;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stSetVtrlqualrpt, sizeof(stSetVtrlqualrpt), 0x00, sizeof(AT_MTA_SET_VTRLQUALRPT_REQ_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数赋值 */
    stSetVtrlqualrpt.ulEnable       = gastAtParaList[0].ulParaValue;
    stSetVtrlqualrpt.ulThreshold    = gastAtParaList[1].ulParaValue;

    /* 发送跨核消息到C核 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   gastAtClientTab[ucIndex].opId,
                                   ID_AT_MTA_VTRLQUALRPT_SET_REQ,
                                   &stSetVtrlqualrpt,
                                   (VOS_SIZE_T)sizeof(stSetVtrlqualrpt),
                                   I0_UEPS_PID_MTA);
    if (TAF_SUCCESS != ulRst)
    {
        AT_WARN_LOG("AT_SetVtrlqualrptPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VTRLQUALRPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaSetVtrlqualrptCnf
 功能描述  : AT模块收到MTA回复的消息处理函数
 输入参数  : VOS_VOID                        *pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年8月30日
    作    者   : c00380008
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaSetVtrlqualrptCnf(
    VOS_VOID                        *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstRcvMsg;
    MTA_AT_SET_VTRLQUALRPT_CNF_STRU    *pstSetCnf;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIndex;

    /* 初始化 */
    pstRcvMsg    = (AT_MTA_MSG_STRU *)pMsg;
    pstSetCnf    = (MTA_AT_SET_VTRLQUALRPT_CNF_STRU *)pstRcvMsg->aucContent;
    ucIndex      = 0;
    ulResult     = AT_ERROR;

    /* 通过clientid获取index */
    if (AT_FAILURE == At_ClientIdToUserId(pstRcvMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetVtrlqualrptCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IS_BROADCAST_CLIENT_INDEX(ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaSetVtrlqualrptCnf : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (AT_CMD_VTRLQUALRPT_SET != gastAtClientTab[ucIndex].CmdCurrentOpt)
    {
        AT_WARN_LOG("AT_RcvMtaSetVtrlqualrptCnf : Current Option is not AT_CMD_VTRLQUALRPT_SET.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_STOP_TIMER_CMD_READY(ucIndex);

    /* 格式化命令返回 */
    gstAtSendData.usBufLen = 0;

    if (MTA_AT_RESULT_NO_ERROR == pstSetCnf->enResult)
    {
        ulResult = AT_OK;
    }

    At_FormatResultData(ucIndex, ulResult);
    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaRlQualityInfoInd
 功能描述  : At模块收到MTA模块上报消息处理函数
 输入参数  : VOS_VOID                           *pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年8月31日
    作    者   : c00380008
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaRlQualityInfoInd(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_RL_QUALITY_INFO_IND_STRU    *pstRlQualityInfoInd = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* 初始化消息变量 */
    ucIndex                = 0;
    pstMtaMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstRlQualityInfoInd    = (MTA_AT_RL_QUALITY_INFO_IND_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaRlQualityInfoInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s^LTERLQUALINFO: %d,%d,%d,%d%s",
                                            gaucAtCrLf,
                                            pstRlQualityInfoInd->sRsrp,
                                            pstRlQualityInfoInd->sRsrq,
                                            pstRlQualityInfoInd->sRssi,
                                            pstRlQualityInfoInd->usBler,
                                            gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : AT_RcvMtaVideoDiagInfoRpt
 功能描述  : At模块收到MTA模块上报消息处理函数
 输入参数  : VOS_VOID                           *pMsg
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年8月31日
    作    者   : c00380008
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_RcvMtaVideoDiagInfoRpt(
    VOS_VOID                           *pMsg
)
{
    AT_MTA_MSG_STRU                    *pstMtaMsg           = VOS_NULL_PTR;
    MTA_AT_VIDEO_DIAG_INFO_RPT_STRU    *pstVideoDiagInfoRpt = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    /* 初始化消息变量 */
    ucIndex                = 0;
    pstMtaMsg              = (AT_MTA_MSG_STRU *)pMsg;
    pstVideoDiagInfoRpt    = (MTA_AT_VIDEO_DIAG_INFO_RPT_STRU *)pstMtaMsg->aucContent;

    /* 通过ClientId获取ucIndex */
    if (AT_FAILURE == At_ClientIdToUserId(pstMtaMsg->stAppCtrl.usClientId, &ucIndex))
    {
        AT_WARN_LOG("AT_RcvMtaVideoDiagInfoRpt: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    gstAtSendData.usBufLen  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s^LPDCPINFORPT: %u,%u,%u,%u%s",
                                            gaucAtCrLf,
                                            pstVideoDiagInfoRpt->ulCurrBuffTime,
                                            pstVideoDiagInfoRpt->ulCurrBuffPktNum,
                                            pstVideoDiagInfoRpt->ulMacUlThrput,
                                            pstVideoDiagInfoRpt->ulMaxBuffTime,
                                            gaucAtCrLf);

    At_SendResultData(ucIndex, pgucAtSndCodeAddr, gstAtSendData.usBufLen);

    return VOS_OK;
}

/* Added by c00380008 for WIFI 共天线&VOLTE视频调速, 2016-08-22, end */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

