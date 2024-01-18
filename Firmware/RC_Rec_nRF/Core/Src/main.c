/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MY_NRF24.h"
#include "nRF24L01.h"
#include"stdbool.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//Define structure for PWM value from TX
typedef struct{
	uint16_t ch1;
	uint16_t ch2;
	uint16_t ch3;
	uint16_t ch4;

//	uint16_t aux1;
//	uint16_t aux2;
//	uint16_t aux3;

	uint16_t swa;
	uint16_t swb;
//	uint16_t swc;
//	uint16_t swd;
//	uint16_t swe;
//	uint16_t swf;
//	uint16_t swg;
//	uint16_t swh;


}RxBufferTypedef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//IBUS
#define IBUS_FRAME_LENGTH			0x20
#define IBUS_COMMAND_CODE			0x40

//#define SBUS_PACKET_SIZE 25
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint64_t RxpipeAddrs = 0x11223344AA;
RxBufferTypedef Rx_Data;
uint32_t lastTime = 0;
uint32_t loop_timer;
uint8_t btn_state;
bool Is_Bind = false;
bool Is_NRF_Available = false;
bool myAckPayload = true;

//IBUS
uint8_t ibus_txBuffer[32];
uint8_t checksum[2];
uint8_t ibusEmpty =0;

//SBUS part
//uint8_t sbusBuffer[SBUS_PACKET_SIZE];
//uint16_t pwmValues[Rx_Data];
// uint16_t RxBufferTypedef[15];
//uint16_t pwmValues[16];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void delayUs(uint16_t delay){
	__HAL_TIM_SET_COUNTER(&htim2,0);
	while ( __HAL_TIM_GET_COUNTER(&htim2) < delay );
}

void ibus_send(){

	uint16_t check = 0xffff;
	ibus_txBuffer[1] = IBUS_COMMAND_CODE;
	ibus_txBuffer[0] = IBUS_FRAME_LENGTH;

	ibus_txBuffer[3] = Rx_Data.ch1 >> 8;
	ibus_txBuffer[2] = Rx_Data.ch1 & 0x00FF;

	ibus_txBuffer[5] = Rx_Data.ch2 >> 8;
	ibus_txBuffer[4] = Rx_Data.ch2 & 0x00FF;

	ibus_txBuffer[7] = Rx_Data.ch3 >> 8;
	ibus_txBuffer[6] = Rx_Data.ch3 & 0x00FF;

	ibus_txBuffer[9] = Rx_Data.ch4 >> 8;
	ibus_txBuffer[8] = Rx_Data.ch4 & 0x00FF;

	ibus_txBuffer[11] = Rx_Data.swa >> 8;
	ibus_txBuffer[10] = Rx_Data.swa & 0x00FF;

	ibus_txBuffer[13] = Rx_Data.swb >> 8;
	ibus_txBuffer[12] = Rx_Data.swb & 0x0FF;

	ibus_txBuffer[15] = ibusEmpty >> 8;
	ibus_txBuffer[14] = ibusEmpty & 0x00FF;

	ibus_txBuffer[17] = ibusEmpty >> 8;
	ibus_txBuffer[16] = ibusEmpty & 0x00FF;

	ibus_txBuffer[19] = ibusEmpty >> 8;
	ibus_txBuffer[18] = ibusEmpty & 0x00FF;

	ibus_txBuffer[21] = ibusEmpty >> 8;
	ibus_txBuffer[20] = ibusEmpty & 0x00FF;

	ibus_txBuffer[23] = ibusEmpty >> 8;
	ibus_txBuffer[22] = ibusEmpty & 0x00FF;

	ibus_txBuffer[25] = ibusEmpty >> 8;
	ibus_txBuffer[24] = ibusEmpty & 0x00FF;

	ibus_txBuffer[27] = ibusEmpty >> 8;
	ibus_txBuffer[26] = ibusEmpty & 0x00FF;

	ibus_txBuffer[29] = 0x05;
	ibus_txBuffer[28] = 0xDC;

	for (uint8_t i = 0; i<30; i++){
		check = check - ibus_txBuffer[i];
	}
	ibus_txBuffer[31] = check >> 8;
	ibus_txBuffer[30] = check & 0xFFFF;
	HAL_UART_Transmit(&huart1, ibus_txBuffer, sizeof(ibus_txBuffer), 100);

	HAL_Delay(4);
}
void fail_safe (){
  Rx_Data.ch1 = 1500;
  Rx_Data.ch2 = 1500;
  Rx_Data.ch3 = 0; //THR
  Rx_Data.ch4 = 1500;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, 1); //SBUS Inverter testing
  btn_state = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);
    if ( btn_state == 1 ) Is_Bind = true;
    else Is_Bind = false;

    NRF24_begin(SPI1_CSN_GPIO_Port, SPI1_CSN_Pin, SPI1_CE_Pin, hspi1);
    NRF24_setAutoAck(true);
   // NRF24_setChannel(81); //RX1
   // NRF24_setChannel(82); //RX2
   //NRF24_setChannel(83); //RX3
     // NRF24_setChannel(84); //RX4
    //NRF24_setChannel(85); //RX5
   //NRF24_setChannel(86); //RX6
     //NRF24_setChannel(87); //RX7
  // NRF24_setChannel(88); //RX8
    // NRF24_setChannel(89); //RX9
     //NRF24_setChannel(90); //RX10
    NRF24_setChannel(91); //RX
    NRF24_setPayloadSize(14);
    NRF24_openReadingPipe(1, RxpipeAddrs);
    NRF24_enableDynamicPayloads();
    NRF24_enableAckPayload();
    NRF24_startListening();

    HAL_TIM_Base_Start(&htim3);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

		while (Is_Bind) {
			if (NRF24_available()) {
				NRF24_read(&Rx_Data, sizeof(RxBufferTypedef));
				NRF24_writeAckPayload(1, &myAckPayload, 32);
				Is_Bind = false;
			}

			if (HAL_GetTick() - loop_timer > 100) {
				loop_timer = HAL_GetTick();
				HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
			}
		}


	  	if (HAL_GetTick() - loop_timer > 500) {
			loop_timer = HAL_GetTick();
			HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		}

		if (NRF24_available()) {
			NRF24_read(&Rx_Data, sizeof(RxBufferTypedef));
			lastTime = HAL_GetTick();
		}
		if ((HAL_GetTick() - lastTime > 500)) {
			fail_safe();
		}

	  	ibus_send();
	  	while ( __HAL_TIM_GET_COUNTER(&htim3) - loop_timer < 1000 )
	  	loop_timer = __HAL_TIM_GET_COUNTER(&htim3);


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
