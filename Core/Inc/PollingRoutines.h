#ifndef POLLING_ROUTINE_H
#define POLLING_ROUTINE_H

#include "main.h"
#include "CAN_Buffer.h"


void PollingRoutine(void); // main entry
void BlinkkLed(void);

void SendHardwareInfo(void);
void SendVersionInfo(void);
void SendFrequency(void);
void Send_CAN_BTR(CAN_HandleTypeDef *hcan);

void ParseUsbRec(void);
void ParseCanRec(void);

void SendUsbMsgToCan(CanTxMsgTypeDef *msg);
void SendCanToUart(CanTxMsgTypeDef *msg);
void CanBusActivityStatus(uint8_t status);
void CanSnifferCanInit(CAN_HandleTypeDef *hcan, uint8_t *data);


#endif // POLLING_ROUTINE_H
