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

#define CFG_REGEN_BRAKE_CUTOFF_VAL   230    // Sert hidrolik frende Regen'i kapatma eşiği (0-255 arası, ~%90)


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
#define CFG_CAN_TIMEOUT_MS           500    // CAN mesajı gelmezse sistemin FAULT'a geçeceği süre (ms) (FS T 11.9.4)

#define CFG_TELEM_TX_PERIOD_MS       100    // Telemetri paketi gönderim periyodu (ms)
                                            // 100ms = Saniyede 10 paket (Yer istasyonunda akıcı grafik için yeterli)


/* ═══════════════════════════════════════════════════════════════════════════
 *  GÜVENLİK DEVRESİ (SDC) AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */

#define CFG_TS_MIN_VOLTAGE              60    // TS_ACTIVE olmak için gereken minimum Inverter voltajı
#define CFG_PRECHARGE_TIMEOUT_MS        2000  // Precharge işleminin maksimum süresi (ms). Geçerse HATA.
#define CFG_PRECHARGE_SUCCESS_PERCENT   95    // İnverterin bataryaya göre şarj olma yüzdesi (FS EV 5.7.1 kuralı)


/* ═══════════════════════════════════════════════════════════════════════════
 *  ADC GÜVENLİK SINIRLARI (AÇIK/KISA DEVRE KORUMASI)
 * ═══════════════════════════════════════════════════════════════════════════ */
#define CFG_ADC_MIN_VALID               100   // Kopuk kablo (Açık devre) sınırı (12-bit ADC)
#define CFG_ADC_MAX_VALID               4000  // Kısa devre sınırı (12-bit ADC)

/* ═══════════════════════════════════════════════════════════════════════════
 *  GÜÇ VE AKIM LİMİTLERİ (FS KURALLARI EV 2.2)
 * ═══════════════════════════════════════════════════════════════════════════ */
#define CFG_MAX_POWER_W                 80000 // EV 2.2.1: Maksimum anlık güç (80 kW)
#define CFG_POWER_DERATE_THRESHOLD_W    78000 // 78 kW'ı geçince torku kısmaya başla
#define CFG_POWER_DERATE_FACTOR         200   // Her kaç Watt fazlalık için 1 Nm kısılacak? (Örn: 200W)
#define CFG_MAX_CURRENT_A               500   // EV 2.2.2: Maksimum anlık akım (500 Amper)
#define CFG_CURRENT_DERATE_THRESHOLD_A  480   // 480 Amperi geçince torku kısmaya başla
#define CFG_CURRENT_DERATE_FACTOR       5     // Her 1A fazlalık için kaç Nm kısılacak? (Örn: 5 Nm)

/* ═══════════════════════════════════════════════════════════════════════════
 *  SENSÖR FİLTRELERİ & SD KART DATALOGGER AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */
#define CFG_MA_FILTER_WINDOW_SIZE       10    // Hareketli ortalama filtresi için son kaç örneğin (pencere) ortalaması alınacak
#define CFG_SD_SECTOR_SIZE              512   // SD kart fiziksel sektör yazma boyutu (Ömrü korumak için 512 baytta bir yazılır)

/* ═══════════════════════════════════════════════════════════════════════════
 *  ZAMANLAYICI (TIMER) LİMİTLERİ VE DASHBOARD AYARLARI
 * ═══════════════════════════════════════════════════════════════════════════ */
#define CFG_BOOT_TIMER_MAX_MS           10000 // Boot sayacının taşmasını engellemek için sınır
#define CFG_DASHBOARD_LED_TEST_MS       2000  // T 11.9.6: Açılışta gösterge LED'lerinin yanık kalma süresi

#endif // VEHICLE_CONFIG_H
