#include "stm32f4xx.h"
#include "stdio.h"

#define RS_Pin GPIO_PIN_12
#define RS_GPIO_Port GPIOB
#define EN_Pin GPIO_PIN_13
#define EN_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_14
#define D4_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_15
#define D5_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_10
#define D6_GPIO_Port GPIOB
#define D7_Pin GPIO_PIN_11
#define D7_GPIO_Port GPIOB

#define B1_Pin GPIO_PIN_0
#define B1_GPIO_Port GPIOA

#define LED1_Pin GPIO_PIN_5
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_6
#define LED2_GPIO_Port GPIOA

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);

void delay_us (uint16_t us) //delay function
{
__HAL_TIM_SET_COUNTER(&htim1,0);  // setting the delay counter to 0.
while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // while loop till the counter reaches the delay given (us).
}

void init_leds(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // Configure LED1 and LED2 pins as output
    GPIO_InitStruct.Pin = LED1_Pin | LED2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void toggle_leds(void) {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

void LCD_SendCommand(uint8_t command);
void LCD_SendData(uint8_t data);
void LCD_Clear(void);
void LCD_WriteString(char* str);
void LCD_Init(void);

static uint8_t GAIN; // Gain for clock cycles.

void delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

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

void EXTI0_IRQHandler(void) {
    // Handle button press and hold
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
        // Button is pressed
        LCD_Clear();
        static int foodIndex = 0;
        char* foodOptions[] = {
            "Please select a food",
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
        LCD_WriteString(foodOptions[foodIndex]);
        foodIndex = (foodIndex + 1) % (sizeof(foodOptions) / sizeof(foodOptions[0]));
        HAL_Delay(500); // Debounce delay
    } else if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        // Button is held down
        LCD_Clear();
        LCD_WriteString("Caloric Feedback");
        int32_t hx711_value = hx711_get_value();
        double weightInGrams = (((double)hx711_value) / 1000) + 263;
        // Convert to kilograms
        double weightInKg = weightInGrams / 1000.0;
        char weightStr[16];
        sprintf(weightStr, "Weight: %.2f Kg", weightInKg);
        LCD_WriteString(weightStr);
        // Perform your calculations or actions here
        HAL_Delay(1000); // Adjust delay as needed
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
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (command >> 3) & 1);
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
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (data >> 7) & 1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void LCD_Clear(void) {
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

void LCD_WriteString(char* str) {
    while (*str) {
        LCD_SendData(*str++);
        HAL_Delay(1);
    }
}

int main(void) {
    HAL_Init();

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = RS_Pin | EN_Pin | D4_Pin | D5_Pin | D6_Pin | D7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = B1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    LCD_Init();
    LCD_Clear();
    LCD_WriteString("Please select a food");
    init_leds();

    while (1) {
        toggle_leds();  // Toggle the LEDs
        HAL_Delay(3);    // Wait for 3 ms

        // Check if button 1 is pressed
        if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
            break;  // Exit the loop if the button is pressed
        }
    }

    while (1) {
        // Additional code in the main loop after button press
    }

}
