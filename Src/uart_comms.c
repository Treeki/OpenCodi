#include "opencodi.h"
#include "stm32l4xx_ll_usart.h"

UART_HandleTypeDef lpuart1Handle;

/****************************************/
// Input

typedef struct _CircularBuffer {
	uint8_t *buffer;
	int length;
	int nextRead, nextWrite;
} CircularBuffer;

static uint8_t inBuffer[80001]; // matches length of original CoDi FW buffer
static CircularBuffer mainCB = {inBuffer, sizeof(inBuffer), 0, 0};

static void cbPushByte(uint8_t byte) {
	mainCB.buffer[mainCB.nextWrite] = byte;
	if (mainCB.nextWrite == (mainCB.length - 1))
		mainCB.nextWrite = 0;
	else
		mainCB.nextWrite++;
}

static int cbGetAvailable() {
	int written = mainCB.nextWrite, read = mainCB.nextRead;
	if (written >= read)
		return written - read;
	else
		return written + (mainCB.length - read) - 1;
}

uint8_t ocCommsPeek8(int offset) {
	int pos = mainCB.nextRead + offset;
	if (pos >= mainCB.length)
		pos -= mainCB.length;
	return mainCB.buffer[pos];
}
uint16_t ocCommsPeek16(int offset) {
	return ((uint32_t)ocCommsPeek8(offset + 0) << 8) |
	       ((uint32_t)ocCommsPeek8(offset + 1));
}
uint32_t ocCommsPeek32(int offset) {
	return ((uint32_t)ocCommsPeek8(offset + 0) << 24) |
	       ((uint32_t)ocCommsPeek8(offset + 1) << 16) |
	       ((uint32_t)ocCommsPeek8(offset + 2) << 8) |
	       ((uint32_t)ocCommsPeek8(offset + 3));
}
static void cbEatBytes(int amount) {
	mainCB.nextRead += amount;
	if (mainCB.nextRead >= mainCB.length)
		mainCB.nextRead -= mainCB.length;
}


void ocCommsInit() {
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
}

void ocCommsStartReceiving() {
	static uint8_t buf[1];
	HAL_UART_Receive_IT(&lpuart1Handle, buf, 1);
}

void ocCommsCheckQueue() {
	for (;;) {
		// ignore junk before a valid packet
		while (cbGetAvailable() && ocCommsPeek8(0) != 'X')
			cbEatBytes(1);

		int available = cbGetAvailable();
		if (available >= 0x10) {
			if (ocCommsPeek32(0) == 0x58215821 /* 'X!X!' */) {
				uint32_t packetSize = ocCommsPeek32(4);
				if (packetSize > 0x20000) {
					// invalid, skip it
					cbEatBytes(0x10);
				} else if (packetSize > available) {
					// more data necessary, so wait
					break;
				} else {
					uint32_t id = ocCommsPeek32(8);
					uint32_t sequence = ocCommsPeek32(12);
					cbEatBytes(0x10);
					ocPacketParseAndDispatch(id, sequence, packetSize - 0x10);
					cbEatBytes(packetSize - 0x10);
				}
			} else {
				// bad header, skip it
				cbEatBytes(1);
			}
		} else {
			// not enough data, bail
			break;
		}
	}
}



void LPUART1_IRQHandler() {
	// this seems to be how the original FW does it
	uint8_t byte = LL_USART_ReceiveData8(LPUART1);
	cbPushByte(byte);

	READ_REG(LPUART1->ISR);
	LL_USART_ReceiveData8(LPUART1);
	READ_REG(LPUART1->CR1);
	LL_USART_ClearFlag_TXFE(LPUART1);
}


/****************************************/
// Output

static uint8_t outBuffer[2048]; // may need expanding sometime...
static int outBufferPosition = 0;

void ocCommsInitPacket(uint32_t id, uint32_t sequence) {
	outBufferPosition = 0;
	ocCommsPut32(0x58215821);
	ocCommsPut32(0x10); // packet size placeholder
	ocCommsPut32(id);
	ocCommsPut32(sequence);
}

void ocCommsPut8(uint8_t v) {
	outBuffer[outBufferPosition++] = v;
}
void ocCommsPut16(uint16_t v) {
	ocCommsPut8((v >> 8) & 0xFF);
	ocCommsPut8(v & 0xFF);
}
void ocCommsPut32(uint32_t v) {
	ocCommsPut8(v >> 24);
	ocCommsPut8((v >> 16) & 0xFF);
	ocCommsPut8((v >> 8) & 0xFF);
	ocCommsPut8(v & 0xFF);
}
void ocCommsPutString(const char *str) {
	size_t len = strlen(str);
	ocCommsPut32(len);
	memcpy(&outBuffer[outBufferPosition], str, len);
	outBufferPosition += len;
}

void ocCommsSendPacket() {
	// write the packet size in
	uint32_t packetSize = outBufferPosition;
	outBufferPosition = 4;
	ocCommsPut32(packetSize);

	HAL_UART_Transmit(&lpuart1Handle, outBuffer, packetSize, 5000);
}


