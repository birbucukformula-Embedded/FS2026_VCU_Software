# FS2026 VCU - İÇİNDEKİLER
# ══════════════════════════════════════════════════════════════
# Her kod değişikliğinde bu dosya güncellenecektir.
# Son Güncelleme: 18 Temmuz 2026
# ══════════════════════════════════════════════════════════════


# ┌─────────────────────────────────────────────────────────────
# │  Inc/FS2026_CAN_Dictionary.h  (CAN Bus Sözlüğü)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 13-21 .... CAN Mesaj ID Numaraları
# │     ├── 0x100  VCU_Control    (VCU → İnverter)         :14
# │     ├── 0x101  VCU_Status     (VCU → Ekran, Telemetri) :15
# │     ├── 0x102  VCU_Faults     (VCU → Tüm Ağ)          :16
# │     ├── 0x200  BMS_Volt_Curr  (BMS → VCU)              :17
# │     ├── 0x201  BMS_Status     (BMS → VCU, Ekran)       :18
# │     ├── 0x300  INV_Dynamics   (İnverter → VCU, Ekran)  :19
# │     ├── 0x301  INV_Faults     (İnverter → VCU)         :20
# │     └── 0x400  STM_IMU        (Telemetri → VCU)        :21
# │
# │   Satır 29-34 .... CAN_VCU_Control_t   → Tork komutu, İnverter aç/kapa
# │   Satır 37-44 .... CAN_VCU_Status_t    → Araç durumu, Gaz %, Fren, SDC
# │   Satır 47-51 .... CAN_BMS_VoltCurr_t  → HV Voltaj ve Akım
# │   Satır 54-60 .... CAN_BMS_Status_t    → SOC %, Sıcaklık, Hata kodu
# │   Satır 63-68 .... CAN_INV_Dynamics_t  → Motor RPM, Sıcaklıklar
# │   Satır 71-77 .... CAN_STM_IMU_t       → İvme X/Y, Yaw Rate, GPS
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/state_machine.h  (Durum Makinesi Tanımları)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 12-19 .... VehicleState_t (Araç Durumları)
# │     ├── 0 = STATE_INIT             :13
# │     ├── 1 = STATE_LV_READY         :14
# │     ├── 2 = STATE_TS_ACTIVE        :15
# │     ├── 3 = STATE_RTD_TRANSITION   :16
# │     ├── 4 = STATE_DRIVING          :17
# │     └── 5 = STATE_FAULT            :18
# │
# │   Satır 22-28 .... FaultCode_t (Hata Kodları)
# │     ├── 0 = FAULT_NONE                :23
# │     ├── 1 = FAULT_APPS_PLAUSIBILITY   :24
# │     ├── 2 = FAULT_BRAKE_THROTTLE      :25
# │     ├── 3 = FAULT_BMS                 :26
# │     └── 4 = FAULT_SDC_OPEN            :27
# │
# │   Satır 32-40 .... VCU_Inputs_t (Sensör Girişleri)
# │   Satır 43-49 .... VCU_Outputs_t (Araç Çıktıları)
# │   Satır 52-60 .... StateMachine_t (Ana Yapı)
# │   Satır 65-66 .... Fonksiyon Prototipleri (SM_Init, SM_Update)
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/can_manager.h & Src/can_manager.c  (CAN Mesaj Yöneticisi)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 16-19 .... CAN_Manager_t (Zamanlayıcılar) (Header)
# │   Satır 24-34 .... CAN_Manager_Init(), Update() ve Hardware Şablonu
# │
# │   Satır 10-21 .... Send_VCU_Control() — 10ms'de bir Tork ve Yön Gönder
# │   Satır 24-35 .... Send_VCU_Status()  — 20ms'de bir Araç Durumu Gönder
# │   Satır 39-53 .... CAN_Manager_Update() — ZAMANLAYICI ANA DÖNGÜSÜ
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/torque_control.h & Src/torque_control.c  (Tork Hesaplamaları)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 12-14 .... TC_CalculateTorque() Prototipleri (Header)
# │
# │   Satır 4-15  .... TC_ApplyTorqueMap()  — Ölü bölge (Deadzone) ve Gaz→Tork dönüşümü
# │   Satır 18-35 .... TC_ApplyRegen()      — Rejeneratif frenleme (Motoru jeneratör yap)
# │   Satır 38-55 .... TC_ApplySafetyLimits() — Pil sıcaksa veya şarj bitiyorsa torku kıs
# │
# │   Satır 59-74 .... TC_CalculateTorque() — ANA ÇAĞRI FONKSİYONU
# │     ├── :63      Gaza basılıyorsa → TC_ApplyTorqueMap()
# │     ├── :67      Gazdan çekildiyse → TC_ApplyRegen()
# │     └── :71      Son olarak → TC_ApplySafetyLimits() uygula ve İnverter'e gönder
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Src/state_machine.c  (Ana Beyin Algoritması)
# ├─────────────────────────────────────────────────────────────
# │
# │   Satır 4-33  .... CheckForErrors()  — Hata kontrol fonksiyonu
# │     ├── :6-8     Dış hata kontrolü (BMS vb.)
# │     ├── :10-13   SDC (Güvenlik Devresi) kopma kontrolü
# │     ├── :15-24   ★ FS KURALI EV 5.7: Fren-Gaz çakışması
# │     └── :27-44   ★ FS KURALI T 11.8.8: APPS %10 Sapma Kuralı
# │
# │   Satır 30-41 .... SM_Init()  — Sistemi güvenli başlatma
# │
# │   Satır 72-214 ... SM_Update()  — ANA DÖNGÜ (switch-case)
# │     │
# │     ├── :94-97   STATE_INIT → LV_READY geçişi
# │     │
# │     ├── :99-107  STATE_LV_READY
# │     │   └── SDC Kapalıysa → PRECHARGING geçişi        :103
# │     │
# │     ├── :109-131 STATE_PRECHARGING (Ön Şarj / Kontaktörler)
# │     │   ├── AIR- ve Precharge Rölesini Kapat            :112
# │     │   ├── İnverter V. > %90 Batarya V. ise → TS_ACTIVE :120
# │     │   └── 2 Saniye Timeout olursa → FAULT             :126
# │     │
# │     ├── :133-159 STATE_TS_ACTIVE
# │     │   ├── SDC Açılırsa → Kontaktörleri aç, LV_READY dön :138
# │     │   └── ★ FS KURALI EV 4.12.1: RTD Şartları         :146-158
# │     │       └── Fren > %15 + Start basılı → Buzzer
# │     │
# │     ├── :161-173 STATE_RTD_TRANSITION
# │     │   └── ★ FS KURALI EV 4.12.3: Buzzer 2sn           :163-172
# │     │       └── 2000ms dolunca → DRIVING moduna geç
# │     │
# │     ├── :175-187 STATE_DRIVING (Sürüş Modu)
# │     │   ├── Tork hesaplama (Torque Control'den)         :181
# │     │   └── Start tekrar basılırsa → TS_ACTIVE          :184
# │     │
# │     └── :189-209 STATE_FAULT (Hata Durumu)
# │         ├── Gücü Kes ve Tüm Kontaktörleri Aç            :195
# │         └── Hata gitti + Reset basılı → LV_READY        :205
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/telemetry.h  (Telemetri TX Veri Paketi Tanımları)
# ├─────────────────────────────────────────────────────────────
# │
# │   TelemetryPacket_t struct (21 Byte):
# │     ├── Byte 0:     vehicleState (Araç durumu)
# │     ├── Byte 1:     faultCode (Hata kodu)
# │     ├── Byte 2-7:   Sürüş verileri (APPS, Fren, Tork, RPM)
# │     ├── Byte 8-12:  Batarya verileri (Voltaj, Akım, SOC)
# │     ├── Byte 13-15: Sıcaklık verileri (Motor, İnverter, Hücre)
# │     ├── Byte 16:    systemFlags (Kontaktörler + SDC, Bit bazlı)
# │     └── Byte 17-20: uptimeMs (Zaman damgası)
# │
# │   TelemetryFrame_t struct (Header + Length + Data + Checksum):
# │     └── 0xAA 0x55 + 1B Length + 21B Data + 1B XOR Checksum = 25 Byte
# │
# │   Bit Maskeleri: TELEM_FLAG_AIR_NEG/POS, TELEM_FLAG_PRECHARGE,
# │                  TELEM_FLAG_SDC_CLOSED, TELEM_FLAG_INV_ENABLE
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Src/telemetry.c  (Telemetri TX Modülü)
# ├─────────────────────────────────────────────────────────────
# │
# │   TELEM_Init()              — Frame başlığını sabitler (0xAA, 0x55)
# │   TELEM_BuildPacket()       — StateMachine'den verileri alıp pakete doldurur
# │   TELEM_CalculateChecksum() — XOR checksum hesaplar
# │   TELEM_PeriodicSend()      — Zamanlayıcı ile 100ms'de bir gönderim
# │   TELEM_Hardware_Transmit() — Donanım soyutlama (STM32 gelince doldurulacak)
# │
# └─────────────────────────────────────────────────────────────


# ┌─────────────────────────────────────────────────────────────
# │  Inc/moving_average_filter.h & Src/moving_average_filter.c  (Filtre)
# ├─────────────────────────────────────────────────────────────
# │
# │   MA_FILTER_WINDOW_SIZE    — Pencere boyutu (varsayılan: 10)
# │   MovingAverage_Init()     — Filtre tamponunu sıfırlar
# │   MovingAverage_Update()   — Yeni veri girer, yumuşatılmış ortalamayı döndürür
# │
# └─────────────────────────────────────────────────────────────
# 
# ┌─────────────────────────────────────────────────────────────
# │  Inc/can_parser_buffer.h & Src/can_parser_buffer.c  (RAM Tampon)
# ├─────────────────────────────────────────────────────────────
# │
# │   CAN_SimulateData()       — Sanal Telemetri (TelemetryPacket_t) verisi üretir
# │   CAN_FormatCSV()          — Telemetri paketini "Uptime,State,Fault..." CSV'ye çevirir
# │   CAN_Buffer_Push()        — 512 bayt RAM tamponuna ekler, dolarsa SD_Logger_Write çağırır
# │   CAN_Buffer_Flush()       — Kalan tampon verisini diskte zorla (sync) oluşturur
# │
# └─────────────────────────────────────────────────────────────
# 
# ┌─────────────────────────────────────────────────────────────
# │  Inc/sd_file_system.h & Src/sd_file_system.c  (SD Mock)
# ├─────────────────────────────────────────────────────────────
# │
# │   SD_Logger_Init()         — SD kart montaj (mock PC ortamında)
# │   SD_Logger_OpenCSV()      — FATFS f_open (mock fopen)
# │   SD_Logger_Write()        — Sektör tamponunu f_write ile diske yazar
# │   SD_Logger_Sync()         — PC diskinde dosya kapatılmadan anlık kaydeder
# │
# └─────────────────────────────────────────────────────────────
# 
# ══════════════════════════════════════════════════════════════
# HIZLI ERİŞİM: BİR ŞEYİ DEĞİŞTİRMEK İSTİYORSAN
# ══════════════════════════════════════════════════════════════
### FS Yarışma Kuralları (Kodun İçine Gömülü Kurallar)
| Kural Numarası | Kural Adı | Ne Yapıyor? | Dosya Yolu | Satır |
|----------------|-----------|------------|-----------|-------|
| **EV 5.7** | Fren-Gaz Çakışması | Fren >30 Bar + Gaz >%25 ise tork kesilir | `Src/state_machine.c` | **Satır 19-27** |
| **T 11.8.8** | APPS %10 Sapma | Gaz sensörleri %10'dan fazla saparsa (100ms) gücü kes | `Src/state_machine.c` | **Satır 29-47** |
| **EV 4.11** | Precharge Sıralaması | AIR- → Precharge → %90 Voltaj → AIR+ | `Src/state_machine.c` | **Satır 109-131** |
| **EV 4.12.1** | RTD Koşulları | Fren >%15 basılı + Start butonu → Buzzer | `Src/state_machine.c` | **Satır 146-158** |
| **EV 4.12.3** | RTD Buzzer Süresi | 2 saniye kesintisiz ses → Sürüş moduna geç | `Src/state_machine.c` | **Satır 161-173** |
#
#   Tork miktarını ayarla ............. vehicle_config.h → CFG_MOTOR_MAX_TORQUE_NM
#   Buzzer süresini değiştir .......... vehicle_config.h → CFG_RTD_BUZZER_DURATION_MS
#   RTD fren eşiğini değiştir ........ vehicle_config.h → CFG_BRAKE_RTD_THRESHOLD
#   Fren-Gaz çakışma eşiği ........... vehicle_config.h → CFG_BRAKE_THROTTLE_*
#   Precharge süresini değiştir ....... vehicle_config.h → CFG_PRECHARGE_TIMEOUT_MS
#   Telemetri gönderim hızını değiştir  vehicle_config.h → CFG_TELEM_TX_PERIOD_MS
#   Yeni CAN mesajı ekle ............. FS2026_CAN_Dictionary.h :21 altına
#   Yeni hata türü ekle .............. state_machine.h :28 altına
#   Yeni sensör girişi ekle .......... state_machine.h :39 altına
#   Telemetriye yeni veri ekle ....... telemetry.h → TelemetryPacket_t struct'ına
#
# ══════════════════════════════════════════════════════════════
