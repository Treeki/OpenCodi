/**
  ******************************************************************************
  * File Name          : stm32l4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
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

#include "opencodi.h"

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphCLKConfig = {};

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_LPUART1_CLK_ENABLE();

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSICalibrationValue = 0x40;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    while (1) {}
  PeriphCLKConfig.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
  PeriphCLKConfig.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&PeriphCLKConfig);

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(LPUART1_IRQn);

  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_UART4_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  // TODO FIX ME
}


void HAL_DSI_MspInit(DSI_HandleTypeDef *hdsi) {
  GPIO_InitTypeDef gpio;

  __HAL_RCC_DSI_CLK_ENABLE();
  __HAL_RCC_DSI_FORCE_RESET();
  __HAL_RCC_DSI_RELEASE_RESET();
 
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
  if (RCC_OscInitStruct.HSEState == RCC_HSE_OFF) {
    __HAL_RCC_GPIOH_CLK_ENABLE();
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = GPIO_PIN_0;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOH, &gpio);
    HAL_GPIO_WritePin(GPIOH, GPIO_PIN_0, GPIO_PIN_RESET);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
      printf("OscConfig call inside HAL_DSI_MspInit failed\n");
      while(1);
    }
  }

  __HAL_RCC_GPIOF_CLK_ENABLE();
  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_NOPULL;
  gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF11_DSI;
  gpio.Pin       = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOF, &gpio);

  HAL_NVIC_SetPriority(DSI_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DSI_IRQn);
}

void HAL_DSI_MspDeInit(DSI_HandleTypeDef *hdsi) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  GPIO_InitTypeDef  gpio_init_structure;

  HAL_NVIC_DisableIRQ(DSI_IRQn);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_OFF;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    printf("OscConfig call inside HAL_DSI_MspDeInit failed\n");
    while(1);
  }

  gpio_init_structure.Pin       = GPIO_PIN_11;
  HAL_GPIO_DeInit(GPIOF, gpio_init_structure.Pin);

  __HAL_RCC_DSI_FORCE_RESET();
  __HAL_RCC_DSI_CLK_DISABLE();
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
  __HAL_RCC_LTDC_CLK_ENABLE();

  __HAL_RCC_LTDC_FORCE_RESET();
  __HAL_RCC_LTDC_RELEASE_RESET();

  RCC_PeriphCLKInitTypeDef  PeriphClkInit;
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInit.PLLSAI2.PLLSAI2Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI2.PLLSAI2M = 1;
  PeriphClkInit.PLLSAI2.PLLSAI2N = 84;
  PeriphClkInit.PLLSAI2.PLLSAI2R = RCC_PLLR_DIV4;
  PeriphClkInit.LtdcClockSelection = RCC_LTDCCLKSOURCE_PLLSAI2_DIV4;
  PeriphClkInit.PLLSAI2.PLLSAI2ClockOut = RCC_PLLSAI2_LTDCCLK;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    while(1);
  }

  /* NVIC configuration for LTDC interrupts that are now enabled */
  HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(LTDC_IRQn);
  HAL_NVIC_SetPriority(LTDC_ER_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(LTDC_ER_IRQn);
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc)
{
  HAL_NVIC_DisableIRQ(LTDC_IRQn);
  HAL_NVIC_DisableIRQ(LTDC_ER_IRQn);
  __HAL_LTDC_DISABLE(hltdc);
  __HAL_RCC_LTDC_FORCE_RESET();
  __HAL_RCC_LTDC_CLK_DISABLE();
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
