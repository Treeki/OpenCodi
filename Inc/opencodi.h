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

// main.c
extern UART_HandleTypeDef lpuart1Handle, uart4Handle;
extern bool ioMysteryPin6Flag;
void Error_Handler(void);

// system_stm32l4xx.c
void SystemClock_Config(void);
