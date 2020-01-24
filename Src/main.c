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


char console[2048];
lv_obj_t *consoleObj = NULL;
void consolePush(const char *str) {
  if (consoleObj == NULL) return;
  int toAdd = strlen(str);
  int currentLen = strlen(console);
  int maxLen = sizeof(console) - 1;
  if ((toAdd + currentLen) >= maxLen) {
    int toRemove = maxLen - toAdd;
    memmove(console, &console[toRemove], currentLen - toRemove);
    currentLen -= toRemove;
  }
  strcpy(&console[currentLen], str);
  lv_label_set_static_text(consoleObj, console);
}



void brightnessSliderCB(lv_obj_t *slider, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    ocDisplaySetBrightness(lv_slider_get_value(slider));
  }
}


bool ioMysteryPin6Flag = false;


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

  lv_obj_t *slider = lv_slider_create(lv_scr_act(), NULL);
  lv_slider_set_range(slider, 10, 160); // TODO: check me?
  lv_slider_set_value(slider, 40, LV_ANIM_OFF);
  lv_obj_set_pos(slider, 10, 10);
  lv_obj_set_width(slider, SCREEN_WIDTH - 20);
  lv_obj_set_event_cb(slider, brightnessSliderCB);

  lv_obj_t *consolePage = lv_page_create(lv_scr_act(), NULL);
  lv_obj_set_pos(consolePage, 0, 60);
  lv_obj_set_size(consolePage, SCREEN_WIDTH, SCREEN_HEIGHT - 60);

  consoleObj = lv_label_create(consolePage, NULL);
  lv_label_set_long_mode(consoleObj, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(consoleObj, lv_page_get_fit_width(consolePage));
  console[0] = 0;
  lv_label_set_static_text(consoleObj, console);

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
