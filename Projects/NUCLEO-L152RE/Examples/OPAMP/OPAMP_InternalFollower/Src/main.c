/**
  ******************************************************************************
  * @file    OPAMP/OPAMP_InternalFollower/Src/main.c
  * @author  MCD Application Team
  * @brief   This example provides a short description of how to use
  *          the OPAMP peripheral in internal follower mode.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32L1xx_HAL_Examples
  * @{
  */

/** @addtogroup OPAMP_InternalFollower
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RANGE_12BITS                   ((uint32_t)4095)    /* Max value with a full range of 12 bits */
#define TIMER_FREQUENCY_HZ             ((uint32_t)1000)    /* Timer frequency (unit: Hz). With SysClk set to 32MHz, timer frequency TIMER_FREQUENCY_HZ range is min=1Hz, max=32.719kHz. */
#define WAVEFORM_RAMP_12BITS_5SAMPLES  ((uint32_t)   5)    /* Size of waveform Waveform_Ramp_12bits_5samples[] */
#define ADCCONVERTEDVALUES_BUFFER_SIZE ((uint32_t)  32)    /* Size of array aADCxConvertedValues[] */
#define ADC_MEASUREMENT_TOLERANCE_LSB  ((uint32_t) 140)    /* ADC measurement tolerance of uncertainty (unit: LSB). Note: higher than theorical tolerance beacause board routing and connectors are not designed for optimal ADC performance. */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

ADC_HandleTypeDef    AdcHandle;    /* ADC handle declaration */
DAC_HandleTypeDef    DacHandle;    /* DAC handle declaration */
TIM_HandleTypeDef    TimHandle;    /* TIM handle declaration */
OPAMP_HandleTypeDef  OpampHandle;  /* OPAMP handle declaration */

/* Variable containing waveform samples to be generated by DAC */
const uint16_t Waveform_Ramp_12bits_5samples[WAVEFORM_RAMP_12BITS_5SAMPLES] = {0, 1023, 2047, 3071, 4095};

/* Variable containing ADC conversions results */
__IO uint16_t   aADCxConvertedValues[ADCCONVERTEDVALUES_BUFFER_SIZE];

/* Variables to manage push button on board: interface between ExtLine interruption and main program */
__IO uint8_t    ubUserButtonClickEventToggle = RESET;  /* Event detection: Toggle after User Button interrupt */

/* Variable to report OPAMP input vs output voltage comparison status: */
/*   RESET <=> OPAMP output is different of non-inverting input */
/*   SET   <=> OPAMP output is equal to non-inverting input */
uint8_t         ubOPAMPOutputInputVoltageStatus = RESET;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void Error_Handler(void);
static void ADC_Config(void);
static void DAC_Config(void);
static void TIM_Config(void);
static void OPAMP_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32L1xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();
  
  /* Configure the system clock to 32 MHz */
  SystemClock_Config();
  
  /*## Configure peripherals #################################################*/
  /* Initialize LED on board */
  BSP_LED_Init(LED2);

  /* Configure User push-button in Interrupt mode */
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Configure the ADC peripheral */
  ADC_Config();
  
  /* Configure the TIM peripheral */
  TIM_Config();
  
  /* Configure the DAC peripheral */
  DAC_Config();

  /* Configure the OPAMP peripheral */
  OPAMP_Config();
  

  /*## Enable peripherals ####################################################*/

  /* Enable DAC selected channel with transfer by DMA */
  if(HAL_DAC_Start_DMA(&DacHandle, DACx_CHANNEL_TO_OPAMP1_NONINV_INPUT, (uint32_t*)Waveform_Ramp_12bits_5samples, 
                       sizeof (Waveform_Ramp_12bits_5samples) / sizeof (uint16_t), 
                       DAC_ALIGN_12B_R) != HAL_OK)
  {
    /* Start DMA Error */
    Error_Handler();
  }
    
  /* Start ADC conversion on regular group with transfer by DMA */
  if (HAL_ADC_Start_DMA(&AdcHandle,
                        (uint32_t *)aADCxConvertedValues,
                        ADCCONVERTEDVALUES_BUFFER_SIZE
                       ) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }

  /* Timer counter enable */
  if (HAL_TIM_Base_Start(&TimHandle) != HAL_OK)
  {
    /* Counter Enable Error */
    Error_Handler();
  }

  /* Enable OPAMP */
  HAL_OPAMP_Start(&OpampHandle);
  
  
  /* Infinite loop */
  while (1)
  {
    /* Turn-on/off LED2 in function of OPAMP output vs input voltage          */
    /* comparison status:                                                     */
    /*  - Turn-off if OPAMP output is different of non-inverting input        */ 
    /*  - Turn-on if OPAMP output is equal to non-inverting input             */

    /* Variable of OPAMP voltage comparison status is set into ADC conversion */
    /* complete callback.                                                     */
    if (ubOPAMPOutputInputVoltageStatus == RESET)
    {
      BSP_LED_Off(LED2);
    }
    else
    {
      BSP_LED_On(LED2);
    }
  
    /* Optionally, for this example purpose: enable/disable OPAMP when        */
    /* pressing on User push-button to watch the impact on OPAMP output      */
    /* voltage and LED status.                                                */
    if (ubUserButtonClickEventToggle == RESET)
    {
      /* Enable OPAMP */
      HAL_OPAMP_Start(&OpampHandle);
    }
    else
    {
      /* Disable OPAMP */
      HAL_OPAMP_Stop(&OpampHandle);
    }

    /* For information: ADC conversion results are stored into array          */
    /* "aADCxConvertedValues" (for debug: check into watch window)            */
    /* These results can be compared to reference waveform                    */
    /* "Waveform_Ramp_12bits_5samples".                                       */
  }
  
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLLMUL                         = 6
  *            PLLDIV                         = 3
  *            Flash Latency(WS)              = 1
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Enable HSE Oscillator and Activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Set Voltage scale1 as MCU will run at 32MHz */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  ADC configuration
  * @param  None
  * @retval None
  */
static void ADC_Config(void)
{
  ADC_ChannelConfTypeDef   sConfig;
  
  /* Configuration of ADCx init structure: ADC parameters and regular group */
  AdcHandle.Instance = ADCx;

  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV4;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
  AdcHandle.Init.LowPowerAutoWait      = ADC_AUTOWAIT_DISABLE;
  AdcHandle.Init.LowPowerAutoPowerOff  = ADC_AUTOPOWEROFF_DISABLE;
  AdcHandle.Init.ChannelsBank          = ADC_CHANNELS_BANK_A;
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode disabled to have only 1 conversion at each conversion trig */
  AdcHandle.Init.NbrOfConversion       = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_Tx_TRGO;  /* Trig of conversion start done by external event */
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_FALLING; /* Trig of conversion on timer falling edge */
  AdcHandle.Init.DMAContinuousRequests = ENABLE;

  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
    /* ADC initialization error */
    Error_Handler();
  }
 
  /* Configuration of channel on ADCx regular group on sequencer rank 1 */
  /* Note: Considering IT occurring after each number of                      */
  /*       "ADCCONVERTEDVALUES_BUFFER_SIZE"  ADC conversions (IT by DMA end   */
  /*       of transfer), select sampling time and ADC clock with sufficient   */
  /*       duration to not create an overhead situation in IRQHandler.        */
  sConfig.Channel      = ADCx_CHANNEL_TO_OPAMP1_OUTPOUT;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_96CYCLES;
  
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    Error_Handler();
  }
    
}

/**
  * @brief  TIM configuration
  * @param  None
  * @retval None
  */
static void TIM_Config(void)
{
  TIM_MasterConfigTypeDef sMasterConfig;
  
  /* Time Base configuration */
  TimHandle.Instance = TIMx;
  
  /* Configure timer frequency */
  /* Note: Setting of timer prescaler to 489 to increase the maximum range    */
  /*       of the timer, to fit within timer range of 0xFFFF.                 */
  /*       Setting of reload period to SysClk/489 to maintain a base          */
  /*       frequency of 1us.                                                  */
  /*       With SysClk set to 32MHz, timer frequency (defined by label        */
  /*       TIMER_FREQUENCY_HZ range) is min=1Hz, max=32.719kHz.               */
  /* Note: Timer clock source frequency is retrieved with function            */
  /*       HAL_RCC_GetPCLK1Freq().                                            */
  /*       Alternate possibility, depending on prescaler settings:            */
  /*       use variable "SystemCoreClock" holding HCLK frequency, updated by  */
  /*       function HAL_RCC_ClockConfig().                                    */
  TimHandle.Init.Period = ((HAL_RCC_GetPCLK1Freq() / (489 * TIMER_FREQUENCY_HZ)) - 1);
  TimHandle.Init.Prescaler = (489-1);
  TimHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    /* Timer initialization Error */
    Error_Handler();
  }

  /* Timer TRGO selection */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

  if (HAL_TIMEx_MasterConfigSynchronization(&TimHandle, &sMasterConfig) != HAL_OK)
  {
    /* Timer TRGO selection Error */
    Error_Handler();
  }
  
}

/**
  * @brief  DAC configuration
  * @param  None
  * @retval None
  */
static void DAC_Config(void)
{
  static DAC_ChannelConfTypeDef sConfig;

  /* Configuration of DACx peripheral */
  DacHandle.Instance = DACx;

  if (HAL_DAC_Init(&DacHandle) != HAL_OK)
  {
    /* DAC initialization error */
    Error_Handler();
  }

  /* Configuration of DACx channel 1 */
  sConfig.DAC_Trigger = DAC_TRIGGER_Tx_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

  if (HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DACx_CHANNEL_TO_OPAMP1_NONINV_INPUT) != HAL_OK)
  {
    /* Channel configuration error */
    Error_Handler();
  }
}

/**
  * @brief  OPAMP configuration
  * @param  None
  * @retval None
  */
void OPAMP_Config(void)
{
  OpampHandle.Instance = OPAMP2; 

  /* Select PGAMode */
  OpampHandle.Init.Mode = OPAMP_FOLLOWER_MODE;  

  /* Select internal connection to DAC channel as input for OPAMP */
  OpampHandle.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_DAC_CH1;

  /* Select the inverting input of the OPAMP: no inverting input connection since OPAMP is on follower mode */
  /* OpampHandle.Init.InvertingInput = OPAMP_INVERTINGINPUT_IO0; */
  
  /* Select power mode */
  OpampHandle.Init.PowerMode = OPAMP_POWERMODE_NORMAL;
  
  /* Select power supply range */
  OpampHandle.Init.PowerSupplyRange = OPAMP_POWERSUPPLY_HIGH;

  /* Select the user trimming */
  OpampHandle.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  
  /* Init */
  HAL_OPAMP_Init(&OpampHandle);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == USER_BUTTON_PIN)
  {
    /* Set variable to report push button event to main program */
    if (ubUserButtonClickEventToggle == RESET)
    {
      ubUserButtonClickEventToggle = SET;
    }
    else
    {
      ubUserButtonClickEventToggle = RESET;
    }
    
  }
}

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
  uint16_t tmp_index = 0;
  uint16_t tmp_index_waveform_start = 0;
  uint16_t tmp_waveform[WAVEFORM_RAMP_12BITS_5SAMPLES];
  
  /* Perform a basic signal processing:                                       */
  /* - Extract temporary waveform from ADC acquired waveform                  */
  /* - Compare temporary waveform with reference waveform                     */
  
  /* Search index of waveform start (lowest value) */
  for (tmp_index=0; tmp_index<ADCCONVERTEDVALUES_BUFFER_SIZE; tmp_index++)
  {
    /* Search measured signal first value: ramp waveform start after the      */
    /* previous period.                                                       */
    if ((aADCxConvertedValues[tmp_index -1] > Waveform_Ramp_12bits_5samples[WAVEFORM_RAMP_12BITS_5SAMPLES - 2]) &&
        (aADCxConvertedValues[tmp_index   ] < Waveform_Ramp_12bits_5samples[1])                                   )
    {
      tmp_index_waveform_start = tmp_index;
      break;
    }
  }
  
  if (tmp_index_waveform_start == 0)
  {
    /* No start point start could be isolated into the waveform */
    ubOPAMPOutputInputVoltageStatus = RESET;
  }
  else
  {
    /* Start point start could be isolated into the waveform: continue processing */
    
    /* Extract temporary waveform from ADC acquired waveform */
    for (tmp_index=0; tmp_index<WAVEFORM_RAMP_12BITS_5SAMPLES; tmp_index++)
    {
      tmp_waveform[tmp_index] = aADCxConvertedValues[tmp_index + tmp_index_waveform_start];
    }
    
    /* Set comparison status to signals similar, and reset it at the first    */ 
    /* difference observed.                                                   */
    ubOPAMPOutputInputVoltageStatus = SET;
    
    /* Compare temporary waveform with reference waveform */
    for (tmp_index=0; tmp_index<WAVEFORM_RAMP_12BITS_5SAMPLES; tmp_index++)
    {
      /* Compare temporary waveform to reference waveform taking in           */
      /* account measurement tolerance range high limit.                      */
      if (tmp_waveform[tmp_index] + ADC_MEASUREMENT_TOLERANCE_LSB > Waveform_Ramp_12bits_5samples[tmp_index])
      {
        if (tmp_waveform[tmp_index] >= ADC_MEASUREMENT_TOLERANCE_LSB)
        {
          /* Compare temporary waveform to reference waveform taking in       */
          /* account measurement tolerance range low limit.                   */
          if (!(tmp_waveform[tmp_index] - ADC_MEASUREMENT_TOLERANCE_LSB < Waveform_Ramp_12bits_5samples[tmp_index]))
          {
            /* Temporary waveform is different of reference waveform */
            ubOPAMPOutputInputVoltageStatus = RESET;
          }
        }
      }
      else
      {
        /* Temporary waveform is different of reference waveform */
        ubOPAMPOutputInputVoltageStatus = RESET;
      }
    }
  }

}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode 
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{

}
  
/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  /* In case of ADC error, call main error handler */
  Error_Handler();
}

/**
  * @brief  Error DAC callback for Channel1.
  * @param  hdac: pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* In case of DAC error, call main error handler */
  Error_Handler();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with a potential error */
  
  /* In case of error, LED2 is toggling at a frequency of 1Hz */
  while(1)
  {
    /* Toggle LED2 */
    BSP_LED_Toggle(LED2);
    HAL_Delay(500);
  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
