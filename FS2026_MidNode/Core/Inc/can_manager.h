#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "state_machine.h"
#include "FS2026_CAN_Dictionary.h"

/*===========================================================================*
 * FS2026 CAN MANAGER (SCHEDULER)
 * Bu modül, State Machine'den çıkan verileri belirli periyotlarla
 * CAN hattına fırlatmaktan sorumludur.
 *===========================================================================*/

typedef struct {
    uint32_t timerVcuControl; // 0x100 mesajı için zamanlayıcı
    uint32_t timerVcuStatus;  // 0x101 mesajı için zamanlayıcı
    // İleride eklenecek mesajların zamanlayıcıları buraya gelir
} CAN_Manager_t;

/* --- Fonksiyon Prototipleri --- */

// Zamanlayıcıları sıfırlar
void CAN_Manager_Init(CAN_Manager_t *canMgr);

// Ana döngüden çağrılır, süresi gelen mesajları yollar
void CAN_Manager_Update(CAN_Manager_t *canMgr, const StateMachine_t *sm, uint32_t deltaTimeMs);

/*===========================================================================*
 * DONANIM SOYUTLAMA (HAL) ŞABLONU
 * Gerçekte bu fonksiyon, STM32'nin HAL_CAN_AddTxMessage fonksiyonunu çağıracaktır.
 * Biz şimdilik sadece boş bir prototip (şablon) bırakıyoruz.
 *===========================================================================*/
void CAN_Hardware_Transmit(uint32_t id, uint8_t *data, uint8_t length);

#endif // CAN_MANAGER_H
