#include "stm32f4xx.h"

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

void LCD_SendCommand(uint8_t command);
void LCD_SendData(uint8_t data);
void LCD_Clear(void);
void LCD_WriteString(char* str);
void LCD_Init(void);

void EXTI0_IRQHandler(void) {
    // Handle button press
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
    }
}

void EXTI0_IRQHandler(void) {
    // Handle button hold
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
        // Button is held down
        LCD_Clear();
        LCD_WriteString("Caloric Feedback");
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
    HAL_GPIO_WritePin(D7_GPIO_Port, D7_Pin, (data >> & 1);
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

    while (1) {
        // Main loop
    }
}
