/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : gps.c
  * @brief          : GPS Source File
  * @author			: Hubert Zajączkowski
  ******************************************************************************
  */
/* USER CODE END Header */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gps.h"
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */
void gps_init(GPS_TypeDef *hgps, UART_DMA_HandleTypeDef *huart_dma) {

	for(uint8_t i=0; i<100; i++) hgps->line_buffer[i] = '\0';
	hgps->writer_position = 0;
	hgps->reader_position = 0;
	for(uint8_t i=0; i<30; i++) hgps->field_buffer[i] = '\0';
	hgps->field_position = 0;

	hgps->date_day = 0;
	hgps->date_mounth = 0;
	hgps->date_year = 0;
	hgps->time_hour = 0;
	hgps->time_min = 0;
	hgps->time_sec = 0;

	hgps->latitude = 0.0;
	hgps->latitude_direction = '?';
	hgps->longitude = 0.0;
	hgps->longitude_direction = '?';
	hgps->altitude = 0.0;

	hgps->speed_knots = 0.0;
	hgps->speed_kilometers = 0.0;

	hgps->satelites_number = 0;
	hgps->quality = '?';
	hgps->dop = 0.0;
	hgps->hdop = 0.0;
	hgps->vdop = 0.0;

	hgps->huart_dma = huart_dma;
}

void gps_rcv_handler(GPS_TypeDef *hgps) {
	uint8_t recv_char;

	while (ringbuff_get_full(&hgps->huart_dma->usart_rx_buff)) {
		ringbuff_read(&(hgps->huart_dma->usart_rx_buff), &recv_char, 1);

		if (hgps->writer_position == 0 && recv_char == '$') {
			hgps->writer_position++;
		} else if (hgps->writer_position >= 1 && hgps->writer_position < 99) {
			if (recv_char == '\r' || recv_char == '\n') {
				hgps->line_buffer[hgps->writer_position - 1] = '\0';
				hgps->writer_position = 0;
				gps_process_line(hgps);
			} else {
				hgps->line_buffer[hgps->writer_position - 1] = recv_char;
				hgps->writer_position++;
			}
		} else {
			hgps->writer_position = 0;
		}
	}
}

void gps_read_field(GPS_TypeDef *hgps) {
	hgps->field_position = 0;
	while(hgps->line_buffer[hgps->reader_position] != ',' && hgps->line_buffer[hgps->reader_position] != '\0'
			&& hgps->field_position < 29) {
		hgps->field_buffer[hgps->field_position] = hgps->line_buffer[hgps->reader_position];
		hgps->reader_position++;
		hgps->field_position++;
	}
	hgps->field_buffer[hgps->field_position] = '\0';
	hgps->reader_position++;
}

void gps_process_line(GPS_TypeDef *hgps) {
	hgps->reader_position = 0;
	gps_read_field(hgps);
	     if(strcmp(hgps->field_buffer, "GPRMC") == 0) gps_process_gprmc(hgps);
	else if(strcmp(hgps->field_buffer, "GPVTG") == 0) gps_process_gpvtg(hgps);
	else if(strcmp(hgps->field_buffer, "GPGGA") == 0) gps_process_gpgga(hgps);
	else if(strcmp(hgps->field_buffer, "GPGSA") == 0) gps_process_gpgsa(hgps);
}

void gps_process_gprmc(GPS_TypeDef *hgps)
{
	//$GPRMC,212846.00,A,5025.81511,N,01639.92090,E,0.196,,140417,,,A*73
	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0) {
		uint32_t tmp;
		sscanf(hgps->field_buffer, "%d", &tmp);
		hgps->time_sec = tmp % 100;
		hgps->time_min = (tmp / 100) % 100;
		hgps->time_hour = (tmp / 10000) % 100;
	}

	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->latitude));

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%c", &(hgps->latitude_direction));

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->longitude));

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%c", &(hgps->longitude_direction));

	gps_read_field(hgps);
	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0) {
		uint32_t tmp;
		sscanf(hgps->field_buffer, "%d", &tmp);
		hgps->date_year = tmp % 100;
		hgps->date_mounth = (tmp / 100) % 100;
		hgps->date_day = (tmp / 10000) % 100;
	}
}

void gps_process_gpvtg(GPS_TypeDef *hgps)
{
	//$GPVTG,,T,,M,0.196,N,0.363,K,A*2B
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->speed_knots));

	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->speed_kilometers));
}

void gps_process_gpgga(GPS_TypeDef *hgps)
{
	//$GPGGA,212846.00,5025.81511,N,01639.92090,E,1,04,4.72,281.1,M,42.0,M,,*5F
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%d", &(hgps->quality));

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%d", &(hgps->satelites_number));

	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->altitude));
}

void gps_process_gpgsa(GPS_TypeDef *hgps)
{
	//$GPGSA,A,3,10,18,21,15,,,,,,,,,6.79,4.72,4.89*01
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);
	gps_read_field(hgps);

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->dop));

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->hdop));

	gps_read_field(hgps);
	if(strlen(hgps->field_buffer) > 0)
		sscanf(hgps->field_buffer, "%lf", &(hgps->vdop));
}
/* USER CODE END 0 */

