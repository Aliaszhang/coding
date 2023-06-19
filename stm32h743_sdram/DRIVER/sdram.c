#include "sdram.h"

SDRAM_HandleTypeDef      hsdram;
FMC_SDRAM_TimingTypeDef  SDRAM_Timing;
FMC_SDRAM_CommandTypeDef command;

static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);

void MX_FMC_SDRAM_Init(void)
{
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

    /* 配置SDRAM时钟源*/
    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FMC;
    RCC_PeriphClkInit.PLL2.PLL2M = 5;
    RCC_PeriphClkInit.PLL2.PLL2N = 144;
    RCC_PeriphClkInit.PLL2.PLL2P = 2;
    RCC_PeriphClkInit.PLL2.PLL2Q = 2;
    RCC_PeriphClkInit.PLL2.PLL2R = 3;
    RCC_PeriphClkInit.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    RCC_PeriphClkInit.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    RCC_PeriphClkInit.PLL2.PLL2FRACN = 0;
    RCC_PeriphClkInit.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit) != HAL_OK) {
        while (1);
    }
    /* 使能 FMC 时钟 */
    __FMC_CLK_ENABLE();
    /*##-1- Configure the SDRAM device #########################################*/
    /* SDRAM device configuration */
    hsdram.Instance = FMC_SDRAM_DEVICE;

    /* Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz) */
    SDRAM_Timing.LoadToActiveDelay    = 2;
    SDRAM_Timing.ExitSelfRefreshDelay = 8;
    SDRAM_Timing.SelfRefreshTime      = 6;
    SDRAM_Timing.RowCycleDelay        = 6;
    SDRAM_Timing.WriteRecoveryTime    = 2;
    SDRAM_Timing.RPDelay              = 2;
    SDRAM_Timing.RCDDelay             = 2;

    hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
    hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
    hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
    hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
    hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

    /* Initialize the SDRAM controller */
    HAL_SDRAM_Init(&hsdram, &SDRAM_Timing);

    /* Program the SDRAM external device */
    SDRAM_Initialization_Sequence(&hsdram, &command);

}

/**
  * @brief SDRAM MSP Initialization
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param hsram: SDRAM handle pointer
  * @retval None
  */
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
    GPIO_InitTypeDef  GPIO_Init_Structure;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    /* Enable GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* Enable FMC clock */
    __HAL_RCC_FMC_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* Common GPIO configuration */
    GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull      = GPIO_PULLUP;
    GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Init_Structure.Alternate = GPIO_AF12_FMC;

    /* GPIOC configuration */
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;

    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);
    
    /* GPIOD configuration */
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                                GPIO_PIN_14 | GPIO_PIN_15;


    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    /* GPIOE configuration */  
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                                GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                GPIO_PIN_15;
        
    HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

    /* GPIOF configuration */  
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                                GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                                GPIO_PIN_15;

    HAL_GPIO_Init(GPIOF, &GPIO_Init_Structure);

    /* GPIOG configuration */  
    GPIO_Init_Structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |\
                                GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);
}

/**
  * @brief SDRAM MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to their default state
  * @param hsram: SDRAM handle pointer
  * @retval None
  */
void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram)
{
    /*## Disable peripherals and GPIO Clocks ###################################*/
      __HAL_RCC_FMC_CLK_DISABLE();
    /* Configure FMC as alternate function  */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |\
                            GPIO_PIN_14 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                            GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                            GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                            GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                            GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 |\
                            GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15);
}

/**
  * @brief  Perform the SDRAM exernal memory inialization sequence
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
    __IO uint32_t tmpmrd =0;
    /* Step 1:  Configure a clock configuration enable command */
    Command->CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    HAL_Delay(1);

    /* Step 3: Configure a PALL (precharge all) command */
    Command->CommandMode = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 4 : Configure a Auto-Refresh command */
    Command->CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 8;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 5: Program the external memory mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |
                        SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                        SDRAM_MODEREG_CAS_LATENCY_2           |
                        SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                        SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command->AutoRefreshNumber = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);

    /* Step 6: Set the refresh rate counter */
    /* Set the device refresh rate */
    HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT); 

}
/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the buffer to fill
  * @param  uwBufferLenght: size of the buffer to fill
  * @param  uwOffset: first value to fill on the buffer
  * @retval None
  */
void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset)
{
    uint32_t tmpIndex = 0;

    /* Put in global buffer different values */
    for (tmpIndex = 0; tmpIndex < uwBufferLenght; tmpIndex++ )
    {
        pBuffer[tmpIndex] = tmpIndex + uwOffset;
    }
}


