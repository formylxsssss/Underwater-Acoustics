/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usb.h"
#include "gpio.h"
#include "diff_signal.h"
#include "stdint.h"
#include "ADXL345.h"
#include "DHT22.h"
#include "stm32f1xx_hal_gpio_ex.h"
#include "diff_signal.h"
#include "SEGGER_RTT.h"
#include "Custcom_Pin.h"
#include "battery_adc.h"
#include "uart3_drv.h"
#include "bridge_app.h"
#include "soft_timer.h"
#include "data_collect.h"
#include "stm32f1xx_hal_tim.h"
#include "rs485_port.h"
#include "uw_link_rx.h"
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
void dht22_data_read_callback(void);
void ADXL345_data_read_call_back(void);
static void Debug_PrintAcousticRxFrame(uint8_t device_id, const uint8_t *payload, uint16_t payload_len);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void Debug_PrintAcousticRxFrame(uint8_t device_id, const uint8_t *payload, uint16_t payload_len)
{
  uint16_t i;
  myprintf("[UW_RX] device=0x%02X len=%u\r\n", device_id, payload_len);
  for (i = 0u; i < payload_len; i++)
  {
    myprintf("[UW_RX] payload[%u]=0x%02X\r\n", i, payload[i]);
  }
}

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
  myprintf("hal_init success\n");
  SystemClock_Config();
  myprintf("systemclock init success\n");
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
  __HAL_AFIO_REMAP_TIM1_PARTIAL();
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM6_Init(); // CubeMX 生成的 TIM6 初始化
  MX_TIM4_Init();
  MX_TIM8_Init();
  HAL_TIM_Base_Start_IT(&htim6);
  BatteryADC_Init();
  HAL_Delay(100);
  DiffSignal_Init();
  DiffSignal_SetBitRate(100);
  UW_LinkRx_Init(0x01u);
  USART3_Driver_Init(9600);
  BridgeApp_Init();
  HAL_Delay(2000);
  /* USER CODE BEGIN 2 */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    BridgeApp_Process();
    if (UW_LinkRx_FrameReady())
    {
      uint8_t device_id = 0u;
      uint8_t payload[UW_LINK_RX_PAYLOAD_MAX_LEN];
      uint16_t payload_len = 0u;
      if (UW_LinkRx_GetFrame(&device_id, payload, &payload_len))
      {
        Debug_PrintAcousticRxFrame(device_id, payload, payload_len);
        (void)device_id;
        (void)BridgeApp_PushAcousticRxPacket(payload, (uint8_t)payload_len);
      }
    }
  }
}

void dht22_data_read_callback(void)
{
  float temp, hum = 0x00;
  uint8_t power = 0x00;
  DHT22_ReadData(&temp, &hum);
  power = BatteryADC_ReadPercent();
  // myprintf("temper is %f hum is %f\n", temp, hum);
  set_local_dht22_data(temp, hum);
  set_local_power_data(power);
}

void ADXL345_data_read_call_back(void)
{
  float x_data, y_data, z_data;
  ADXL345_ReadXYZ(&x_data, &y_data, &z_data);
  set_local_adxl_345_data(x_data, y_data, z_data);
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** 配置外部晶振 HSE 和 PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; // 使用外部晶振
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                   // 打开 HSE
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;    // HSE不分频
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                   // 打开 HSI（备份）
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;               // 打开 PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;       // PLL输入源为HSE
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;               // 8 MHz × 9 = 72 MHz

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    myprintf("error 1\n");
    Error_Handler();
  }

  /** 初始化系统总线时钟 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // SYSCLK = PLL 输出 = 72 MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // HCLK = 72 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;         // PCLK1 = 36 MHz (需 ≤ 36 MHz)
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;         // PCLK2 = 72 MHz

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    myprintf("error 2\n");
    Error_Handler();
  }

  /** 设置 USB 时钟为 48 MHz（需要 SYSCLK = 72 MHz）*/
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5; // 72 MHz / 1.5 = 48 MHz

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    myprintf("error 3\n");
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
