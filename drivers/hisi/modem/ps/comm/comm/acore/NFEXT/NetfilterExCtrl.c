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

/******************************************************************************
   1 头文件包含
******************************************************************************/
#include "v_typdef.h"
#include "PsTypeDef.h"
#include "IpsMntn.h"
#include "TtfOamInterface.h"
#include "TTFComm.h"
#include "NetfilterEx.h"
#include "TTFUtil.h"

#ifdef STATIC
#undef STATIC
#endif

#ifdef VOS_WIN32
#define STATIC
#else
#define STATIC static
#endif

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_ACPU_NFEX_CTRL_C


/*****************************************************************************
  2 宏定义
*****************************************************************************/

/*****************************************************************************
  3 全局变量声明
*****************************************************************************/
NF_EXT_ENTITY_STRU                  g_stExEntity            = {0};
VOS_UINT32                          g_ulNFExtTaskId         = 0;
VOS_UINT32                          g_ulNFExtInitFlag       = 0;

#if(NF_EXT_DBG == DBG_ON)
NF_EXT_STATS_STRU                   g_stNfExtStats = {{0}};
#endif

#if (FEATURE_ON == FEATURE_NFEXT)
NF_EXT_FLOW_CTRL_ENTITY             g_stExFlowCtrlEntity;
NF_EXT_NV_STRU                      g_stNfExtNv;
NF_EXT_HOOK_MASK_NV_STRU            g_stExHookMask;

/*****************************************************************************
  4 结构定义
*****************************************************************************/
/* 扩展netfilter开关映射表 */
NF_EXT_MASK_OPS_STRU g_stNfExtMaskOps[NF_EXT_HOOK_ON_MASK_IDX_ENUM_BUTT];


STATIC VOS_UINT32 NFExt_ReadNvCfg(VOS_VOID);
STATIC VOS_VOID NFExt_SetDefaultNvCfg(VOS_VOID);
STATIC VOS_VOID NFExt_EntityInit(VOS_VOID);
STATIC VOS_VOID NFExt_MaskOpsInit(VOS_VOID);
STATIC VOS_INT NFExt_Init(VOS_VOID);
STATIC VOS_VOID NFExt_SndDataNotify(VOS_VOID);

STATIC VOS_INT NFExt_RingBufferGet( OM_RING_ID rngId, VOS_CHAR *buffer, VOS_INT maxbytes );
STATIC VOS_VOID NFExt_RcvNfExtInfoCfgReq(VOS_VOID *pMsg);
STATIC VOS_VOID NFExt_EventProc(VOS_UINT32 ulEvent);
STATIC VOS_UINT32 NFExt_SaveBrDev(VOS_VOID);
STATIC VOS_INT NFExt_RingBufferPut( OM_RING_ID rngId, VOS_CHAR *buffer, VOS_INT nbytes );
STATIC VOS_VOID NFExt_FlowCtrlInit(VOS_VOID);

STATIC VOS_INT  NFExt_ReRegHooks(VOS_UINT32 ulMask);

/******************************************************************************
   5 函数实现
******************************************************************************/
/*****************************************************************************
 函 数 名  : NFExt_UnregHooks
 功能描述  : 根据需要停止抓包模块的掩码，将抓包的钩子函数解除内核注册
 输入参数  : VOS_UINT32 ulMask    钩子函数掩码
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_VOID  NFExt_UnregHooks(VOS_UINT32 ulMask)
{
    VOS_UINT i;

    for ( i = 0; i < ARRAY_SIZE(g_stNfExtMaskOps); i++ )
    {
        if ( g_stNfExtMaskOps[i].ulHookMask == (ulMask & g_stNfExtMaskOps[i].ulHookMask) )
        {
            /*卸载钩子函数*/
            nf_unregister_hook(&(g_stNfExtMaskOps[i].stNfExtOps));

            /* 重置相应的掩码位 */
            g_stExEntity.ulCurHookOnMask &= ~g_stNfExtMaskOps[i].ulHookMask;
        }
    }

}

/*****************************************************************************
 函 数 名  : NFExt_RegHooks
 功能描述  : 根据需要开启抓包模块的掩码，将抓包的钩子函数注册到内核
 输入参数  : VOS_UINT32 ulMask        钩子函数掩码
 输出参数  : 无
 返 回 值  : VOS_INT
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_INT  NFExt_RegHooks(VOS_UINT32 ulMask)
{
    VOS_INT iRet;
    VOS_UINT i;

    for ( i = 0; i < ARRAY_SIZE(g_stNfExtMaskOps); i++ )
    {
        if ( g_stNfExtMaskOps[i].ulHookMask != (ulMask & g_stNfExtMaskOps[i].ulHookMask) )
        {
            continue;
        }

        /*注册相应的钩子函数*/
        iRet = nf_register_hook(&(g_stNfExtMaskOps[i].stNfExtOps));
        if ( 0 != iRet )
        {
            TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,"register_hook error!!\n");

            /*若有一个注册失败则卸载当前所有已经注册上的钩子函数*/
            NFExt_UnregHooks(g_stExEntity.ulCurHookOnMask);
            return iRet;
        }

        g_stExEntity.ulCurHookOnMask |= g_stNfExtMaskOps[i].ulHookMask;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : NFExt_ReRegHooks
 功能描述  : 重新注册钩子函数
 输入参数  : VOS_UINT32 ulMask    钩子函数掩码
 输出参数  : 无
 返 回 值  : VOS_INT
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
STATIC VOS_INT  NFExt_ReRegHooks(VOS_UINT32 ulMask)
{
    VOS_INT iRet;

    /*重新注册前先卸载当前所有的钩子函数*/
    if ( 0 != g_stExEntity.ulCurHookOnMask )
    {
        NFExt_UnregHooks(g_stExEntity.ulCurHookOnMask);
    }

    iRet = NFExt_RegHooks(ulMask);

    return iRet;
}

/*****************************************************************************
 函 数 名  : NFExt_ConfigEffective
 功能描述  : 根据配置注册对应的钩子函数
 输入参数  : NF_EXT_TRACE_CONFIG_REQ_STRU *pRcvMsg
 输出参数  : 无
 返 回 值  : PS_BOOL_ENUM_UINT8
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
PS_BOOL_ENUM_UINT8 NFExt_ConfigEffective(IPS_MNTN_TRACE_CONFIG_REQ_STRU *pRcvMsg)
{
    VOS_INT             iRet;
    VOS_UINT32          ulMask = 0;

    /* 流控hook，默认挂上 */
    ulMask |= NF_EXT_DEF_FLOW_CTRL_HOOK_ON_MASK;

    if ( (pRcvMsg->stBridgeArpTraceCfg.ulChoice > IPS_MNTN_TRACE_NULL_CHOSEN)
        && (pRcvMsg->stBridgeArpTraceCfg.ulChoice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN) )
    {
        ulMask |= NF_EXT_DEF_BR_ARP_HOOK_ON_MASK;
    }

    if ( (pRcvMsg->stPreRoutingTraceCfg.ulChoice > IPS_MNTN_TRACE_NULL_CHOSEN)
        && (pRcvMsg->stPreRoutingTraceCfg.ulChoice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN))
    {
        ulMask |= NF_EXT_DEF_PRE_ROUTING_HOOK_ON_MASK;
    }

    if ( (pRcvMsg->stPostRoutingTraceCfg.ulChoice > IPS_MNTN_TRACE_NULL_CHOSEN)
        && (pRcvMsg->stPostRoutingTraceCfg.ulChoice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN))
    {
        ulMask |= NF_EXT_DEF_POST_ROUTING_HOOK_ON_MASK;
    }

    if ( (pRcvMsg->stLocalTraceCfg.ulChoice > IPS_MNTN_TRACE_NULL_CHOSEN)
        && (pRcvMsg->stLocalTraceCfg.ulChoice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN))
    {
        ulMask |= NF_EXT_DEF_LOCAL_HOOK_ON_MASK;
    }

    iRet = NFExt_ReRegHooks(ulMask);

    if (iRet < 0)
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,"NFExt_ConfigEffective: NFExt_ReRegHooks failed!\n");
        return PS_FALSE;
    }

    return PS_TRUE;
}

/*****************************************************************************
 函 数 名  : NFExt_Get1stInetIpv4Addr
 功能描述  : 获取设备第一个Ipv4地址
 输入参数  : struct net_device *pstDev 需要获取IP地址的设备
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年01月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
VOS_UINT32  NFExt_Get1stInetIpv4Addr(struct net_device *pstDev)
{
    struct in_device   *pinDev;

    /* 使用Linux内核结构，使用Linux风格 */
    if (NULL == pstDev)
    {
        return 0;
    }

    if (NULL == pstDev->ip_ptr)
    {
        return 0;
    }

    pinDev = (struct in_device*)(pstDev->ip_ptr);

    if (NULL == pinDev->ifa_list)
    {
        return 0;
    }

    return (VOS_UINT32)htonl((VOS_ULONG)pinDev->ifa_list->ifa_address);
}

/*****************************************************************************
 函 数 名  : NFExt_ReadNvCfg
 功能描述  : 读取NV配置
 输入参数  :
 输出参数  : 无
 返 回 值  : VOS_OK, VOS_ERR
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年8月27日
    作    者   : c00191211
    修改内容   : 新生成函数
*****************************************************************************/
STATIC VOS_UINT32 NFExt_ReadNvCfg(VOS_VOID)
{
    VOS_UINT32                     ulRet;

    /* 读取钩子函数注册点掩码 */
    ulRet = NV_Read (en_NV_Item_NETFILTER_HOOK_MASK, &g_stNfExtNv , sizeof(NF_EXT_NV_STRU));
    if (NV_OK != ulRet)
    {
        PS_PRINTF("NFExt_ReadNvCfg Fail, Read NV FAIL, Error Code \n");
        return VOS_ERR;
    }

    /* 将NV结构中读出的掩码赋给g_stExHookMask*/
    g_stExHookMask.ulBrArpHookValue         = g_stNfExtNv.ulNetfilterPara1;
    g_stExHookMask.ulInHookValue            = g_stNfExtNv.ulNetfilterPara2;
    g_stExHookMask.ulOutHookValue           = g_stNfExtNv.ulNetfilterPara3;
    g_stExHookMask.ulFlowCtrlHookValue      = g_stNfExtNv.ulNetfilterPara4;
    g_stExHookMask.ulLocalHookValue         = g_stNfExtNv.ulNetfilterPara5;

    return VOS_OK;
}

/*****************************************************************************
 函 数 名  : NFExt_SetDefaultNvCfg
 功能描述  : 设置默认NV配置
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年8月27日
    作    者   : c00191211
    修改内容   : 新生成函数
*****************************************************************************/
STATIC VOS_VOID NFExt_SetDefaultNvCfg(VOS_VOID)
{
    g_stExHookMask.ulBrArpHookValue     = (  NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX)
                                           | NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX) );

    g_stExHookMask.ulInHookValue        = (  NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX)
                                           | NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX) );

    g_stExHookMask.ulOutHookValue       = (  NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX)
                                           | NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX) );

    g_stExHookMask.ulFlowCtrlHookValue  = 0;

    g_stExHookMask.ulLocalHookValue     = (  NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX)
                                           | NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX)
                                           | NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX)
                                           | NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX));

    return;
}

/*****************************************************************************
 函 数 名  : NFExt_EntityInit
 功能描述  : NFExt模块实体全局变量初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_EntityInit(VOS_VOID)
{
    g_stExEntity.ulCurHookOnMask    = 0;
    g_stExEntity.ulIsBlkflag        = NF_EXT_FLAG_BLOCKED;
    g_stExEntity.ulIsDeviceOpen     = 0;
    g_stExEntity.ulOmIp             = 0;
}

/*****************************************************************************
 函 数 名  : NFExt_MaskOpsInit
 功能描述  : NFExt模块MaskOps全局变量初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年2月7日
    作    者   : d00314740
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_MaskOpsInit(VOS_VOID)
{
    NF_EXT_MASK_OPS_STRU               *pstMaskOps  = &(g_stNfExtMaskOps[0]);

    PSACORE_MEM_SET(pstMaskOps,
        NF_EXT_HOOK_ON_MASK_IDX_ENUM_BUTT * sizeof(NF_EXT_MASK_OPS_STRU),
        0x00,
        NF_EXT_HOOK_ON_MASK_IDX_ENUM_BUTT * sizeof(NF_EXT_MASK_OPS_STRU));

    pstMaskOps[NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hook      = (nf_hookfn*)NFExt_BrPreRoutingHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priv      = NULL,
#endif
    pstMaskOps[NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.pf        = NFPROTO_BRIDGE;
    pstMaskOps[NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hooknum   = NF_BR_PRE_ROUTING;
    pstMaskOps[NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priority  = NF_EXT_BR_PRI_FILTER_OTHER;   /* 网桥hook点的最低优先级 */

    pstMaskOps[NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_BrPostRoutingHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_BRIDGE;
    pstMaskOps[NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hooknum  = NF_BR_POST_ROUTING;
    pstMaskOps[NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_BR_PRI_FILTER_OTHER;

    pstMaskOps[NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.hook      = (nf_hookfn*)NFExt_BrForwardHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.priv      = NULL,
#endif
    pstMaskOps[NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.pf        = NFPROTO_BRIDGE;
    pstMaskOps[NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.hooknum   = NF_BR_FORWARD;
    pstMaskOps[NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.priority  = NF_EXT_BR_PRI_FILTER_OTHER;

    pstMaskOps[NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_BrLocalInHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_BRIDGE;
    pstMaskOps[NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.hooknum  = NF_BR_LOCAL_IN;
    pstMaskOps[NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_BR_PRI_FILTER_OTHER;

    pstMaskOps[NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_BrLocalOutHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_BRIDGE;
    pstMaskOps[NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.hooknum     = NF_BR_LOCAL_OUT;
    pstMaskOps[NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_BR_PRI_FILTER_OTHER;

    pstMaskOps[NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX);
    pstMaskOps[NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_ArpInHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_ARP;
    pstMaskOps[NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX].stNfExtOps.hooknum  = NF_ARP_IN;
    pstMaskOps[NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_IP_PRI_CONNTRACK;      /* ARP hook点的优先级 */

    pstMaskOps[NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX);
    pstMaskOps[NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_ArpOutHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_ARP;
    pstMaskOps[NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX].stNfExtOps.hooknum     = NF_ARP_OUT;
    pstMaskOps[NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_IP_PRI_CONNTRACK;

    pstMaskOps[NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_Ip4PreRoutingHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_IPV4;
    pstMaskOps[NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hooknum  = NF_INET_PRE_ROUTING;
    pstMaskOps[NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_IP_PRI_MANGLE;         /* 高于DNAT hook点的优先级 */

    pstMaskOps[NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_Ip4PostRoutingHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_IPV4;
    pstMaskOps[NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hooknum     = NF_INET_POST_ROUTING;
    pstMaskOps[NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_IP_PRI_SELINUX_LAST;   /* 低于SNAT hook点的优先级 */

    pstMaskOps[NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_Ip4LocalInHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_IPV4;
    pstMaskOps[NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.hooknum     = NF_INET_LOCAL_IN;
    pstMaskOps[NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.hook       = (nf_hookfn*)NFExt_Ip4LocalOutHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.priv       = NULL,
#endif
    pstMaskOps[NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.pf         = NFPROTO_IPV4;
    pstMaskOps[NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.hooknum    = NF_INET_LOCAL_OUT;
    pstMaskOps[NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.priority   = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_Ip4ForwardHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_IPV4;
    pstMaskOps[NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.hooknum  = NF_INET_FORWARD;
    pstMaskOps[NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_Ip6PreRoutingHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_IPV6;
    pstMaskOps[NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hooknum  = NF_INET_PRE_ROUTING;
    pstMaskOps[NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_IP_PRI_MANGLE;

    pstMaskOps[NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_Ip6PostRoutingHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_IPV6;
    pstMaskOps[NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.hooknum     = NF_INET_POST_ROUTING;
    pstMaskOps[NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_Ip6LocalInHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_IPV6;
    pstMaskOps[NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.hooknum     = NF_INET_LOCAL_IN;
    pstMaskOps[NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.hook       = (nf_hookfn*)NFExt_Ip6LocalOutHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.priv       = NULL,
#endif
    pstMaskOps[NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.pf         = NFPROTO_IPV6;
    pstMaskOps[NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.hooknum    = NF_INET_LOCAL_OUT;
    pstMaskOps[NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX].stNfExtOps.priority   = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.hook     = (nf_hookfn*)NFExt_Ip6ForwardHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.priv     = NULL,
#endif
    pstMaskOps[NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.pf       = NFPROTO_IPV6;
    pstMaskOps[NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.hooknum  = NF_INET_FORWARD;
    pstMaskOps[NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX].stNfExtOps.priority = NF_EXT_IP_PRI_SELINUX_LAST;

    pstMaskOps[NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX].ulHookMask
        = NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX);
    pstMaskOps[NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX].stNfExtOps.hook        = (nf_hookfn*)NFExt_BrForwardFlowCtrlHook;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
    pstMaskOps[NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX].stNfExtOps.priv        = NULL,
#endif
    pstMaskOps[NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX].stNfExtOps.pf          = NFPROTO_BRIDGE;
    pstMaskOps[NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX].stNfExtOps.hooknum     = NF_BR_FORWARD;
    pstMaskOps[NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX].stNfExtOps.priority    = NF_EXT_BR_PRI_FILTER_BRIDGED;             /* 与包过滤优先级相同,优先级在这里不能为0 */

}

/*****************************************************************************
 函 数 名  : NFExt_Init
 功能描述  : 模块初始化函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
STATIC VOS_INT NFExt_Init(VOS_VOID)
{
    VOS_UINT32  ulRet;

    ulRet = NFExt_ReadNvCfg();
    if (VOS_ERR == ulRet)
    {
        NFExt_SetDefaultNvCfg();
    }

    /* NFExt模块实体全局变量初始化 */
    NFExt_EntityInit();

    /* NFExt模块MaskOps全局变量初始化 */
    NFExt_MaskOpsInit();

    /* 流控信息初始化 */
    NFExt_FlowCtrlInit();

    return VOS_OK;
}

/*****************************************************************************
                        流控功能
*****************************************************************************/
/*****************************************************************************
 函 数 名  : NFExt_FlowCtrlInit
 功能描述  : 流控实体初始化
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年01月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_FlowCtrlInit(VOS_VOID)
{
    if (NFExt_RegHooks(NF_EXT_DEF_FLOW_CTRL_HOOK_ON_MASK))
    {
        PS_PRINTF("NFExt_FlowCtrlInit NFExt_RegHooks fail");

        return;
    }

    g_stExFlowCtrlEntity.ulFlowCtrlMsk = 0;
    g_stExFlowCtrlEntity.pstBrDev      = NULL;

    PSACORE_MEM_SET(g_stExFlowCtrlEntity.aulTxBytesCnt, sizeof(g_stExFlowCtrlEntity.aulTxBytesCnt), 0, sizeof(g_stExFlowCtrlEntity.aulTxBytesCnt));
}

/*****************************************************************************
 函 数 名  : NFExt_BrSetFlowCtrl
 功能描述  : 网桥流控状态设置
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年01月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
/*lint -e{550,438} */
VOS_VOID  NFExt_BrSetFlowCtrl(VOS_VOID)
{
    VOS_ULONG       ulFlags = 0UL;

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    g_stExFlowCtrlEntity.ulFlowCtrlMsk |= NF_EXT_BR_FORWARD_FLOW_CTRL_MASK;
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);

    IPS_MNTN_FlowCtrl(NF_EXT_BR_FORWARD_FLOW_CTRL_MASK, ID_IPS_TRACE_BR_FORWARD_FLOW_CTRL_START);
}

/*****************************************************************************
 函 数 名  : NFExt_BrStopFlowCtrl
 功能描述  : 网桥流控状态解除
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年01月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
/*lint -e{550,438} */
VOS_VOID  NFExt_BrStopFlowCtrl(VOS_VOID)
{
    VOS_ULONG       ulFlags = 0UL;

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    g_stExFlowCtrlEntity.ulFlowCtrlMsk &= ~NF_EXT_BR_FORWARD_FLOW_CTRL_MASK;
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);

    IPS_MNTN_FlowCtrl(NF_EXT_BR_FORWARD_FLOW_CTRL_MASK, ID_IPS_TRACE_BR_FORWARD_FLOW_CTRL_STOP);
}

/*****************************************************************************
 函 数 名  : NFExt_GetBrBytesCnt
 功能描述  : 获取当前网桥发送接收数据统计信息
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 当前网桥发送接收数据统计信息
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年01月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
VOS_UINT32 NFExt_GetBrBytesCnt(VOS_VOID)
{
    return 0;
}


/*****************************************************************************
 函 数 名  : NFExt_SaveBrDev
 功能描述  : 保存当前流控所使用的网桥设备
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年01月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
STATIC VOS_UINT32 NFExt_SaveBrDev(VOS_VOID)
{
    struct net_device   *dev;
    dev = dev_get_by_name(&init_net, NF_EXT_BR_NAME);

    if (NULL == dev)
    {
        return VOS_ERR;
    }

    g_stExFlowCtrlEntity.pstBrDev   = dev;

    /* 只获取第一个IPv4地址 */
    g_stExEntity.ulOmIp             = NFExt_Get1stInetIpv4Addr(dev);

    return VOS_OK;
}

#if(NF_EXT_DBG == DBG_ON)
/*****************************************************************************
 函 数 名  : NFExt_StatsShow
 功能描述  : Netfilter ex debug参数打印函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年02月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_StatsShow(VOS_VOID)
{
    vos_printf("网桥forward流控丢掉的数据量 %ld \n", g_stNfExtStats.aulStats[NF_EXT_STATS_BR_FC_DROP]);
    vos_printf("进入网桥forward hook的数据量 %ld \n", g_stNfExtStats.aulStats[NF_EXT_STATS_BR_FC_ENTER]);
    vos_printf("环形buf满之后导致丢包数量 %ld \n", g_stNfExtStats.aulStats[NF_EXT_STATS_BUF_FULL_DROP]);
    vos_printf("入环形buf失败次数 %ld \n", g_stNfExtStats.aulStats[NF_EXT_STATS_PUT_BUF_FAIL]);
    vos_printf("出环形buf失败次数 %ld \n", g_stNfExtStats.aulStats[NF_EXT_STATS_GET_BUF_FAIL]);
    vos_printf("申请内存失败次数 %ld \n", g_stNfExtStats.aulStats[NF_EXT_STATS_ALLOC_MEM_FAIL]);

    vos_printf("当前环形缓存中未处理的数据量 %ld \n", ((VOS_UINT32)OM_RingBufferNBytes(g_stExEntity.pRingBufferId) / sizeof(NF_EXT_DATA_RING_BUF_STRU)));
    vos_printf("当前的Hook Mask %ld \n", g_stExEntity.ulCurHookOnMask);
    vos_printf("当前的流控状态Mask %ld \n", g_stExFlowCtrlEntity.ulFlowCtrlMsk);
    vos_printf("当前OM WIFI所使用的IP地址 %x \n", g_stExEntity.ulOmIp);
    vos_printf("当前网桥转发字节数 %x \n", g_stExFlowCtrlEntity.aulTxBytesCnt[NF_EXT_TX_BYTES_CNT_BR]);
}

/*****************************************************************************
 函 数 名  : NFExt_ResetPri
 功能描述  : 重置netfilter ex hook点优先级函数,Debug使用
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2012年02月11日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_ResetPri(VOS_UINT32 ulHookNode, VOS_INT32 iPri)
{
    VOS_UINT32  ulCurHookMask = 0;

    if (ulHookNode > (ARRAY_SIZE(g_stNfExtMaskOps) - 1))
    {
        return;
    }

    ulCurHookMask = g_stExEntity.ulCurHookOnMask;
    g_stNfExtMaskOps[ulHookNode].stNfExtOps.priority = iPri;

    NFExt_ReRegHooks(ulCurHookMask);

    vos_printf("Current ulHookNode %d pri = %d\n", ulHookNode, iPri);
}
#endif

#endif      /* #if (FEATURE_ON == FEATURE_NFEXT) */

/******************************************************************************
 Prototype       : NFExt_SndDataNotify
 Description     : 发送数据处理指示NFEXT_DATA_PROC_NOTIFY
 Input           :
 Output          : NONE
 Return Value    : PS_SUCC   --- 成功
                   PS_FAIL   --- 失败
 History         :
   1.Date        : 2008-07-16
     Author      : l47619
     Modification: 增加PPP数据队列处理机制
******************************************************************************/
STATIC VOS_VOID NFExt_SndDataNotify(VOS_VOID)
{
    (VOS_VOID)VOS_EventWrite(g_ulNFExtTaskId, NFEXT_DATA_PROC_NOTIFY);

    return;
} /* NFExt_SndDataNotify */

/*****************************************************************************
 函 数 名  : NFExt_RingBufferPut
 功能描述  : 将数据放到ring buffer里
 输入参数  : OM_RING_ID rngId:      环形buff
             VOS_CHAR *buffer:      待放入环形buff的数据头指针
             VOS_INT nbytes:        待放入环形buff的数据长度
 输出参数  : 无
 返 回 值  : VOS_INT
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
/*lint -e{550,438} */
STATIC VOS_INT NFExt_RingBufferPut( OM_RING_ID rngId, VOS_CHAR *buffer, VOS_INT nbytes )
{
    VOS_ULONG   ulFlags = 0UL;
    VOS_INT     iRst;

    iRst = 0;

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    if ((VOS_UINT32)OM_RingBufferFreeBytes(g_stExEntity.pRingBufferId) >= sizeof(NF_EXT_DATA_RING_BUF_STRU) )
    {
        iRst = OM_RingBufferPut(rngId, buffer, nbytes);
    }
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);

    return iRst;
}

/*****************************************************************************
 函 数 名  : NFExt_RingBufferGet
 功能描述  : 将数据从ring buffer里取出
 输入参数  : OM_RING_ID rngId:      环形buff
             VOS_CHAR *buffer:      环形buff中待取出的数据头指针
             VOS_INT nbytes:        环形buff中待取出的数据长度
 输出参数  : 无
 返 回 值  : VOS_INT
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
/*lint -e{550,438} */
STATIC VOS_INT NFExt_RingBufferGet( OM_RING_ID rngId, VOS_CHAR *buffer, VOS_INT maxbytes )
{
    VOS_ULONG   ulFlags = 0UL;
    VOS_INT     iRst;

    iRst = 0;

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    if (!OM_RingBufferIsEmpty(rngId))
    {
        iRst = OM_RingBufferGet(rngId, buffer, maxbytes );
    }
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);

    return iRst;
}

/*****************************************************************************
 函 数 名  : NFExt_FlushRingBuffer
 功能描述  : 将RingBuffer里面的数据都清除
 输入参数  : OM_RING_ID rngId OM ringbuffer id
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
/*lint -e{550,438} */
VOS_VOID NFExt_FlushRingBuffer(OM_RING_ID rngId)
{
    NF_EXT_DATA_RING_BUF_STRU   stData;
    VOS_ULONG                   ulFlags = 0UL;
    VOS_INT                     iRst = 0;

    /* 初始化 */
    PSACORE_MEM_SET(&stData, sizeof(stData), 0x0, sizeof(stData));

    while (!OM_RingBufferIsEmpty(rngId))
    {
        iRst = NFExt_RingBufferGet(rngId, (VOS_CHAR*)(&stData), sizeof(NF_EXT_DATA_RING_BUF_STRU));
        if (iRst == sizeof(NF_EXT_DATA_RING_BUF_STRU))
        {
            NF_EXT_MEM_FREE(ACPU_PID_NFEXT, stData.pData);
        }
        else
        {
            TTF_LOG1(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
                    "NFExt_FlushRingBuffer : ERROR : Get data error from ring buffer!", iRst);

            break;
        }
    }

    VOS_SpinLockIntLock(&g_stExEntity.stLockTxTask, ulFlags);
    OM_RingBufferFlush(rngId);
    VOS_SpinUnlockIntUnlock(&g_stExEntity.stLockTxTask, ulFlags);
}


/*****************************************************************************
 函 数 名  : NFExt_AddDataToRingBuf
 功能描述  : 将需要通过OAM发送的数据放到自处理任务的RingBuf里面，由自处理任务发送出去
 输入参数  : NF_EXT_DATA_RING_BUF_STRU *pstData 放入ringbuffer中的数据
 输出参数  : 无
 返 回 值  : VOS_UINT32 成功或者失败
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : t00148005
    修改内容   : Created
  2.日    期   :  2015年10月31日
    作    者   :  g00178567
    修改内容   :  勾包改造，降低勾包任务切换频率
*****************************************************************************/
VOS_UINT32 NFExt_AddDataToRingBuf(NF_EXT_DATA_RING_BUF_STRU *pstData)
{
    VOS_UINT32                  ulRst           = VOS_OK;
    VOS_UINT32                  ulNeedWakeUp    = VOS_FALSE;
    VOS_INT                     iRst;
    NF_EXT_DATA_RING_BUF_STRU   stData;

    if ( VOS_NULL_PTR == g_stExEntity.pRingBufferId )
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,"NFExt_AddDataToRingBuf: Warning : g_stExEntity.pRingBufferId is null!\n");
        return VOS_ERR;
    }

    /* 空到非空，唤醒任务处理勾包 */
    if (OM_RingBufferIsEmpty(g_stExEntity.pRingBufferId))
    {
        ulNeedWakeUp = VOS_TRUE;
    }

    iRst = NFExt_RingBufferPut(g_stExEntity.pRingBufferId, (VOS_CHAR *)pstData, (VOS_INT)(sizeof(NF_EXT_DATA_RING_BUF_STRU)));
    if (sizeof(NF_EXT_DATA_RING_BUF_STRU) == iRst)
    {
        if (VOS_TRUE == ulNeedWakeUp)
        {
            NFExt_SndDataNotify();
        }

        ulRst = VOS_OK;
    }
    else if (0 == iRst)
    {
        NF_EXT_STATS_INC(1, NF_EXT_STATS_BUF_FULL_DROP);

        /* 队列满，唤醒任务处理勾包 */
        NFExt_SndDataNotify();

        ulRst = VOS_ERR;
    }
    else
    {
        TTF_LOG2(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,
                "RingBufferPut Fail found ulRst = %u, sizeof=%u \r\n", iRst, sizeof(NF_EXT_DATA_RING_BUF_STRU));

        (VOS_VOID)NFExt_RingBufferGet(g_stExEntity.pRingBufferId, (VOS_CHAR *)(&stData), iRst);

        NF_EXT_STATS_INC(1, NF_EXT_STATS_PUT_BUF_FAIL);

        ulRst = VOS_ERR;
    }

    return ulRst;
}

/*****************************************************************************
 函 数 名  : NFExt_RcvNfExtInfoCfgReq
 功能描述  : 接收到OM配置可维可测信息捕获配置请求
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_RcvNfExtInfoCfgReq(VOS_VOID *pMsg)
{
    OM_IPS_MNTN_INFO_CONFIG_REQ_STRU    *pstNfExtCfgReq;
    IPS_OM_MNTN_INFO_CONFIG_CNF_STRU    stNfExtCfgCnf;
    IPS_MNTN_RESULT_TYPE_ENUM_UINT32    enResult;

    pstNfExtCfgReq  = (OM_IPS_MNTN_INFO_CONFIG_REQ_STRU *)pMsg ;

    enResult        = IPS_MNTN_RESULT_OK;

    /*================================*/
    /*构建回复消息*/
    /*================================*/

    /* Fill DIAG trans msg header */
    stNfExtCfgCnf.stDiagHdr.ulSenderCpuId   = VOS_LOCAL_CPUID;
    stNfExtCfgCnf.stDiagHdr.ulSenderPid     = ACPU_PID_NFEXT;
    stNfExtCfgCnf.stDiagHdr.ulReceiverCpuId = VOS_LOCAL_CPUID;
    stNfExtCfgCnf.stDiagHdr.ulReceiverPid   = MSP_PID_DIAG_APP_AGENT;   /* 把应答消息发送给DIAG，由DIAG把透传命令的处理结果发送给HIDS工具*/
    stNfExtCfgCnf.stDiagHdr.ulLength        = sizeof(IPS_OM_MNTN_INFO_CONFIG_CNF_STRU) - VOS_MSG_HEAD_LENGTH;

    stNfExtCfgCnf.stDiagHdr.ulMsgId         = ID_IPS_OM_MNTN_INFO_CONFIG_CNF;

    /* DIAG透传命令中的特定信息*/
    stNfExtCfgCnf.stDiagHdr.usOriginalId  = pstNfExtCfgReq->stDiagHdr.usOriginalId;
    stNfExtCfgCnf.stDiagHdr.usTerminalId  = pstNfExtCfgReq->stDiagHdr.usTerminalId;
    stNfExtCfgCnf.stDiagHdr.ulTimeStamp   = pstNfExtCfgReq->stDiagHdr.ulTimeStamp;
    stNfExtCfgCnf.stDiagHdr.ulSN          = pstNfExtCfgReq->stDiagHdr.ulSN;

    /* 填充回复OM申请的确认信息 */
    stNfExtCfgCnf.stIpsMntnCfgCnf.enCommand  = pstNfExtCfgReq->stIpsMntnCfgReq.enCommand;
    stNfExtCfgCnf.stIpsMntnCfgCnf.enRslt     = enResult;

    /* 发送OM透明消息 */
    IPS_MNTN_SndCfgCnf2Om( ID_IPS_OM_MNTN_INFO_CONFIG_CNF,
        sizeof(IPS_OM_MNTN_INFO_CONFIG_CNF_STRU), &stNfExtCfgCnf );

    return;

}

/*****************************************************************************
 函 数 名  : NFExt_RcvOmMsg
 功能描述  : NFExt接收到来自OM模块的消息处理
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_VOID NFExt_RcvOmMsg(VOS_VOID *pMsg)
{
    VOS_UINT16          usMsgId;

    usMsgId = (VOS_UINT16)(*((VOS_UINT32 *)((VOS_UINT8 *)(pMsg) + VOS_MSG_HEAD_LENGTH)));

    switch ( usMsgId )
    {
        case ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ:
            IPS_MNTN_TraceAdvancedCfgReq(pMsg);
            break;

        case ID_OM_IPS_MNTN_INFO_CONFIG_REQ:
            NFExt_RcvNfExtInfoCfgReq(pMsg);
            break;

        default:
            TTF_LOG1(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,
                "NFExt_RcvConfig:Receive Unkown Type Message !\n", usMsgId);
            break;
    }

    return;
}

/******************************************************************************
 Prototype       : NFExt_BindToCpu
 Description     : 绑定Task到指定CPU上面
 Input           :
 Output          : NONE
 Return Value    : PS_SUCC   --- 成功
                   PS_FAIL   --- 失败
 History         :
   1.Date        : 2016-06-16
     Author      :
     Modification:
******************************************************************************/
VOS_VOID NFExt_BindToCpu(VOS_VOID)
{
    VOS_LONG            ret;
    pid_t               target_pid;
    VOS_INT             cpu;

    /* 获取当前线程的Pid */
    target_pid = current->pid;

    /* 获取当前线程的affinity */
    ret = sched_getaffinity(target_pid, &(g_stExEntity.orig_mask));
    if (ret < 0)
    {
        PS_PRINTF("warning: unable to get cpu affinity\n");
        return;
    }

    PSACORE_MEM_SET(&(g_stExEntity.curr_mask), cpumask_size(), 0, cpumask_size());

    /* 设置当前线程的affinity */
    for_each_cpu(cpu, &(g_stExEntity.orig_mask))
    {
        /* 去绑定CPU0 */
        if ((0 < cpu) && (cpumask_test_cpu(cpu, &(g_stExEntity.orig_mask))))
        {
            cpumask_set_cpu((unsigned int)cpu, &(g_stExEntity.curr_mask));
        }
    }

    if (0 == cpumask_weight(&(g_stExEntity.curr_mask)))
    {
        cpumask_set_cpu(0, &(g_stExEntity.curr_mask));
        return;
    }

    ret = sched_setaffinity(target_pid, &(g_stExEntity.curr_mask));
    if (ret < 0)
    {
        PS_PRINTF("warning: unable to set cpu affinity\n");
        return;
    }

    return;
}

/*****************************************************************************
 函 数 名  : NFExt_ProcDataNotify
 功能描述  : NFExt消息处理
 输入参数  : VOS_VOID *pMsg
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_VOID NFExt_ProcDataNotify(VOS_VOID)
{
    NF_EXT_DATA_RING_BUF_STRU   stData;
    VOS_INT                     iRst;
    DIAG_TRANS_IND_STRU        *pstDiagTransData;
    VOS_UINT32                  ulDealCntOnce;

    if (VOS_NULL_PTR == g_stExEntity.pRingBufferId)
    {
        TTF_LOG(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
            "NFExt_ProcDataNotify : ERROR : pRingBufferId is NULL!" );
        return;
    }

    ulDealCntOnce = 0;

    while (!OM_RingBufferIsEmpty(g_stExEntity.pRingBufferId))
    {
        /* 一次任务调度，最多处理200个勾包 */
        if (NF_ONCE_DEAL_MAX_CNT <= ulDealCntOnce)
        {
            NFExt_SndDataNotify();
            break;
        }

        iRst = NFExt_RingBufferGet(g_stExEntity.pRingBufferId, (VOS_CHAR *)&stData, sizeof(NF_EXT_DATA_RING_BUF_STRU));
        if (sizeof(NF_EXT_DATA_RING_BUF_STRU) == iRst)
        {
            pstDiagTransData = (DIAG_TRANS_IND_STRU *)(stData.pData);

            if ( VOS_OK != DIAG_TransReport(pstDiagTransData))
            {
                TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_ERROR, "NFExt_ProcDataNotify, ERROR, Call DIAG_TransReport fail!");
            }

            NF_EXT_MEM_FREE(ACPU_PID_NFEXT, stData.pData);
        }
        else if (0 == iRst)
        {
            TTF_LOG(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
                "NFExt_ProcDataNotify : ERROR : Get null from ring buffer!");

            break;
        }
        else
        {
            TTF_LOG2(ACPU_PID_NFEXT, 0, PS_PRINT_WARNING,
                "NFExt_ProcDataNotify : ERROR : Get data error from ring buffer!", iRst, sizeof(NF_EXT_DATA_RING_BUF_STRU));

            NF_EXT_STATS_INC(1, NF_EXT_STATS_GET_BUF_FAIL);

            NFExt_FlushRingBuffer(g_stExEntity.pRingBufferId);

            break;
        }

        ulDealCntOnce++;
    }
}

/*****************************************************************************
 函 数 名  : NFExt_EventProc
 功能描述  : NFExt事件处理函数
 输入参数  : struct MsgCB * pMsg
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
STATIC VOS_VOID NFExt_EventProc(VOS_UINT32 ulEvent)
{
    if (ulEvent & NFEXT_DATA_PROC_NOTIFY)
    {
        NFExt_ProcDataNotify();
    }

    return;
}

/*****************************************************************************
 函 数 名  : NFExt_MsgProc
 功能描述  : NFExt可维可测控制消息处理函数
 输入参数  : struct MsgCB * pMsg
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_VOID NFExt_MsgProc( struct MsgCB * pMsg )
{
    if ( VOS_NULL_PTR == pMsg )
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING,"NFExt_MsgProc: Message is NULL !" );
        return;
    }

    switch ( pMsg->ulSenderPid )
    {
        case MSP_PID_DIAG_APP_AGENT:      /* 来自OM的透传消息处理 */
            NFExt_RcvOmMsg( (void *)pMsg );
            break;

        default:
            break;
    }

    return;
}

/*****************************************************************************
 函 数 名  : NFExt_FidTask
 功能描述  : 负责调用OAM消息发送接口，发送消息到SDT
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : t00148005
    修改内容   : Created
*****************************************************************************/
VOS_VOID NFExt_FidTask(VOS_VOID)
{
    MsgBlock                           *pMsg          = VOS_NULL_PTR;
    VOS_UINT32                          ulEvent       = 0;
    VOS_UINT32                          ulTaskID      = 0;
    VOS_UINT32                          ulRtn         = VOS_ERR;
    VOS_UINT32                          ulEventMask   = 0;
    VOS_UINT32                          ulExpectEvent = 0;

    ulTaskID = VOS_GetCurrentTaskID();
    if (PS_NULL_UINT32 == ulTaskID)
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_FidTask: TaskID is invalid.");
        return;
    }

    if (VOS_OK != VOS_CreateEvent(ulTaskID))
    {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_FidTask: create event fail.");
        return;
    }

    g_ulNFExtTaskId = ulTaskID;

    NFExt_BindToCpu();

    ulExpectEvent = NFEXT_DATA_PROC_NOTIFY | VOS_MSG_SYNC_EVENT;
    ulEventMask   = VOS_EVENT_ANY | VOS_EVENT_WAIT;

#ifndef __UT_CENTER__
    for ( ; ; )
#endif
    {
        ulRtn = VOS_EventRead(ulExpectEvent, ulEventMask, 0, &ulEvent);
        if (VOS_OK != ulRtn)
        {
            TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_FidTask: read event error.");

#ifndef __UT_CENTER__
            continue;
#endif
        }

        /* 事件处理 */
        if (VOS_MSG_SYNC_EVENT != ulEvent)
        {
            NFExt_EventProc(ulEvent);

#ifndef __UT_CENTER__
            continue;
#endif
        }

        pMsg = (MsgBlock *)VOS_GetMsg(ulTaskID);
        if (VOS_NULL_PTR != pMsg)
        {
            if (ACPU_PID_NFEXT == pMsg->ulReceiverPid)
            {
                NFExt_MsgProc(pMsg);
            }

            (VOS_VOID)VOS_FreeMsg(ACPU_PID_NFEXT, pMsg);
        }
    }
}

/*****************************************************************************
 函 数 名  : NFExt_PidInit
 功能描述  : NFExt_Pid初始化
 输入参数  : ip - 初始化状态
 输出参数  : 无
 返 回 值  : 成功VOS_OK, 失败VOS_ERR
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_UINT32 NFExt_PidInit( enum VOS_INIT_PHASE_DEFINE ip )
{
    switch ( ip )
    {
        case VOS_IP_LOAD_CONFIG:
            /* 申请RingBuffer */
            g_stExEntity.pRingBufferId = OM_RingBufferCreate(NF_EXT_RING_BUF_SIZE);
            if ( VOS_NULL_PTR == g_stExEntity.pRingBufferId )
            {
                PS_PRINTF("NFExt_PidInit : ERROR : Create ring buffer Failed!" );
                return VOS_ERR;
            }

            /* 初始化锁 */
            VOS_SpinLockInit(&(g_stExEntity.stLockTxTask));
            break;

        case VOS_IP_FARMALLOC:
        case VOS_IP_INITIAL:
        case VOS_IP_ENROLLMENT:
        case VOS_IP_LOAD_DATA:
        case VOS_IP_FETCH_DATA:
        case VOS_IP_STARTUP:
        case VOS_IP_RIVAL:
        case VOS_IP_KICKOFF:
        case VOS_IP_STANDBY:
        case VOS_IP_BROADCAST_STATE:
        case VOS_IP_RESTART:
            break;
        default:
            break;
    }

    return VOS_OK;
}


/*****************************************************************************
 函 数 名  : NFExt_FidInit
 功能描述  : NFExt_Fid初始化
 输入参数  : ip - 初始化状态
 输出参数  : 无
 返 回 值  : 成功VOS_OK, 失败VOS_ERR
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2011年11月22日
    作    者   : caikai
    修改内容   : Created
*****************************************************************************/
VOS_UINT32 NFExt_FidInit ( enum VOS_INIT_PHASE_DEFINE ip )
{
    VOS_UINT32                          ulRslt;

    switch ( ip )
    {
        case   VOS_IP_LOAD_CONFIG:
#if (FEATURE_ON == FEATURE_NFEXT)
            /* 先完成模块初始化 */
            if ( 0 != NFExt_Init() )
            {
                PS_PRINTF("NFExt_FidInit NFExt_Init FAIL!\n");
                return VOS_ERR;
            }
#endif

            /* 可维可测模块注册PID */
            ulRslt = VOS_RegisterPIDInfo(ACPU_PID_NFEXT,
                                (Init_Fun_Type)NFExt_PidInit,
                                (Msg_Fun_Type)NFExt_MsgProc);
            if( VOS_OK != ulRslt )
            {
                PS_PRINTF("NFExt_FidInit VOS_RegisterPIDInfo FAIL!\n");
                return VOS_ERR;
            }

            ulRslt = VOS_RegisterMsgTaskEntry(ACPU_FID_NFEXT, (VOS_VOIDFUNCPTR)NFExt_FidTask);
            if (VOS_OK != ulRslt)
            {
                PS_PRINTF("NFExt_FidInit VOS_RegisterMsgTaskEntry fail!\n");
                return VOS_ERR;
            }

            ulRslt = VOS_RegisterMsgTaskPrio(ACPU_FID_NFEXT, VOS_PRIORITY_M4);
            if( VOS_OK != ulRslt )
            {
                PS_PRINTF("NFExt_FidInit VOS_RegisterTaskPrio Failed!\n");
                return VOS_ERR;
            }

            break;

        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
        case   VOS_IP_BUTT:
            break;

        default:
            break;
    }

    return PS_SUCC;
}


