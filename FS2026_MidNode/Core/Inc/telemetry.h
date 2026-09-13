#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>

/*===========================================================================*
 * FS2026 TELEMETRI TX VERİ PAKETİ
 *===========================================================================*
 *
 *  Bu modül, araçtaki tüm önemli verileri tek bir pakete sıkıştırır.
 *  Paket STM32 Telemetri Node'una CAN Bus üzerinden gönderilir.
 *  STM32 bu paketi LoRa telsiz modülüne aktararak pit alanına iletir.
 *
 *  Paket Formatı:
 *  ┌────────┬────────┬──────────────────────┬──────────┐
 *  │ HEADER │ LENGTH │        DATA          │ CHECKSUM │
 *  │ 0xAA55 │ 1 Byte │     N Byte           │ 1 Byte   │
 *  └────────┴────────┴──────────────────────┴──────────┘
 *
 *  HEADER  : Sabit 2 byte başlık (0xAA, 0x55). Alıcı taraf bu başlığı
 *            arayarak paketin nerede başladığını bulur.
 *  LENGTH  : Data bölümünün byte uzunluğu.
 *  DATA    : Telemetri verileri (TelemetryPacket_t struct'ı).
 *  CHECKSUM: Tüm DATA byte'larının XOR'lanmasıyla elde edilen hata kontrolü.
 *
 *===========================================================================*/

/* ═══════════════════════════════════════════════════════════════════════════
 *  TELEMETRİ VERİ PAKETİ (Data Bölümü)
 *  Bu struct, araçtan pit alanına gönderilen tüm verileri içerir.
 *  Toplam: 20 Byte (CAN ile 3 frame'de veya UART ile tek seferde)
 * ═══════════════════════════════════════════════════════════════════════════ */
typedef struct __attribute__((packed)) {
    // --- Araç Durumu (2 Byte) ---
    uint8_t  vehicleState;      // Byte 0:  Araç durumu (STATE_INIT ... STATE_FAULT)
    uint8_t  faultCode;         // Byte 1:  Aktif hata kodu (FAULT_NONE ... FAULT_PRECHARGE_FAIL)
    
    // --- Sürüş Verileri (6 Byte) ---
    uint8_t  appsPercent;       // Byte 2:  Gaz pedalı yüzdesi (0-100%)
    uint8_t  brakePressure;     // Byte 3:  Fren basıncı (0-255 Bar)
    int16_t  torqueCommand;     // Byte 4-5: Tork komutu (-32000 ... +32000)
    int16_t  motorRPM;          // Byte 6-7: Motor devri (RPM)
    
    // --- Batarya Verileri (5 Byte) ---
    uint16_t batteryVoltage;    // Byte 8-9: Toplam batarya voltajı (V * 10, Örn: 4000 = 400.0V)
    int16_t  batteryCurrent;    // Byte 10-11: Batarya akımı (A, negatif = şarj)
    uint8_t  batterySOC;        // Byte 12: Şarj durumu (0-100%)
    
    // --- Sıcaklık Verileri (3 Byte) ---
    uint8_t  motorTemp;         // Byte 13: Motor sıcaklığı (°C)
    uint8_t  inverterTemp;      // Byte 14: İnverter sıcaklığı (°C)
    uint8_t  maxCellTemp;       // Byte 15: En sıcak batarya hücresi (°C)
    
    // --- Kontaktör ve Güvenlik (1 Byte - Bit field) ---
    // Bit 0: AIR- (Eksi Kontaktör)
    // Bit 1: AIR+ (Artı Kontaktör)
    // Bit 2: Precharge Kontaktör
    // Bit 3: SDC Durumu
    // Bit 4: İnverter Enable
    // Bit 5-7: Rezerve
    uint8_t  systemFlags;       // Byte 16: Bit bazlı durum bayrakları
    
    // --- Zaman Damgası (4 Byte) ---
    uint32_t uptimeMs;          // Byte 17-20: Aracın açılış süresinden itibaren geçen ms
} TelemetryPacket_t;

// System Flags Bit Maskeleri
#define TELEM_FLAG_AIR_NEG       (1 << 0)  // Bit 0: contactorNegative
#define TELEM_FLAG_AIR_POS       (1 << 1)  // Bit 1: contactorPositive
#define TELEM_FLAG_PRECHARGE     (1 << 2)  // Bit 2: contactorPrecharge
#define TELEM_FLAG_SDC_CLOSED    (1 << 3)  // Bit 3: sdcClosed
#define TELEM_FLAG_INV_ENABLE    (1 << 4)  // Bit 4: inverterEnable

/* ═══════════════════════════════════════════════════════════════════════════
 *  UART TELEMETRİ FRAME (Header + Length + Data + Checksum)
 *  Bu yapı, telemetri paketini UART üzerinden göndermeye hazır hale getirir.
 * ═══════════════════════════════════════════════════════════════════════════ */
#define TELEM_HEADER_BYTE_1    0xAA
#define TELEM_HEADER_BYTE_2    0x55
#define TELEM_DATA_LENGTH      sizeof(TelemetryPacket_t)
#define TELEM_FRAME_LENGTH     (2 + 1 + TELEM_DATA_LENGTH + 1)  // Header(2) + Len(1) + Data + Checksum(1)

typedef struct __attribute__((packed)) {
    uint8_t header1;                        // 0xAA
    uint8_t header2;                        // 0x55
    uint8_t length;                         // Data uzunluğu
    TelemetryPacket_t data;                 // Telemetri verisi
    uint8_t checksum;                       // XOR checksum
} TelemetryFrame_t;

/*===========================================================================*
 * FONKSİYON PROTOTİPLERİ
 *===========================================================================*/

// Telemetri modülünü başlatır (Frame'in header ve length alanlarını sabitler)
void TELEM_Init(TelemetryFrame_t *frame);

// State Machine çıktılarını alıp telemetri paketine doldurur
// sm: Ana durum makinesi (inputs + outputs okunur)
// uptimeMs: Sistem açılışından beri geçen toplam süre
void TELEM_BuildPacket(TelemetryFrame_t *frame, 
                       const void *sm, 
                       uint32_t uptimeMs);

// Checksum hesaplar ve frame'e yazar
void TELEM_CalculateChecksum(TelemetryFrame_t *frame);

// Donanım Soyutlama Katmanı: Bu fonksiyonun içi STM32 gelince doldurulacak
// UART veya SPI ile frame'i fiziksel olarak gönderir
void TELEM_Hardware_Transmit(const uint8_t *data, uint16_t length);

#endif // TELEMETRY_H
