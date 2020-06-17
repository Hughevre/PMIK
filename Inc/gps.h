/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gps.h
  * @brief   This file contains the headers of the interrupt handlers.
  * @author  Hubert Zajączkowski
 ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS_H
#define __GPS_H

#ifdef __cplusplus
 extern "C" {
#endif

 /* Private includes ----------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 /* USER CODE BEGIN Includes */
#include "stm32f4xx_hal.h"
#include "uart_dma.h"
 /* USER CODE END Includes */

 /* Exported types ------------------------------------------------------------*/
 /* USER CODE BEGIN Exported types */
typedef struct {
	char    line_buffer[100];
	uint8_t writer_position;
	uint8_t reader_position;
	char    field_buffer[30];
	uint8_t field_position;

	uint8_t date_day;
	uint8_t date_mounth;
	uint8_t date_year;
	uint8_t time_hour;
	uint8_t time_min;
	uint8_t time_sec;

	double latitude;
	char latitude_direction;
	double longitude;
	char longitude_direction;
	double altitude;

	double speed_knots;
	double speed_kilometers;

	uint8_t satelites_number;
	uint8_t quality;
	double dop;
	double hdop;
	double vdop;

	UART_DMA_HandleTypeDef *huart_dma;
}GPS_TypeDef;
/* USER CODE END Exported types */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void gps_init(GPS_TypeDef*, UART_DMA_HandleTypeDef*);
void gps_rcv_handler(GPS_TypeDef*);
void gps_read_field(GPS_TypeDef*);
void gps_process_line(GPS_TypeDef*);
void gps_process_gprmc(GPS_TypeDef*);
void gps_process_gpvtg(GPS_TypeDef*);
void gps_process_gpgga(GPS_TypeDef*);
void gps_process_gpgsa(GPS_TypeDef*);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __GPS_H */
