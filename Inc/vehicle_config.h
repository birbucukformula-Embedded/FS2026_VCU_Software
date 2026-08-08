#ifndef VEHICLE_CONFIG_H
#define VEHICLE_CONFIG_H

/*===========================================================================*
 * FS2026 ARAÇ AYAR KÜTÜPHANESİ (DEĞİŞKEN PARAMETRELERİ)
 *===========================================================================*
 *
 *  BU DOSYA ARAÇTAKI TÜM AYARLANABİLİR DEĞERLERİ İÇERİR.
 *
 *  Motor değişirse, sensör değişirse veya bir eşik değeri ayarlanacaksa
 *  SADECE bu dosyadaki ilgili satırı değiştirin.
 *  Kodun geri kalanına DOKUNMAYIN.
 *
 *===========================================================================*/


/* ═══════════════════════════════════════════════════════════════════════════
 *  MOTOR & İNVERTER PARAMETRELERİ
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_MOTOR_MAX_TORQUE_NM      230    // Motorun maksimum tork değeri (Nm)
                                            // Motor belli olunca güncelle!

#define CFG_MOTOR_MAX_RPM            6000   // Motorun maksimum devri (RPM)

#define CFG_MOTOR_DIRECTION          0      // 0 = İleri, 1 = Geri


/* ═══════════════════════════════════════════════════════════════════════════
 *  GAZ PEDALI (APPS) AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_APPS_DEADZONE_PERCENT    5      // %0-%5 arası gaz yok sayılır
                                            // (Ayak titremesi koruması)

#define CFG_APPS_PLAUSIBILITY_PERCENT 10    // İki APPS sensörü arasındaki
                                            // maksimum sapma (FS kuralı)

#define CFG_APPS_PLAUSIBILITY_TIME_MS 100   // Sapma kaç ms sürer ise hata verir


/* ═══════════════════════════════════════════════════════════════════════════
 *  FREN AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_BRAKE_RTD_THRESHOLD      15     // RTD için minimum fren basıncı
                                            // (Bu değerin üstünde fren basılı sayılır)

#define CFG_BRAKE_THROTTLE_BRAKE_MIN 30     // Fren-Gaz çakışması: Fren eşiği (Bar)

#define CFG_BRAKE_THROTTLE_GAS_MAX   25     // Fren-Gaz çakışması: Gaz eşiği (%)
                                            // Fren > 30 VE Gaz > %25 ise HATA

#define CFG_BRAKE_THROTTLE_CLEAR_GAS 5      // Hata temizleme: Gaz bu %'nin altına
                                            // düşünce hata kalkar


/* ═══════════════════════════════════════════════════════════════════════════
 *  REJENERATİF FRENLEME (REGEN) AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_REGEN_ENABLED            1      // 1 = Regen aktif, 0 = Regen kapalı

#define CFG_REGEN_MAX_PERCENT        20     // Maksimum regen frenleme gücü (%)
                                            // Motorun max torkunun %'si kadar


/* ═══════════════════════════════════════════════════════════════════════════
 *  BUZZER & RTD AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_RTD_BUZZER_DURATION_MS   2000   // Buzzer kaç ms çalacak (FS: 1000-3000)


/* ═══════════════════════════════════════════════════════════════════════════
 *  SICAKLIK & PİL GÜVENLİK LİMİTLERİ
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_MOTOR_TEMP_WARN          80     // Motor sıcaklığı uyarı (°C)
#define CFG_MOTOR_TEMP_LIMIT         100    // Motor sıcaklığı limit (°C)
                                            // Bu değerde tork %50'ye düşer

#define CFG_INVERTER_TEMP_WARN       60     // İnverter sıcaklığı uyarı (°C)
#define CFG_INVERTER_TEMP_LIMIT      80     // İnverter sıcaklığı limit (°C)

#define CFG_BATTERY_TEMP_LIMIT       60     // Hücre sıcaklık limiti (°C)
                                            // Bu değerde tork %50'ye düşer

#define CFG_BATTERY_SOC_LOW          20     // SOC %20'nin altında tork kısılır
#define CFG_BATTERY_SOC_CRITICAL     10     // SOC %10'un altında tork %25'e düşer


/* ═══════════════════════════════════════════════════════════════════════════
 *  HIZ LİMİTİ
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_SPEED_LIMIT_KMH          80     // Aracın maksimum hızı (km/h)
                                            // FS kuralı: Autocross/Endurance hız limiti

#define CFG_WHEEL_DIAMETER_MM        510    // Tekerlek dış çapı (mm)
                                            // RPM → km/h dönüşümünde kullanılır

#define CFG_GEAR_RATIO               3.5    // Dişli oranı (Motor RPM / Tekerlek RPM)
                                            // Dişli kutusu değişirse güncelle!


/* ═══════════════════════════════════════════════════════════════════════════
 *  CAN BUS AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_CAN_BAUDRATE             500000 // CAN Bus hızı (bps)

#define CFG_CAN_VCU_CONTROL_PERIOD   10     // VCU_Control mesaj periyodu (ms)
#define CFG_CAN_VCU_STATUS_PERIOD    20     // VCU_Status mesaj periyodu (ms)

#define CFG_TELEM_TX_PERIOD_MS       100    // Telemetri paketi gönderim periyodu (ms)
                                            // 100ms = Saniyede 10 paket (Yer istasyonunda akıcı grafik için yeterli)


/* ═══════════════════════════════════════════════════════════════════════════
 *  GÜVENLİK DEVRESİ (SDC) AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_TS_MIN_VOLTAGE              60    // TS_ACTIVE olmak için gereken minimum Inverter voltajı
#define CFG_PRECHARGE_TIMEOUT_MS        2000  // Precharge işleminin maksimum süresi (ms). Geçerse HATA.
#define CFG_PRECHARGE_SUCCESS_PERCENT   90    // İnverterin bataryaya göre şarj olma yüzdesi (FS EV 4.11 kuralı)


/* ═══════════════════════════════════════════════════════════════════════════
 *  SENSÖR FİLTRELERİ & SD KART DATALOGGER AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_MA_FILTER_WINDOW_SIZE       10    // Hareketli ortalama filtresi için son kaç örneğin (pencere) ortalaması alınacak
#define CFG_SD_SECTOR_SIZE              512   // SD kart fiziksel sektör yazma boyutu (Ömrü korumak için 512 baytta bir yazılır)

#endif // VEHICLE_CONFIG_H
