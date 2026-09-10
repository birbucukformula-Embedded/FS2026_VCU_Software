/**
 * @file test_can_tx.c
 * @brief CAN Bus Transmitter Test Code (Node A)
 * 
 * Bu dosya 1. STM32 Nucleo (Verici) kartına yüklenecektir.
 * Saniyede 1 kez ID'si 0x123 olan 8 bytelık bir veri paketi fırlatır.
 */

#include "main.h"

extern CAN_HandleTypeDef hcan1; // CubeIDE'de aktif ettiğiniz CAN birimi

// Test Mesajı Değişkenleri
CAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
uint32_t TxMailbox;

/**
 * @brief CAN donanımını başlatır ve filtreleri ayarlar.
 */
void VCU_Test_CAN_TX_Init(void)
{
    // Verici de olsa, donanımın düzgün çalışması için filtre ayarlanması gerekir.
    CAN_FilterTypeDef canfilterconfig;
    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 10;
    canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canfilterconfig.FilterIdHigh = 0;
    canfilterconfig.FilterIdLow = 0x0000;
    canfilterconfig.FilterMaskIdHigh = 0;
    canfilterconfig.FilterMaskIdLow = 0x0000;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 14;

    HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);

    // Mesaj Başlığı (Header) Ayarları
    TxHeader.DLC = 8;                 // 8 Byte Veri Uzunluğu
    TxHeader.ExtId = 0;               // Genişletilmiş ID kullanmıyoruz
    TxHeader.IDE = CAN_ID_STD;        // Standart 11-bit ID (CAN 2.0A)
    TxHeader.RTR = CAN_RTR_DATA;      // Data Frame (Veri Paketi)
    TxHeader.StdId = 0x123;           // Göndereceğimiz Paketin ID'si
    TxHeader.TransmitGlobalTime = DISABLE;

    // CAN hattını başlat
    HAL_CAN_Start(&hcan1);
}

/**
 * @brief Saniyede 1 kez çağrılması gereken ana test fonksiyonu (main while içinde)
 */
void VCU_Test_CAN_TX_Process(void)
{
    // Boş posta kutusu varsa mesajı fırlat
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0)
    {
        if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) == HAL_OK)
        {
            // Mesaj başarıyla sıraya alındı, LED'i yak-söndür (Nucleo üzerindeki LD2/LD3)
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // Nucleo LED pininize göre düzeltin
        }
    }
}
