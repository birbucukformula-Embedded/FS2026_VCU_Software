#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include "state_machine.h" // FaultCode_t tanımı için

// EEPROM veya Flash'a yazılacak hata kaydı yapısı
typedef struct {
    FaultCode_t lastFault;
    uint32_t    uptimeAtFaultMs;
    bool        isUnread;
} ErrorLog_t;

// Hata Kaydedici Fonksiyonları
void ErrorLogger_Init(void);
void ErrorLogger_SaveFault(FaultCode_t fault, uint32_t uptimeMs);
ErrorLog_t ErrorLogger_ReadLastFault(void);
void ErrorLogger_ClearFault(void);

#endif // ERROR_LOGGER_H
