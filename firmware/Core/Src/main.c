/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : STM32 NUCLEO-F401RE GNSS-UART-MCP2515-CAN-Telemetry
  *
  * This firmware verifies external sensor interfacing using a mechanical
  * rotary encoder. Encoder channel states are read through GPIO inputs with
  * internal pull-up resistors and streamed over USART2 for debugging.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MCP2515_CS_PORT     GPIOB
#define MCP2515_CS_PIN      GPIO_PIN_6

#define MCP2515_CMD_RESET       0xC0
#define MCP2515_CMD_READ        0x03
#define MCP2515_REG_CANSTAT     0x0E
#define MCP2515_REG_CANCTRL  	0x0F

#define MCP2515_CMD_WRITE       0x02
#define MCP2515_CMD_RTS_TXB0    0x81

#define MCP2515_REG_CANCTRL     0x0F
#define MCP2515_REG_CANSTAT     0x0E

#define MCP2515_REG_CNF3        0x28
#define MCP2515_REG_CNF2        0x29
#define MCP2515_REG_CNF1        0x2A

#define MCP2515_REG_TXB0CTRL    0x30
#define MCP2515_REG_TXB0SIDH    0x31
#define MCP2515_REG_TXB0SIDL    0x32
#define MCP2515_REG_TXB0DLC     0x35
#define MCP2515_REG_TXB0D0      0x36

#define MCP2515_REG_EFLG       0x2D
#define MCP2515_REG_TEC        0x1C
#define MCP2515_REG_REC        0x1D

#define BOOT_RX_BUFFER_SIZE  32U
#define FW_CHUNK_SIZE        8U
#define FW_MAX_CHUNKS        8U

#define TELEMETRY_BUFFER_SIZE    16U


/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    BOOT_STATE_IDLE = 0,
    BOOT_STATE_UPDATE_MODE
} BootState_t;

typedef struct
{
    uint32_t timestamp_ms;
    uint8_t spi_tx[4];
    uint8_t spi_rx[4];
    uint8_t spi_ok;
} TelemetryPacket_t;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;


uint8_t gnss_byte;

//static uint8_t gnss_byte;

static uint32_t gnss_bytes_received = 0;

static uint8_t gnss_valid_stream = 0;

/* USER CODE BEGIN PV */
static BootState_t boot_state = BOOT_STATE_IDLE;
static char boot_rx_buffer[BOOT_RX_BUFFER_SIZE];

static uint8_t firmware_buffer[FW_MAX_CHUNKS][FW_CHUNK_SIZE];
static uint8_t firmware_chunk_count = 0U;

static TelemetryPacket_t telemetry_buffer[TELEMETRY_BUFFER_SIZE];
static uint8_t telemetry_write_index = 0U;
static uint8_t telemetry_count = 0U;

static uint32_t encoder_counter = 0U;
static GPIO_PinState encoder_prev_state = GPIO_PIN_SET;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void Encoder_Update(void);
static void Encoder_PrintRawState(void);
static void Bootloader_PrintBanner(void);
static void Bootloader_ProcessCommand(void);
static void Bootloader_ReceiveFirmwareChunk(void);

static void Telemetry_Push(const uint8_t *tx, const uint8_t *rx, uint8_t spi_ok);
static void SPI_Loopback_Test(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief Redirect printf() output to USART2.
  */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1U, HAL_MAX_DELAY);
    return ch;
}

/**
  * @brief Read and print raw rotary encoder channel states.
  *
  * Encoder wiring:
  * - Channel A: D10 / PB6
  * - Channel B: D9  / PC7
  * - Common:    GND
  */
//static void Encoder_PrintRawState(void)
//{
//    uint8_t encoder_a = HAL_GPIO_ReadPin(ENCODER_A_PORT, ENCODER_A_PIN);
//    uint8_t encoder_b = HAL_GPIO_ReadPin(ENCODER_B_PORT, ENCODER_B_PIN);
//
//    printf("A=%u B=%u\r\n", encoder_a, encoder_b);
//}

#define TELEMETRY_BUFFER_SIZE 16U


static void Telemetry_Push(const uint8_t *tx, const uint8_t *rx, uint8_t spi_ok)
{
    TelemetryPacket_t *packet = &telemetry_buffer[telemetry_write_index];

    packet->timestamp_ms = HAL_GetTick();

    for (uint8_t i = 0; i < 4U; i++)
    {
        packet->spi_tx[i] = tx[i];
        packet->spi_rx[i] = rx[i];
    }

    packet->spi_ok = spi_ok;

    telemetry_write_index++;

    if (telemetry_write_index >= TELEMETRY_BUFFER_SIZE)
    {
        telemetry_write_index = 0;
    }

    if (telemetry_count < TELEMETRY_BUFFER_SIZE)
    {
        telemetry_count++;
    }
}


static void SPI_Loopback_Test(void)
{
    uint8_t tx_data[4] = {0xA5, 0x01, 0x02, 0x03};
    uint8_t rx_data[4] = {0};

    HAL_SPI_TransmitReceive(&hspi1,
                            tx_data,
                            rx_data,
                            sizeof(tx_data),
                            HAL_MAX_DELAY);

    printf("SPI TX: ");
    for (uint8_t i = 0; i < sizeof(tx_data); i++)
    {
        printf("%02X ", tx_data[i]);
    }

    printf("\r\nSPI RX: ");
    for (uint8_t i = 0; i < sizeof(rx_data); i++)
    {
        printf("%02X ", rx_data[i]);
    }

    printf("\r\n\r\n");

    uint8_t spi_ok = 1U;

    for (uint8_t i = 0; i < sizeof(tx_data); i++)
    {
        if (tx_data[i] != rx_data[i])
        {
            spi_ok = 0U;
            break;
        }
    }

    Telemetry_Push(tx_data, rx_data, spi_ok);

    printf("Telemetry buffered: index=%u count=%u spi_ok=%u timestamp=%lu\r\n",
           telemetry_write_index,
           telemetry_count,
           spi_ok,
           HAL_GetTick());
}

static void Bootloader_ProcessCommand(void)
{
    printf("BOOT> Type command: ");

    memset(boot_rx_buffer, 0, sizeof(boot_rx_buffer));

    HAL_UART_Receive(&huart2,
                     (uint8_t *)boot_rx_buffer,
                     6U,
                     HAL_MAX_DELAY);

    printf("\r\nBOOT> Received: %s\r\n", boot_rx_buffer);

    if ((strncmp(boot_rx_buffer, "UPDATE", 6U) == 0) ||
        (strncmp(boot_rx_buffer, "update", 6U) == 0))
    {
        boot_state = BOOT_STATE_UPDATE_MODE;

        printf("BOOT> Update mode entered\r\n");
        printf("BOOT> Ready to receive firmware chunks\r\n");
    }
    else
    {
        printf("BOOT> Unknown command\r\n");
    }
}

static void Bootloader_ReceiveFirmwareChunk(void)
{
    uint8_t chunk[FW_CHUNK_SIZE] = {0};

    if (firmware_chunk_count >= FW_MAX_CHUNKS)
    {
        printf("BOOT> Firmware buffer full\r\n");
        printf("BOOT> Simulated update complete\r\n");
        return;
    }

    printf("BOOT> Send %u-byte firmware chunk\r\n", FW_CHUNK_SIZE);

    HAL_UART_Receive(&huart2,
                     chunk,
                     FW_CHUNK_SIZE,
                     HAL_MAX_DELAY);

    for (uint8_t i = 0; i < FW_CHUNK_SIZE; i++)
    {
        firmware_buffer[firmware_chunk_count][i] = chunk[i];
    }

    printf("BOOT> Chunk %u received: ", firmware_chunk_count);

    for (uint8_t i = 0; i < FW_CHUNK_SIZE; i++)
    {
        printf("%02X ", firmware_buffer[firmware_chunk_count][i]);
    }

    printf("\r\n");

    firmware_chunk_count++;

    printf("BOOT> Total chunks stored: %u/%u\r\n",
           firmware_chunk_count,
           FW_MAX_CHUNKS);
}

static void MCP2515_Select(void)
{
    HAL_GPIO_WritePin(MCP2515_CS_PORT,
                      MCP2515_CS_PIN,
                      GPIO_PIN_RESET);
}

static void MCP2515_Unselect(void)
{
    HAL_GPIO_WritePin(MCP2515_CS_PORT,
                      MCP2515_CS_PIN,
                      GPIO_PIN_SET);
}

static void MCP2515_Reset(void)
{
    uint8_t cmd = MCP2515_CMD_RESET;

    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MCP2515_Unselect();

    HAL_Delay(20);

    printf("MCP2515 RESET command sent\r\n");
}

static uint8_t MCP2515_ReadRegister(uint8_t address)
{
    uint8_t tx[3] = {MCP2515_CMD_READ, address, 0x00};
    uint8_t rx[3] = {0};

    MCP2515_Select();

    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, HAL_MAX_DELAY);

    MCP2515_Unselect();

    return rx[2];
}

static void MCP2515_WriteRegister(uint8_t address, uint8_t value)
{
    uint8_t tx[3] = {MCP2515_CMD_WRITE, address, value};

    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, tx, 3, HAL_MAX_DELAY);
    MCP2515_Unselect();
}

static void MCP2515_RequestToSendTXB0(void)
{
    uint8_t cmd = MCP2515_CMD_RTS_TXB0;

    MCP2515_Select();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MCP2515_Unselect();
}

static void MCP2515_InitCAN(void)
{
	printf("CAN init started\r\n");

    MCP2515_Reset();

    /* 125 kbps @ 8 MHz oscillator */
    MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x01);
    MCP2515_WriteRegister(MCP2515_REG_CNF2, 0xB1);
    MCP2515_WriteRegister(MCP2515_REG_CNF3, 0x05);

    /* Normal mode */
    MCP2515_WriteRegister(MCP2515_REG_CANCTRL, 0x00);

    HAL_Delay(10);

    uint8_t canstat = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    uint8_t canctrl = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);

    printf("CAN init done\r\n");
    printf("CANSTAT = 0x%02X\r\n", canstat);
    printf("CANCTRL = 0x%02X\r\n", canctrl);
}

static void MCP2515_SendTestFrame(void)
{
    //uint8_t data[4] = {0xA5, 0x01, 0x02, 0x03};
   // Encoder_Update();

   // uint8_t encoder_state = (uint8_t)HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7);

    //CAN Payload for GNSS
    uint8_t data[4] =
    {
        gnss_valid_stream,

        (uint8_t)(gnss_bytes_received & 0xFFU),

        (uint8_t)((gnss_bytes_received >> 8) & 0xFFU),

        0x55
    };

    //CAN Payload for rotary Encoder

//    uint8_t data[4] =
//    {
//        encoder_state,
//        (uint8_t)(encoder_counter & 0xFFU),
//        (uint8_t)((encoder_counter >> 8) & 0xFFU),
//        0xA5
//    };

    MCP2515_WriteRegister(MCP2515_REG_TXB0CTRL, 0x00);

    /* Standard ID 0x123 */
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDH, 0x24);
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDL, 0x60);

    MCP2515_WriteRegister(MCP2515_REG_TXB0DLC, 4);

    for (uint8_t i = 0; i < 4; i++)
    {
        MCP2515_WriteRegister(MCP2515_REG_TXB0D0 + i, data[i]);
    }

    MCP2515_RequestToSendTXB0();

    HAL_Delay(5);

    uint8_t txb0ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);

    uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
    uint8_t tec  = MCP2515_ReadRegister(MCP2515_REG_TEC);
    uint8_t rec  = MCP2515_ReadRegister(MCP2515_REG_REC);

    printf("TXB0CTRL=0x%02X EFLG=0x%02X TEC=%u REC=%u\r\n",
           txb0ctrl, eflg, tec, rec);

//    printf("TXB0CTRL = 0x%02X\r\n", txb0ctrl);
//    printf("EFLG=0x%02X TEC=%u REC=%u\r\n", eflg, tec, rec);
//
//    printf("CAN TX frame sent: ID=0x123 Data=A5 01 02 03\r\n");
//    printf("CAN TX encoder frame: state=%u count=%lu\r\n",
//           encoder_state,
//           encoder_counter);

    printf("CAN TX GNSS frame: stream=%u bytes=%lu\r\n",
           gnss_valid_stream,
           gnss_bytes_received);
}

static void Encoder_Update(void)
{
    GPIO_PinState current_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7);

    if ((encoder_prev_state == GPIO_PIN_SET) &&
        (current_state == GPIO_PIN_RESET))
    {
        encoder_counter++;
    }

    encoder_prev_state = current_state;
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

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
//  printf("\r\nBOOT> Ready\r\n");
//  printf("BOOT> Send UPDATE to enter firmware update mode\r\n");

  printf("\r\nMCP2515 CAN TX test\r\n");

  MCP2515_Unselect();
  HAL_Delay(100);

  MCP2515_InitCAN();

//  MCP2515_Unselect();
//  HAL_Delay(100);

//  printf("\r\nMCP2515 SPI test\r\n");
//
//  MCP2515_Reset();
//  printf("MCP2515 reset sent\r\n");
//
//  uint8_t canstat = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
//  uint8_t canctrl = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);
//
//  printf("CANSTAT = 0x%02X\r\n", canstat);
//  printf("CANCTRL = 0x%02X\r\n", canctrl);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (HAL_UART_Receive(&huart1,
                           &gnss_byte,
                           1,
                           10) == HAL_OK)
      {
          gnss_bytes_received++;

          HAL_UART_Transmit(&huart2,
                            &gnss_byte,
                            1,
                            HAL_MAX_DELAY);

          gnss_valid_stream = 1;
      }

      MCP2515_SendTestFrame();

      HAL_Delay(1000);
  }

//  while (1)
//  {
//    /* USER CODE END WHILE */
//	  if (HAL_UART_Receive(&huart1, &gnss_byte, 1, HAL_MAX_DELAY) == HAL_OK)
//	  {
//		  HAL_UART_Transmit(&huart2, &gnss_byte, 1, HAL_MAX_DELAY);
//	  }
//
//    /* USER CODE BEGIN 3 */
//  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

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

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart1.Init.BaudRate = 9600;
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
  * @brief USART2 Initialization Function
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
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
