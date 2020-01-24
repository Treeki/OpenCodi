/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32l4xx_it.h"

void NMI_Handler() {
}

void HardFault_Handler() {
  printf("HardFault\n");
  while (1);
}

void MemManage_Handler() {
  printf("MemManage\n");
  while (1);
}

void BusFault_Handler() {
  printf("BusFault\n");
  while (1);
}

void UsageFault_Handler() {
  printf("UsageFault\n");
  while (1);
}

void SVC_Handler() {
}

void DebugMon_Handler() {
}

void PendSV_Handler() {
}

void SysTick_Handler() {
  // not used - see the timer handler in stm32l4xx_hal_timebase_tim.c
}

void DSI_IRQHandler() {
  HAL_DSI_IRQHandler(&dsiHandle);
}
void LTDC_IRQHandler() {
  HAL_LTDC_IRQHandler(&ltdcHandle);
}
void LTDC_ER_IRQHandler() {
  HAL_LTDC_IRQHandler(&ltdcHandle);
}
void DMA2D_IRQHandler() {
  DMA2D->IFCR = DMA2D_IFCR_CTCIF;
  DMA2D->IFCR = DMA2D_IFCR_CCTCIF;
}


void EXTI9_5_IRQHandler() {
  if (__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_9) != RESET)
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
  if (__HAL_GPIO_EXTI_GET_FLAG(GPIO_PIN_6) != RESET)
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
}
void EXTI15_10_IRQHandler() {
  printf("Got a 1015!\n");
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  switch (GPIO_Pin) {
    case GPIO_PIN_6:
      ioMysteryPin6Flag = true;
      break;
    case GPIO_PIN_9:
      ocTouchUpdateFromIRQ();
      break;
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
