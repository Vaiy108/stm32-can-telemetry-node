/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : STM32 NUCLEO-F401RE UART bootloader workflow demo
  *
  * This firmware demonstrates a lightweight bootloader-style update workflow:
  * - UART command reception
  * - UPDATE command detection
  * - bootloader state transition
  * - fixed-size firmware chunk reception
  * - static RAM buffering of received chunks
  *
  * Note:
  * This is an educational bootloader workflow demo. It does not erase or program
  * internal Flash memory.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define BOOT_RX_BUFFER_SIZE      32U
#define FW_CHUNK_SIZE            8U
#define FW_MAX_CHUNKS            8U

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

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart2;

static BootState_t boot_state = BOOT_STATE_IDLE;
static char boot_rx_buffer[BOOT_RX_BUFFER_SIZE];

static uint8_t firmware_buffer[FW_MAX_CHUNKS][FW_CHUNK_SIZE];
static uint8_t firmware_chunk_count = 0U;

static TelemetryPacket_t telemetry_buffer[TELEMETRY_BUFFER_SIZE];
static uint8_t telemetry_write_index = 0U;
static uint8_t telemetry_count = 0U;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
static void Bootloader_PrintBanner(void);
static void Bootloader_ProcessCommand(void);
static void Bootloader_ReceiveFirmwareChunk(void);

static void Telemetry_Push(const uint8_t *tx, const uint8_t *rx, uint8_t spi_ok);
static void SPI_Loopback_Test(void);
/* USER CODE END PFP */

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
  * @brief Print bootloader startup information.
  */
static void Bootloader_PrintBanner(void)
{
    printf("\r\nBOOT> Ready\r\n");
    printf("BOOT> Send UPDATE to enter firmware update mode\r\n");
}

/**
  * @brief Process bootloader command received over UART.
  */
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

/**
  * @brief Receive one simulated firmware chunk and store it in static RAM.
  */
static void Bootloader_ReceiveFirmwareChunk(void)
{
    uint8_t chunk[FW_CHUNK_SIZE] = {0};

    if (firmware_chunk_count >= FW_MAX_CHUNKS)
    {
        printf("BOOT> Firmware buffer full\r\n");
        printf("BOOT> Simulated update complete\r\n");

        while (1)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            HAL_Delay(500U);
        }
    }

    printf("BOOT> Send %u-byte firmware chunk\r\n", FW_CHUNK_SIZE);

    HAL_UART_Receive(&huart2,
                     chunk,
                     FW_CHUNK_SIZE,
                     HAL_MAX_DELAY);

    for (uint8_t i = 0U; i < FW_CHUNK_SIZE; i++)
    {
        firmware_buffer[firmware_chunk_count][i] = chunk[i];
    }

    printf("BOOT> Chunk %u received: ", firmware_chunk_count);

    for (uint8_t i = 0U; i < FW_CHUNK_SIZE; i++)
    {
        printf("%02X ", firmware_buffer[firmware_chunk_count][i]);
    }

    printf("\r\n");

    firmware_chunk_count++;

    printf("BOOT> Total chunks stored: %u/%u\r\n",
           firmware_chunk_count,
           FW_MAX_CHUNKS);
}

/**
  * @brief Store SPI telemetry packet into fixed-size circular buffer.
  *
  * This function is retained from Step 6 for future telemetry reuse.
  */
static void Telemetry_Push(const uint8_t *tx, const uint8_t *rx, uint8_t spi_ok)
{
    TelemetryPacket_t *packet = &telemetry_buffer[telemetry_write_index];

    packet->timestamp_ms = HAL_GetTick();

    for (uint8_t i = 0U; i < 4U; i++)
    {
        packet->spi_tx[i] = tx[i];
        packet->spi_rx[i] = rx[i];
    }

    packet->spi_ok = spi_ok;

    telemetry_write_index++;

    if (telemetry_write_index >= TELEMETRY_BUFFER_SIZE)
    {
        telemetry_write_index = 0U;
    }

    if (telemetry_count < TELEMETRY_BUFFER_SIZE)
    {
        telemetry_count++;
    }
}

/**
  * @brief SPI loopback test retained from Step 3.
  *
  * Not called during the bootloader demo.
  */
static void SPI_Loopback_Test(void)
{
    uint8_t tx_data[4] = {0xA5, 0x01, 0x02, 0x03};
    uint8_t rx_data[4] = {0};
    uint8_t spi_ok = 1U;

    HAL_SPI_TransmitReceive(&hspi1,
                            tx_data,
                            rx_data,
                            sizeof(tx_data),
                            HAL_MAX_DELAY);

    for (uint8_t i = 0U; i < sizeof(tx_data); i++)
    {
        if (tx_data[i] != rx_data[i])
        {
            spi_ok = 0U;
            break;
        }
    }

    Telemetry_Push(tx_data, rx_data, spi_ok);

    printf("SPI TX: ");
    for (uint8_t i = 0U; i < sizeof(tx_data); i++)
    {
        printf("%02X ", tx_data[i]);
    }

    printf("\r\nSPI RX: ");
    for (uint8_t i = 0U; i < sizeof(rx_data); i++)
    {
        printf("%02X ", rx_data[i]);
    }

    printf("\r\nTelemetry buffered: index=%u count=%u spi_ok=%u timestamp=%lu\r\n\r\n",
           telemetry_write_index,
           telemetry_count,
           spi_ok,
           HAL_GetTick());
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_SPI1_Init();

    Bootloader_PrintBanner();

    while (1)
    {
        if (boot_state == BOOT_STATE_IDLE)
        {
            Bootloader_ProcessCommand();
        }
        else if (boot_state == BOOT_STATE_UPDATE_MODE)
        {
            Bootloader_ReceiveFirmwareChunk();
        }
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

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;

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
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
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
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    /* Onboard LD2 LED on PA5 */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Encoder channel B from Step 2: PC7 / D9 */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Encoder channel A from Step 2: PB6 / D10 */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(100U);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif /* USE_FULL_ASSERT */