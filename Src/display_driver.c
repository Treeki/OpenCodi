#include "opencodi.h"

__attribute__((section(".bss.FrameBufferSection")))
static lv_color_t frameBuffer1[SCREEN_HEIGHT * SCREEN_WIDTH];
__attribute__((section(".bss.FrameBufferSection")))
static lv_color_t frameBuffer2[SCREEN_HEIGHT * SCREEN_WIDTH];

static bool screenOn;
static int needsRedraw;
LTDC_HandleTypeDef ltdcHandle;
DSI_HandleTypeDef dsiHandle;


void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef *hdsi) {
  if (needsRedraw > 0 && screenOn) {
    LTDC_Layer1->CFBAR = (uint32_t)((needsRedraw == 1) ? frameBuffer1 : frameBuffer2);
	LTDC->SRCR = LTDC_SRCR_IMR;
    HAL_DSI_Refresh(hdsi);
  }
}
void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi) {
  if (needsRedraw > 0) {
    needsRedraw = 0;
  }
}
void HAL_DSI_ErrorCallback(DSI_HandleTypeDef *hdsi) {
  printf("DSIError\n");
}


static void initLayer(unsigned int layer_id, void *buffer) {
  LTDC_LayerCfgTypeDef cfg;
  cfg.WindowX0 = 0;
  cfg.WindowX1 = 240;
  cfg.WindowY0 = 0;
  cfg.WindowY1 = 536;
  cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
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


void ocDisplayInit() {
	screenOn = false;
	needsRedraw = 0;

	GPIO_InitTypeDef gpio;
	DSI_PLLInitTypeDef dsiPllInit;
	DSI_PHY_TimerTypeDef PhyTimings;
	DSI_HOST_TimeoutTypeDef HostTimeouts;
	DSI_LPCmdTypeDef LPCmd;
	DSI_CmdCfgTypeDef CmdCfg;

	printf("Doing pre-LTCD DMA2D reset...\n");
	__HAL_RCC_DMA2D_CLK_ENABLE();
	__HAL_RCC_DMA2D_FORCE_RESET();
	__HAL_RCC_DMA2D_RELEASE_RESET();

	printf("Doing pre-LTCD GPIO stuff...\n");
	// sub_80ABC9E
	__HAL_RCC_GPIOB_CLK_ENABLE();
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
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
	if (HAL_LTDC_Init(&ltdcHandle) != HAL_OK) {
		printf("LTDC init failed\n");
		while (1);
	}

	printf("Initialising LCD layer...\n");
	initLayer(LTDC_LAYER_1, frameBuffer1);

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
		while (1);
	}

	printf("config phy...\n");
	PhyTimings.ClockLaneHS2LPTime = 33;
	PhyTimings.ClockLaneLP2HSTime = 30;
	PhyTimings.DataLaneHS2LPTime = 11;
	PhyTimings.DataLaneLP2HSTime = 21;
	PhyTimings.DataLaneMaxReadTime = 0;
	PhyTimings.StopWaitTime = 7;
	if (HAL_DSI_ConfigPhyTimer(&dsiHandle, &PhyTimings) != HAL_OK) {
		printf("DSI phy timings failed\n");
		while (1);
	}

	printf("config host...\n");
	HostTimeouts.TimeoutCkdiv = 1;
	HostTimeouts.HighSpeedTransmissionTimeout = 0;
	HostTimeouts.LowPowerReceptionTimeout = 0;
	HostTimeouts.HighSpeedReadTimeout = 0;
	HostTimeouts.LowPowerReadTimeout = 0;
	HostTimeouts.HighSpeedWriteTimeout = 0;
	HostTimeouts.HighSpeedWritePrespMode = 0;
	HostTimeouts.LowPowerWriteTimeout = 0;
	HostTimeouts.BTATimeout = 0;
	if (HAL_DSI_ConfigHostTimeouts(&dsiHandle, &HostTimeouts) != HAL_OK) {
		printf("DSI host timeouts failed\n");
		while (1);
	}

	printf("config lp...\n");
	LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_ENABLE;
	LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_ENABLE;
	LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_ENABLE;
	LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_ENABLE;
	LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_ENABLE;
	LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_ENABLE;
	LPCmd.LPGenLongWrite = DSI_LP_GLW_DISABLE;
	LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_ENABLE;
	LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
	LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_ENABLE;
	LPCmd.LPDcsLongWrite = DSI_LP_DLW_DISABLE;
	LPCmd.LPMaxReadPacket = DSI_LP_MRDP_DISABLE;
	LPCmd.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;
	if (HAL_DSI_ConfigCommand(&dsiHandle, &LPCmd) != HAL_OK) {
		printf("DSI config command failed\n");
		while (1);
	}

	printf("config cmd...\n");
	CmdCfg.VirtualChannelID = 0;
	CmdCfg.ColorCoding = DSI_RGB888;
	CmdCfg.CommandSize = SCREEN_HEIGHT;
	CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
	CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
	CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
	CmdCfg.AutomaticRefresh = DSI_AR_DISABLE;
	CmdCfg.TearingEffectPolarity = DSI_TE_FALLING_EDGE;
	CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
	CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
	CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_ENABLE;
	if (HAL_DSI_ConfigAdaptedCommandMode(&dsiHandle, &CmdCfg) != HAL_OK) {
		printf("DSI config adapted command mode failed\n");
		while (1);
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
	ocDisplaySetPowerState(true);
	HAL_Delay(50);
	ocDisplaySetBrightness(40);
	HAL_Delay(50);

	printf("almost done...\n");
	__HAL_DSI_WRAPPER_ENABLE(&dsiHandle);

	DMA2D->CR |= DMA2D_CR_TCIE;
	HAL_NVIC_SetPriority(DMA2D_IRQn, 0x8, 0x0);
	HAL_NVIC_EnableIRQ(DMA2D_IRQn);
	printf("out of lcd init\n");
}

void ocDisplaySetPowerState(bool on) {
	if (screenOn != on) {
		HAL_DSI_ShortWrite(&dsiHandle, 0,
			DSI_DCS_SHORT_PKT_WRITE_P1,
			on ? DSI_SET_DISPLAY_ON : DSI_SET_DISPLAY_OFF,
			0x00);
		screenOn = on;
	}
}
void ocDisplaySetBrightness(int value) {
	HAL_DSI_ShortWrite(&dsiHandle, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x51, value);
}

static lv_disp_buf_t lvDispBuf;

static void guiDispFlushCB(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
	if (color_p == frameBuffer1)
		needsRedraw = 1;
	else
		needsRedraw = 2;
	lv_disp_flush_ready(disp);
}

void ocDisplaySetupGUI() {
	lv_disp_buf_init(&lvDispBuf, frameBuffer1, frameBuffer2, SCREEN_WIDTH * SCREEN_HEIGHT);
	printf("lv_disp_buf_init done\n");

	lv_disp_drv_t drv;
	lv_disp_drv_init(&drv);
	drv.flush_cb = guiDispFlushCB;
	drv.buffer = &lvDispBuf;
	lv_disp_drv_register(&drv);
}
