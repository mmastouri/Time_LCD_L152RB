/**
  ******************************************************************************
  * @file    rtc.c
  * @author  MCD Application Team
  * @brief   lcd 7 segments driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */   
/* Private macro -------------------------------------------------------------*/

//#define USE_LSE

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
uint32_t vbat_value;
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void RTC_SetTime(uint8_t hour, uint8_t min)
{
  RTC_TimeTypeDef stime;

  stime.Hours = hour;
  stime.Minutes = min;
  stime.Seconds = 0x00;
  stime.TimeFormat = RTC_HOURFORMAT12_PM;
  stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  stime.StoreOperation = RTC_STOREOPERATION_RESET;
  
  HAL_RTC_SetTime(&hrtc,&stime,RTC_FORMAT_BIN);
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void RTC_GetTime(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
  RTC_DateTypeDef sdate;
  RTC_TimeTypeDef stime;
  
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
  
  *hour = stime.Hours;
  *min = stime.Minutes;
  *sec = stime.Seconds;  
}

/**
  * @brief  Function.
  * @param  None
  * @retval None
  */
void RTC_Init(void)
{ 
  
 /* Configure the RTC */
  hrtc.Instance = RTC; 
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  HAL_RTC_Init(&hrtc);
  
  
  HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 0x0, 0);
  
  /* Enable the RTC global Interrupt */
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
  
  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) ;
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x800, RTC_WAKEUPCLOCK_RTCCLK_DIV16);    
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  __HAL_RCC_RTC_ENABLE(); 
}

/**
  * @brief  This function handles RTC wakeup interrupt request.
  * @param  None
  * @retval None
  */
void RTC_WKUP_IRQHandler(void)
{
  HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
