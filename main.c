#include "stm32f4xx.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"

TIM_HandleTypeDef htim1;
I2C_HandleTypeDef hi2c1;
const double BMR = 1000.0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
void Error_Handler(void);

void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

void LCD_SendCommand(uint8_t command);
void LCD_SendData(uint8_t data);
void LCD_Clear(void);
void LCD_WriteString(char* str);
void LCD_Init(void);

typedef struct {
    const char* name;
    double caloriesPerGram;
} Food;

Food foodOptions[] = {
    {"Apple", 0.52},          // Calories per gram for an apple
    {"Sandwich", 2.5},        // Calories per gram for a sandwich
    {"Slice of Pizza", 3.0},  // Calories per gram for a slice of pizza
    {"Burger", 2.8},          // Calories per gram for a burger
    {"Salad", 0.3},           // Calories per gram for a salad
    {"Pasta", 1.5},           // Calories per gram for pasta
    {"Ice Cream", 2.0},       // Calories per gram for ice cream
    {"Sushi", 0.9},           // Calories per gram for sushi
    // Add more food options as needed
};

static uint8_t GAIN; // Gain for clock cycles.

void hx711_powerUp(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}

void hx711_setGain(uint8_t gain) {
    if (gain < 64)
        GAIN = 2; // 32, channel B
    else if (gain < 128)
        GAIN = 3; // 64, channel A
    else
        GAIN = 1; // 128, channel A
}

void hx711_init(void) {
    hx711_setGain(128);
    hx711_powerUp();
}

int32_t hx711_get_value(void) {
    uint32_t data = 0;
    uint8_t dout;
    int32_t filler;
    int32_t ret_value;

    for (uint8_t i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        delay_us(1);
        dout = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
        data = data << 1;
        if (dout) {
            data++;
        }
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        delay_us(1);
    }

    for (int i = 0; i < GAIN; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        delay_us(1);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        delay_us(1);
    }

    if (data & 0x800000) {
        filler = 0xFF000000;
    } else {
        filler = 0x00000000;
    }

    ret_value = filler + data;
    return ret_value;
}

uint8_t hx711_is_ready(void) {
    return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET;
}

void displayFoodOptions(void) {
    LCD_Clear();
    char* foodOptions[] = {
        "Please select a food: ",
        "Apple",
        "Sandwich",
        "Slice of Pizza",
        "Burger",
        "Salad",
        "Pasta",
        "Ice Cream",
        "Sushi"
        // Add more food options as needed
    };

    static int foodIndex = 0;

    LCD_WriteString(foodOptions[foodIndex]);
    HAL_Delay(500); // Debounce delay

    while (1) {
        if (hx711_is_ready()) {
            // If needed, you can add code here to handle the HX711 sensor being ready
        }

        // Check if button is pressed
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
            // Button is pressed
            LCD_Clear();
            foodIndex = (foodIndex + 1) % (sizeof(foodOptions) / sizeof(foodOptions[0]));
            LCD_WriteString(foodOptions[foodIndex]);
            HAL_Delay(500); // Debounce delay
        } else if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
            // Button is held down
            LCD_Clear();

            // Get weight from HX711 sensor
            int32_t hx711_value = hx711_get_value();
            double weightInGrams = (((double)hx711_value) / 1000) + 263;

            char weightStr[16];
            sprintf(weightStr, "Weight of food: %.2f g", weightInGrams);
            LCD_WriteString(weightStr);

            // Calculate calories in the food
            double caloriesInFood = weightInGrams * foodOptions[foodIndex].caloriesPerGram;

            // Calculate weight change needed
            double weightChangeNeeded = ((BMR / 3.0) - caloriesInFood) * (1.0 / foodOptions[foodIndex].caloriesPerGram);

            char changeStr[26];
            sprintf(changeStr, "Weight change needed: %.2f g", weightChangeNeeded);
            LCD_WriteString(changeStr);

            // Wait for button release
            while (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
                // You might want to add a delay here to prevent repeated detection
            }

            // Display food options again
            displayFoodOptions();
        }
    }
}

void LCD_Init(void) {
    LCD_SendCommand(0x02);
    LCD_SendCommand(0x28);
    LCD_SendCommand(0x0C);
    LCD_SendCommand(0x06);
}

void LCD_SendCommand(uint8_t command) {
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (command >> 4) & 1);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (command >> 5) & 1);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (command >> 6) & 1);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (command >> 7) & 1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, command & 1);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (command >> 1) & 1);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (command >> 2) & 1);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (command >> 7) & 1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void LCD_SendData(uint8_t data) {
    HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, (data >> 4) & 1);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (data >> 5) & 1);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (data >> 6) & 1);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (data >> 7) & 1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(D4_GPIO_Port, D4_Pin, data & 1);
    HAL_GPIO_WritePin(D5_GPIO_Port, D5_Pin, (data >> 1) & 1);
    HAL_GPIO_WritePin(D6_GPIO_Port, D6_Pin, (data >> 2) & 1);
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (data >> 3) & 1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = col + (row == 1 ? 0x40 : 0x00);
    LCD_SendCommand(0x80 | address);
}

void LCD_WriteString(char* str) {
    while (*str) {
        LCD_SendData((uint8_t)*str++);
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_USART1_UART_Init();
    MX_I2C1_Init();
    // Initialize LCD
    LCD_Init();

    // Initialize HX711
    hx711_init();

    // Set up EXTI for the button
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    displayFoodOptions();

    // Main loop
    while (1) {
        if (hx711_is_ready()) {
            // If needed, you can add code here to handle the HX711 sensor being ready
        }
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    	Error_Handler();
    }

    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }

    /** Configure the Systick interrupt time
    */
    __HAL_RCC_TIMCLKPRESCALER(RCC_SYSCLK_DIV1);

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /** Configure the Systick
    */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /** SysTick_IRQn interrupt configuration
    */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void Error_Handler(void) {
    while(1) {
    }
}
void MX_GPIO_Init(void) {
    // GPIO initialization code should be written here
}

void MX_TIM1_Init(void) {
    // TIM1 initialization code should be written here
}

void MX_USART1_UART_Init(void) {
    // USART1 UART initialization code should be written here
}
void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000; // Replace with your actual clock speed
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}