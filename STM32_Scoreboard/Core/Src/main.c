/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2; /* USART2 KULLANILACAK  */

/* USER CODE BEGIN PV */
volatile uint8_t is_match_running = 0;
volatile uint8_t is_match_finished = 0;

volatile uint8_t current_team = 0;
volatile uint32_t score[2] = {0, 0};

volatile uint8_t minutes = 0;
volatile uint8_t seconds = 0;

volatile uint32_t match_period = 1;
volatile uint8_t anim_request_start = 0;
volatile uint8_t anim_request_end = 0;

volatile uint32_t ldr_value = 0;     // Işık seviyesi (0-4095 arası)
uint8_t gelen_veri; // Bilgisayardan gelen harf burada duracak

#define MAX_MINUTES 2
#define BUZZER_PIN GPIO_PIN_12
#define BUZZER_PORT GPIOB

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void); /* USART2 Init Fonksiyonu */
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Send_Data_To_IoT(void) // iot a verileri paket halinde göndermek için
{
    char tx_buffer[64]; // Buffer boyutu artırıldı


    sprintf(tx_buffer, "A:%lu,B:%lu,D:%lu,S:%d\n", score[0], score[1], minutes, seconds);

    HAL_UART_Transmit(&huart2, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
}


void Delay_us_Rough(uint32_t us)  // buzzer ses tonlaması yapmak için
{
    uint32_t count = us * 4;
    while(count--)
    {
        __NOP();
    }
}


void Play_Tone(uint32_t pitch, uint32_t duration_ms) //buzzer çaldıran fonksiyon
{
    uint32_t elapsed = 0;
    while (elapsed < duration_ms * 1000)
    {
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        Delay_us_Rough(pitch);
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
        Delay_us_Rough(pitch);

        elapsed += (pitch * 2); // ses için
    }
}


void Play_Referee_Whistle(void) // hakem düdüğü sesi (maç bitiminde)
{
    for(int i=0; i<8; i++)
    {
        Play_Tone(150, 40);
        Play_Tone(250, 40);
    }
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

void Play_Goal_Sound(void) // gol atılınca çıkan sevinç efekti. (gool)
{
    for(int i=500; i>100; i-=10)
    {
        Play_Tone(i, 10);
    }
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}


void Play_LED_Sequence(void) // maç başladığında ve bittiğinde sıralı led yakımı
{
    for(int i=0; i<3; i++)
    {
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,1);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 1);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_WHITE_GPIO_Port, LED_WHITE_Pin, 1);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, 1);
        HAL_Delay(500);

        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,0);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, 0);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_WHITE_GPIO_Port, LED_WHITE_Pin, 0);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, 0);
        HAL_Delay(500);
    }
}


void Score_Increment(void) // seçili takımın skorunu artırma
{
  score[current_team]++;
}


void Score_Decrement(void) // seçili takımın skorunu azaltma
{
  if (score[current_team] > 0)
  {
    score[current_team]--;
  }
}


void Play_Buzzer(uint16_t duration_ms)
{
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
  HAL_Delay(duration_ms);
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    (void)score;
    (void)minutes;
    (void)seconds;

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
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim3);

  // RX Kesmesini başlat (USART2 üzerinden)
  HAL_UART_Receive_IT(&huart2, &gelen_veri, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      if (anim_request_start ==1) // animasyon (yarı) başladığında ledleri yakmaya başla
      {
          Play_LED_Sequence();
          anim_request_start=0;
      }
      if (anim_request_end ==1) // animasyon (yarı) bittiğinde ledleri yakmaya başla +buzzer
      {
          Play_LED_Sequence();
          Play_Referee_Whistle();
          anim_request_end=0;
      }

      if (is_match_finished == 1) // maç bittiyse
          {
              HAL_GPIO_WritePin(RGB_B_GPIO_Port, RGB_B_Pin, GPIO_PIN_SET);
              Play_Buzzer(200);
              HAL_Delay(800);
              HAL_GPIO_WritePin(RGB_B_GPIO_Port, RGB_B_Pin, GPIO_PIN_RESET);
              HAL_Delay(200);
          }
          else if (is_match_running == 1) // maç devam ediyorsa mavi ledi toggle yap
          {
              HAL_GPIO_TogglePin(RGB_B_GPIO_Port, RGB_B_Pin);
              HAL_Delay(500);
          }
          else // diğer durumlarda blue ledi resetle. (Ledin maç sonrası yanmaması için)
          {
              HAL_GPIO_WritePin(RGB_B_GPIO_Port, RGB_B_Pin, GPIO_PIN_RESET);

          }
      // 1. ADC'yi Başlat
          HAL_ADC_Start(&hadc1);

          // 2. Çevirme işleminin bitmesini bekle (Maksimum 10ms)
          if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
          {
              // 3. Değeri Oku
              ldr_value = HAL_ADC_GetValue(&hadc1);
          }

          // --- KARAR ANI (STADYUM IŞIKLARI) ---
          if (ldr_value < 1500)
          {
              // Hava Karardı -> Ekstra LED'leri Yak
              if(is_match_finished == 0) {           // Maç bitmemişse yak
                   HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
              }
          }
          else
          {
              if(is_match_running == 0)
              {
                  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
              }
          }
  }
 }
  /* USER CODE END 3 */


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 1023;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 62499;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function (GÜNCELLENDİ)
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BUZZER_Pin|RGB_B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_5_GPIO_Port, LED_5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_WHITE_Pin|LED_BLUE_Pin|LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BUTON1_Pin BUTON2_Pin BUTON3_Pin */
  GPIO_InitStruct.Pin = BUTON1_Pin|BUTON2_Pin|BUTON3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BUZZER_Pin RGB_B_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin|RGB_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_5_Pin */
  GPIO_InitStruct.Pin = LED_5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_WHITE_Pin LED_BLUE_Pin LED_GREEN_Pin LED_RED_Pin */
  GPIO_InitStruct.Pin = LED_WHITE_Pin|LED_BLUE_Pin|LED_GREEN_Pin|LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin) // kullanıcı butonlara bastığında ne olacağını yönlendirir
{
  static uint32_t last_interrupt_time = 0;
  uint32_t current_time = HAL_GetTick();

  if ((current_time - last_interrupt_time) < 200) return;
  last_interrupt_time = current_time;


  if (GPIO_Pin == BUTON1_Pin) // buton1 maç durdur/başlat
  {
     if (is_match_finished)
     {

         is_match_finished = 0;
         is_match_running = 0;
         match_period = 1;
         minutes = 0;
         seconds = 0;
         score[0] = 0;
         score[1] = 0;
         anim_request_start = 1;
     }
     else
     {

         if (is_match_running == 0) {
             anim_request_start = 1;
         }
         is_match_running = !is_match_running;
     }
  }

  else if (GPIO_Pin == BUTON2_Pin) // takım seçtirir a/b
  {
      current_team = !current_team;
  }
  else if (GPIO_Pin == BUTON3_Pin) // seçili takıma skor artırır (eğer maç durdurulmuşsa skor azaltır)
    {
       if(is_match_running == 1)
       {
          score[current_team]++;
          Play_Goal_Sound();
       }
       else
       {
          if(score[current_team] > 0) // gol iptali durumunda iptal mesajı buzzer çaldırır
          {
              score[current_team]--;
              Play_Tone(300, 200);
          }
       }
       Send_Data_To_IoT();
    }

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) // kronometreyi işletir.
{
  if (htim->Instance == TIM3)
  {
    if (is_match_running == 1)
    {
      seconds++;
      if (seconds >= 60) { seconds = 0; minutes++; }

      if (minutes == 1 && seconds == 0 && match_period == 1) // maç süresi 2 dakika ayarlandı
      {
          is_match_running = 0;
          match_period = 2;
          anim_request_end = 1;
          Send_Data_To_IoT();
      }
      else if (minutes >= 2) // 2. yarı başlangıcı
      {
        is_match_running = 0;
        is_match_finished = 1;
        anim_request_end = 1;
        Send_Data_To_IoT();
      }
    }
  }
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
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
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
#endif /* USE_FULL_ASSERT */
