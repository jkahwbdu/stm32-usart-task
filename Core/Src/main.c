/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ===================== 实验任务选择（★ 改这里） =====================
 *  改完这一行，重新编译下载，即可切换要做的任务。
 * ==================================================================== */
#define TASK_1_1_NO_DELAY_TX      0   /* 任务一(1) 连续发送，不加延时         */
#define TASK_1_2_QUERY_PAUSE      1   /* 任务一(2) 查询发送 + # 暂停 / * 继续 */
#define TASK_2_1_IT_PAUSE         2   /* 任务二(1) 中断方式发送 + # / * 控制  */
#define TASK_2_3_DMA_CONTINUOUS   3   /* 任务二(3) DMA 连续发送               */

#define APP_TASK   TASK_2_1_IT_PAUSE        /* ★★ 改这一行 ★★ */

/* 发送内容。作业原文是 "hello windows！"（全角感叹号），
   这里默认用纯 ASCII 版，任何串口助手都不会乱码；
   想用全角版就把下面第二行的注释打开。 */
#define HELLO_STR   "hello windows!\r\n"
/* #define HELLO_STR   "hello windows！\r\n" */

#define HELLO_LEN   ((uint16_t)strlen(HELLO_STR))

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */
#if (APP_TASK == TASK_1_2_QUERY_PAUSE) || (APP_TASK == TASK_2_1_IT_PAUSE)
volatile uint8_t g_tx_enable = 1;   /* 1 = 允许发送, 0 = 已暂停  */
uint8_t          g_rx_byte;         /* 中断接收的单字节缓冲      */
#endif

#if (APP_TASK == TASK_2_1_IT_PAUSE)
volatile uint8_t g_tx_busy = 0;     /* 中断发送是否进行中        */
#endif

#if (APP_TASK == TASK_2_3_DMA_CONTINUOUS)
uint8_t g_dma_buf[64];              /* DMA 待发送数据            */
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if (APP_TASK == TASK_1_2_QUERY_PAUSE) || (APP_TASK == TASK_2_1_IT_PAUSE)
/**
  * @brief  串口接收完成回调：解析上位机发来的 '#' 和 '*'
  *         '#' → 暂停发送    '*' → 继续发送
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    if (g_rx_byte == '#')                     /* 收到 # ：暂停 */
    {
      g_tx_enable = 0;
    }
    else if (g_rx_byte == '*')                /* 收到 * ：继续 */
    {
      g_tx_enable = 1;
#if (APP_TASK == TASK_2_1_IT_PAUSE)
      if (g_tx_busy == 0)                     /* 若此刻空闲，立即补发一帧 */
      {
        g_tx_busy = 1;
        HAL_UART_Transmit_IT(&huart1, (uint8_t *)HELLO_STR, HELLO_LEN);
      }
#endif
    }
    HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);   /* 重新打开接收中断 */
  }
}
#endif

#if (APP_TASK == TASK_2_1_IT_PAUSE)
/**
  * @brief  串口发送完成回调：一帧发完接着发下一帧 → 实现"连续发送"
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    g_tx_busy = 0;
    if (g_tx_enable)                          /* 没被暂停就接着发 */
    {
      g_tx_busy = 1;
      HAL_UART_Transmit_IT(&huart1, (uint8_t *)HELLO_STR, HELLO_LEN);
    }
  }
}
#endif

#if (APP_TASK == TASK_2_3_DMA_CONTINUOUS)
/**
  * @brief  DMA 发送完成回调：再次启动 DMA → 形成连续发送
  *         （若把 DMA 改成 Circular 模式，本函数可留空）
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    HAL_UART_Transmit_DMA(&huart1, g_dma_buf, sizeof(g_dma_buf));
  }
}
#endif

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
  /* USER CODE BEGIN 2 */

#if   (APP_TASK == TASK_1_1_NO_DELAY_TX)
  /* 任务一(1)：不需要额外初始化 */

#elif (APP_TASK == TASK_1_2_QUERY_PAUSE)
  /* 任务一(2)：打开接收中断，等待 # / * 命令 */
  HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);

#elif (APP_TASK == TASK_2_1_IT_PAUSE)
  /* 任务二(1)：接收中断 + 用中断方式发出第一帧，之后靠回调接力 */
  HAL_UART_Receive_IT(&huart1, &g_rx_byte, 1);
  g_tx_busy = 1;
  HAL_UART_Transmit_IT(&huart1, (uint8_t *)HELLO_STR, HELLO_LEN);

#elif (APP_TASK == TASK_2_3_DMA_CONTINUOUS)
  /* 任务二(3)：准备数据，然后用 DMA 启动发送 */
  {
    uint16_t i;
    for (i = 0; i < sizeof(g_dma_buf); i++)
    {
      g_dma_buf[i] = (uint8_t)('A' + (i % 26));
    }
  }
  HAL_UART_Transmit_DMA(&huart1, g_dma_buf, sizeof(g_dma_buf));
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

#if (APP_TASK == TASK_1_1_NO_DELAY_TX)
    /* ========= 任务一(1)：连续发送 "hello windows!"，不加任何延时 =========
     * 【现象】HAL_UART_Transmit() 是阻塞式、内部会等 TXE 标志，MCU 侧不会丢字节；
     *         但发送速度极快，PC 端串口助手(sscom)显示刷新不过来，会看到刷屏卡顿，
     *         看起来就像"丢了数据"。
     * 【对比1】把下面这句换成 USART1->DR = 'A';（不等 TXE 直接写寄存器）就会真丢字节。
     * 【对比2】把 HAL_Delay(1) 的注释打开，再观察"丢数据"现象是否消失。
     */
    HAL_UART_Transmit(&huart1, (uint8_t *)HELLO_STR, HELLO_LEN, HAL_MAX_DELAY);
/*  HAL_Delay(1);  */

#elif (APP_TASK == TASK_1_2_QUERY_PAUSE)
    /* ========= 任务一(2)：查询方式发送，靠接收中断处理 # / * ========= */
    if (g_tx_enable)
    {
      HAL_UART_Transmit(&huart1, (uint8_t *)HELLO_STR, HELLO_LEN, HAL_MAX_DELAY);
      HAL_Delay(500);      /* 加个延时便于观察 */
    }
    else
    {
      HAL_Delay(10);       /* 已暂停，空转等待 '*' */
    }

#elif (APP_TASK == TASK_2_1_IT_PAUSE)
    /* ========= 任务二(1)：全部由"串口中断"驱动，主循环空转 =========
     * 发送在 HAL_UART_TxCpltCallback() 里接力，接收在 HAL_UART_RxCpltCallback() 里解析。
     * 任务三要观察的波形就在 PA9(TX) 上。 */

#elif (APP_TASK == TASK_2_3_DMA_CONTINUOUS)
    /* ========= 任务二(3)：全部由"DMA"驱动，主循环空转 =========
     * 一帧 64 字节发完，在 HAL_UART_TxCpltCallback() 里再次启动 DMA。 */

#endif

    /* USER CODE END 3 */
  }
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
  /* HSE 8MHz 晶振 → PLL x9 → 72MHz（蓝色小板标配）。
     万一板上晶振不起振，自动退回 HSI（8MHz/2 x9 = 36MHz），避免卡死在 Error_Handler。 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* HSE 起振失败 → 退回 HSI */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
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

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
