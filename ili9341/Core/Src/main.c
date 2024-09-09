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
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
#include "ili9341.h"
#include "fonts.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void LCDTest(void);  // 确保在这里声明 LCDTest 函数

/* USER CODE BEGIN PFP */

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
  MX_FSMC_Init();
  MX_USART1_UART_Init();
  
  ILI9341_Init ();             //LCD 初始化
  
  printf("Hello, USART1!\n");
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    LCDTest();    
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

extern uint16_t lcdid;

/*用于测试各种液晶的函数*/
void LCDTest(void)
{
    /*演示显示变量*/
    static uint8_t testCNT = 0;    
    char dispBuff[100];
    
    testCNT++;    
    
    LCD_SetFont(&Font8x16);
    LCD_SetColors(RED,BLACK);

    ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);    /* 清屏，显示全黑 */
    /********显示字符串示例*******/
    ILI9341_DispStringLine_EN(LINE(0),"BH 3.2 inch LCD para:");
    ILI9341_DispStringLine_EN(LINE(1),"Image resolution:240x320 px");
    if(lcdid == LCDID_ILI9341)
    {
        ILI9341_DispStringLine_EN(LINE(2),"ILI9341 LCD driver");
    }
    else if(lcdid == LCDID_ST7789V)
    {
        ILI9341_DispStringLine_EN(LINE(2),"ST7789V LCD driver");
    }
    ILI9341_DispStringLine_EN(LINE(3),"XPT2046 Touch Pad driver");
    
    /********显示变量示例*******/
    LCD_SetFont(&Font16x24);
    LCD_SetTextColor(GREEN);

    /*使用c标准库把变量转化成字符串*/
    sprintf(dispBuff,"Count : %d ",testCNT);
    LCD_ClearLine(LINE(4));    /* 清除单行文字 */
    
    /*然后显示该字符串即可，其它变量也是这样处理*/
    ILI9341_DispStringLine_EN(LINE(4),dispBuff);

    /*******显示图形示例******/
    LCD_SetFont(&Font24x32);
    /* 画直线 */
    
    LCD_ClearLine(LINE(4));/* 清除单行文字 */
    LCD_SetTextColor(BLUE);

    ILI9341_DispStringLine_EN(LINE(4),"Draw line:");
    
    LCD_SetTextColor(RED);
    ILI9341_DrawLine(50,170,210,230);  
    ILI9341_DrawLine(50,200,210,240);
    
    LCD_SetTextColor(GREEN);
    ILI9341_DrawLine(100,170,200,230);  
    ILI9341_DrawLine(200,200,220,240);
    
    LCD_SetTextColor(BLUE);
    ILI9341_DrawLine(110,170,110,230);  
    ILI9341_DrawLine(130,200,220,240);
    
    HAL_Delay(0xFFFFFF);
    
    ILI9341_Clear(0,16*8,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);    /* 清屏，显示全黑 */
    
    
    /*画矩形*/

    LCD_ClearLine(LINE(4));    /* 清除单行文字 */
    LCD_SetTextColor(BLUE);

    ILI9341_DispStringLine_EN(LINE(4),"Draw Rect:");

    LCD_SetTextColor(RED);
    ILI9341_DrawRectangle(50,200,100,30,1);
    
    LCD_SetTextColor(GREEN);
    ILI9341_DrawRectangle(160,200,20,40,0);
    
    LCD_SetTextColor(BLUE);
    ILI9341_DrawRectangle(170,200,50,20,1);
    
    
    HAL_Delay(0xFFFFFF);
    
    ILI9341_Clear(0,16*8,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);    /* 清屏，显示全黑 */

    /* 画圆 */
    LCD_ClearLine(LINE(4));    /* 清除单行文字 */
    LCD_SetTextColor(BLUE);
    
    ILI9341_DispStringLine_EN(LINE(4),"Draw Cir:");

    LCD_SetTextColor(RED);
    ILI9341_DrawCircle(100,200,20,0);
    
    LCD_SetTextColor(GREEN);
    ILI9341_DrawCircle(100,200,10,1);
    
    LCD_SetTextColor(BLUE);
    ILI9341_DrawCircle(140,200,20,0);

    HAL_Delay(0xFFFFFF);
    
    ILI9341_Clear(0,16*8,LCD_X_LENGTH,LCD_Y_LENGTH-16*8);    /* 清屏，显示全黑 */
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

