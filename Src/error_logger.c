#include "error_logger.h"
#include <stdio.h> // Mock için (printf vs.)

// Donanımsal EEPROM/Flash olmadığı için RAM'de mockluyoruz
static ErrorLog_t eepromMock;

void ErrorLogger_Init(void) {
    // Gerçek sistemde burada I2C/SPI veya Flash okuma başlatılır.
    // Şimdilik RAM üzerinde temiz başlatıyoruz.
    eepromMock.lastFault = FAULT_NONE;
    eepromMock.uptimeAtFaultMs = 0;
    eepromMock.isUnread = false;
}

void ErrorLogger_SaveFault(FaultCode_t fault, uint32_t uptimeMs) {
    // Sadece gerçekten bir hata varsa kaydet (Gereksiz yazmayı önle - EEPROM Ömrü)
    if (fault != FAULT_NONE) {
        // TODO: HAL_I2C_Mem_Write(...) veya Flash Yazma işlemi buraya eklenecek
        eepromMock.lastFault = fault;
        eepromMock.uptimeAtFaultMs = uptimeMs;
        eepromMock.isUnread = true;
    }
}

ErrorLog_t ErrorLogger_ReadLastFault(void) {
    // TODO: HAL_I2C_Mem_Read(...) veya Flash Okuma işlemi buraya eklenecek
    return eepromMock;
}

void ErrorLogger_ClearFault(void) {
    // TODO: EEPROM'dan silme işlemi buraya eklenecek
    eepromMock.lastFault = FAULT_NONE;
    eepromMock.uptimeAtFaultMs = 0;
    eepromMock.isUnread = false;
}
