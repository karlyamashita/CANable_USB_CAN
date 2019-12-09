#ifndef CONVERT_USB_AND_CAN_H
#define CONVERT_USB_AND_CAN_H

#include "CAN_Buffer.h"



// Node defines
#define CAN1_NODE 0
#define CAN2_NODE 1
#define SWCAN1_NODE 2
#define LSFTCAN1_NODE 3
#define LIN1_NODE 4
#define ETH1_NODE 5
#define SWCAN2_NODE 6
#define LSFTCAN2_NODE 7

// the structure sent by the PC.
typedef	union UsbCanStruct {
	struct {
		uint8_t msgArray[17];
	}array;
	struct {
		uint8_t Command; // command
		uint8_t IDE; // ide
		uint8_t RTR;// rtr
		uint8_t na; // n/a
		uint32_t ArbID; // ArbID

		uint8_t DLC; // dlc
		uint8_t array[8]; // 8 data bytes
	}dataBytes;
	struct  {
		uint8_t Command;
		uint8_t IDE; // byte1
		// RTR, ERR, Node byte6
		unsigned RTR:1;
		unsigned Err:1;
		unsigned Node:4; // see Node defines
		unsigned :10; // n/a

		// ArbID byte4:7
		uint32_t ArbId; // holds 11 and 29 bit ID. This has to align within 32bit address
		//
		uint8_t DLC; // byte8
		uint8_t Byte1; // byte9
		uint8_t Byte2;
		uint8_t Byte3;
		uint8_t Byte4;
		uint8_t Byte5;
		uint8_t Byte6;
		uint8_t Byte7;
		uint8_t Byte8; // byte16
	}msgBytes;
}UsbCanStruct;

void SendUsbDataToCanBus(uint8_t canChannel, uint8_t *data);
void SendCanDataToUsb(CanRxMsgTypeDef *msg);
uint8_t GetNode(uint8_t *data);

#endif // CONVERT_USB_AND_CAN_H
