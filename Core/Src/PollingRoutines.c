
#include "PollingRoutines.h"
#include "main.h"
#include "usbd_conf.h" // path to USBD_CUSTOMHID_OUTREPORT_BUF_SIZE define

#include "CAN_Buffer.h"
#include "protocolCommands.h"
#include "USB_Buffer.h"
#include "UsbAndCanConvert.h"
#include "GPIO_Ports.h"

//#include "IntArrayToString.h"


// these variables are defined in main.c
extern CAN_HandleTypeDef hcan;
extern uint8_t usbRxBuffer[USB_MAX_RING_BUFF][USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];
extern RING_BUFF_INFO usbRxRingBuffPtr;

uint8_t USB_TX_Buffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE]; // To send usb data to PC
uint8_t canBusActive = 0;

const char* Version = "v1.0.1";
const char* Hardware = "CANable_0.2";
const char* Frequency = "APB1_48mHz";// this is the APB1 clock frequency

uint8_t ledBlinkMode = 0;
uint32_t currentHalCount = 0;

/*
 *  Description: The main entry point. Typically in all my projects I call this routine from main all do all my polling here, if i am not using Tasks.
 *
 */
void PollingRoutine(void){
	PortB_On(LED_Green_Pin);
	// polling is currently done in Tasks
	ParseUsbRec();
	SendCanTxMessage1(&hcan);
	ParseCanRec();
	SendUsbMessage();

	BlinkkLed();
}

/*
 * function: blink blue led when there is CAN bus activity
 *
 */
void BlinkkLed(void) {
	switch(ledBlinkMode)  {
	case 0:
		if(canBusActive) {
			canBusActive = 0;
			PortB_On(LED_Blue_Pin);
			ledBlinkMode++;
		}
		break;
	case 1:
		if(++currentHalCount > 2000) {
			PortB_Off(LED_Blue_Pin);
			ledBlinkMode++;
			currentHalCount = 0;
		}
		break;
	case 2:
		if(++currentHalCount > 50000) {
			ledBlinkMode = 0;
			currentHalCount = 0;
		}
		break;
	}
}

/*
 * function: Parse the USB data in the buffer.
 * input: none
 * output: none
 *
 */
void ParseUsbRec(void) {
	uint8_t usbData[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];
	if(UsbDataAvailable(usbData)) {
		switch(usbData[0])
		{
		case COMMAND_MESSAGE:
			SendUsbDataToCanBus(CAN1_NODE, usbData);
			break;
		case COMMAND_BAUD:
			CanSnifferCanInit(&hcan, usbData);
			break;
		case COMMAND_INFO:
			SendHardwareInfo();
			SendVersionInfo();
			SendFrequency();
			Send_CAN_BTR(&hcan);
			break;
		}
	}
}

void SendHardwareInfo(void) {
	uint8_t data[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] = {0};
	uint8_t i = 0;
	data[0] = COMMAND_HARDWARE;
	while( Hardware[i] != '\0') {
		data[i + 1] = (uint8_t) Hardware[i];
		i++;
	}
	AddUsbTxBuffer(data);
}

void SendVersionInfo(void) {
	uint8_t data[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] = {0};
	uint8_t i = 0;
	data[0] = COMMAND_VERSION;
	while( Version[i] != '\0') {
		data[i + 1] = (uint8_t) Version[i];
		i++;
	}
	AddUsbTxBuffer(data);
}

void SendFrequency(void) {
	uint8_t data[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] = {0};
	uint8_t i = 0;
	data[0] = COMMAND_FREQUENCY;
	while( Frequency[i] != '\0') {
		data[i + 1] = (uint8_t) Frequency[i];
		i++;
	}
	AddUsbTxBuffer(data);
}

void Send_CAN_BTR(CAN_HandleTypeDef *hcan) {
	uint8_t data[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] = {0};
	uint32_t btrValue = READ_REG(hcan->Instance->BTR);

	data[0] = COMMAND_CAN_BTR;
	data[1] = btrValue >> 24 & 0xFF;
	data[2] = btrValue >> 16 & 0xFF;
	data[3] = btrValue >> 8 & 0xFF;
	data[4] = btrValue & 0xFF;
	AddUsbTxBuffer(data);
}

/*
 * function: Parse the CAN data in the buffer.
 * input: none
 * output: none
 *
 */
void ParseCanRec(void) {
	uint8_t canMsgAvailableFlag = 0;
	CanRxMsgTypeDef canRxMsg;
	uint8_t usbData[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];

	memset(&usbData, 0, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

	canMsgAvailableFlag = Can1DataAvailable(&canRxMsg); // check ring buffer for new message
	if(canMsgAvailableFlag) {
		if(canRxMsg.CAN_RxHeaderTypeDef.IDE == CAN_EXT_ID) { // EXT ID
			SendCanDataToUsb(&canRxMsg, CAN1_NODE);
		} else { // STD ID
			SendCanDataToUsb(&canRxMsg, CAN1_NODE);
		}
	}
}


/*
 * function: This is copied from the CAN_Buffer.c file. You can use this to toggle LED to indicate CAN bus activity
 * input: On or Off state of LED
 * output: none
 */
void CanBusActivityStatus(uint8_t status){
	canBusActive = status;
}

/*
 * Description: Changes the CAN handle baud rate received from the PC. Use the calculator from "bittiming.can-wiki.info" to get the CAN_BTR value
 * Input: the CAN Handle and the CAN_BTR value
 * Output: none
 */
void CanSnifferCanInit(CAN_HandleTypeDef *hcan, uint8_t *data) {

	uint32_t btrValue = 0;
	uint8_t usbData[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE] = {0};

	btrValue = data[1] << 24 | data[2] << 16 | data[3] << 8 | data[4]; // parse the BTR data

	// some of these snippets were copied from HAL_CAN_Init()
	HAL_CAN_DeInit(hcan);

	if (hcan->State == HAL_CAN_STATE_RESET)
	{
		/* Init the low level hardware: CLOCK, NVIC */
		HAL_CAN_MspInit(hcan);
	}

	/* Set the bit timing register */
	WRITE_REG(hcan->Instance->BTR, (uint32_t)(btrValue));

	/* Initialize the error code */
	hcan->ErrorCode = HAL_CAN_ERROR_NONE;

	/* Initialize the CAN state */
	hcan->State = HAL_CAN_STATE_READY;

	if(HAL_CAN_Start(hcan) != HAL_OK) { // start the CAN module
		usbData[0] = COMMAND_NAK; // NAK PC
		AddUsbTxBuffer(usbData);
		return;
	}
	usbData[0] = COMMAND_ACK; // ACK PC back
	AddUsbTxBuffer(usbData);
}
