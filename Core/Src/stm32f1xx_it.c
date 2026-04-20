#include "main.h"
#include "stm32f1xx_it.h"
#include "uart3_drv.h"
#include "SEGGER_RTT.h"
#include "bridge_app.h"
#include "tim.h"
#include "diff_signal.h"
void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        BridgeApp_UartRxCpltCallback(huart);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        BridgeApp_UartErrorCallback(huart);
    }
}
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM4)
    {
       DiffSignal_TimerPeriodElapsedCallback(htim);
    }
     if (htim->Instance == TIM6)
    {
         UW_LinkRx_1msTick();
    }
}
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}