#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include "moving_average_filter.h"

/*===========================================================================*
 * FS2026 VCU STATE MACHINE DEFINITIONS
 *===========================================================================*/

// Aracın anlık bulunabileceği tüm durumlar (States)
typedef enum {
    STATE_INIT = 0,         // Donanım başlatma ve self-test
    STATE_LV_READY = 1,     // Düşük gerilim (12V) sistemleri hazır, iletişim tamam
    STATE_PRECHARGING = 2,  // Yüksek Gerilim kontaktörleri sırayla kapatılıyor
    STATE_TS_ACTIVE = 3,    // Yüksek Gerilim (TS > 60V) aktif
    STATE_RTD_TRANSITION = 4, // Ready-To-Drive Buzzer çalıyor (2 Saniye)
    STATE_DRIVING = 5,      // Sürüş modu (Tork gönderiliyor)
    STATE_FAULT = 6         // Hata durumu (Güç kesilir, kilitlenir)
} VehicleState_t;

// Hata Kodları (Şimdilik temel hatalar)
typedef enum {
    FAULT_NONE = 0,
    FAULT_APPS_PLAUSIBILITY = 1, // %10 sapma kuralı ihlali
    FAULT_BRAKE_THROTTLE = 2,    // Sert fren + %25 gaz kuralı ihlali
    FAULT_BMS = 3,               // Batarya hatası
    FAULT_SDC_OPEN = 4,          // Güvenlik devresi koptu (E-Stop vb.)
    FAULT_PRECHARGE_FAIL = 5     // Ön şarj işlemi zaman aşımına uğradı
} FaultCode_t;

// Sensörlerden Gelecek Sanal Veriler (Hardware Abstraction için)
// Koda her döngüde bu veriler beslenir, kod kararlarını bunlara göre alır.
typedef struct {
    uint8_t  appsPercent;        // Hesaplanmış son Gaz yüzdesi (0-100%)
    uint8_t  apps1Percent;       // Sensör 1 (Güvenlik için ham veri)
    uint8_t  apps2Percent;       // Sensör 2 (Güvenlik için ham veri)
    uint8_t  brakePressure;      // 0-255 (örn: Bar)
    uint16_t tsVoltage;          // Yüksek Gerilim (İnverter Voltajı)
    uint16_t bmsVoltage;         // Yüksek Gerilim (Batarya Voltajı)
    bool     startButtonPressed; // Start butonu anlık durumu
    bool     resetButtonPressed; // Hata sıfırlama butonu
    bool     sdcClosed;          // Güvenlik devresi kapalı (Güvenli) mi?
    FaultCode_t externalFault;   // Dışarıdan gelen donanım hataları
} VCU_Inputs_t;

// State Machine'in üreteceği çıktılar
typedef struct {
    VehicleState_t currentState; // Mevcut araç durumu
    bool           rtdBuzzerOn;  // Buzzer çalsın mı?
    bool           inverterEnable; // Motor Inverteri aktif mi?
    int16_t        torqueCommand;  // İstenilen Tork
    FaultCode_t    activeFault;    // Şu anki aktif hata
    bool           contactorNegative;  // AIR- (Eksi Kontaktör)
    bool           contactorPrecharge; // Precharge Kontaktörü
    bool           contactorPositive;  // AIR+ (Artı Kontaktör)
} VCU_Outputs_t;

// Ana Durum Makinesi Yapısı (Global Objeler İçin)
typedef struct {
    VehicleState_t currentState;
    VehicleState_t previousState;
    uint32_t       rtdTimerMs;       // Buzzer süresi takibi
    bool           isRtdTimerActive; // Buzzer sayacı çalışıyor mu?
    
    uint32_t       appsTimerMs;      // %10 sapma kuralı zamanlayıcısı
    bool           isAppsTimerActive;// %10 sapma sayacı çalışıyor mu?
    
    uint32_t       prechargeTimerMs; // Ön şarj zamanlayıcısı
    
    // Sensor Filters
    MovingAverageFilter_t       apps1Filter;
    MovingAverageFilter_t       apps2Filter;
    MovingAverageFilter_t       brakeFilter;
    
    VCU_Inputs_t   inputs;
    VCU_Outputs_t  outputs;
} StateMachine_t;

/*===========================================================================*
 * FONKSİYON PROTOTİPLERİ
 *===========================================================================*/
void SM_Init(StateMachine_t *sm);
void SM_Update(StateMachine_t *sm, uint32_t deltaTimeMs);

#endif // STATE_MACHINE_H
