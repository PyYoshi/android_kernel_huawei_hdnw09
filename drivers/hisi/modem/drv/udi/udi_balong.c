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

/*
#include <vxWorks.h>
#include <logLib.h>
#include <usrLib.h>
#include <string.h>
#include <stdio.h>
#include "BSP_UDI.h"
#include "BSP_UDI_DRV.h"
*/
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>

#include "udi_balong.h"


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define UDI_MAX_MAIN_DEV_NUM UDI_DEV_MAX
#define UDI_MAX_DEV_TYPE_NUM 32
#define UDI_MAX_OPEN_NODE_NUM 64
//#define UDI_MAX_INIT_FUNC_NUM UDI_TYPE_MAX
#define UDI_OPEN_NODE_HEADER  0x5A0000

/* Mutex ��Ϊ��װ */
#if 0
#define UDI_MUTEX_T SEM_ID
#define UDI_MTX_CREATE(mtx) \
do {\
    /* ʹ�ÿ�����Mutex */\
    mtx = semMCreate(SEM_Q_FIFO | SEM_DELETE_SAFE);\
}while((0))
#define UDI_MTX_DELETE(mtx) {semDelete(mtx); mtx = NULL;}
#define UDI_MTX_ENTER(mtx) semTake(mtx, WAIT_FOREVER)
#define UDI_MTX_LEAVE(mtx) semGive(mtx)
#endif

/**************************************************************************
  ���Ͷ���
**************************************************************************/
/* �豸�򿪽ڵ� */
typedef struct tagUDI_OPEN_NODE
{
	int bOpen;
	void* pPrivate;
	UDI_DRV_INTEFACE_TABLE *pstDrvTable;
}UDI_OPEN_NODE;

/* �豸ʵ������ */
typedef struct tagUDI_DEV_INSTANCE
{
	//unsigned int u32MagicNum;				/* ����� */
	//unsigned int u32DevId;					/* �豸ID */
	unsigned int u32Capability;					/* �豸���� */
	//BSP_U8* strDevName;					/* �豸��,��������ϵͳ��׼�豸 */
	UDI_DRV_INTEFACE_TABLE *pDrvInterface;  /* �ӿڻص��б� */
	void* pPrivate;					/* ÿ������˽��ȫ�� */
}UDI_DEV_INSTANCE;


/**************************************************************************
  ȫ�ֱ���
**************************************************************************/
UDI_OPEN_NODE			g_openNodeTable[UDI_MAX_OPEN_NODE_NUM];
unsigned int					g_openNodeCurPos = 0;
UDI_DEV_INSTANCE			g_deviceTable[UDI_MAX_MAIN_DEV_NUM][UDI_MAX_DEV_TYPE_NUM];
extern UDI_ADP_INIT_CB_T	g_udiInitFuncTable[UDI_DEV_MAX+1];
/*UDI_MUTEX_T g_udiMtxOpen;*/
struct semaphore			g_udiMtxOpen;

/**************************************************************************
  ��ʵ��
**************************************************************************/
#define UDI_IDX_TO_HANDLE(idx) ((UDI_HANDLE)(UDI_OPEN_NODE_HEADER | (idx)))
#define UDI_HANDLE_TO_IDX(hdl) ((unsigned int)((hdl) & 0xFFFF))

#define UDI_IS_INVALID_TABLE(pstDrvTable) \
    (0 == (unsigned long)(pstDrvTable) || (unsigned long)(-1) == (unsigned long)(pstDrvTable))

#define UDI_PARSE_DEV_ID(devId, mainId, devType) \
do{\
	mainId = UDI_GET_MAIN_DEV_ID(devId);\
	devType = UDI_GET_DEV_TYPE(devId);\
	\
	bsp_assert(mainId < UDI_MAX_MAIN_DEV_NUM);\
	bsp_assert(devType < UDI_MAX_DEV_TYPE_NUM);\
}while(0)
/**************************************************************************
  �ڲ�����
**************************************************************************/
static int udi_check_and_get_index(UDI_HANDLE handle)
{
	int index;

	index = handle & 0xFFFF;
	if (((handle & 0xFF0000)!= UDI_OPEN_NODE_HEADER) || 
		index >= UDI_MAX_OPEN_NODE_NUM) {
		return -1;
	}

	if (UDI_IS_INVALID_TABLE(g_openNodeTable[index].pstDrvTable)) {
		return (-1);
	}

	return index;
}

static UDI_HANDLE udiGetOutOpenNode(void)
{
	unsigned int u32Cnt;
	UDI_HANDLE handle = UDI_INVALID_HANDLE;

	down(&g_udiMtxOpen);
	/* ���ȴӵ�ǰλ���� */
	for (u32Cnt = g_openNodeCurPos; u32Cnt < UDI_MAX_OPEN_NODE_NUM; u32Cnt++)
	{
		if (0 == g_openNodeTable[u32Cnt].bOpen)
		{
			handle = UDI_IDX_TO_HANDLE(u32Cnt);
			break;
		}
	}

	/* ����, �ٴ�ͷ�� */
	if(UDI_INVALID_HANDLE == handle)
	{
		for (u32Cnt = 0; u32Cnt < g_openNodeCurPos; u32Cnt++)
		{
			if (0 == g_openNodeTable[u32Cnt].bOpen)
			{
				handle = UDI_IDX_TO_HANDLE(u32Cnt);
				break;
			}
		}
	}

	/* �ҵ�һ�����õ�handle */
	if (UDI_INVALID_HANDLE != handle)
	{
		g_openNodeCurPos = (u32Cnt+1) % UDI_MAX_OPEN_NODE_NUM;
		g_openNodeTable[u32Cnt].bOpen = 1;
	}
	up(&g_udiMtxOpen);

	return handle;
}

static int udiReturnOpenNode(unsigned int u32Idx)
{
	bsp_assert(u32Idx < UDI_MAX_OPEN_NODE_NUM);
    /*lint 661*/
	if( u32Idx >=UDI_MAX_OPEN_NODE_NUM )
	{
		printk("BSP_UDI_SetCapability para error: u32Idx=%u\n", u32Idx);
		return (-1);
	}
	down(&g_udiMtxOpen);
	g_openNodeTable[u32Idx].bOpen = 0;
	g_openNodeTable[u32Idx].pstDrvTable = (UDI_DRV_INTEFACE_TABLE*)(-1);
	up(&g_udiMtxOpen);

	return 0;
}

/**************************************************************************
  �ӿ�ʵ��
**************************************************************************/

/*****************************************************************************
* �� �� ��  : BSP_UDI_SetPrivate
*
* ��������  : ���������ڲ�˽������
*
* �������  : devId: �豸ID
*             pPrivate: ˽������
* �������  : ��
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
int BSP_UDI_SetPrivate(UDI_DEVICE_ID_E devId, void* pPrivate)
{
	unsigned int u32MainId;
	unsigned int u32DevType;
    /*lint  e661 e662*/
	UDI_PARSE_DEV_ID(devId, u32MainId, u32DevType);
	if((u32MainId >= UDI_MAX_MAIN_DEV_NUM)||(u32DevType >= UDI_MAX_DEV_TYPE_NUM))
	{
      printk("BSP_UDI_SetCapability para error: u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);
	  return (-1);
	}
	g_deviceTable[u32MainId][u32DevType].pPrivate = pPrivate;

	return 0;
}

/*****************************************************************************
* �� �� ��  : BSP_UDI_SetCapability
*
* ��������  : �����豸����ֵ
*
* �������  : devId: �豸ID
*             u32Capability: �豸����ֵ
* �������  : ��
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
int BSP_UDI_SetCapability(UDI_DEVICE_ID_E devId, unsigned int u32Capability)
{
	unsigned int u32MainId;
	unsigned int u32DevType;

    /*lint e662 e661*/
	UDI_PARSE_DEV_ID(devId, u32MainId, u32DevType);
	if((u32MainId >= UDI_MAX_MAIN_DEV_NUM)||(u32DevType >= UDI_MAX_DEV_TYPE_NUM))
	{
      printk("BSP_UDI_SetCapability para error: u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);
	  return (-1);
	}
	g_deviceTable[u32MainId][u32DevType].u32Capability = u32Capability;
	//printk("BSP_UDI_SetCapability **********************  u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);


	return 0;
}

/*****************************************************************************
* �� �� ��  : BSP_UDI_SetInterfaceTable
*
* ��������  : �����豸�ص������б�(����������)
*
* �������  : devId: �豸ID
*             pDrvInterface: ������Ļص������б�
* �������  : ��
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
int BSP_UDI_SetInterfaceTable(UDI_DEVICE_ID_E devId, UDI_DRV_INTEFACE_TABLE *pDrvInterface)
{
	unsigned int u32MainId;
	unsigned int u32DevType;
    /*lint e661 e662*/
	UDI_PARSE_DEV_ID(devId, u32MainId, u32DevType);
	if((u32MainId >= UDI_MAX_MAIN_DEV_NUM)||(u32DevType >= UDI_MAX_DEV_TYPE_NUM))
	{
      printk("BSP_UDI_SetInterfaceTable para error: u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);
	  return (-1);
	}
	g_deviceTable[u32MainId][u32DevType].pDrvInterface = pDrvInterface;
	//printk("BSP_UDI_SetInterfaceTable  **********************  u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);

	return 0;
}

/*****************************************************************************
* �� �� ��  : bsp_udi_init
*
* ��������  : UDI ģ���ʼ��
*
* �������  : ��
* �������  : ��
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
static int __init bsp_udi_init(void)
{
	UDI_ADP_INIT_CB_T initCB;
	unsigned int u32Cnt;

	memset(g_deviceTable, 0, sizeof(g_deviceTable));
	memset(g_openNodeTable, 0, sizeof(g_openNodeTable));

	sema_init(&g_udiMtxOpen, 1);

	/* ���ó�ʼ������ */
	for (u32Cnt = 0; u32Cnt < (unsigned int)UDI_DEV_MAX; u32Cnt++)
	{
		initCB = g_udiInitFuncTable[u32Cnt];
		if (initCB)
		{
			if (initCB() != 0)
			{
				printk(KERN_ERR "BSP_MODU_UDI usr initCB fail, line:%d\n", __LINE__);
				return (-1);
			}
		}
	}
	return 0;
}

/*****************************************************************************
* �� �� ��  : udi_get_capability
*
* ��������  : �����豸ID��ȡ��ǰ�豸֧�ֵ�����
*
* �������  : devId: �豸ID
* �������  : ��
* �� �� ֵ  : ֧�ֵ�����ֵ
*****************************************************************************/
int udi_get_capability(UDI_DEVICE_ID_E devId)
{
	unsigned int u32MainId;
	unsigned int u32DevType;

	UDI_PARSE_DEV_ID(devId, u32MainId, u32DevType);
	/* lint e661 e662*/
	if((u32MainId >= UDI_MAX_MAIN_DEV_NUM)||(u32DevType >= UDI_MAX_DEV_TYPE_NUM))
	{
      printk("udi_get_capability para error: u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);
	  return (-1);
	}
	return (int)g_deviceTable[u32MainId][u32DevType].u32Capability;
}
#if 0
/*****************************************************************************
* �� �� ��  : BSP_UDI_GetPrivate
*
* ��������  : �õ��豸 �����ڲ�˽������
*
* �������  : handle: �豸��handle
* �������  : ��
* �� �� ֵ  : �豸�ڲ�˽������
*****************************************************************************/
int BSP_UDI_GetPrivate(UDI_HANDLE handle)
{
	unsigned int u32Idx = 0;

	if (UDI_INVALID_HANDLE == handle)
	{
		printk(KERN_ERR "BSP_MODU_UDI can't find open node, line:%d\n", __LINE__);
		return (int)NULL;
	}

	u32Idx = UDI_HANDLE_TO_IDX(handle);

	return (int)g_openNodeTable[u32Idx].pPrivate;
}
#endif
/*****************************************************************************
* �� �� ��  : mdrv_udi_open
*
* ��������  : ���豸(����ͨ��)
*
* �������  : pParam: �豸�Ĵ����ò���
* �������  : ��
* �� �� ֵ  : -1:ʧ�� / ����:�ɹ�
*****************************************************************************/
UDI_HANDLE mdrv_udi_open(UDI_OPEN_PARAM_S *pParam)
{
	unsigned int u32MainId = 0;
	unsigned int u32DevType = 0;
	unsigned int u32Idx = 0;
	UDI_HANDLE handle;
	UDI_DRV_INTEFACE_TABLE *pstDrvTable;

	if (NULL == pParam)
	{
		goto UDI_OPEN_ERR;
	}
	/*lint e662 e661*/
	UDI_PARSE_DEV_ID(pParam->devid, u32MainId, u32DevType);
   if((u32MainId >= UDI_MAX_MAIN_DEV_NUM)||(u32DevType >= UDI_MAX_DEV_TYPE_NUM))
	{
      /*printk("mdrv_udi_open para error: u32MainId=%u u32DevType=%u\n", u32MainId, u32DevType);*/
	  goto UDI_OPEN_ERR;
	}
	/* ����һ�����õĽڵ� */
	handle = udiGetOutOpenNode();
	if (UDI_INVALID_HANDLE == handle)
	{
		/*printk(KERN_ERR "BSP_MODU_UDI can't find open node, line:%d\n", __LINE__);*/
		goto UDI_OPEN_ERR;
	}
	u32Idx = UDI_HANDLE_TO_IDX(handle);

	/* �����û��ص����� */
	pstDrvTable = g_deviceTable[u32MainId][u32DevType].pDrvInterface;
	if (NULL == pstDrvTable || NULL == pstDrvTable->udi_open_cb)
	{
		/*printk(KERN_ERR "BSP_MODU_UDI usr open cb is NULL, line:%d\n", __LINE__);*/
		goto UDI_OPEN_ERR_RET_NODE;
	}
	if (pstDrvTable->udi_open_cb(pParam, handle))
	{
		/*printk(KERN_ERR "BSP_MODU_UDI usr open fail, line:%d\n", __LINE__);*/
		goto UDI_OPEN_ERR_RET_NODE;
	}

	/* ��������˽������ */
	g_openNodeTable[u32Idx].pstDrvTable =
	g_deviceTable[u32MainId][u32DevType].pDrvInterface;
	g_openNodeTable[u32Idx].pPrivate =
	g_deviceTable[u32MainId][u32DevType].pPrivate;

	return handle;

UDI_OPEN_ERR_RET_NODE:
	(void)udiReturnOpenNode(u32Idx);
UDI_OPEN_ERR:
	return UDI_INVALID_HANDLE;
}
EXPORT_SYMBOL(mdrv_udi_open);

/*****************************************************************************
* �� �� ��  : mdrv_udi_close
*
* ��������  : �ر��豸(����ͨ��)
*
* �������  : handle: �豸��handle
* �������  : ��
* �� �� ֵ  : ��
*****************************************************************************/
int mdrv_udi_close(UDI_HANDLE handle)
{
	int index;
	int ret = -1;
	UDI_DRV_INTEFACE_TABLE *drv_table;
	void *priv;
	
	index = udi_check_and_get_index(handle);
	if (index < 0) {
		return index;
	}

	drv_table = g_openNodeTable[index].pstDrvTable;
	if (!(drv_table->udi_close_cb)) {
		return ret;
	}
	
	priv = g_openNodeTable[index].pPrivate; 
	ret = drv_table->udi_close_cb(priv);
	(void)udiReturnOpenNode((unsigned int)index);

	return ret;
}
EXPORT_SYMBOL(mdrv_udi_close);

/*****************************************************************************
* �� �� ��  : mdrv_udi_write
*
* ��������  : ����д
*
* �������  : handle:  �豸��handle
*             pMemObj: buffer�ڴ� �� �ڴ���������
*             u32Size: ����д�ߴ� �� �ڴ���������ɲ�����
* �������  :
*
* �� �� ֵ  : ����ֽ��� �� �ɹ�/ʧ��
*****************************************************************************/
int mdrv_udi_write(UDI_HANDLE handle, void* pMemObj, unsigned int u32Size)
{
	int index;
	int ret = -1;
	UDI_DRV_INTEFACE_TABLE *drv_table;
	void *priv;
	
	index = udi_check_and_get_index(handle);
	if (index < 0) {
		return index;
	}

	drv_table = g_openNodeTable[index].pstDrvTable;
	if (!(drv_table->udi_write_cb)) {
		return ret;
	}
	
	priv = g_openNodeTable[index].pPrivate;	
	ret = drv_table->udi_write_cb(priv, pMemObj, u32Size);

	return ret;
}
EXPORT_SYMBOL(mdrv_udi_write);

/*****************************************************************************
* �� �� ��  : mdrv_udi_read
*
* ��������  : ���ݶ�
*
* �������  : handle:  �豸��handle
*             pMemObj: buffer�ڴ� �� �ڴ���������
*             u32Size: ���ݶ��ߴ� �� �ڴ���������ɲ�����
* �������  :
*
* �� �� ֵ  : ����ֽ��� �� �ɹ�/ʧ��
*****************************************************************************/
int mdrv_udi_read(UDI_HANDLE handle, void* pMemObj, unsigned int u32Size)
{
	int index;
	int ret = -1;
	UDI_DRV_INTEFACE_TABLE *drv_table;
	void *priv;
	
	index = udi_check_and_get_index(handle);
	if (index < 0) {
		return index;
	}

	drv_table = g_openNodeTable[index].pstDrvTable;
	if (!(drv_table->udi_read_cb)) {
		return ret;
	}
	
	priv = g_openNodeTable[index].pPrivate; 
	ret = drv_table->udi_read_cb(priv, pMemObj, u32Size);

	return ret;
}
EXPORT_SYMBOL(mdrv_udi_read);

/*****************************************************************************
* �� �� ��  : mdrv_udi_ioctl
*
* ��������  : ����ͨ����������
*
* �������  : handle: �豸��handle
*             u32Cmd: IOCTL������
*             pParam: ��������
* �������  :
*
* �� �� ֵ  : �ɹ�/ʧ��
*****************************************************************************/
int mdrv_udi_ioctl(UDI_HANDLE handle, unsigned int u32Cmd, void* pParam)
{
	int index;
	int ret = -1;
	UDI_DRV_INTEFACE_TABLE *drv_table;
	void *priv;
	
	index = udi_check_and_get_index(handle);
	if (index < 0) {
		return index;
	}

	drv_table = g_openNodeTable[index].pstDrvTable;
	if (!(drv_table->udi_ioctl_cb)) {
		return ret;
	}
	
	priv = g_openNodeTable[index].pPrivate; 
	ret = drv_table->udi_ioctl_cb(priv, u32Cmd, pParam);

	return ret;
}

EXPORT_SYMBOL(mdrv_udi_ioctl);

#if 0
/**************************************************************************
  ������Ϣʵ��
**************************************************************************/
int BSP_UDI_DumpOpenNode(int bDumpAll)
{
	unsigned int cnt;

	printk("+- BSP UDI OpenNode Info Dump:\n");
	printk("|-- g_openNodeCurPos:%d\n", g_openNodeCurPos);
	for (cnt = 0; cnt < UDI_MAX_OPEN_NODE_NUM; cnt++)
	{
		if (bDumpAll || g_openNodeTable[cnt].bOpen)
		{
			/*
			printk("|-+-- OpenNode idx:           %d:\n", cnt);
			printk("  |-- OpenNode private:       0x%x\n", g_openNodeTable[cnt].pPrivate);
			printk("  |-- OpenNode close cb:      0x%x\n", g_openNodeTable[cnt].pstDrvTable->udi_close_cb);
			printk("  |-- OpenNode open  cb:      0x%x\n", g_openNodeTable[cnt].pstDrvTable->udi_open_cb);
			printk("  |-- OpenNode write cb:      0x%x\n", g_openNodeTable[cnt].pstDrvTable->udi_write_cb);
			printk("  |-- OpenNode read  cb:      0x%x\n", g_openNodeTable[cnt].pstDrvTable->udi_read_cb);
			printk("  |-- OpenNode ioctl cb:      0x%x\n", g_openNodeTable[cnt].pstDrvTable->udi_ioctl_cb);
			*/
		}
	}
	return 0;
}

#define BSP_UDI_PRINT_DEVID_BEG(devid) {switch(devid){
#define BSP_UDI_PRINT_DEVID_ITEM(name) case(name):return (#name);
#define BSP_UDI_PRINT_DEVID_END() default:return ("unknown"); }}

static BSP_U8* udiGetDevID(UDI_DEVICE_ID_E devid)
{
	BSP_UDI_PRINT_DEVID_BEG(devid);

	BSP_UDI_PRINT_DEVID_ITEM(UDI_ACM_CTRL_ID);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_ACM_AT_ID);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_ACM_SHELL_ID);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_ACM_3G_DIAG_ID);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_ACM_MODEM_ID);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_NCM_NDIS_ID);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_NCM_CTRL_ID);

	BSP_UDI_PRINT_DEVID_END();
}

static BSP_U8* udiGetDevMainID(UDI_DEVICE_MAIN_ID mainId)
{
	BSP_UDI_PRINT_DEVID_BEG(mainId);

	BSP_UDI_PRINT_DEVID_ITEM(UDI_DEV_USB_ACM);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_DEV_USB_NCM);
	BSP_UDI_PRINT_DEVID_ITEM(UDI_DEV_ICC);

	BSP_UDI_PRINT_DEVID_END();
}

static BSP_U8* udiGetDevTypeID(unsigned int mainId, unsigned int type)
{
	switch(mainId)
	{
		case UDI_DEV_USB_ACM:
			BSP_UDI_PRINT_DEVID_BEG(type);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_ACM_CTRL_ID);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_ACM_AT);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_ACM_SHELL);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_ACM_LTE_DIAG);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_ACM_3G_DIAG);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_ACM_MODEM);
			BSP_UDI_PRINT_DEVID_END();
			break;
		case UDI_DEV_USB_NCM:
			BSP_UDI_PRINT_DEVID_BEG(type);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_NCM_NDIS);
			BSP_UDI_PRINT_DEVID_ITEM(UDI_USB_NCM_CTRL);
			BSP_UDI_PRINT_DEVID_END();
			break;
		default:
			return "unknown";
	}
}

int BSP_UDI_DumpDev(int bAll)
{
	unsigned int mainId, typeId;

	printk("+- BSP UDI OpenNode Info Dump:\n");
	for (mainId = 0; mainId < UDI_MAX_MAIN_DEV_NUM; mainId++)
	{
		for (typeId = 0; typeId < UDI_MAX_DEV_TYPE_NUM; typeId++)
		{
			if (bAll || NULL != g_deviceTable[mainId][typeId].pDrvInterface)
			{
				/*
				printk("|-Dev Name:%s\n", udiGetDevID(UDI_BUILD_DEV_ID(mainId, typeId)));
				printk("|-+-- mainId:%s, typeId:%s\n", udiGetDevMainID(mainId), udiGetDevTypeID(mainId, typeId));
				printk("  |-- dev private:       0x%x\n", g_deviceTable[mainId][typeId].pPrivate);
				printk("  |-- dev close cb:      0x%x\n", g_deviceTable[mainId][typeId].pDrvInterface->udi_close_cb);
				printk("  |-- dev open  cb:      0x%x\n", g_deviceTable[mainId][typeId].pDrvInterface->udi_open_cb);
				printk("  |-- dev write cb:      0x%x\n", g_deviceTable[mainId][typeId].pDrvInterface->udi_write_cb);
				printk("  |-- dev read  cb:      0x%x\n", g_deviceTable[mainId][typeId].pDrvInterface->udi_read_cb);
				printk("  |-- dev ioctl cb:      0x%x\n", g_deviceTable[mainId][typeId].pDrvInterface->udi_ioctl_cb);
				printk("  |  \n");
				*/
			}
		}
	}
	return 0;
}
#endif
arch_initcall(bsp_udi_init);

#ifdef __cplusplus
}
#endif /* __cplusplus */


