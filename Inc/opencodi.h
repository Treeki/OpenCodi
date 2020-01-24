#pragma once
#include "stm32l4r9i_eval.h"
#include "stdio.h"
#include "lvgl.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 536


// display_driver.c
void ocDisplayInit();
void ocDisplaySetPowerState(bool on);
void ocDisplaySetBrightness(int value);
void ocDisplaySetupGUI();
extern LTDC_HandleTypeDef ltdcHandle;
extern DSI_HandleTypeDef dsiHandle;

// ts_driver.c
void ocTouchInit();
void ocTouchSetupGUI();
void ocTouchUpdateFromIRQ();

// uart_comms.c
extern UART_HandleTypeDef lpuart1Handle;

void ocCommsInit();
void ocCommsStartReceiving();
void ocCommsCheckQueue();
uint8_t ocCommsPeek8(int offset);
uint32_t ocCommsPeek32(int offset);

void ocCommsInitPacket(uint32_t id, uint32_t sequence);
void ocCommsPut8(uint8_t v);
void ocCommsPut32(uint32_t v);
void ocCommsPutString(const char *str);
void ocCommsSendPacket();

// packets.c
void ocPacketParseAndDispatch(uint32_t id, uint32_t sequence, int size);

// main.c
extern bool ioMysteryPin6Flag;
void Error_Handler(void);

#define consolePrint(...) do { char strbuf[0x100]; sprintf(strbuf, __VA_ARGS__); consolePush(strbuf); } while (0)
void consolePush(const char *str);

// system_stm32l4xx.c
void SystemClock_Config(void);
