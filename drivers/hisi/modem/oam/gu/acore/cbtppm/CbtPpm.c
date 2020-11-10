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

                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : CbtPpm.c
  �� �� ��   : ����
  ��    ��   :
  ��������   :
  ����޸�   :
  ��������   : CBT������˿ڹ���ģ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2014��5��31��
    ��    ��   : L00256032
    �޸�����   : V8R1 OM_Optimize��Ŀ����

***************************************************************************** */

/*****************************************************************************
  1 ͷ�ļ�����
**************************************************************************** */
#include "AtAppVcomInterface.h"
#include "CbtPpm.h"
#include "CbtCpm.h"
#include "pamappom.h"
#include "omprivate.h"
#include "cbtapprl.h"
#include "PamOamSpecTaskDef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define    THIS_FILE_ID        PS_FILE_ID_CBT_PPM_C

/* ****************************************************************************
  2 ȫ�ֱ�������
**************************************************************************** */
VOS_SEM                            g_ulCbtUsbPseudoSyncSemId;   /* α��Ϊͬ���ӿ���ʹ�õ��ź��� */

CBT_ACPU_VCOM_DEBUG_INFO           g_stCbtVComAcpuDebugInfo;

/* ����ACPU��CBT�˿ڵ�UDI��� */
UDI_HANDLE                         g_ulCbtPortUDIHandle    = VOS_ERROR;

/*****************************************************************************
  3 �ⲿ��������
*****************************************************************************/
extern  CBTCPM_RCV_FUNC                 g_pCbtRcvFunc;
extern  CBTCPM_SEND_FUNC                g_pCbtSndFunc;

extern VOS_UINT32 CBTSCM_SoftDecodeDataRcv(VOS_UINT8 *pucBuffer, VOS_UINT32 ulLen);

/*****************************************************************************
  4 ����ʵ��
*****************************************************************************/

/*****************************************************************************
 �� �� ��  : CBTPPM_OamCbtPortDataSnd
 ��������  : ����CBT����
 �������  : pucVirAddr:   �������ַ
             pucPhyAddr:   ����ʵ��ַ
             ulDataLen:    ���ݳ���
 �������  : ��
 �� �� ֵ  : VOS_ERR/VOS_OK
 �޸���ʷ  :
   1.��    ��  : 2014��5��26��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_UINT32 CBTPPM_OamCbtPortDataSnd(VOS_UINT8 *pucVirAddr, VOS_UINT8 *pucPhyAddr, VOS_UINT32 ulDataLen)
{
    CBTCPM_SEND_FUNC                    pFunc;

    pFunc = CBTCPM_GetSndFunc();

    if (VOS_NULL_PTR == pFunc)
    {
        return VOS_ERR;
    }

    return pFunc(pucVirAddr, pucPhyAddr, ulDataLen);
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamUsbCbtSendData
 ��������  : �����������ͨ��USB(APP��)���͸�PC��
 �������  : pucVirAddr:   �������ַ
             pucPhyAddr:   ����ʵ��ַ
             ulDataLen:    ���ݳ���
 �������  : ��
 �� �� ֵ  : VOS_ERR/VOS_OK
 ���ú���  :
 ��������  :
 �޸���ʷ  :
   1.��    ��  : 2011��10��8��
     ��    ��  : g47350
     �޸�����  : Creat Function
*****************************************************************************/
VOS_UINT32 CBTPPM_OamUsbCbtSendData(VOS_UINT8 *pucVirAddr, VOS_UINT8 *pucPhyAddr, VOS_UINT32 ulDataLen)
{
    VOS_INT32                           lRet;
    ACM_WR_ASYNC_INFO                   stVcom;
    /*VOS_UINT32                          ulInSlice;
    VOS_UINT32                          ulOutSlice;
    VOS_UINT32                          ulWriteSlice;*/

    if ((VOS_NULL_PTR == pucVirAddr) || (VOS_NULL_PTR == pucPhyAddr))
    {
        /* ��ӡ���� */
        LogPrint("\r\n CBTPPM_OamUsbCbtSendData: Vir or Phy Addr is Null \n");

        return CBTCPM_SEND_PARA_ERR;
    }

    stVcom.pVirAddr = (VOS_CHAR*)pucVirAddr;
    stVcom.pPhyAddr = (VOS_CHAR*)pucPhyAddr;
    stVcom.u32Size  = ulDataLen;
    stVcom.pDrvPriv = VOS_NULL_PTR;

    /*g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteNum1++;

    ulInSlice = OM_GetSlice();*/

    lRet = mdrv_udi_ioctl(g_ulCbtPortUDIHandle, ACM_IOCTL_WRITE_ASYNC, &stVcom);

    /*g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteNum2++;

    ulOutSlice = OM_GetSlice();

    if(ulInSlice > ulOutSlice)
    {
        ulWriteSlice = ulInSlice - ulOutSlice;
    }
    else
    {
        ulWriteSlice = ulOutSlice - ulInSlice;
    }*/

    /*if(ulWriteSlice > g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteMaxTime)
    {
        g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteMaxTime = ulWriteSlice;
    }*/


    if (MDRV_OK == lRet)     /*��ǰ���ͳɹ�*/
    {
        if (VOS_OK != VOS_SmP(g_ulCbtUsbPseudoSyncSemId, 0))
        {
            LogPrint1("\r\n CBTPPM_OamUsbCbtSendData: mdrv_udi_ioctl Send Data return Error %d\n", lRet);
            return VOS_ERR;
        }

        return VOS_OK;
    }
    else if (MDRV_OK > lRet)    /*��ʱ����*/
    {
        /*��ӡ��Ϣ������UDI�ӿڵĴ�����Ϣ*/
        LogPrint1("\r\n CBTPPM_OamUsbCbtSendData: mdrv_udi_ioctl Send Data return Error %d\n", lRet);


        /*        g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteErrNum++;
        g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteErrLen    += ulDataLen;
        g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteErrValue  = (VOS_UINT32)lRet;
        g_stAcpuDebugInfo.astPortInfo[OM_USB_CBT_PORT_HANDLE].ulUSBWriteErrTime   = OM_GetSlice();*/

        return VOS_ERR; /*������ʱ������Ҫ����NULL��������*/
    }
    else    /*����������Ҫ��λ����*/
    {
        /*��ӡ��Ϣ������UDI�ӿ�*/
        LogPrint1("\r\n CBTPPM_OamUsbCbtSendData: mdrv_udi_ioctl Send Data return Error %d\n", lRet);

        VOS_ProtectionReboot(OAM_USB_SEND_ERROR, (VOS_INT)THIS_FILE_ID, (VOS_INT)__LINE__,
                             (VOS_CHAR *)&lRet, sizeof(VOS_INT32));

        return VOS_ERR;
    }

}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamUsbCbtPortClose
 ��������  : ���ڹر�USBУ׼�˿ڵĴ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_OamUsbCbtPortClose(VOS_VOID)
{
    if (VOS_ERROR == g_ulCbtPortUDIHandle)
    {
        return;
    }

    (VOS_VOID)mdrv_udi_close(g_ulCbtPortUDIHandle);

    g_ulCbtPortUDIHandle = VOS_ERROR;

    /* CBT�˿ڴ�USB��VCOM�л���UARTʱ���պ���ָ��Ϊ�գ��յ�USB״̬���ʱ�����Ͽ����� */
    if (VOS_NULL_PTR == CBTCPM_GetRcvFunc())
    {
        return;
    }

    (VOS_VOID)PPM_DisconnectGUPort(OM_LOGIC_CHANNEL_CBT);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamUsbCbtWriteDataCB
 ��������  : USB���ص�CBT�˿��첽�������ݵĻص�
 �������  : pucData:   ���͵���������
             ulDataLen: ʵ�ʷ������ݳ���
 �������  : ��
 �� �� ֵ  : ��
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : V8R1 OM_Optimize��Ŀ����
*****************************************************************************/
VOS_VOID CBTPPM_OamUsbCbtWriteDataCB(VOS_UINT8* pucVirData, VOS_UINT8* pucPhyData, VOS_INT lLen)
{
    if (lLen < 0)
    {
        LogPrint("\r\n CBTPPM_OamUsbCbtWriteDataCB: lLen < 0. \n");
    }

    /* αͬ���ӿڣ��ͷ��ź��� */
    (VOS_VOID)VOS_SmV(g_ulCbtUsbPseudoSyncSemId);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamUsbCbtStatusCB
 ��������  : ����ACPU���洦��USB����˿ڶϿ����У׼�Ͽ�
 �������  : enPortState:   �˿�״̬
 �������  : ��
 �� �� ֵ  : ��
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_OamUsbCbtStatusCB(ACM_EVT_E enPortState)
{
    /* CBT�˿ڴ�USB��VCOM�л���UARTʱ���պ���ָ��Ϊ�գ��յ�USB״̬���ʱ�����Ͽ����� */
    if (VOS_NULL_PTR == CBTCPM_GetRcvFunc())
    {
        return;
    }

    /* CBT�˿�ֻ����GU�Ķ˿ڶϿ� */
    (VOS_VOID)PPM_DisconnectGUPort(OM_LOGIC_CHANNEL_CBT);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamUsbCbtPortOpen
 ��������  : ���ڳ�ʼ��USB���ص�CBTʹ�õĶ˿�
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_OamUsbCbtPortOpen(VOS_VOID)
{
    CBTPPM_OamCbtPortDataInit(OM_USB_CBT_PORT_HANDLE,
                              CBTPPM_OamUsbCbtReadDataCB,
                              CBTPPM_OamUsbCbtWriteDataCB,
                              CBTPPM_OamUsbCbtStatusCB);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamUsbCbtReadDataCB
 ��������  : ����ACPU��������USB���ص�CBT������ͨ��ICC���͸�OMģ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_ERR/VOS_OK
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_INT32 CBTPPM_OamUsbCbtReadDataCB(VOS_VOID)
{
    ACM_WR_ASYNC_INFO                   stInfo  = {0};
    CBTCPM_RCV_FUNC                     pFunc;

    if (VOS_ERROR == g_ulCbtPortUDIHandle)
    {
        LogPrint("\r\n CBTPPM_OamUsbCbtReadDataCB: Input HANDLE  is err. \n");

        return VOS_ERR;
    }

    PAM_MEM_SET_S(&stInfo, sizeof(stInfo), 0, sizeof(stInfo));

    /* ��ȡUSB��IO CTRL�ڵĶ����� */
    if (VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, UDI_ACM_IOCTL_GET_READ_BUFFER_CB, &stInfo))
    {
        LogPrint("\r\n CBTPPM_OamUsbCbtReadDataCB:Call mdrv_udi_ioctl is Failed\n");

        return VOS_ERR;
    }

    /*lint -e40*/
    CBT_ACPU_DEBUG_TRACE((VOS_UINT8*)stInfo.pVirAddr, stInfo.u32Size, CBT_ACPU_USB_CB);
    /*lint +e40*/

    /* ���ݽ��պ��� */
    pFunc = CBTCPM_GetRcvFunc();

    if (VOS_NULL_PTR != pFunc)
    {
        if (VOS_OK != pFunc((VOS_UINT8 *)stInfo.pVirAddr, stInfo.u32Size))
        {
            /* ���ӿ�ά�ɲ���� */
        }
    }

    if(VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, UDI_ACM_IOCTL_RETUR_BUFFER_CB, &stInfo))
    {
        LogPrint("\r\n CBTPPM_OamUsbCbtReadDataCB:Call mdrv_udi_ioctl UDI_ACM_IOCTL_RETUR_BUFFER_CB is Failed\n");

        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamCbtPortDataInit
 ��������  : ���ڳ�ʼ��CBTʹ�õĶ˿��豸
 �������  : enHandle: �˿ڵľ��
             pReadCB: �ö˿�����Ķ�ȡ�ص�����
             pWriteCB: �ö˿�������첽д�ص�����
             pStateCB: �ö˿������״̬�ص�����
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERR
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_OamCbtPortDataInit(OM_PROT_HANDLE_ENUM_UINT32          enHandle,
                                        VOS_VOID                            *pReadCB,
                                        VOS_VOID                            *pWriteCB,
                                        VOS_VOID                            *pStateCB)
{
    UDI_OPEN_PARAM_S                    stUdiPara;
    ACM_READ_BUFF_INFO                  stReadBuffInfo;


    stUdiPara.devid            = UDI_ACM_OM_ID;

    stReadBuffInfo.u32BuffSize = OM_ICC_BUFFER_SIZE;
    stReadBuffInfo.u32BuffNum  = OM_DRV_MAX_IO_COUNT;

    /*g_stAcpuDebugInfo.astPortInfo[enHandle].ulUSBOpenNum++;
    g_stAcpuDebugInfo.astPortInfo[enHandle].ulUSBOpenSlice = OM_GetSlice();*/


    if (VOS_ERROR != g_ulCbtPortUDIHandle)
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit: The UDI Handle is not Null !");

        return;
    }

    /* ��CBTʹ�õ�USBͨ�� */
    g_ulCbtPortUDIHandle = mdrv_udi_open(&stUdiPara);

    if (VOS_ERROR == g_ulCbtPortUDIHandle)
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit: Open UDI ACM failed!");

        return;
    }


    /*g_stAcpuDebugInfo.astPortInfo[enHandle].ulUSBOpenOkNum++;
    g_stAcpuDebugInfo.astPortInfo[enHandle].ulUSBOpenOkSlice = OM_GetSlice();*/

    /* ����CBTʹ�õ�USBͨ������ */
    if (VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, ACM_IOCTL_RELLOC_READ_BUFF, &stReadBuffInfo))
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit, mdrv_udi_ioctl ACM_IOCTL_RELLOC_READ_BUFF Failed\r\n");

        return;
    }

    /* ע��OMʹ�õ�USB�����ݻص����� */
    if (VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, UDI_ACM_IOCTL_SET_READ_CB, pReadCB))
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit, mdrv_udi_ioctl UDI_ACM_IOCTL_SET_READ_CB Failed\r\n");

        return;
    }

    if(VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, ACM_IOCTL_SET_WRITE_CB, pWriteCB))
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit, mdrv_udi_ioctl ACM_IOCTL_SET_WRITE_CB Failed\r\n");

        return;
    }

    if(VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, ACM_IOCTL_SET_EVT_CB, pStateCB))
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit, mdrv_udi_ioctl ACM_IOCTL_SET_EVT_CB Failed\r\n");

        return;
    }

    if (VOS_OK != mdrv_udi_ioctl(g_ulCbtPortUDIHandle, ACM_IOCTL_WRITE_DO_COPY, VOS_NULL_PTR))
    {
        LogPrint("\r\n CBTPPM_OamCbtPortDataInit, mdrv_udi_ioctl ACM_IOCTL_WRITE_DO_COPY Failed\r\n");

        return;
    }


    /*g_stAcpuDebugInfo.astPortInfo[enHandle].ulUSBOpenOk2Num++;
    g_stAcpuDebugInfo.astPortInfo[enHandle].ulUSBOpenOk2Slice = OM_GetSlice();*/

    return;
}

/*****************************************************************************
 �� �� ��  : GU_OamUsbCbtPortInit
 ��������  : ����USB ��CBT�˿�ͨ���ĳ�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERR
 �޸���ʷ  :
   1.��    ��  : 2014��5��25��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_UINT32 CBTPPM_OamUsbCbtPortInit(VOS_VOID)
{
    /* CBT�˿���PCVOICE���ã���̬ע�����ݽ��պ��� */
    CBTCPM_PortRcvReg(CBTSCM_SoftDecodeDataRcv);

    /* ��̬ע�����ݷ��ͺ��� */
    CBTCPM_PortSndReg(CBTPPM_OamUsbCbtSendData);

    /* USB���ص�CBT�˿ڣ����õ�����첽�ӿڷ������ݣ�������Ҫα���ͬ���ӿڣ������ź��� */
    if(VOS_OK != VOS_SmCCreate("UCBT", 0, VOS_SEMA4_FIFO, &g_ulCbtUsbPseudoSyncSemId))
    {
        LogPrint("\r\n CBTPPM_OamUsbCbtPortInit: create g_ulCbtUsbPseudoSyncSemId failedd\r\n");

        return VOS_ERR;
    }

    /* USB���ص�CBT�˿ڳ�ʼ��UDI�豸��� */
    g_ulCbtPortUDIHandle = VOS_ERROR;

    /* CBT�˿�ͨ��CBT��CPM������ע�������ͺ��� */
    (VOS_VOID)mdrv_usb_reg_enablecb((USB_UDI_ENABLE_CB_T)CBTPPM_OamUsbCbtPortOpen);

    (VOS_VOID)mdrv_usb_reg_disablecb((USB_UDI_DISABLE_CB_T)CBTPPM_OamUsbCbtPortClose);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamVComCbtReadData
 ��������  : ��VCOM�˿ڶ�ȡCBT������
 �������  :  ucDevIndex: ����˿�
              pData    : �յ�����
              ullength : ���ݳ���

 �������  : ��
 �� �� ֵ  : VOS_ERR/VOS_OK
 �޸���ʷ  :
   1.��    ��  : 2014��5��26��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_INT CBTPPM_OamVComCbtReadData(VOS_UINT8 ucDevIndex, VOS_UINT8 *pData, VOS_UINT32 ullength)
{
    CBTCPM_RCV_FUNC                     pFunc;

    if (ucDevIndex != APP_VCOM_DEV_INDEX_LOG)
    {
        (VOS_VOID)vos_printf("\r\n CBTPPM_OamVComCbtReadData:PhyPort port is error: %d\n", ucDevIndex);

        return VOS_ERR;
    }

    g_stCbtVComAcpuDebugInfo.ulVCOMRcvNum++;
    g_stCbtVComAcpuDebugInfo.ulVCOMRcvLen += ullength;

    if ((VOS_NULL_PTR == pData) || (0 == ullength))
    {
        (VOS_VOID)vos_printf("\r\n CBTPPM_OamVComCbtReadData:Send data is NULL\n");

        return VOS_ERR;
    }

    /*lint -e40*/
    CBT_ACPU_DEBUG_TRACE((VOS_UINT8*)pData, ullength, CBT_ACPU_VCOM_CB);
    /*lint +e40*/

    pFunc = CBTCPM_GetRcvFunc();

    /* ���ݽ��պ��� */
    if (VOS_NULL_PTR == pFunc)
    {
        return VOS_ERR;
    }

    if (VOS_OK != pFunc((VOS_UINT8*)pData, ullength))
    {
        g_stCbtVComAcpuDebugInfo.ulVCOMRcvErrNum++;
        g_stCbtVComAcpuDebugInfo.ulVCOMRcvErrLen += ullength;

        (VOS_VOID)vos_printf("\r\n Info: CBTPPM_OamVComCbtReadData:Call CBT Data Rcv Func fail\n");

        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamVComCbtSendData
 ��������  : ��VCOM�˿ڷ���CBT����
 �������  : pucVirAddr:   �������ַ
             pucPhyAddr:   ����ʵ��ַ
             ulDataLen:    ���ݳ���
 �������  : ��
 �� �� ֵ  : CBTCPM_SEND_ERR/CBTCPM_SEND_OK
 �޸���ʷ  :
   1.��    ��  : 2014��5��26��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_UINT32 CBTPPM_OamVComCbtSendData(VOS_UINT8 *pucVirAddr, VOS_UINT8 *pucPhyAddr, VOS_UINT32 ulDataLen)
{
    g_stCbtVComAcpuDebugInfo.ulVCOMSendNum++;
    g_stCbtVComAcpuDebugInfo.ulVCOMSendLen += ulDataLen;

    if (VOS_OK != APP_VCOM_Send(APP_VCOM_DEV_INDEX_LOG, pucVirAddr, ulDataLen))
    {
        g_stCbtVComAcpuDebugInfo.ulVCOMSendErrNum++;
        g_stCbtVComAcpuDebugInfo.ulVCOMSendErrLen += ulDataLen;

        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamVComCbtPortInit
 ��������  : ���� Vcom CBT�˿ڵĳ�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 �޸���ʷ  :
   1.��    ��  : 2014��5��24��
     ��    ��  : h59254
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_OamVComCbtPortInit(VOS_VOID)
{
    /* CBT�˿���PCVOICE���ã���̬ע�����ݽ��պ��� */
    CBTCPM_PortRcvReg(CBTSCM_SoftDecodeDataRcv);

    /* У׼��VCOM28�����������·������ݵĻظ�����CPM��ֱ�ӷ��� */
    (VOS_VOID)APP_VCOM_RegDataCallback(APP_VCOM_DEV_INDEX_LOG, CBTPPM_OamVComCbtReadData);

    CBTCPM_PortSndReg(CBTPPM_OamVComCbtSendData);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_OamCbtPortInit
 ��������  : CBT�˿ڳ�ʼ��
 �������  : enCbtPort:CBT����˿�����
 �������  : ��
 �� �� ֵ  : VOS_VOID
 �޸���ʷ  :
  1.��    ��   : 2014��5��25��
    ��    ��   : h59254
    �޸�����   : V8R1 OM_Optimize��Ŀ����
*****************************************************************************/
VOS_VOID CBTPPM_OamCbtPortInit(VOS_VOID)
{
    OM_CHANNLE_PORT_CFG_STRU            stPortCfg = {0};

    /* ��ʼ������ */
    PAM_MEM_SET_S(&g_stCbtVComAcpuDebugInfo,
                   sizeof(g_stCbtVComAcpuDebugInfo),
                   0,
                   sizeof(g_stCbtVComAcpuDebugInfo));

    /* ��ȡOM���������ͨ�� */
    if (NV_OK != NV_Read(en_NV_Item_Om_Port_Type, &stPortCfg, sizeof(OM_CHANNLE_PORT_CFG_STRU)))
    {
        stPortCfg.enCbtPortNum = CPM_CBT_PORT_VCOM;
    }

    if (CPM_CBT_PORT_USB == stPortCfg.enCbtPortNum)
    {
        /* USB ���ص�CBT�˿ڵĳ�ʼ�� */
        (VOS_VOID)CBTPPM_OamUsbCbtPortInit();
    }
    else if (CPM_CBT_PORT_SOCKET == stPortCfg.enCbtPortNum)
    {
        CBTPPM_SocketPortInit();
    }
    /* Ĭ�϶���VCOM��CBT */
    else
    {
        /* Vcom ��CBTͨ���ĳ�ʼ�� */
        CBTPPM_OamVComCbtPortInit();
    }

    return;
}

#if (FEATURE_HISOCKET == FEATURE_ON)

/* ****************************************************************************
  ȫ�ֱ�������
**************************************************************************** */

CBTPPM_SOCKET_CTRL_INFO_STRU    g_stCbtPpmSocketCtrlInfo;

CBTPPM_SOCKET_DEBUG_INFO_STRU   g_stCbtPpmSocketDebugInfo;

/*****************************************************************************
 �� �� ��  : CBTPPM_SockBindListen
 ��������  :  socket�������󶨼���
 �������  : ��
 �������  : ��
 �� �� ֵ  :   ��
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_SockBindListen(VOS_VOID)
{
    struct sockaddr_in  sAddr;
    VOS_INT             lAddLen;

    if((CBTPPM_SOCK_NULL != g_stCbtPpmSocketCtrlInfo.listener)||
       (CBTPPM_SOCK_NULL != g_stCbtPpmSocketCtrlInfo.socket))
    {
        return;
    }

    g_stCbtPpmSocketCtrlInfo.listener = CBT_socket(AF_INET, SOCK_STREAM, 0);

    if (VOS_OK > g_stCbtPpmSocketCtrlInfo.listener)
    {
        vos_printf("[%s][%d]:socket err !\n",__FUNCTION__,__LINE__);
        g_stCbtPpmSocketCtrlInfo.listener = CBTPPM_SOCK_NULL;
        VOS_TaskDelay(100);/*delay 100 ms*/
        return ;
    }

    PAM_MEM_SET_S(&sAddr, sizeof(sAddr), 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;

    sAddr.sin_addr.s_addr = 0;

    /* �����Ķ˿ں� */
    sAddr.sin_port = htons(CBTPPM_SOCK_PORT_NUM);

    lAddLen = sizeof(struct sockaddr_in);

    /* ������Socket�󶨵���Ӧ�Ķ˿��� */
    if (CBTPPM_SOCKET_ERROR == CBT_bind(g_stCbtPpmSocketCtrlInfo.listener, (struct sockaddr *)&sAddr, lAddLen))
    {
        CBT_closesocket(g_stCbtPpmSocketCtrlInfo.listener);
        g_stCbtPpmSocketCtrlInfo.listener = CBTPPM_SOCK_NULL;
        vos_printf("[%s][%d]:bind err !\n",__FUNCTION__,__LINE__);
        return ;
    }

    /* ���÷������˼��������ͻ����� */
    if (CBTPPM_SOCKET_ERROR == CBT_listen(g_stCbtPpmSocketCtrlInfo.listener, CBTPPM_SOCKET_NUM_MAX))
    {
        CBT_closesocket(g_stCbtPpmSocketCtrlInfo.listener);
        g_stCbtPpmSocketCtrlInfo.listener = CBTPPM_SOCK_NULL;
        vos_printf("[%s][%d]:listen err !\n",__FUNCTION__,__LINE__);
        return ;
    }
}

/*****************************************************************************
 �� �� ��     : CBTPPM_SockAcceptRecv
 ��������  :  ���ڴ���ͻ��˵����󣬽��������ύ���ϲ㴦��
 �������  : ��
 �������  : ��
 �� �� ֵ  :   ��
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_SockAcceptRecv(VOS_VOID)
{
    CBT_Ip_fd_set                           listen1;
    VOS_INT32                               ret;
    VOS_INT32                               len;
    struct sockaddr_in                      from;
    CBTPPM_SOCKET                           socket;
    CBTPPM_SOCKET                           maxSocket;
    CBTCPM_RCV_FUNC                         pFunc;

    if((CBTPPM_SOCK_NULL == g_stCbtPpmSocketCtrlInfo.listener)&&(CBTPPM_SOCK_NULL == g_stCbtPpmSocketCtrlInfo.socket))
    {
        vos_printf("[%s][%d]:listener && socket err !\n",__FUNCTION__,__LINE__);
        return;
    }

    HI_FD_ZERO(&listen1);

    HI_FD_SET(g_stCbtPpmSocketCtrlInfo.listener, &listen1);

    if(CBTPPM_SOCK_NULL != g_stCbtPpmSocketCtrlInfo.socket)
    {
        HI_FD_SET(g_stCbtPpmSocketCtrlInfo.socket, &listen1);
    }

    maxSocket = g_stCbtPpmSocketCtrlInfo.socket > g_stCbtPpmSocketCtrlInfo.listener ? g_stCbtPpmSocketCtrlInfo.socket : g_stCbtPpmSocketCtrlInfo.listener;

    ret = CBT_select((int)maxSocket + 1,&listen1, NULL, NULL, NULL);
    if(ret < 0)
    {
        vos_printf("[%s][%d]:select err !\n",__FUNCTION__,__LINE__);
        return;
    }

    if((g_stCbtPpmSocketCtrlInfo.listener != CBTPPM_SOCK_NULL) && (HI_FD_ISSET(g_stCbtPpmSocketCtrlInfo.listener, &listen1)))
    {
        len = sizeof(struct sockaddr_in);
        socket = CBT_accept(g_stCbtPpmSocketCtrlInfo.listener, (struct sockaddr *)&from, (int *)&len);
        if(socket < 0)
        {
            CBT_closesocket(g_stCbtPpmSocketCtrlInfo.listener);
            g_stCbtPpmSocketCtrlInfo.listener = CBTPPM_SOCK_NULL;
            vos_printf("[%s][%d]:accept err !\n",__FUNCTION__,__LINE__);

            return;
        }

        g_stCbtPpmSocketDebugInfo.ulListernNum1++;

        /* a new socket reconnct*/
        if(CBTPPM_SOCK_NULL != g_stCbtPpmSocketCtrlInfo.socket)
        {
            (VOS_VOID)VOS_SmP(g_stCbtPpmSocketCtrlInfo.SmClose, 0);
            CBT_closesocket(g_stCbtPpmSocketCtrlInfo.socket);
            (VOS_VOID)VOS_SmV(g_stCbtPpmSocketCtrlInfo.SmClose);
        }

        g_stCbtPpmSocketDebugInfo.ulListernNum2++;

        (VOS_VOID)VOS_SmP(g_stCbtPpmSocketCtrlInfo.SmClose, 0);
        g_stCbtPpmSocketCtrlInfo.socket = socket;
        (VOS_VOID)VOS_SmV(g_stCbtPpmSocketCtrlInfo.SmClose);
    }

    if((g_stCbtPpmSocketCtrlInfo.socket != CBTPPM_SOCK_NULL) && (HI_FD_ISSET(g_stCbtPpmSocketCtrlInfo.socket, &listen1)))
    {
        ret = CBT_recv((int)g_stCbtPpmSocketCtrlInfo.socket, g_stCbtPpmSocketCtrlInfo.aucBuf, CBTPPM_SOCK_MSG_LEN, 0);
        if(ret <= 0)    /*�ͻ��˶Ͽ�֮�����˻�����ܵ�����Ϊ0�����ݰ�*/
        {
            (VOS_VOID)VOS_SmP(g_stCbtPpmSocketCtrlInfo.SmClose, 0);
            CBT_closesocket(g_stCbtPpmSocketCtrlInfo.socket);
            g_stCbtPpmSocketCtrlInfo.socket   = CBTPPM_SOCK_NULL;
            (VOS_VOID)VOS_SmV(g_stCbtPpmSocketCtrlInfo.SmClose);

            vos_printf("[%s][%d]:rcv err !\n",__FUNCTION__,__LINE__);
            return;
        }

        g_stCbtPpmSocketDebugInfo.ulRcvData += ret;

        /*�����յ��������ύ���ϲ㴦��*/
        pFunc = CBTCPM_GetRcvFunc();

        if (VOS_NULL_PTR != pFunc)
        {
            if (VOS_OK != pFunc((VOS_UINT8 *)g_stCbtPpmSocketCtrlInfo.aucBuf, ret))
            {
                /* ���ӿ�ά�ɲ���� */
                g_stCbtPpmSocketDebugInfo.ulFailToProcess++;
            }
        }
    }

}

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketServerTask
 ��������  :  socket�������̣߳���������������˺Ϳͻ��˵���������
  �������  : ��
  �������  : ��
 �� �� ֵ  :   ��
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/

VOS_VOID CBTPPM_SocketServerTask(VOS_VOID)
{
    for( ; ; )
    {
        CBTPPM_SockBindListen();
        CBTPPM_SockAcceptRecv();
    }

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_SockIsEnable
 ��������  :  socket�����Ƿ�֧��
  �������  : ��
  �������  : ��
 �� �� ֵ  :  VOS_TRUE/VOS_FALSE
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/

VOS_BOOL CBTPPM_SockIsEnable(VOS_VOID)
{
    OM_CHANNLE_PORT_CFG_STRU            stPortCfg;

    /* ��ȡOM���������ͨ�� */
    if (NV_OK != NV_Read(en_NV_Item_Om_Port_Type, &stPortCfg, sizeof(OM_CHANNLE_PORT_CFG_STRU)))
    {
        stPortCfg.enCbtPortNum = CPM_CBT_PORT_VCOM;
    }

    if (CPM_CBT_PORT_SOCKET != stPortCfg.enCbtPortNum)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketTaskInit
 ��������  :  ��ʼ��socket��������
 �������  : ��
 �������  : ��
 �� �� ֵ  :   VOS_OK/VOS_ERR
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/
VOS_UINT32 CBTPPM_SocketTaskInit(VOS_VOID)
{
    VOS_UINT32 ulRelVal;

    if((BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
        && (VOS_TRUE == CBTPPM_SockIsEnable()))
    {
        g_stCbtPpmSocketCtrlInfo.listener = CBTPPM_SOCK_NULL;

        g_stCbtPpmSocketCtrlInfo.socket = CBTPPM_SOCK_NULL;

        PAM_MEM_SET_S(&g_stCbtPpmSocketDebugInfo,
                       sizeof(g_stCbtPpmSocketDebugInfo),
                       0,
                       sizeof(g_stCbtPpmSocketDebugInfo));

        if(VOS_OK != VOS_SmMCreate("CBTSOCK", VOS_SEMA4_PRIOR | VOS_SEMA4_INVERSION_SAFE, &(g_stCbtPpmSocketCtrlInfo.SmClose)))
        {
            return VOS_ERR;
        }

        /* ����SOCKET���ݵ��Դ������� */
        ulRelVal = VOS_RegisterSelfTaskPrio(ACPU_FID_CBT,
                         (VOS_TASK_ENTRY_TYPE)CBTPPM_SocketServerTask,
                         COMM_SOCK_SELFTASK_PRIO, CBT_SOCKET_TASK_STACK_SIZE);
        if ( VOS_NULL_BYTE == ulRelVal )
        {
            return VOS_ERR;
        }

    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketSendData
 ��������  : ��socket����CBT����
 �������  : pucVirAddr �������ַ
                           pucPhyAddr ����ʵ��ַ
                           ulDataLen ���ݳ���
 �������  :
 �� �� ֵ  :   VOS_OK/VOS_ERR
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/

VOS_UINT32 CBTPPM_SocketSendData(VOS_UINT8 *pucVirAddr, VOS_UINT8 *pucPhyAddr, VOS_UINT32 ulDataLen)
{
    CBTPPM_SOCKET socket;
    VOS_INT       nSndNum;

    (VOS_VOID)VOS_SmP(g_stCbtPpmSocketCtrlInfo.SmClose, 0);

    socket = g_stCbtPpmSocketCtrlInfo.socket;

    if (CBTPPM_SOCK_NULL == socket)
    {
        (VOS_VOID)VOS_SmV(g_stCbtPpmSocketCtrlInfo.SmClose);
        vos_printf("[%s][%d]:socket err !\n",__FUNCTION__,__LINE__);
        return VOS_ERR;
    }

    /* ����send������ͨ��socket���ͳ�ȥ����TCP */
    nSndNum = CBT_send(socket, pucVirAddr, ulDataLen, 0);

    (VOS_VOID)VOS_SmV(g_stCbtPpmSocketCtrlInfo.SmClose);

    if (nSndNum != ulDataLen)
    {
        g_stCbtPpmSocketDebugInfo.ulFailToSend++;

        return VOS_ERR;
    }

    g_stCbtPpmSocketDebugInfo.ulSndData += ulDataLen;

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketPortInit
 ��������  :  ����socket��CBT�˿�ͨ���ĳ�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  :  ��
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/

VOS_VOID CBTPPM_SocketPortInit(VOS_VOID)
{
    /* CBT�˿���PCVOICE���ã���̬ע�����ݽ��պ��� */
    CBTCPM_PortRcvReg(CBTSCM_SoftDecodeDataRcv);

    CBTCPM_PortSndReg(CBTPPM_SocketSendData);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketCtrlInfoShow
 ��������  :  ��ӡ������Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  :  ��
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_SocketCtrlInfoShow(VOS_VOID)
{
    (VOS_VOID)vos_printf("CBTPPM listen: %d.\r\n", g_stCbtPpmSocketCtrlInfo.listener);

    (VOS_VOID)vos_printf("CBTPPM socket: %d.\r\n", g_stCbtPpmSocketCtrlInfo.socket);

    return;
}

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketDebugInfoShow
 ��������  :  ��ӡ������Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  :  ��
 �޸���ʷ  :
   1.��    ��  : 2015��12��21��
     ��    ��  : x51137
     �޸�����  : Creat Function
*****************************************************************************/
VOS_VOID CBTPPM_SocketDebugInfoShow(VOS_VOID)
{
    (VOS_VOID)vos_printf("CBTPPM ulListernNum1: %d.\r\n", g_stCbtPpmSocketDebugInfo.ulListernNum1);

    (VOS_VOID)vos_printf("CBTPPM ulListernNum2: %d.\r\n", g_stCbtPpmSocketDebugInfo.ulListernNum2);

    (VOS_VOID)vos_printf("CBTPPM ulRcvData: %d.\r\n", g_stCbtPpmSocketDebugInfo.ulRcvData);

    (VOS_VOID)vos_printf("CBTPPM ulFailToProcess: %d.\r\n", g_stCbtPpmSocketDebugInfo.ulFailToProcess);

    (VOS_VOID)vos_printf("CBTPPM ulSndData: %d.\r\n", g_stCbtPpmSocketDebugInfo.ulSndData);

    (VOS_VOID)vos_printf("CBTPPM ulFailToSend: %d.\r\n", g_stCbtPpmSocketDebugInfo.ulFailToSend);

    return;
}


#else

/*****************************************************************************
 �� �� ��  : CBTPPM_SocketTaskInit
 ��������  : CBT�˿ڳ�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERR
 �޸���ʷ  :
  1.��    ��   : 2015��12��21��
    ��    ��   : x51137
    �޸�����   :  Creat Function
*****************************************************************************/
VOS_UINT32 CBTPPM_SocketTaskInit(VOS_VOID)
{
    return VOS_OK;
}

VOS_VOID CBTPPM_SocketPortInit(VOS_VOID)
{
    return;
}

#endif

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif




