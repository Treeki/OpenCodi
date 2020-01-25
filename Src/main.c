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
#include "opencodi.h"
#include "stm32l4r9i_eval_sram.h"
#include "stm32l4r9i_eval_ospi_nor.h"
#include "string.h"

int __io_putchar(int ch) {
  HAL_UART_Transmit(&lpuart1Handle, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}


bool ioMysteryPin6Flag = false;


static uint64_t timeLastSetValue = 0;
static uint32_t timeLastSetTicks = 0;

uint64_t ocTime() {
  return timeLastSetValue + ((HAL_GetTick() - timeLastSetTicks) / 1000);
}
void ocSetTime(int day, int month, int year, int hour, int minute, int second) {
  if (month <= 2) {
    month += 12;
    year--;
  }
  int dayTotal = day
    + (30 * month)
    + (3 * (month + 1) / 5)
    + (365 * year)
    + (year / 400)
    + (year / 4)
    - (year / 100)
    - 719561;
  timeLastSetValue = second + 60 * minute + 3600 * hour + 86400L * (uint64_t)dayTotal;
  timeLastSetTicks = HAL_GetTick();
}

static void perSecondTask(lv_task_t *task) {
  ocUiUpdatePerSecond();
}

static void initTask(lv_task_t *task) {
  // boot packet doesn't have a sensible ID for some reason
  ocCommsInitPacket(0, 0);
  ocCommsSendPacket();
  ocCommsInitPacket(CMD_ST32_GET_DATETIME, 1);
  ocCommsSendPacket();
  ocCommsInitPacket(CMD_ST32_GET_BATTERY_LEVEL, 2);
  ocCommsSendPacket();
}


int main(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  // this is where CoDi does the thirdPartyAdress thing

  HAL_Init();

  SystemClock_Config();
  __HAL_RCC_CRC_CLK_ENABLE();

  ocCommsInit();

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
  ocCommsStartReceiving();
  BSP_LED_Off(LED1);

  // UART4 init would go here but we don't seem to use that

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

  ocDisplayInit();
  ocTouchInit();

  lv_init();

  ocDisplaySetupGUI();
  ocTouchSetupGUI();

  ocUiSetup();

  lv_task_create(perSecondTask, 1000, LV_TASK_PRIO_MID, NULL);
  lv_task_once(lv_task_create(initTask, 1000, LV_TASK_PRIO_LOW, NULL));

  lv_disp_t *disp = lv_disp_get_default();
  while (1)
  {
    HAL_Delay(5);
    lv_task_handler();
    ocCommsCheckQueue();

    if (ioMysteryPin6Flag) {
      consolePush("Pin6 detected\n");
      ioMysteryPin6Flag = false;
    }

    ocDisplaySetPowerState((disp->last_activity_time + 5000) >= lv_tick_get());
  }
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
