#include "can_manager.h"
#include "vehicle_config.h"

void CAN_Manager_Init(CAN_Manager_t *canMgr) {
    canMgr->timerVcuControl = 0;
    canMgr->timerVcuStatus = 0;
}

// Özel Yardımcı: 0x100 VCU Control Paketini Gönder
static void Send_VCU_Control(const StateMachine_t *sm) {
    CAN_VCU_Control_t msg = {0}; // İçini 0 ile doldur
    
    msg.torqueRequest = sm->outputs.torqueCommand;
    msg.inverterEnable = sm->outputs.inverterEnable ? 1 : 0;
    msg.direction = CFG_MOTOR_DIRECTION; // Config'den ileri/geri ayarı
    
    // Struct'ı (C nesnesi) byte dizisine (0 ve 1'lere) çevir ve Donanıma ver
    CAN_Hardware_Transmit(CAN_ID_VCU_CONTROL, (uint8_t*)&msg, sizeof(CAN_VCU_Control_t));
}

// Özel Yardımcı: 0x101 VCU Status Paketini Gönder
static void Send_VCU_Status(const StateMachine_t *sm) {
    CAN_VCU_Status_t msg = {0};
    
    msg.stateMachine = (uint8_t)sm->currentState;
    msg.appsPercent = sm->filteredInputs.appsPercent;
    msg.brakePressure = sm->filteredInputs.brakePressure;
    msg.sdcStatus = sm->filteredInputs.sdcClosed ? 1 : 0;
    msg.activeFault = (uint8_t)sm->outputs.activeFault;
    // msg.steeringAngle şu anlık boş, sensör gelince eklenecek.
    
    CAN_Hardware_Transmit(CAN_ID_VCU_STATUS, (uint8_t*)&msg, sizeof(CAN_VCU_Status_t));
}


// ANA ÇAĞRI FONKSİYONU
void CAN_Manager_Update(CAN_Manager_t *canMgr, const StateMachine_t *sm, uint32_t deltaTimeMs) {
    // 1. Zamanlayıcıları artır
    canMgr->timerVcuControl += deltaTimeMs;
    canMgr->timerVcuStatus += deltaTimeMs;
    
    // 2. TORK KOMUTU ZAMANI GELDİ Mİ? (Örn: Her 10ms'de bir)
    if (canMgr->timerVcuControl >= CFG_CAN_VCU_CONTROL_PERIOD) {
        Send_VCU_Control(sm);
        canMgr->timerVcuControl = 0; // Sayacı sıfırla
    }
    
    // 3. DURUM BİLGİSİ ZAMANI GELDİ Mİ? (Örn: Her 20ms'de bir)
    if (canMgr->timerVcuStatus >= CFG_CAN_VCU_STATUS_PERIOD) {
        Send_VCU_Status(sm);
        canMgr->timerVcuStatus = 0; // Sayacı sıfırla
    }
}
