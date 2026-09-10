/**
 * @file test_can_rx.c
 * @brief CAN Bus Receiver Test Code (Node B)
 * 
 * Bu dosya 2. STM32 Nucleo (Alıcı) kartına yüklenecektir.
 * Sadece ID'si 0x123 olan mesajları kabul eder ve veri doğruysa Yeşil LED'i yakar.
 */

#include "main.h"

extern CAN_HandleTypeDef hcan1;

CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];

/**
 * @brief CAN donanımını başlatır ve 0x123 ID'si için filtre ayarlar.
 */
void VCU_Test_CAN_RX_Init(void)
{
    CAN_FilterTypeDef canfilterconfig;

    // Yalnızca 0x123 ID'sine sahip mesajları geçiren filtre
    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 10;
    canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canfilterconfig.FilterIdHigh = 0x123 << 5; // Standart ID 5 bit sola kaydırılır
    canfilterconfig.FilterIdLow = 0x0000;
    canfilterconfig.FilterMaskIdHigh = 0xFFFF; // Tam eşleşme istiyoruz
    canfilterconfig.FilterMaskIdLow = 0xFFFF;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);

    // CAN modülünü başlat
    HAL_CAN_Start(&hcan1);

    // Mesaj geldiğinde otomatik haber vermek için Interrupt (Kesme) aktivasyonu
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/**
 * @brief CAN donanımından mesaj geldiğinde otomatik tetiklenen fonksiyon (Interrupt Callback)
 * Bu fonksiyonu main.c dosyanızın en altına yapıştırabilirsiniz (Eğer orada yoksa).
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1)
    {
        // Mesajı kuyruktan al
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            // Eğer gelen ID 0x123 ise ve ilk data byte'ı 0xAA ise başarılı!
            if (RxHeader.StdId == 0x123 && RxData[0] == 0xAA)
            {
                // Alındığını doğrulamak için Yeşil LED'i yak-söndür
                HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0); // Nucleo Yeşil LED Pinine göre güncelleyin
            }
        }
    }
}
