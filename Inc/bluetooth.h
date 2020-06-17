/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bluetooth.h
  * @brief          : Header for bluetooth.c file.
  * @author			: Hubert Zajączkowski
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart_dma.h"
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
#define LINE_BUFFER_SZ 	1024
#define FIELD_BUFFER_SZ 128
/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN */
typedef struct __Blue_Frame_HandleTypeDef {
	uint8_t 				line_buffer[LINE_BUFFER_SZ];
	uint8_t 				writer_position;
	uint8_t 				reader_position;
	char 					field_buffer[FIELD_BUFFER_SZ];
	uint8_t 				field_position;

	UART_DMA_HandleTypeDef 	*huart_dma;

	double 					latitude;
	double 					longitude;

	void           			(*ParseCpltCallback)();        /* Parsing frame complete callback */
}Blue_Frame_HandleTypeDef;
/* USER CODE BEGIN */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void blue_parser_init(Blue_Frame_HandleTypeDef*, UART_DMA_HandleTypeDef*);
void blue_parser_recv_handler(Blue_Frame_HandleTypeDef*);
void blue_parser_read_field(Blue_Frame_HandleTypeDef*);
void blue_parser_process_frame(Blue_Frame_HandleTypeDef*);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_H */
