/*
*********************************************************************************************************
*	模块名称 : MODEBUS 通信模块 (从站）
*	文件名称 : modbus_slave.h
*	版    本 : V1.4
*	说    明 : 头文件
*********************************************************************************************************
*/

#ifndef __RTU_MODBUY_SERVICE_H__
#define __RTU_MODBUY_SERVICE_H__
#include <stdint.h>
#include "RTUmodbus_CRC.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "project.h"
#include <string.h>


#define SADDR485	0x01				//地址：1
#define SBAUD485	9600			//115200


/* 01H 读强制单线圈 */
/* 05H 写强制单线圈 */
#define REG_D01		0x0101
#define REG_D02		0x0102
#define REG_D03		0x0103
#define REG_D04		0x0104
#define REG_DXX 	REG_D04

/* 02H 读取输入状态 */
#define REG_T01		0x0201
#define REG_T02		0x0202
#define REG_T03		0x0203
#define REG_TXX		REG_T03

/* 03H 读保持寄存器 */
/* 06H 写保持寄存器 */
/* 10H 写多个保存寄存器 */
#define SLAVE_REG_P0000		0x0000			
#define SLAVE_REG_P0001		0x0001			
#define SLAVE_REG_P0002		0x0002			
#define SLAVE_REG_P0003		0x0003			
#define SLAVE_REG_P0004		0x0004			
#define SLAVE_REG_P0005		0x0005			
#define SLAVE_REG_P0006		0x0006			
#define SLAVE_REG_P0007		0x0007			
#define SLAVE_REG_P0008		0x0008			
#define SLAVE_REG_P0009		0x0009			
#define SLAVE_REG_P000A		0x000A			
#define SLAVE_REG_P000B		0x000B			
#define SLAVE_REG_P000C		0x000C			
#define SLAVE_REG_P000D		0x000D			
#define SLAVE_REG_P000E		0x000E			
#define SLAVE_REG_P000F		0x000F			
#define SLAVE_REG_P0010		0x0010			
#define SLAVE_REG_P0011		0x0011			
#define SLAVE_REG_P0012		0x0012			
#define SLAVE_REG_P0013		0x0013			
#define SLAVE_REG_P0014		0x0014			
#define SLAVE_REG_P0015		0x0015			
#define	SLAVE_REG_P0016		0x0016			
#define SLAVE_REG_P0017		0x0017			
#define SLAVE_REG_P0018		0x0018			
#define SLAVE_REG_P0019		0x0019			
#define SLAVE_REG_P001A		0x001A			
#define SLAVE_REG_P001B		0x001B			
#define SLAVE_REG_P001C		0x001C			

/* 04H 读取输入寄存器(模拟信号) */
#define REG_A01		0x0401
#define REG_AXX		REG_A01


/* RTU 应答代码 */
#define RSP_OK				0		/* 成功 */
#define RSP_ERR_CMD			0x01	/* 不支持的功能码 */
#define RSP_ERR_REG_ADDR	0x02	/* 寄存器地址错误 */
#define RSP_ERR_VALUE		0x03	/* 数据值域错误 */
#define RSP_ERR_WRITE		0x04	/* 写入失败 */

#define S_RX_BUF_SIZE		30
#define S_TX_BUF_SIZE		128

typedef struct
{
	uint8_t RxBuf[S_RX_BUF_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[S_TX_BUF_SIZE];
	uint8_t TxCount;
}MODS_T;

void MODS_ReciveNew_no_timer(uint8_t _byte);
void MODS_ReciveNew(uint8_t _byte);
void MODS_Poll(void);
void MODS_RxTimeOut(void);

extern MODS_T g_tModS;
#endif