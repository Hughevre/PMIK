/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart_dma.h
  * @brief          : Header for uart_dma.c file.
  * @author			: Hubert Zajączkowski
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_DMA_H
#define __UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#include "circ_buffer.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define UART_TX_RINGBUFF_SZ 	4096
#define UART_RX_RINGBUFF_SZ 	4096

#define UART_RX_RAW_BUFFER_SZ	2048
/* USER CODE END Private defines */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN Exported macros */

/**
 * \brief           Calculate length of statically allocated array
 */
#define ARRAY_LEN(x)            (sizeof(x) / sizeof((x)[0]))
/* USER CODE END Exported macros */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN */
typedef struct __UART_DMA_HandleTypeDef
{
	ringbuff_t 			usart_tx_buff;
	uint8_t 			usart_tx_buff_data[UART_TX_RINGBUFF_SZ];
	size_t 				usart_dma_tx_len;

	ringbuff_t 			usart_rx_buff;
	uint8_t 			usart_rx_buff_data[UART_RX_RINGBUFF_SZ];
	size_t 				usart_dma_rx_ptr;

	UART_HandleTypeDef 	*huart;
	DMA_HandleTypeDef 	*hdma_tx;
	DMA_HandleTypeDef 	*hdma_rx;

	uint8_t 			uart_rx_raw_data[UART_RX_RAW_BUFFER_SZ];
}UART_DMA_HandleTypeDef;
/* USER CODE END */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void 	uart_dma_init(UART_DMA_HandleTypeDef*, UART_HandleTypeDef*, DMA_HandleTypeDef*, DMA_HandleTypeDef*);
void    uart_send_string(UART_DMA_HandleTypeDef*, const char*);

void    uart_dma_tx_handler(UART_DMA_HandleTypeDef*);
void 	uart_dma_rx_handler(UART_DMA_HandleTypeDef*);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __UART_DMA_H */
