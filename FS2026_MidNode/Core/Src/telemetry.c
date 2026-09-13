#include "telemetry.h"
#include "state_machine.h"
#include "vehicle_config.h"
#include <string.h>  // memset() için

/*===========================================================================*
 * FS2026 TELEMETRİ TX MODÜLÜ
 *===========================================================================*
 *
 *  Bu dosya, aracın tüm kritik verilerini tek bir pakete sıkıştırır ve
 *  UART üzerinden STM32 Telemetri Node'una (veya direkt LoRa modülüne) 
 *  göndermeye hazır hale getirir.
 *
 *  can_manager.c'deki zamanlayıcı gibi, burada da periyodik olarak
 *  TELEM_PeriodicSend() çağrılarak paket hazırlanır ve gönderilir.
 *
 *===========================================================================*/

// Modül içi zamanlayıcı (can_manager.c ile aynı mantık)
static uint32_t telemTimerMs = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 *  TELEM_Init — Telemetri frame'ini ilk kez hazırlar
 * ═══════════════════════════════════════════════════════════════════════════ */
void TELEM_Init(TelemetryFrame_t *frame) {
    // Tüm alanları sıfırla
    memset(frame, 0, sizeof(TelemetryFrame_t));
    
    // Sabit başlık alanlarını ayarla (Bunlar hiç değişmez)
    frame->header1 = TELEM_HEADER_BYTE_1;  // 0xAA
    frame->header2 = TELEM_HEADER_BYTE_2;  // 0x55
    frame->length  = TELEM_DATA_LENGTH;    // sizeof(TelemetryPacket_t) = 21
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  TELEM_BuildPacket — State Machine'den verileri alıp pakete doldurur
 *
 *  Bu fonksiyona "void *sm" olarak StateMachine_t gönderin.
 *  Böylece telemetry.h dosyasının state_machine.h'yi include etmesine
 *  gerek kalmaz (Döngüsel bağımlılık önlenir).
 * ═══════════════════════════════════════════════════════════════════════════ */
void TELEM_BuildPacket(TelemetryFrame_t *frame, 
                       const void *smPtr, 
                       uint32_t uptimeMs) {
    // void pointer'ı gerçek türüne dönüştür
    const StateMachine_t *sm = (const StateMachine_t *)smPtr;
    
    // --- Araç Durumu ---
    frame->data.vehicleState = (uint8_t)sm->outputs.currentState;
    frame->data.faultCode    = (uint8_t)sm->outputs.activeFault;
    
    // --- Sürüş Verileri ---
    frame->data.appsPercent    = sm->inputs.appsPercent;
    frame->data.brakePressure  = sm->inputs.brakePressure;
    frame->data.torqueCommand  = sm->outputs.torqueCommand;
    frame->data.motorRPM       = 0;  // İnverterden CAN ile gelecek (şimdilik 0)
    
    // --- Batarya Verileri ---
    frame->data.batteryVoltage = sm->inputs.bmsVoltage;
    frame->data.batteryCurrent = 0;  // BMS'ten CAN ile gelecek (şimdilik 0)
    frame->data.batterySOC     = 0;  // BMS'ten CAN ile gelecek (şimdilik 0)
    
    // --- Sıcaklık Verileri ---
    frame->data.motorTemp    = 0;    // İnverterden CAN ile gelecek (şimdilik 0)
    frame->data.inverterTemp = 0;    // İnverterden CAN ile gelecek (şimdilik 0)
    frame->data.maxCellTemp  = 0;    // BMS'ten CAN ile gelecek (şimdilik 0)
    
    // --- Kontaktör ve Güvenlik (Bit Bazlı) ---
    // Her boolean değeri tek tek bit'lere yerleştiriyoruz.
    // Böylece 5 adet bool değişkeni sadece 1 byte'a sığdırıyoruz.
    uint8_t flags = 0;
    if (sm->outputs.contactorNegative)  flags |= TELEM_FLAG_AIR_NEG;
    if (sm->outputs.contactorPositive)  flags |= TELEM_FLAG_AIR_POS;
    if (sm->outputs.contactorPrecharge) flags |= TELEM_FLAG_PRECHARGE;
    if (sm->inputs.sdcClosed)           flags |= TELEM_FLAG_SDC_CLOSED;
    if (sm->outputs.inverterEnable)     flags |= TELEM_FLAG_INV_ENABLE;
    frame->data.systemFlags = flags;
    
    // --- Zaman Damgası ---
    frame->data.uptimeMs = uptimeMs;
    
    // Checksum'ı hesapla
    TELEM_CalculateChecksum(frame);
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  TELEM_CalculateChecksum — XOR Checksum hesaplar
 *
 *  Data bölümündeki tüm byte'lar XOR'lanarak tek bir kontrol byte'ı üretilir.
 *  Alıcı (Yer İstasyonu) aynı XOR'u yapıp sonuç eşleşmezse paketi atar.
 * ═══════════════════════════════════════════════════════════════════════════ */
void TELEM_CalculateChecksum(TelemetryFrame_t *frame) {
    uint8_t checksum = 0;
    const uint8_t *dataBytes = (const uint8_t *)&frame->data;
    
    for (uint16_t i = 0; i < TELEM_DATA_LENGTH; i++) {
        checksum ^= dataBytes[i];
    }
    
    frame->checksum = checksum;
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  TELEM_PeriodicSend — Periyodik gönderim zamanlayıcısı
 *
 *  can_manager.c'deki CAN_PeriodicSend() ile aynı mantıkta çalışır.
 *  Ana döngüden her deltaTimeMs'de bir çağrılır.
 *  İç sayacı CFG_TELEM_TX_PERIOD_MS'e ulaşınca paketi oluşturur ve gönderir.
 * ═══════════════════════════════════════════════════════════════════════════ */
void TELEM_PeriodicSend(TelemetryFrame_t *frame,
                        const void *sm,
                        uint32_t deltaTimeMs,
                        uint32_t uptimeMs) {
    telemTimerMs += deltaTimeMs;
    
    if (telemTimerMs >= CFG_TELEM_TX_PERIOD_MS) {
        telemTimerMs = 0;
        
        // 1. Paketi doldur (State Machine'den verileri çek)
        TELEM_BuildPacket(frame, sm, uptimeMs);
        
        // 2. Frame'i UART'a gönder
        // Bu fonksiyon donanım geldiğinde doldurulacak (HAL_UART_Transmit vb.)
        TELEM_Hardware_Transmit((const uint8_t *)frame, TELEM_FRAME_LENGTH);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 *  DONANIM SOYUTLAMA KATMANI (HAL PLACEHOLDER)
 *  STM32 gelince bu fonksiyonun içine gerçek UART gönderim kodu yazılacak.
 *  Örnek: HAL_UART_Transmit(&huart2, data, length, 100);
 * ═══════════════════════════════════════════════════════════════════════════ */
__attribute__((weak))
void TELEM_Hardware_Transmit(const uint8_t *data, uint16_t length) {
    // Şimdilik boş — Donanım gelince doldurulacak.
    // STM32'de: HAL_UART_Transmit(&huart_telemetry, (uint8_t*)data, length, 100);
    (void)data;
    (void)length;
}
