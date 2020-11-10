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
#include  "AtTimer.h"
#include  "ATCmdProc.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define    THIS_FILE_ID        PS_FILE_ID_AT_TIMER_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


/*****************************************************************************
 函 数 名  : AT_StartRelTimer
 功能描述  : AT启动REL定时器函数
 输入参数  : HTIMER *phTm
             VOS_PID Pid
             VOS_UINT32 ulLength
             VOS_UINT32 ulName
             VOS_UINT32 ulParam
             VOS_UINT8 ucMode
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年10月5日
    作    者   : 鲁琳/l60609
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_StartRelTimer(
    HTIMER                             *phTm,
    VOS_UINT32                          ulLength,
    VOS_UINT32                          ulName,
    VOS_UINT32                          ulParam,
    VOS_UINT8                           ucMode
)
{
    VOS_UINT32                          ulRet;
    AT_TIMER_OPERATION_STRU             stTimer;

    /* Added by L60609 for AT Project，2011-10-22,  Begin*/
    if (0 == ulLength)
    {
        AT_ERR_LOG("AT_StartRelTimer: Start Rel Timer Length is 0!");
        return VOS_ERR;
    }
    /* Added by L60609 for AT Project，2011-10-22,  End*/

    if ( ulLength >= VOS_TIMER_MAX_LENGTH )
    {
        ulLength = VOS_TIMER_MAX_LENGTH - 1;
    }

    ulRet = VOS_StartRelTimer(phTm, WUEPS_PID_AT, ulLength, ulName, ulParam, ucMode, VOS_TIMER_PRECISION_5);

    if (VOS_ERR == ulRet)
    {
        AT_ERR_LOG("AT_StartRelTimer: Start Rel Timer Fail!");
    }

    stTimer.ulSenderCpuId   = VOS_LOCAL_CPUID;
    stTimer.ulSenderPid     = WUEPS_PID_AT;
    stTimer.ulReceiverCpuId = VOS_LOCAL_CPUID;
    stTimer.ulReceiverPid   = VOS_PID_TIMER;
    stTimer.ulLength        = sizeof(AT_TIMER_OPERATION_STRU) - VOS_MSG_HEAD_LENGTH;
    stTimer.ulMsgName       = ulName;
    stTimer.enTimeAction    = AT_TIMER_OPERATION_START;
    stTimer.ulTimeLen       = ulLength;

    DIAG_TraceReport(&stTimer);

    return ulRet;
}

/*****************************************************************************
 函 数 名  : AT_StopRelTimer
 功能描述  : AT停止REL定时器函数
 输入参数  : VOS_UINT32 ulName
             HTIMER *phTm
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年10月5日
    作    者   : 鲁琳/l60609
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 AT_StopRelTimer(
    VOS_UINT32                          ulName,
    HTIMER                             *phTm
)
{
    VOS_UINT32                          ulRet;
    AT_TIMER_OPERATION_STRU             stTimer;

    ulRet = VOS_StopRelTimer(phTm);

    if (VOS_ERR == ulRet)
    {
        AT_ERR_LOG("AT_StopRelTimer: Stop Rel Timer Fail!");
    }

    stTimer.ulSenderCpuId   = VOS_LOCAL_CPUID;
    stTimer.ulSenderPid     = WUEPS_PID_AT;
    stTimer.ulReceiverCpuId = VOS_LOCAL_CPUID;
    stTimer.ulReceiverPid   = VOS_PID_TIMER;
    stTimer.ulLength        = sizeof(AT_TIMER_OPERATION_STRU) - VOS_MSG_HEAD_LENGTH;
    stTimer.ulMsgName       = ulName;
    stTimer.enTimeAction    = AT_TIMER_OPERATION_STOP;
    stTimer.ulTimeLen       = 0x0;

    DIAG_TraceReport(&stTimer);

    return ulRet;
}

/*****************************************************************************
 函 数 名  : At_StartTimer
 功能描述  : AT启动定时器
 输入参数  : TAF_UINT32 usLength
             TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : TAF_UINT32
 调用函数  :
 被调函数  :

 修改历史      :

  1.Date        : 2005-04-19
    Author      : ---
    Modification: Created function
  2.日    期   : 2009-03-25
    作    者   : s62952
    修改内容   : 问题单号:AT2D10008
  3.日    期   : 2011年10月5日
    作    者   : 鲁琳/l60609
    修改内容   : AT Project:定时器由AT内部实现

*****************************************************************************/
VOS_UINT32 At_StartTimer(
    VOS_UINT32                          usLength,
    VOS_UINT8                           ucIndex
)
{
    VOS_StopRelTimer(&gastAtClientTab[ucIndex].hTimer);
    if(VOS_OK != AT_StartRelTimer(&gastAtClientTab[ucIndex].hTimer,usLength,ucIndex,0,VOS_RELTIMER_NOLOOP))
    {
        AT_ERR_LOG("At_StartTimer:ERROR:Start Timer");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}








#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
