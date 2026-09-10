/**
 * @file test_led_blink.c
 * @brief CubeIDE ve ST-Link İletişim Doğrulama Testi (Hello World)
 * 
 * Bu kod, STM32 NUCLEO-F446ZE kartı üzerindeki dahili LED'leri sırayla yakıp söndürür.
 * Amacı: Bilgisayarınızın ST-Link sürücülerinin doğru kurulduğunu ve 
 * CubeIDE'nin koda derleyip karta atabildiğini (Flash) kanıtlamaktır.
 */

#include "main.h"

/**
 * @brief Mavi, Kırmızı ve Yeşil LED'leri sırayla yakıp söndürür.
 * Bu fonksiyonu main.c içindeki while(1) döngüsüne ekleyin.
 */
void VCU_Test_LED_Blink(void)
{
    // Yeşil LED'i (LD1) yak, diğerlerini söndür
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);   // Yeşil YANAR
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // Mavi SÖNER
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);// Kırmızı SÖNER
    HAL_Delay(500);

    // Mavi LED'i (LD2) yak, diğerlerini söndür
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // Yeşil SÖNER
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);   // Mavi YANAR
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);// Kırmızı SÖNER
    HAL_Delay(500);

    // Kırmızı LED'i (LD3) yak, diğerlerini söndür
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // Yeşil SÖNER
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); // Mavi SÖNER
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);  // Kırmızı YANAR
    HAL_Delay(500);
}
