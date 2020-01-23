/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gfxmmu_lut.h"
#include "stm32l4r9i_eval.h"
#include "stm32l4r9i_eval_sram.h"
#include "stm32l4r9i_eval_ospi_nor.h"
#include "stdio.h"
#include "img.h"

// pixels are stored B,G,R
__attribute__((section(".bss.FrameBufferSection")))
static unsigned char screen_buffer[536 * 240 * 3];

UART_HandleTypeDef lpuart1Handle, uart4Handle;
LTDC_HandleTypeDef ltdcHandle;
DSI_HandleTypeDef dsiHandle;

int __io_putchar(int ch) {
  HAL_UART_Transmit(&lpuart1Handle, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

void SystemClock_Config(void);


void LCD_LL_LayerInit(unsigned int layer_id, void *buffer) {
  LTDC_LayerCfgTypeDef cfg;
  cfg.WindowX0 = 0;
  cfg.WindowX1 = 240;
  cfg.WindowY0 = 0;
  cfg.WindowY1 = 536;
  cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
  cfg.ImageWidth = 240;
  cfg.Alpha0 = 0;
  cfg.Alpha = 0xFF;
  cfg.Backcolor.Red = 0;
  cfg.Backcolor.Green = 0;
  cfg.Backcolor.Blue = 0;
  cfg.Backcolor.Reserved = 0xFF;
  cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  cfg.FBStartAdress = (uint32_t)buffer;
  cfg.ImageHeight = 536;
  if (HAL_LTDC_ConfigLayer(&ltdcHandle, &cfg, layer_id) != HAL_OK) {
    printf("HAL_LTDC_ConfigLayer failed!\n");
    while(1);
  }
  __HAL_LTDC_LAYER_ENABLE(&ltdcHandle, layer_id);
}


void init_lcd() {
  GPIO_InitTypeDef gpio;
  DSI_PLLInitTypeDef      dsiPllInit;
  DSI_PHY_TimerTypeDef    PhyTimings;
  DSI_HOST_TimeoutTypeDef HostTimeouts;
  DSI_LPCmdTypeDef        LPCmd;
  DSI_CmdCfgTypeDef       CmdCfg;

  printf("Doing pre-LTCD DMA2D reset...\n");
  __HAL_RCC_DMA2D_CLK_ENABLE();
  __HAL_RCC_DMA2D_FORCE_RESET();
  __HAL_RCC_DMA2D_RELEASE_RESET();

  printf("Doing pre-LTCD GPIO stuff...\n");
  // sub_80ABC9E
  __HAL_RCC_GPIOB_CLK_ENABLE();
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &gpio);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_Delay(20);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

  __HAL_RCC_GPIOC_CLK_ENABLE();
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pin = GPIO_PIN_8;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &gpio);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(2);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_Delay(120);
  // end sub_80ABC9E

  HAL_LTDC_DeInit(&ltdcHandle);
  ltdcHandle.Instance = LTDC;
  __HAL_LTDC_RESET_HANDLE_STATE(&ltdcHandle);
  ltdcHandle.Init.VerticalSync = 4;
  ltdcHandle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  ltdcHandle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  ltdcHandle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  ltdcHandle.Init.AccumulatedVBP = 16;
  ltdcHandle.Init.HorizontalSync = 20;
  ltdcHandle.Init.AccumulatedActiveH = 552;
  ltdcHandle.Init.AccumulatedHBP = 60;
  ltdcHandle.Init.TotalHeigh = 572;
  ltdcHandle.Init.AccumulatedActiveW = 300;
  ltdcHandle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  ltdcHandle.Init.TotalWidth = 320;
  ltdcHandle.Init.Backcolor.Red = 0;
  ltdcHandle.Init.Backcolor.Green = 0;
  ltdcHandle.Init.Backcolor.Blue = 0;
  ltdcHandle.Init.Backcolor.Reserved = 0xFF;
  printf("Initialising LTDC...\n");
  if(HAL_LTDC_Init(&ltdcHandle) != HAL_OK)
  {
    printf("LTDC init failed\n");
    while(1);
  }

  printf("Initialising LCD layer...\n");
  LCD_LL_LayerInit(LTDC_LAYER_1, screen_buffer);

  printf("Initialising DSI...\n");
  dsiHandle.Instance = DSI;
  dsiHandle.State = HAL_DSI_STATE_RESET;
  dsiHandle.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  dsiHandle.Init.TXEscapeCkdiv = 4;
  dsiHandle.Init.NumberOfLanes = DSI_ONE_DATA_LANE;
  dsiPllInit.PLLNDIV = 100;
  dsiPllInit.PLLIDF = 5;
  dsiPllInit.PLLODF = 0;
  if (HAL_DSI_Init(&dsiHandle, &dsiPllInit) != HAL_OK) {
    printf("DSI init failed\n");
    while(1);
  }

  printf("config phy...\n");
  PhyTimings.ClockLaneHS2LPTime  = 33;
  PhyTimings.ClockLaneLP2HSTime  = 30;
  PhyTimings.DataLaneHS2LPTime   = 11;
  PhyTimings.DataLaneLP2HSTime   = 21;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime        = 7;
  if (HAL_DSI_ConfigPhyTimer(&dsiHandle, &PhyTimings) != HAL_OK) {
    printf("DSI phy timings failed\n");
    while(1);
  }

  printf("config host...\n");
  HostTimeouts.TimeoutCkdiv                 = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout     = 0;
  HostTimeouts.HighSpeedReadTimeout         = 0;
  HostTimeouts.LowPowerReadTimeout          = 0;
  HostTimeouts.HighSpeedWriteTimeout        = 0;
  HostTimeouts.HighSpeedWritePrespMode      = 0;
  HostTimeouts.LowPowerWriteTimeout         = 0;
  HostTimeouts.BTATimeout                   = 0;
  if (HAL_DSI_ConfigHostTimeouts(&dsiHandle, &HostTimeouts) != HAL_OK) {
    printf("DSI host timeouts failed\n");
    while(1);
  }

  printf("config lp...\n");
  LPCmd.LPGenShortWriteNoP  = DSI_LP_GSW0P_ENABLE;
  LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_ENABLE;
  LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_ENABLE;
  LPCmd.LPGenShortReadNoP   = DSI_LP_GSR0P_ENABLE;
  LPCmd.LPGenShortReadOneP  = DSI_LP_GSR1P_ENABLE;
  LPCmd.LPGenShortReadTwoP  = DSI_LP_GSR2P_ENABLE;
  LPCmd.LPGenLongWrite      = DSI_LP_GLW_DISABLE;
  LPCmd.LPDcsShortWriteNoP  = DSI_LP_DSW0P_ENABLE;
  LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
  LPCmd.LPDcsShortReadNoP   = DSI_LP_DSR0P_ENABLE;
  LPCmd.LPDcsLongWrite      = DSI_LP_DLW_DISABLE;
  LPCmd.LPMaxReadPacket     = DSI_LP_MRDP_DISABLE;
  LPCmd.AcknowledgeRequest  = DSI_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigCommand(&dsiHandle, &LPCmd) != HAL_OK) {
    printf("DSI config command failed\n");
    while(1);
  }

  printf("config cmd...\n");
  CmdCfg.VirtualChannelID      = 0;
  CmdCfg.ColorCoding           = DSI_RGB888;
  CmdCfg.CommandSize           = 536;
  CmdCfg.TearingEffectSource   = DSI_TE_EXTERNAL;
  CmdCfg.HSPolarity            = DSI_HSYNC_ACTIVE_HIGH;
  CmdCfg.VSPolarity            = DSI_VSYNC_ACTIVE_HIGH;
  CmdCfg.AutomaticRefresh      = DSI_AR_DISABLE;
  CmdCfg.TearingEffectPolarity = DSI_TE_FALLING_EDGE;
  CmdCfg.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.VSyncPol              = DSI_VSYNC_FALLING;
  CmdCfg.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;
  if (HAL_DSI_ConfigAdaptedCommandMode(&dsiHandle, &CmdCfg) != HAL_OK) {
    printf("DSI config adapted command mode failed\n");
    while(1);
  }

  __HAL_DSI_ENABLE(&dsiHandle);

  printf("send stuff...\n");
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x04);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x6A, 0x00);

  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x05);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x05, 0x00);

  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x07);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x07, 0x4F);

  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x01);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x27, 0x18);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x28, 0x18);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2A, 0x02);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x2B, 0x73);

  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x0A);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x29, 0x10);

  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0xFE, 0x00);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_TEAR_ON, 0x00);
  HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_EXIT_SLEEP_MODE, 0x00);
  HAL_Delay(120);
  int status = HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, DSI_SET_DISPLAY_ON, 0x00);
  HAL_Delay(50);
  printf("screenon status result:%d\n", status);
  status = HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, 80);
  HAL_Delay(50);
  printf("brightness status result:%d\n", status);

  printf("almost done...\n");
  __HAL_DSI_WRAPPER_ENABLE(&dsiHandle);

  DMA2D->CR |= DMA2D_CR_TCIE;
  HAL_NVIC_SetPriority(DMA2D_IRQn, 0x8, 0x0);
  HAL_NVIC_EnableIRQ(DMA2D_IRQn);
  printf("out of lcd init\n");
}

int toDraw = 0;

void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef *hdsi) {
  if (toDraw) {
    HAL_DSI_Refresh(hdsi);
    // printf("T");
  }
}
void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi) {
  if (toDraw) {
    toDraw = 0;
    // printf("E");
  }
}
void HAL_DSI_ErrorCallback(DSI_HandleTypeDef *hdsi) {
  printf("DSIError\n");
}

int main(void)
{
  uint8_t result;

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  // this is where CoDi does the thirdPartyAdress thing

  HAL_Init();

  SystemClock_Config();
  __HAL_RCC_CRC_CLK_ENABLE();

  // init UART asap so we have debug output
  lpuart1Handle.Instance = LPUART1;
  HAL_UART_DeInit(&lpuart1Handle);
  lpuart1Handle.Init.WordLength = UART_WORDLENGTH_8B;
  lpuart1Handle.Init.Parity = UART_PARITY_NONE;
  lpuart1Handle.Init.Mode = UART_MODE_TX_RX;
  lpuart1Handle.Init.StopBits = UART_STOPBITS_1;
  lpuart1Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  lpuart1Handle.Init.OverSampling = UART_OVERSAMPLING_16;
  lpuart1Handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  lpuart1Handle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  lpuart1Handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  lpuart1Handle.Init.BaudRate = 115200;
  if (HAL_UART_Init(&lpuart1Handle) != HAL_OK)
    Error_Handler();

  printf("-- init start --\n" __TIMESTAMP__ "\n");

  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PEMPTY) != 0)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PEMPTY);

  // start sub_80A6F8C
  printf("BSP_SRAM_Init() 1\n");
  if (BSP_SRAM_Init() != SRAM_OK)
    while (1) {}
  printf("BSP_LED_Init()\n");
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  // printf("BSP_OSPI_NOR_Init()\n");
  // if ((result = BSP_OSPI_NOR_Init()) != OSPI_NOR_OK) {
  //   printf("returned status %d\n", result);
  //   while (1) {}
  // }
  // printf("BSP_OSPI_NOR_EnableMemoryMappedMode()\n");
  // if (BSP_OSPI_NOR_EnableMemoryMappedMode() != OSPI_NOR_OK)
  //   while (1) {}
  // printf("BSP_SRAM_Init() 2\n");
  // if (BSP_SRAM_Init() != SRAM_OK)
  //   while (1) {}

  __HAL_RCC_CRC_CLK_ENABLE();
  // end sub_80A6F8C

  printf("BSP_PB_Init()\n");
  BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_EXTI);
  printf("BSP_LED_On()\n");
  BSP_LED_On(LED1);
  // this is where orig code does HAL_UART_Receive_IT on lpuart1
  BSP_LED_Off(LED1);

  // uart4Handle.Instance = UART4;
  // uart4Handle.Init.WordLength = UART_WORDLENGTH_8B;
  // uart4Handle.Init.Parity = UART_PARITY_NONE;
  // uart4Handle.Init.StopBits = UART_STOPBITS_1;
  // uart4Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  // uart4Handle.Init.OverSampling = UART_OVERSAMPLING_16;
  // uart4Handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  // uart4Handle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  // uart4Handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  // uart4Handle.Init.Mode = UART_MODE_TX_RX;
  // uart4Handle.Init.BaudRate = 115200;
  // if (HAL_UART_Init(&uart4Handle) != HAL_OK)
  //   Error_Handler();

  printf("setting gpios\n");
  GPIO_InitTypeDef i;
  //gpio init?
  // not sure what these are for, yet...
  __HAL_RCC_GPIOE_CLK_ENABLE();
  i.Pin = GPIO_PIN_6;
  i.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  i.Pull = GPIO_PULLDOWN;
  i.Mode = GPIO_MODE_IT_RISING;
  HAL_GPIO_Init(GPIOE, &i);
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0xF, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  __HAL_RCC_GPIOB_CLK_ENABLE();
  i.Pin = GPIO_PIN_15;
  i.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  i.Pull = GPIO_PULLDOWN;
  i.Mode = GPIO_MODE_IT_RISING;
  HAL_GPIO_Init(GPIOB, &i);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0xF, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  printf("We're in\n");

  for (int i = 0; i < sizeof(screen_buffer); i += 123) {
    screen_buffer[i] = 90;
  }

  init_lcd();

  for (int i = 0; i < sizeof(screen_buffer); i += 23) {
    screen_buffer[i] = 180;
  }

  for (int i = 0; i < sizeof(img); i++)
    screen_buffer[i] = img[i];

  for (int i = 0; i < 240; i++) {
    screen_buffer[i * 3] = 0xFF;
  }

int ct = 0;
  while (1)
  {
    ++ct;
    HAL_Delay(500);
    printf("running...\n");
    toDraw = 1;
    // uint8_t buf[4] = {'a','b','c','d'};
    // HAL_UART_Transmit(&lpuart1Handle, buf, 4, 5000);
    for (int i = 100*240; i < 120*240; i++) {
      screen_buffer[i*3] += 8;
    }
  }
}



void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  static RCC_CRSInitTypeDef RCC_CRSInitStruct;
  
  /* Enable voltage range 1 boost mode for frequency above 80 Mhz */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  __HAL_RCC_PWR_CLK_DISABLE();
  
  /* Enable the LSE Oscilator */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State          = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_OFF;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  /* Enable MSI Oscillator and activate PLL with MSI as source   */
  /* (Default MSI Oscillator enabled at system reset remains ON) */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Enable MSI Auto-calibration through LSE */
  HAL_RCCEx_EnableMSIPLLMode();

  /* Select HSI84 output as USB clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* To avoid undershoot due to maximum frequency, select PLL as system clock source */
  /* with AHB prescaler divider 2 as first step */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* AHB prescaler divider at 1 as second step */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
  
    /*Configure the clock recovery system (CRS)**********************************/
  
  /* Enable CRS Clock */
  __HAL_RCC_CRS_CLK_ENABLE();
    
  /* Default Synchro Signal division factor (not divided) */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  
  /* Set the SYNCSRC[1:0] bits according to CRS_Source value */
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  
  /* HSI48 is synchronized with USB SOF at 1KHz rate */
  RCC_CRSInitStruct.ReloadValue =  __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000, 1000);
  RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  
  /* Set the TRIM[5:0] to the default value*/
  RCC_CRSInitStruct.HSI48CalibrationValue = 0x20; 
  
  /* Start automatic synchronization */ 
  HAL_RCCEx_CRSConfig (&RCC_CRSInitStruct);
}


void Error_Handler(void)
{
}

#ifdef  USE_FULL_ASSERT
void assert_failed(char *file, uint32_t line)
{ 
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
