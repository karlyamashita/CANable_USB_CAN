#include "main.h"
#include "ConvertUsbAndCan.h"
#include "CAN_Buffer.h"
#include "USB_Buffer.h"
#include "protocolCommands.h"

#include "usbd_conf.h" // path to USBD_CUSTOMHID_OUTREPORT_BUF_SIZE define


/* Description: parse usb data
 * Input canChannel: which CAN node to send message to
 * Input data: The USB data
 * Output none
 */
void SendUsbDataToCanBus(uint8_t canChannel, uint8_t *data) {

	CanTxMsgTypeDef msg;
	UsbCanStruct usbCanStruct;

	memcpy(usbCanStruct.array.msgArray, data, 20); // remove command

	PortB_Toggle(LED_Blue_Pin);

	// data[0] is command from USB
	msg.CAN_TxHeaderTypeDef.IDE = usbCanStruct.msgBytes.IDE;
	if(msg.CAN_TxHeaderTypeDef.IDE == CAN_ID_STD) {
		msg.CAN_TxHeaderTypeDef.StdId = usbCanStruct.msgBytes.ArbId;
	} else {
		msg.CAN_TxHeaderTypeDef.ExtId = usbCanStruct.msgBytes.ArbId;
	}
	msg.CAN_TxHeaderTypeDef.RTR = usbCanStruct.msgBytes.RTR;// RTR
	msg.CAN_TxHeaderTypeDef.DLC = usbCanStruct.msgBytes.DLC;//

	for(int i = 0; i < 8; i++) { // copy 8 bytes even though DLC could be less
		msg.Data[i] = usbCanStruct.dataBytes.array[i];
	}

	if(canChannel == CAN1_NODE) {
		AddCanTxBuffer1(&msg);
	} else {
#ifdef USE_CAN_BUFFER_2
		AddCanTxBuffer2(&msg);
#endif
	}
}

// just the opposite, copy CAN to USB data
void SendCanDataToUsb(CanRxMsgTypeDef *msg) {
	uint8_t i = 0;
	UsbCanStruct usbCanStruct = {0};


	usbCanStruct.msgBytes.Command = COMMAND_MESSAGE;
	usbCanStruct.msgBytes.IDE = msg->CAN_RxHeaderTypeDef.IDE & 0x0F;

	if(usbCanStruct.msgBytes.IDE == CAN_EXT_ID) {
		usbCanStruct.msgBytes.ArbId = msg->CAN_RxHeaderTypeDef.ExtId;
	} else {
		usbCanStruct.msgBytes.ArbId = msg->CAN_RxHeaderTypeDef.StdId;
	}

	usbCanStruct.msgBytes.RTR = msg->CAN_RxHeaderTypeDef.RTR & 0x0F;
	usbCanStruct.msgBytes.DLC = msg->CAN_RxHeaderTypeDef.DLC & 0x0F;

	for(i = 0; i < 8; i++) {
		usbCanStruct.dataBytes.array[i] = msg->Data[i];
	}

	AddUsbTxBuffer(usbCanStruct.array.msgArray);
}

/*
 * function: get the node PC wants to send message to. This can be CAN1, CAN2, SWCAN, etc.
 * input: the USB data that has the node value
 * output: node value
 */
uint8_t GetNode(uint8_t *data) {
	uint8_t node;
	UsbCanStruct usbCanStruct = {0};
	memcpy(usbCanStruct.array.msgArray, data, 20); // remove command

	node = usbCanStruct.msgBytes.Node;
	return node;
}
