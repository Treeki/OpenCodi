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

UART_HandleTypeDef lpuart1Handle, uart4Handle;

int __io_putchar(int ch) {
  HAL_UART_Transmit(&lpuart1Handle, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}


#define consolePrint(...) do { char strbuf[0x100]; sprintf(strbuf, __VA_ARGS__); consolePush(strbuf); } while (0)

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


// TODO move me elsewhere...
static uint8_t readByteBuf = 0;
static uint8_t packetBuf[0x400];
static int packetBufPos = 0;

uint32_t readBE32(uint8_t *p) {
  return ((uint32_t)p[0] << 24) |
         ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) |
         ((uint32_t)p[3]);
  // return __builtin_bswap32(*(uint32_t *)p);
}
void writeBE32(uint8_t *p, uint32_t v) {
  p[0] = (v >> 24) & 0xFF;
  p[1] = (v >> 16) & 0xFF;
  p[2] = (v >> 8) & 0xFF;
  p[3] = v & 0xFF;
}

void sendPacket(uint32_t id, uint32_t sequence, uint8_t *payload, uint32_t payloadLen) {
  uint8_t header[0x10];
  memcpy(header, "X!X!", 4);
  writeBE32(&header[4], 0x10 + payloadLen);
  writeBE32(&header[8], id);
  writeBE32(&header[12], sequence);
  HAL_UART_Transmit(&lpuart1Handle, header, 0x10, 5000);
  HAL_UART_Transmit(&lpuart1Handle, payload, payloadLen, 5000);
  uint8_t empty = 0;
  HAL_UART_Transmit(&lpuart1Handle, &empty, 1, 5000);
}

#define VERSION_NAME "TestVer " __TIMESTAMP__

void sendDataChangeAlert(uint32_t sequence, uint32_t a, uint32_t b) {
  uint8_t payload[8];
  writeBE32(&payload[0], a);
  writeBE32(&payload[4], b);
  sendPacket(104, sequence, payload, 8);
}

void handleDataChangeAlert(uint32_t sequence, uint32_t a, uint32_t b) {
  if (a == 104) {
    // ping from Android
    sendDataChangeAlert(sequence, a, b);
  }
}

void handlePacket(uint32_t id, uint32_t sequence, uint8_t *payload, uint32_t payloadLen) {
  uint8_t buf[0x100];
  // TODO shouldn't be calling this from an IRQ handler
  // littlevGL is not thread safe
  consolePrint("recv packet %lu\n", id);

  switch (id) {
    case 1: // CMD_MTK_GET_CODI_FLASH_VERSION
      writeBE32(buf, strlen(VERSION_NAME));
      memcpy(&buf[4], VERSION_NAME, strlen(VERSION_NAME));
      for (int i = 0; i < strlen(VERSION_NAME); i++)
        if (buf[4 + i] == ':')
          buf[4 + i] = '_';
      sendPacket(2, sequence, buf, 4 + strlen(VERSION_NAME));
      break;
    case 126: // CMD_MTK_DATA_CHANGE_ALERT
      handleDataChangeAlert(sequence, readBE32(payload), readBE32(&payload[4]));
      break;
  }
}

void pushByteToReadBuffer(uint8_t byte) {
  if (packetBufPos >= sizeof(packetBuf))
    return;

  packetBuf[packetBufPos++] = byte;

  // try to parse a packet: X!X!aaaabbbbcccc where a=length, b=id, c=sequence
  int parsed = 0;
  for (int i = 0; i <= (packetBufPos - 0x10); i++) {
    if (memcmp(&packetBuf[i], "X!X!", 4) == 0) {
      // we have a packet
      uint32_t len = readBE32(&packetBuf[i + 4]);
      uint32_t id = readBE32(&packetBuf[i + 8]);
      uint32_t sequence = readBE32(&packetBuf[i + 12]);

      if ((i + len) > packetBufPos) {
        // we don't have enough data yet
        parsed = i;
        break;
      } else {
        // we have the full packet
        if (len >= 16) {
          handlePacket(id, sequence, &packetBuf[i + 16], len - 16);
        }
        parsed = i + len;
      }
    }
  }

  if (parsed > 0) {
    memmove(&packetBuf[0], &packetBuf[parsed], packetBufPos - parsed);
    packetBufPos -= parsed;
  }
}

#include "stm32l4xx_ll_usart.h"
void LPUART1_IRQHandler() {
  // this seems to be how the original FW does it
  uint8_t byte = LL_USART_ReceiveData8(LPUART1);
  pushByteToReadBuffer(byte);

  READ_REG(LPUART1->RDR);
  LL_USART_ReceiveData8(LPUART1);
  LL_USART_ClearFlag_TXFE(LPUART1);
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
  HAL_UART_Receive_IT(&lpuart1Handle, &readByteBuf, 1);
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
