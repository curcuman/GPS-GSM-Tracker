/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rxData[1];
uint8_t received[400];
uint8_t sending[200];
uint8_t LoraBuffer[200];

int char_counter = 0;
int timeout = 10; //in seconds

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void quickBlink(void);
int found(void);
int foundCREG(void);
int foundRING(void);
int foundMESSAGE(void);
void send_message(void);
void get_position_send_message(void);
void clean(void);
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Receive_DMA(&huart1, rxData, sizeof(rxData));



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  //wait for the A9G to initialize
	  HAL_UART_Transmit(&huart2, "\nStarting...", sizeof("\nStarting..."), 100);

	  HAL_Delay(5000);
	  HAL_UART_Transmit(&huart2, " I waited. Now waiting A9G to reply", sizeof(" I waited. Now waiting A9G to reply"), 100);
	  do{
		  //HAL_UART_Transmit(&huart2, "Polling a9g..", sizeof("Polling a9g.."), 100);
		  HAL_UART_Transmit(&huart2, ".", sizeof("."), 100);
		  quickBlink(); //blink for a9g
		  HAL_Delay(2000);
		  clean();
		  HAL_UART_Transmit(&huart1, "AT\r", sizeof("AT\r"), 100);
		  HAL_Delay(500);
		  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
	  }while(!found());

	  HAL_UART_Transmit(&huart2, "\nA9G responding! Now starting...\n", sizeof("\nA9G responding! Now starting...\n"), 100);
	  HAL_Delay(1000);

	  //Couple of AT to flush
	  clean();
	  HAL_UART_Transmit(&huart1, "AT\r", sizeof("AT\r"), 100);
	  HAL_Delay(200);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
	  HAL_Delay(500);
	  clean();
	  HAL_UART_Transmit(&huart1, "AT\r", sizeof("AT\r"), 100);
	  HAL_Delay(200);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
	  clean();
	  HAL_Delay(1000);

	  //Activate GPS
	  HAL_Delay(500);
	  clean();
	  HAL_UART_Transmit(&huart1, "AT+GPS=1\r", sizeof("AT+GPS=1\r"), 100);
	  HAL_Delay(200);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);

	  //Enable messages in text
	  HAL_Delay(500);
	  clean();
	  HAL_UART_Transmit(&huart1, "AT+CMGF=1\r", sizeof("AT+CMGF=1\r"), 100);
	  HAL_Delay(200);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);

	  //Check if sim correctly inserted
	  HAL_Delay(500);
	  clean();
	  HAL_UART_Transmit(&huart1, "AT+CPIN?\r", sizeof("AT+CPIN?\r"), 100);
	  HAL_Delay(200);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);

	  //Verification of connection to network
	  do{
		  quickBlink();
		  quickBlink();
		  clean();
		  HAL_UART_Transmit(&huart1, "AT+CREG?\r", sizeof("AT+CREG?\r"), 100);
		  HAL_Delay(200);
		  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
		  HAL_UART_Transmit(&huart2, "\nNOT connected to network...", sizeof("\nNOT connected to network..."), 100);
		  HAL_Delay(5000);
	  }while(!foundCREG());
	  HAL_UART_Transmit(&huart2, "\nCONNECTED TO NETWORK!\n", sizeof("\nCONNECTED TO NETWORK!\n"), 100);

	  //The main while to retrieve the position and send it begins
	  while(1){

		  //sending every 5 mins, and repliying if message receieved
		  for(int i=0; i<18; i++){
			  clean(); //clean first!
			  HAL_Delay(10000);
			  HAL_UART_Transmit(&huart2, "time passed, call incoming?", sizeof("time passed, call incoming?"), 100);
			  HAL_Delay(200);
			  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
			  if(foundRING()){
				  HAL_UART_Transmit(&huart2, "- someone calling! -", sizeof("- someone calling! -"), 100);
				  HAL_UART_Transmit(&huart1, "ATA\r", sizeof("ATA\r"), 100);
			  }

			  if(foundMESSAGE()){
				  HAL_UART_Transmit(&huart2, "- someone TEXTING! -", sizeof("- someone TEXTING! -"), 100);
				  get_position_send_message();
			  }

		  }

		  get_position_send_message();

	  }



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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */



void get_position_send_message(){
	  //Initial acquisition of the position
	  do{
		  quickBlink();
		  quickBlink();
		  quickBlink();
		  quickBlink();

		  HAL_Delay(5000);
		  if(foundRING()){
			  HAL_UART_Transmit(&huart2, "- someone calling! -", sizeof("- someone calling! -"), 100);
			  HAL_UART_Transmit(&huart1, "ATA\r", sizeof("ATA\r"), 100);
		  }
		  HAL_Delay(200);
		  clean();
		  HAL_UART_Transmit(&huart1, "AT+LOCATION=2\r", sizeof("AT+LOCATION=2\r"), 100);
		  HAL_Delay(500);
		  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
	  }while(!found());

	  HAL_UART_Transmit(&huart2, "\nPosition found, with response:\n", sizeof("\nPosition found, with response:\n"), 100);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);
	  HAL_UART_Transmit(&huart2, "\nEXTRACTED value:", sizeof("\nEXTRACTED value:"), 100);

	  uint8_t coordinates[20];
	  extractCoordinates(coordinates);
	  HAL_UART_Transmit(&huart2, coordinates, sizeof(coordinates), 100);

	  //Sending a message
	  HAL_UART_Transmit(&huart2, "\nSending message now..\n", sizeof("\nSending message now..\n"), 100);
	  clean();
	  HAL_UART_Transmit(&huart1, "AT+CMGS=\"+393774001005\"\r", sizeof("AT+CMGS=\"+393774001005\"\r"), 100);
	  HAL_Delay(1000);
	  HAL_UART_Transmit(&huart1, coordinates, sizeof(coordinates), 100);
	  uint8_t end_character[] = { 0x1A };
	  HAL_UART_Transmit(&huart1, end_character, sizeof(end_character), 100);

	  HAL_UART_Transmit(&huart2, "\nNow waiting 15 secs before reading if it went well..\n", sizeof("\nNow waiting 15 secs before reading if it went well..\n"), 100);
	  HAL_Delay(15000);
	  HAL_UART_Transmit(&huart2, received, sizeof(received), 100);

	  HAL_UART_Transmit(&huart2, "\nDid you see it? Now I go on\n", sizeof("\nDid you see it? Now I go on\n"), 100);

}

void quickBlink(){
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	  HAL_Delay(100);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
	  HAL_Delay(100);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	  HAL_Delay(100);
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
	  HAL_Delay(100);

}

void extractCoordinates(uint8_t * output) {
	  //uint8_t coordinates[20];
	  int i = 0;
	  for(int a=16; a<36; a++){
		  output[i]=received[a];
		  i++;
	  }
}



int found() {

	uint8_t in[5] = "OK";
    char* okSubstring = strstr(received, in);
    return (okSubstring != NULL);
}

int foundCREG() {

	uint8_t in1[5] = "1,5";
	uint8_t in2[5] = "1,1";
    char* okSubstring1 = strstr(received, in1);
    char* okSubstring2 = strstr(received, in2);
    return (okSubstring1 != NULL || okSubstring1 != NULL);
}

int foundRING() {

	uint8_t in1[6] = "RING";
    char* okSubstring1 = strstr(received, in1);
    return (okSubstring1 != NULL);
}

int foundMESSAGE() {

	uint8_t in1[9] = "MESSAGE";
    char* okSubstring1 = strstr(received, in1);
    return (okSubstring1 != NULL);
}

int finished(){

	int kFound = 0;
	for (int i = 0; i < char_counter; ++i) {
		if (received[i] == '\r') {
			kFound = 1;
			break;
		}
	}
	return kFound;
}


void send_with_wait(uint8_t *send){
	  HAL_UART_Transmit(&huart1, send, sizeof(send), 100);
	  //HAL_Delay(200);

	  //Function to wait in case the response is not complete yet
	  int iterations = 0;
	  while (!finished() && iterations < timeout*1000){
		 HAL_UART_Transmit(&huart2, ".", sizeof("."), 10); //removable line
		 //HAL_Delay(1);
		 iterations++;
	  }
}

void clean(){
	  //reset memory
	  memset(received, '\0', sizeof(received));
	  char_counter = 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	received[char_counter] = rxData[0];
	char_counter = char_counter + 1;

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    HAL_UART_Transmit(&huart2, (uint8_t *)"WakeUP from SLEEP by EXTI\r\n", strlen ("WakeUP from SLEEP by EXTI\r\n"), HAL_MAX_DELAY);
    HAL_PWR_DisableSleepOnExit ();
}

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
