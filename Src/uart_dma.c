/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart_dma.c
  * @brief          : UART DMA Source File
  * @author			: Hubert Zajączkowski
  ******************************************************************************
  */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart_dma.h"
/* USER CODE END Includes */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * \brief           Process received data over UART
 * Data are written to RX ringbuffer for application processing at latter stage
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure
 * \param[in]       data: Data to process
 * \param[in]       len: Length in units of bytes
 */
static void uart_process_data(UART_DMA_HandleTypeDef *huart_dma, const void* data, size_t len) {
    ringbuff_write(&(huart_dma->usart_rx_buff), data, len);  /* Write data to receive buffer */
}

/**
 * \brief           Check for new data received with DMA
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure
 */
static void uart_rx_check(UART_DMA_HandleTypeDef *huart_dma) {
	size_t pos;

	/* Calculate current position in buffer */
	pos = ARRAY_LEN(huart_dma->uart_rx_raw_data) - __HAL_DMA_GET_COUNTER(huart_dma->hdma_rx);
	if (pos != huart_dma->usart_dma_rx_ptr) {		 /* Check change in received data */
		if (pos > huart_dma->usart_dma_rx_ptr) {	 /* Current position is over previous one */
			/* We are in "linear" mode */
			/* Process data directly by subtracting "pointers" */
			uart_process_data(huart_dma, &huart_dma->uart_rx_raw_data[huart_dma->usart_dma_rx_ptr], pos - huart_dma->usart_dma_rx_ptr);
		} else {
			/* We are in "overflow" mode */
			/* First process data to the end of buffer */
			uart_process_data(huart_dma, &huart_dma->uart_rx_raw_data[huart_dma->usart_dma_rx_ptr], ARRAY_LEN(huart_dma->uart_rx_raw_data) - huart_dma->usart_dma_rx_ptr);
			/* Check and continue with beginning of buffer */
			 if (pos > 0) {
				 uart_process_data(huart_dma, &huart_dma->uart_rx_raw_data[0], pos);
			 }
		}
	}
	huart_dma->usart_dma_rx_ptr = pos;               /* Save current position as old */

	/* Check and manually update if we reached end of buffer */
	if (huart_dma->usart_dma_rx_ptr == ARRAY_LEN(huart_dma->uart_rx_raw_data)) {
		huart_dma->usart_dma_rx_ptr = 0;
	}
}

/**
 * \brief           Check if DMA is active and if not try to send data
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure
 */
static void uart_start_tx_dma(UART_DMA_HandleTypeDef *huart_dma) {
    /* If length > 0, DMA transfer is on-going */
    if (huart_dma->usart_dma_tx_len > 0) {
        return;
    }

    /* Check if something to send  */
    huart_dma->usart_dma_tx_len = ringbuff_get_linear_block_read_length(&(huart_dma->usart_tx_buff));
    if (huart_dma->usart_dma_tx_len > 0) {
    	uint8_t* data = ringbuff_get_linear_block_read_address(&(huart_dma->usart_tx_buff));
    	HAL_UART_Transmit_DMA(huart_dma->huart, data, huart_dma->usart_dma_tx_len); /* Start DMA transfer */
    }
}

/**
 * \brief           Initializes UART_DMA_HandleTypeDef structure and starts DMA receive
 * \param[in]
 * \param[in]
 * \param[in]
 * \param[in]
 */
void uart_dma_init(UART_DMA_HandleTypeDef *huart_dma, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma_tx, DMA_HandleTypeDef *hdma_rx) {
	huart_dma->huart 	= huart;
	huart_dma->hdma_tx	= hdma_tx;
	huart_dma->hdma_rx	= hdma_rx;

	ringbuff_init(&(huart_dma->usart_tx_buff), huart_dma->usart_tx_buff_data, sizeof(huart_dma->usart_tx_buff_data));
	ringbuff_init(&(huart_dma->usart_rx_buff), huart_dma->usart_rx_buff_data, sizeof(huart_dma->usart_rx_buff_data));

	__HAL_UART_ENABLE_IT(huart_dma->huart, UART_IT_IDLE); /* Enable UART IDLE interrupt*/

	__HAL_DMA_ENABLE_IT(hdma_rx, DMA_IT_HT); /* Enable Half-Transfer complete interrupt for RX*/
	__HAL_DMA_ENABLE_IT(hdma_rx, DMA_IT_TC); /* Enable Whole-Transfer complete interrupt for RX*/

	HAL_UART_Receive_DMA(huart_dma->huart, huart_dma->uart_rx_raw_data, sizeof(huart_dma->uart_rx_raw_data));
}

/**
 * \brief           Send debug string over UART
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure
 * \param[in]       str: String to send
 */
void uart_send_string(UART_DMA_HandleTypeDef *huart_dma, const char* str) {
    size_t len 	= strlen(str);

    if (ringbuff_get_free(&(huart_dma->usart_tx_buff)) >= len) { /* Make sure that size of the string does not exceed size of the free buffer space*/
        ringbuff_write(&(huart_dma->usart_tx_buff), str, len);  /* Write data to transmit buffer */
        uart_start_tx_dma(huart_dma);
    }
}

/**
 * \brief           DMA channel interrupt handler for UART TX
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure
 */
void uart_dma_tx_handler(UART_DMA_HandleTypeDef *huart_dma) {
	CLEAR_BIT(huart_dma->huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE)); 	/* Disable TXEIE and TCIE interrupts */
    huart_dma->huart->gState = HAL_UART_STATE_READY; 									/* Tx process is ended, restore huart->gState to Ready */
    if(huart_dma->usart_dma_tx_len > 0) {
    		ringbuff_skip(&(huart_dma->usart_tx_buff), huart_dma->usart_dma_tx_len);
    		huart_dma->usart_dma_tx_len = 0;
    		uart_start_tx_dma(huart_dma);
    }
}

/**
 * \brief           DMA channel interrupt handler for UART RX
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure
 */
void uart_dma_rx_handler(UART_DMA_HandleTypeDef *huart_dma) {
	uart_rx_check(huart_dma);
}

/* USER CODE END 0 */

