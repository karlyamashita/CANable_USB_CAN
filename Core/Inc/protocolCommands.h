


// Standard ASCII characters
#define COMMAND_SOH 0x01
#define COMMAND_STX 0x02
#define COMMAND_ETX 0x03
#define COMMAND_EOT 0x04
#define COMMAND_ACK 0x06
#define COMMAND_LF 0x0A
#define COMMAND_CR 0x0D
#define COMMAND_NAK 0x15



// custom defines
#define COMMAND_MESSAGE 0x80
#define COMMAND_BAUD 0x95
#define COMMAND_CAN_MODE 0xA0 // normal, listen, loopback

#define COMMAND_ENABLE_MESSAGES 0xB0 // Enable sending USB messages
#define COMMAND_DISABLE_MESSAGES 0xB1 // Disable sending USB messages

#define COMMAND_INFO 0x90 // PC is requesting for information
#define COMMAND_CAN_BTR 0x91 // the CAN BTC value
#define COMMAND_VERSION 0x92 // send back version to PC
#define COMMAND_HARDWARE 0x93 // send back hardware type to PC

