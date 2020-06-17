/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bluetooth.c
  * @brief          : Bluetooth Source File
  * @author			: Hubert Zajączkowski
  ******************************************************************************
  */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bluetooth.h"
/* USER CODE END Includes */

/* Bluetooth frame structure -------------------------------------------------*/
/* @LA1LA2LA3LA4LA5LA6,LO1LO2LO3LO4LO5LO6 */
/* LA{1,2,3,4,5,6} stands for n-th digit of geo. latitude
 * LO{1,2,3,4,5,6} stands for n-th digit of geo. longitude
 * In both cases first two digits mean degrees, next two minutes and last two seconds.
 * */

/* USER CODE BEGIN 0 */
/**
 * \brief           Initializes UART_DMA_HandleTypeDef structure and sets EN pin on module to SET state
 * \param[in]		hblue: Handle to __Blue_Frame_HandleTypeDef structure
 * \param[in]		huart_dma: Handle to __UART_DMA_HandleTypeDef structure which delivers raw data
 */
void blue_parser_init(Blue_Frame_HandleTypeDef *hblue, UART_DMA_HandleTypeDef *huart_dma) {
	for(uint16_t i=0; i<LINE_BUFFER_SZ; i++)
		hblue->line_buffer[i] = '\0';
	hblue->writer_position = 0;
	hblue->reader_position = 0;
	for(uint8_t i=0; i<FIELD_BUFFER_SZ; i++)
		hblue->field_buffer[i] = '\0';
	hblue->field_position = 0;

	hblue->huart_dma 	= huart_dma;
	hblue->latitude 	= 0.0;
	hblue->longitude 	= 0.0;

	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);
}

/**
 * \brief           Downloads data received from UART and saves it in own buffer
 * \param[in]		hblue: Handle to __Blue_Frame_HandleTypeDef structure
 */
void blue_parser_recv_handler(Blue_Frame_HandleTypeDef *hblue) {
	uint8_t recv_char;

	while (ringbuff_get_full(&hblue->huart_dma->usart_rx_buff)) {
		ringbuff_read(&(hblue->huart_dma->usart_rx_buff), &recv_char, 1); /* Downaload data from ringbuff */

		if (hblue->writer_position == 0 && recv_char == '@') {
			hblue->writer_position++;
		} else if (hblue->writer_position >= 1 && hblue->writer_position < LINE_BUFFER_SZ-1) {
			if (recv_char == '\r') { /* Check if end of the line */
				hblue->line_buffer[hblue->writer_position - 1] = '\0';
				hblue->writer_position = 0;
				blue_parser_process_frame(hblue); /* Process complete frame*/
			} else {
				hblue->line_buffer[hblue->writer_position - 1] = recv_char;
				hblue->writer_position++;
			}
		} else {
			hblue->writer_position = 0;
		}
	}
}

/**
 * \brief           Reads following characters unless "," or EOF occur
 * \param[in]		hblue: Handle to __Blue_Frame_HandleTypeDef structure
 */
void blue_parser_read_field(Blue_Frame_HandleTypeDef *hblue) {
	hblue->field_position = 0;
	while(hblue->line_buffer[hblue->reader_position] != ',' && hblue->line_buffer[hblue->reader_position] != '\0'
			&& hblue->field_position < FIELD_BUFFER_SZ-1) {
		hblue->field_buffer[hblue->field_position] = hblue->line_buffer[hblue->reader_position];
		hblue->reader_position++;
		hblue->field_position++;
	}
	hblue->field_buffer[hblue->field_position] = '\0';
	hblue->reader_position++;
}

/**
 * \brief           Decodes latitude and longitude delimited by ","
 * \param[in]		hblue: Handle to __Blue_Frame_HandleTypeDef structure
 */
void blue_parser_process_frame(Blue_Frame_HandleTypeDef *hblue) {
	hblue->reader_position = 0;
	blue_parser_read_field(hblue);
	sscanf(hblue->field_buffer, "%lf", &(hblue->latitude));

	blue_parser_read_field(hblue);
	sscanf(hblue->field_buffer, "%lf", &(hblue->longitude));

	hblue->ParseCpltCallback(); /* Invoke callback to inform about new coordinates */
}
/* USER CODE END 0 */
