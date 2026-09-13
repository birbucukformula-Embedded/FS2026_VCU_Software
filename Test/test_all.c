#include "test_framework.h"
#include "../Inc/state_machine.h"
#include "../Inc/vehicle_config.h"
#include "../Inc/torque_control.h"
#include "../Inc/telemetry.h"

// Kaynak dosyaları doğrudan include ediyoruz (Basit derleme için)
// Normalde ayrı .o dosyaları link edilir, ama test ortamında bu daha pratik.
#include "../Src/state_machine.c"
#include "../Src/torque_control.c"
#include "../Src/telemetry.c"
#include "../Src/moving_average_filter.c"
#include "../Src/low_pass_filter.c"
#include "../Src/sd_file_system.c"
#include "../Src/can_parser_buffer.c"
#include "../Src/error_logger.c"

/*===========================================================================*
 * FS2026 VCU UNIT TEST DOSYASI
 *===========================================================================*
 *
 *  Derleme:   gcc -o test_runner test_all.c -I../Inc -lm
 *  Çalıştırma: ./test_runner
 *
 *  Bu dosya tüm VCU modüllerini tek bir yürütücüde (runner) test eder.
 *
 *===========================================================================*/

// Yardımcı: Temiz bir State Machine nesnesi hazırla
static StateMachine_t create_clean_sm(void) {
    StateMachine_t sm;
    SM_Init(&sm);
    // Varsayılan güvenli girişler
    sm.inputs.appsPercent = 0;
    sm.inputs.apps1Percent = 0;
    sm.inputs.apps2Percent = 0;
    sm.inputs.apps1Raw = 2000;   // Sağlam ADC değeri
    sm.inputs.apps2Raw = 2000;   // Sağlam ADC değeri
    sm.inputs.brakeRaw = 2000;   // Sağlam ADC değeri
    sm.inputs.brakePressure = 0;
    sm.inputs.tsVoltage = 0;
    sm.inputs.bmsVoltage = 400; // Varsayılan batarya voltajı
    sm.inputs.startButtonPressed = false;
    sm.inputs.resetButtonPressed = false;
    sm.inputs.sdcClosed = true;  // SDC kapalı (güvenli)
    sm.inputs.externalFault = FAULT_NONE;
    sm.inputs.imdFaultActive = false;
    return sm;
}

/*===========================================================================*
 *  TEST 1: DURUM MAKİNESİ GEÇİŞLERİ
 *===========================================================================*/
static void test_state_machine_transitions(void) {
    TEST_SUITE_BEGIN("Durum Makinesi Geçişleri");
    
    // --- Test 1.1: INIT → LV_READY ve LED Self-Test ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.inputs.tsVoltage = 400; // PRECHARGING aşamasını anında geçsin
        TEST_ASSERT_EQ(sm.currentState, STATE_INIT, "Başlangıç durumu STATE_INIT olmalı");
        TEST_ASSERT_EQ(sm.outputs.dashboardLedsOn, true, "Başlangıçta (0 ms) LED'ler yanmalı");
        
        for (int i=0; i<100; i++) { // Toplam 1000ms
            sm.lastCanMessageTimeMs = 0; // CAN timeout engelle
            SM_Update(&sm, 10);
        }
        TEST_ASSERT_EQ(sm.currentState, STATE_TS_ACTIVE, "INIT → PRECHARGING → TS_ACTIVE geçişi");
        TEST_ASSERT_EQ(sm.outputs.dashboardLedsOn, true, "1. Saniyede LED'ler hala yanıyor olmalı");

        for (int i=0; i<150; i++) { // Toplam 1500ms (Toplam 2500ms eder)
            sm.lastCanMessageTimeMs = 0; // CAN timeout engelle
            SM_Update(&sm, 10);
        }
        TEST_ASSERT_EQ(sm.outputs.dashboardLedsOn, false, "2 Saniye sonra LED'ler kapanmalı");
    }
    
    // --- Test 1.2: LV_READY → PRECHARGING (SDC kapalıyken) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_LV_READY;
        sm.inputs.sdcClosed = true;
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_PRECHARGING, "SDC kapalı → PRECHARGING geçişi");
    }
    
    // --- Test 1.3: PRECHARGING → TS_ACTIVE (Voltaj %90'a ulaşınca) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_PRECHARGING;
        sm.inputs.bmsVoltage = 400;
        sm.inputs.tsVoltage = 385;  // %95 × 400 = 380'den büyük olmalı
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_TS_ACTIVE, "Voltaj %90'a ulaştı → TS_ACTIVE");
        TEST_ASSERT_EQ(sm.outputs.contactorPositive, true, "AIR+ kontaktör kapatıldı");
        TEST_ASSERT_EQ(sm.outputs.contactorPrecharge, false, "Precharge devreden çıkarıldı");
    }
    
    // --- Test 1.4: PRECHARGING → FAULT (Timeout) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_PRECHARGING;
        sm.inputs.bmsVoltage = 400;
        sm.inputs.tsVoltage = 100;  // Düşük (şarj olamadı)
        
        // 2100ms simüle et (2000ms timeout'u geçmeli)
        for (int i = 0; i < 210; i++) {
            sm.lastCanMessageTimeMs = 0; // Test sırasında sahte CAN mesajı geliyormuş gibi köpeği besle
            SM_Update(&sm, 10);
        }
        TEST_ASSERT_EQ(sm.currentState, STATE_FAULT, "Precharge timeout → FAULT");
        TEST_ASSERT_EQ(sm.outputs.activeFault, FAULT_PRECHARGE_FAIL, "Hata kodu: PRECHARGE_FAIL");
    }
    
    // --- Test 1.5: TS_ACTIVE → RTD_TRANSITION (Fren + Start) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_TS_ACTIVE;
        sm.inputs.brakePressure = 50;        // Fren basılı (> 15)
        sm.inputs.startButtonPressed = true;  // Start basılı
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_RTD_TRANSITION, "Fren + Start → RTD_TRANSITION");
        TEST_ASSERT_EQ(sm.outputs.rtdBuzzerOn, true, "Buzzer çalıyor");
    }
    
    // --- Test 1.6: RTD_TRANSITION → DRIVING (2 sn buzzer sonrası) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_RTD_TRANSITION;
        sm.isRtdTimerActive = true;
        sm.rtdTimerMs = 0;
        sm.outputs.rtdBuzzerOn = true;
        
        // 2100ms simüle et
        for (int i = 0; i < 210; i++) {
            sm.lastCanMessageTimeMs = 0;
            SM_Update(&sm, 10);
        }
        TEST_ASSERT_EQ(sm.currentState, STATE_DRIVING, "2sn buzzer → DRIVING");
        TEST_ASSERT_EQ(sm.outputs.rtdBuzzerOn, false, "Buzzer kapandı");
    }
    
    // --- Test 1.7: DRIVING → TS_ACTIVE (Start tekrar basılırsa) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_DRIVING;
        sm.inputs.startButtonPressed = true;
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_TS_ACTIVE, "Sürüşte Start → TS_ACTIVE (araç durur)");
    }
}

/*===========================================================================*
 *  TEST 2: GÜVENLİK KURALLARI (FS RULES)
 *===========================================================================*/
static void test_safety_rules(void) {
    TEST_SUITE_BEGIN("Güvenlik Kuralları (FS Kuralları)");
    
    // --- Test 2.1: EV 5.7 — Fren-Gaz Çakışması ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_DRIVING;
        sm.inputs.brakePressure = 50;  // Sert fren (> 30)
        sm.inputs.appsPercent = 30;    // Gaz basılı (> %25)
        sm.inputs.apps1Percent = 30;
        sm.inputs.apps2Percent = 30;
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_FAULT, "EV 5.7: Fren+Gaz → FAULT");
        TEST_ASSERT_EQ(sm.outputs.activeFault, FAULT_BRAKE_THROTTLE, "Hata kodu: BRAKE_THROTTLE");
    }
    
    // --- Test 2.2: T 11.8.8 — APPS %10 Sapma (100ms sonra) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_DRIVING;
        sm.inputs.apps1Percent = 50;
        sm.inputs.apps2Percent = 35;  // Fark = 15% (> %10)
        sm.inputs.appsPercent = 50;
        sm.inputs.brakePressure = 0;
        
        // 50ms boyunca - henüz FAULT olmamalı
        for (int i = 0; i < 5; i++) {
            SM_Update(&sm, 10);
        }
        TEST_ASSERT_NEQ(sm.outputs.activeFault, FAULT_APPS_PLAUSIBILITY, 
                        "T 11.8.8: 50ms'de henüz hata yok (100ms beklenmeli)");
        
        // 60ms daha geçsin (toplam 110ms > 100ms)
        for (int i = 0; i < 6; i++) {
            SM_Update(&sm, 10);
        }
        TEST_ASSERT_EQ(sm.currentState, STATE_FAULT, "T 11.8.8: 110ms sonra FAULT");
        TEST_ASSERT_EQ(sm.outputs.activeFault, FAULT_APPS_PLAUSIBILITY, 
                        "Hata kodu: APPS_PLAUSIBILITY");
    }
    
    // --- Test 2.3: SDC Kopması → FAULT ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_DRIVING;
        sm.inputs.sdcClosed = false;  // SDC koptu (E-Stop basıldı)
        sm.inputs.apps1Percent = 0;
        sm.inputs.apps2Percent = 0;
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_FAULT, "SDC kopması → FAULT");
        TEST_ASSERT_EQ(sm.outputs.activeFault, FAULT_SDC_OPEN, "Hata kodu: SDC_OPEN");
    }
    
    // --- Test 2.4: FAULT'ta kontaktörler açılmalı ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_FAULT;
        sm.outputs.activeFault = FAULT_BMS;
        sm.inputs.sdcClosed = false;
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.outputs.contactorNegative, false, "FAULT: AIR- açık");
        TEST_ASSERT_EQ(sm.outputs.contactorPositive, false, "FAULT: AIR+ açık");
        TEST_ASSERT_EQ(sm.outputs.contactorPrecharge, false, "FAULT: Precharge açık");
        TEST_ASSERT_EQ(sm.outputs.torqueCommand, 0, "FAULT: Tork = 0");
    }
    
    // --- Test 2.5: FAULT'tan çıkış (Reset butonu) ---
    {
        StateMachine_t sm = create_clean_sm();
        sm.currentState = STATE_FAULT;
        sm.outputs.activeFault = FAULT_BMS;
        // Hata giderildi
        sm.inputs.externalFault = FAULT_NONE;
        sm.inputs.sdcClosed = true;
        sm.inputs.resetButtonPressed = true;
        sm.inputs.apps1Percent = 0;
        sm.inputs.apps2Percent = 0;
        
        SM_Update(&sm, 10);
        TEST_ASSERT_EQ(sm.currentState, STATE_LV_READY, "Reset + Hata gitti → LV_READY");
    }
}

/*===========================================================================*
 *  TEST 3: TORK HESAPLAMA
 *===========================================================================*/
static void test_torque_control(void) {
    TEST_SUITE_BEGIN("Tork Hesaplama (torque_control.c)");
    
    VCU_Inputs_t inputs;
    memset(&inputs, 0, sizeof(inputs));
    inputs.vehicleSpeedKmh = 10; // Araç hareket halinde (Geri vites korumasına takılmamak için)
    
    // --- Test 3.1: Deadzone (Ölü Bölge) — Regen aktifse negatif tork beklenir ---
    inputs.appsPercent = 3;  // %3 < %5 (deadzone)
    int16_t torque = TC_CalculateTorque(&inputs);
    if (CFG_REGEN_ENABLED) {
        int16_t expectedRegen = -((CFG_MOTOR_MAX_TORQUE_NM * CFG_REGEN_MAX_PERCENT) / 100);
        TEST_ASSERT_EQ(torque, expectedRegen, "Deadzone + Regen: %3 gaz → Regen tork");
    } else {
        TEST_ASSERT_EQ(torque, 0, "Deadzone: %3 gaz → Tork = 0");
    }
    
    // --- Test 3.2: Deadzone sınırı (tam %5) — Regen aktifse negatif tork beklenir ---
    inputs.appsPercent = 5;
    torque = TC_CalculateTorque(&inputs);
    if (CFG_REGEN_ENABLED) {
        int16_t expectedRegen = -((CFG_MOTOR_MAX_TORQUE_NM * CFG_REGEN_MAX_PERCENT) / 100);
        TEST_ASSERT_EQ(torque, expectedRegen, "Deadzone sınırı + Regen: %5 gaz → Regen tork");
    } else {
        TEST_ASSERT_EQ(torque, 0, "Deadzone sınırı: %5 gaz → Tork = 0");
    }
    
    // --- Test 3.3: Deadzone'un hemen üstü ---
    inputs.appsPercent = 10;
    torque = TC_CalculateTorque(&inputs);
    TEST_ASSERT(torque > 0, "Deadzone üstü: %10 gaz → Tork > 0");
    
    // --- Test 3.4: Tam gaz (%100) ---
    inputs.appsPercent = 100;
    torque = TC_CalculateTorque(&inputs);
    TEST_ASSERT_EQ(torque, CFG_MOTOR_MAX_TORQUE_NM, "Tam gaz: %100 → Max Tork (230 Nm)");
    
    // --- Test 3.5: Regen (Gaza basılmıyor) ---
    inputs.appsPercent = 0;
    torque = TC_CalculateTorque(&inputs);
    if (CFG_REGEN_ENABLED) {
        TEST_ASSERT(torque < 0, "Regen: %0 gaz → Negatif tork (frenleme)");
        int16_t expectedRegen = -((CFG_MOTOR_MAX_TORQUE_NM * CFG_REGEN_MAX_PERCENT) / 100);
        TEST_ASSERT_EQ(torque, expectedRegen, "Regen tork değeri doğru");
    } else {
        TEST_ASSERT_EQ(torque, 0, "Regen kapalı: Tork = 0");
    }
}

/*===========================================================================*
 *  TEST 4: TELEMETRİ PAKETİ
 *===========================================================================*/
static void test_telemetry(void) {
    TEST_SUITE_BEGIN("Telemetri Paketi (telemetry.c)");
    
    TelemetryFrame_t frame;
    TELEM_Init(&frame);
    
    // --- Test 4.1: Header doğrulaması ---
    TEST_ASSERT_EQ(frame.header1, 0xAA, "Header byte 1 = 0xAA");
    TEST_ASSERT_EQ(frame.header2, 0x55, "Header byte 2 = 0x55");
    
    // --- Test 4.2: Length doğrulaması ---
    TEST_ASSERT_EQ(frame.length, sizeof(TelemetryPacket_t), "Length = sizeof(TelemetryPacket_t)");
    
    // --- Test 4.3: Paket doldurma ---
    StateMachine_t sm = create_clean_sm();
    sm.currentState = STATE_DRIVING;
    sm.outputs.currentState = STATE_DRIVING;
    sm.outputs.activeFault = FAULT_NONE;
    sm.outputs.torqueCommand = 150;
    sm.outputs.inverterEnable = true;
    sm.outputs.contactorNegative = true;
    sm.outputs.contactorPositive = true;
    sm.inputs.appsPercent = 65;
    sm.inputs.brakePressure = 0;
    sm.inputs.bmsVoltage = 380;
    
    TELEM_BuildPacket(&frame, &sm, 12345);
    
    TEST_ASSERT_EQ(frame.data.vehicleState, STATE_DRIVING, "Paket: vehicleState = DRIVING");
    TEST_ASSERT_EQ(frame.data.appsPercent, 65, "Paket: appsPercent = 65");
    TEST_ASSERT_EQ(frame.data.torqueCommand, 150, "Paket: torqueCommand = 150");
    TEST_ASSERT_EQ(frame.data.batteryVoltage, 380, "Paket: batteryVoltage = 380");
    TEST_ASSERT_EQ(frame.data.uptimeMs, 12345, "Paket: uptimeMs = 12345");
    
    // --- Test 4.4: System Flags (Bit bazlı) ---
    TEST_ASSERT(frame.data.systemFlags & TELEM_FLAG_AIR_NEG, "Flag: AIR- bit aktif");
    TEST_ASSERT(frame.data.systemFlags & TELEM_FLAG_AIR_POS, "Flag: AIR+ bit aktif");
    TEST_ASSERT(frame.data.systemFlags & TELEM_FLAG_SDC_CLOSED, "Flag: SDC bit aktif");
    TEST_ASSERT(frame.data.systemFlags & TELEM_FLAG_INV_ENABLE, "Flag: INV_ENABLE bit aktif");
    TEST_ASSERT(!(frame.data.systemFlags & TELEM_FLAG_PRECHARGE), "Flag: Precharge bit kapalı");
    
    // --- Test 4.5: XOR Checksum doğrulaması ---
    uint8_t checksum = 0;
    const uint8_t *dataBytes = (const uint8_t *)&frame.data;
    for (uint16_t i = 0; i < sizeof(TelemetryPacket_t); i++) {
        checksum ^= dataBytes[i];
    }
    TEST_ASSERT_EQ(frame.checksum, checksum, "XOR Checksum doğru hesaplandı");
}

/*===========================================================================*
 *  TEST 5: SENSÖR FİLTRELERİ
 *===========================================================================*/
static void test_sensor_filters(void) {
    TEST_SUITE_BEGIN("Sensör Filtreleri (moving_average)");
    
    MovingAverageFilter_t filter;
    MovingAverage_Init(&filter);
    
    TEST_ASSERT_EQ(filter.count, 0, "Filtre başlangıçta boş olmalı");
    
    // Sabit sinyal veriyoruz, ortalaması aynı kalmalı
    float out1 = MovingAverage_Update(&filter, 50.0f);
    TEST_ASSERT_EQ((int)out1, 50, "İlk örnek doğru alınmalı");
    
    MovingAverage_Update(&filter, 50.0f);
    float out2 = MovingAverage_Update(&filter, 50.0f);
    TEST_ASSERT_EQ((int)out2, 50, "Sabit örnekler ortalamayı değiştirmez");
    
    // Ani zıplama (Spike/Gürültü) veriyoruz
    float out3 = MovingAverage_Update(&filter, 150.0f);
    TEST_ASSERT((int)out3 < 100, "Ani zıplama sönümlenmeli (ortalama fırlamamalı)");
}

/*===========================================================================*
 *  TEST 6: SD DATALOGGER
 *===========================================================================*/
static void test_sd_datalogger(void) {
    TEST_SUITE_BEGIN("SD Datalogger ve Buffer (Mock)");
    
    bool init_ok = SD_Logger_Init();
    TEST_ASSERT_EQ(init_ok, true, "SD Mock başlatıldı");
    
    bool open_ok = SD_Logger_OpenCSV("test_vcu_log.csv");
    TEST_ASSERT_EQ(open_ok, true, "Test log dosyası açıldı");
    
    CAN_Buffer_Init();
    
    // Sahte veri paketi oluşturup 20 kez buffer'a atalım (512 byte'ı aşması için)
    TelemetryPacket_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.vehicleState = STATE_DRIVING;
    pkt.appsPercent = 100;
    
    for (int i = 0; i < 20; i++) {
        pkt.uptimeMs = i * 100;
        bool push_ok = CAN_Buffer_Push(&pkt);
        TEST_ASSERT_EQ(push_ok, true, "Buffer'a veri eklenebilmeli veya SD'ye yazılabilmeli");
    }
    
    // Kalanları flush et
    bool flush_ok = CAN_Buffer_Flush();
    TEST_ASSERT_EQ(flush_ok, true, "Kalan buffer SD'ye flush edildi");
    
    bool close_ok = SD_Logger_Close();
    TEST_ASSERT_EQ(close_ok, true, "SD dosya kapatıldı");
    
    // Dosya var mı ve boyutu 0'dan büyük mü kontrol et
    FILE* fp = fopen("test_vcu_log.csv", "r");
    TEST_ASSERT_NEQ(fp, NULL, "CSV dosyası fiziksel olarak oluşmalı");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        TEST_ASSERT(size > 512, "Dosya boyutu en az bir sektör (>512 byte) olmalı");
        fclose(fp);
        remove("test_vcu_log.csv"); // Test sonrası temizlik
    }
}

void test_can_parsing() {
    printf("\n═══════════════════════════════════════════════\n");
    printf("  TEST SÜİTİ: CAN Veri Ayrıştırma (BMS & Inverter)\n");
    printf("═══════════════════════════════════════════════\n");

    VCU_Inputs_t inputs;
    memset(&inputs, 0, sizeof(inputs));

    uint8_t mockBmsData[8];
    uint8_t mockInvData[8];

    // Rear Node'dan sahte veriyi üret
    CAN_Generate_Mock_RearNode_Data(mockBmsData, mockInvData);

    // BMS Mesajını Parse Et
    CAN_Parse_Message(0x200, mockBmsData, 8, &inputs);
    TEST_ASSERT_EQ(inputs.bmsVoltage, 395, "BMS Voltajı doğru parse edildi (395 V)");
    TEST_ASSERT_EQ(inputs.tsCurrent, 120, "BMS Akımı doğru parse edildi (120 A)");

    // Inverter Mesajını Parse Et
    CAN_Parse_Message(0x300, mockInvData, 8, &inputs);
    TEST_ASSERT_EQ(inputs.tsVoltage, 395, "İnverter Voltajı doğru parse edildi (395 V)");
    
    // (2000 RPM * 60 * 157) / 400000 = 18840000 / 400000 = 47 km/h
    TEST_ASSERT_EQ(inputs.vehicleSpeedKmh, 47, "Araç Hızı doğru parse ve hesap edildi (47 km/h)");

    // Front Node (0x110) - Checksum Doğrulama Testi
    CAN_Front_Sensors_t frontData = {0};
    frontData.apps1Percent = 100;
    frontData.apps2Percent = 49;
    frontData.brakePressure = 50;
    frontData.startButton = 1;
    frontData.resetButton = 0;
    
    // Checksum hesapla
    frontData.checksum = frontData.apps1Percent ^ frontData.apps2Percent ^ frontData.brakePressure ^ frontData.startButton ^ frontData.resetButton;

    CAN_Parse_Message(0x110, (uint8_t*)&frontData, 8, &inputs);
    TEST_ASSERT_EQ(inputs.apps1Percent, 100, "APPS1 Yüzdesi");
    TEST_ASSERT_EQ(inputs.apps2Percent, 49, "APPS2 Yüzdesi");
    TEST_ASSERT_EQ(inputs.brakePressure, 50, "Brake Yüzdesi");
    TEST_ASSERT_EQ(inputs.startButtonPressed, true, "Start butonu parse");

    // Checksum yanlış paketi gönderelim
    uint16_t old_apps1 = inputs.apps1Percent;
    frontData.checksum = frontData.checksum + 1; // Yanlış checksum
    frontData.apps1Percent = 0;    // Değeri değiştirelim ki parse edilirse fark edelim

    CAN_Parse_Message(0x110, (uint8_t*)&frontData, 8, &inputs);
    TEST_ASSERT_EQ(inputs.apps1Percent, old_apps1, "Yanlış Checksum'lı mesaj reddedilmeli (Değer değişmemeli)");
}

/*===========================================================================*
 *  ANA FONKSİYON
 *===========================================================================*/
int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║   FS2026 VCU UNIT TEST RUNNER                   ║\n");
    printf("║   Tüm modüller bilgisayarda test ediliyor...    ║\n");
    printf("╚══════════════════════════════════════════════════╝\n");
    
    // Tüm test süitlerini çalıştır
    test_state_machine_transitions();
    test_safety_rules();
    test_torque_control();
    test_telemetry();
    test_sensor_filters();
    test_sd_datalogger();
    test_can_parsing();
    
    // Sonuç raporu
    TEST_REPORT();
    
    return test_failed > 0 ? 1 : 0;
}
