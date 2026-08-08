#include "state_machine.h"
#include "torque_control.h" // Tork hesaplama algoritmaları
#include "vehicle_config.h" // Tüm ayarlanabilir değerler buradan gelir
#include <stdlib.h>         // abs() fonksiyonu için

// Yardımcı Fonksiyon: Hataları kontrol eder
static FaultCode_t CheckForErrors(StateMachine_t *sm, uint32_t deltaTimeMs) {
  // 1. Dış Hatalar (BMS Hatası vb.)
  if (sm->inputs.externalFault != FAULT_NONE) {
    return sm->inputs.externalFault;
  }

  // 2. Güvenlik Devresi (SDC) Koptuysa
  if (!sm->inputs.sdcClosed) {
    return FAULT_SDC_OPEN;
  }

  // =========================================================================
  // FS KURALI: EV 5.7 (FREN VE GAZ ÇAKIŞMASI - APPS PLAUSIBILITY)
  // Sürücü sert frene basarken (Örn: >30 Bar) aynı anda gaza basıyorsa (Örn:
  // >%25) Motora giden güç ANINDA kesilmelidir. (Hata durumuna düşürülür)
  // =========================================================================
  if (sm->inputs.brakePressure > CFG_BRAKE_THROTTLE_BRAKE_MIN &&
      sm->inputs.appsPercent > CFG_BRAKE_THROTTLE_GAS_MAX) {
    // Not: Kurala göre gaz pedalı %5'in altına düşene kadar bu hata KALICI
    // olmalıdır. Şimdilik sadece hatayı tetikleme şartını yazıyoruz.
    return FAULT_BRAKE_THROTTLE;
  }

  // =========================================================================
  // FS KURALI: T 11.8.8 (APPS PLAUSIBILITY - %10 SAPMA KURALI)
  // İki gaz sensörü arasındaki fark %10'dan fazlaysa ve bu durum
  // 100 milisaniyeden uzun sürerse motora giden güç anında KESİLMELİDİR.
  // =========================================================================
  int16_t appsDiff =
      abs((int16_t)sm->inputs.apps1Percent - (int16_t)sm->inputs.apps2Percent);

  if (appsDiff > CFG_APPS_PLAUSIBILITY_PERCENT) {
    sm->isAppsTimerActive = true;
    sm->appsTimerMs += deltaTimeMs; // Sayacı artır

    if (sm->appsTimerMs >= CFG_APPS_PLAUSIBILITY_TIME_MS) {
      return FAULT_APPS_PLAUSIBILITY; // HATA! (%10 sapma 100ms sürdü)
    }
  } else {
    // Sensörler tekrar %10'un altına döndüyse sayacı sıfırla
    sm->isAppsTimerActive = false;
    sm->appsTimerMs = 0;
  }

  return FAULT_NONE;
}

// Durum Makinesini Başlatır
void SM_Init(StateMachine_t *sm) {
  sm->currentState = STATE_INIT;
  sm->previousState = STATE_INIT;
  sm->rtdTimerMs = 0;
  sm->isRtdTimerActive = false;
  sm->appsTimerMs = 0;
  sm->isAppsTimerActive = false;
  sm->prechargeTimerMs = 0;

  // Filtreleri Başlat
  MovingAverage_Init(&sm->apps1Filter);
  MovingAverage_Init(&sm->apps2Filter);
  MovingAverage_Init(&sm->brakeFilter);

  // Çıktıları güvenli değerlere çek
  sm->outputs.inverterEnable = false;
  sm->outputs.rtdBuzzerOn = false;
  sm->outputs.torqueCommand = 0;
  sm->outputs.activeFault = FAULT_NONE;
  sm->outputs.contactorNegative = false;
  sm->outputs.contactorPrecharge = false;
  sm->outputs.contactorPositive = false;
}

// Durum Makinesi Ana Döngüsü (Örn: Her 10ms'de bir çağrılır)
void SM_Update(StateMachine_t *sm, uint32_t deltaTimeMs) {
  // 0. FİLTRELEME (Sensörlerden gelen gürültülü veriyi temizle)
  sm->inputs.apps1Percent = (uint8_t)MovingAverage_Update(&sm->apps1Filter, (float)sm->inputs.apps1Percent);
  sm->inputs.apps2Percent = (uint8_t)MovingAverage_Update(&sm->apps2Filter, (float)sm->inputs.apps2Percent);
  sm->inputs.brakePressure = (uint8_t)MovingAverage_Update(&sm->brakeFilter, (float)sm->inputs.brakePressure);

  // 1. ÖNCE HATA KONTROLÜ (En yüksek öncelik)
  // deltaTimeMs'i hata kontrolüne de gönderiyoruz çünkü APPS için gerekli.
  FaultCode_t currentFault = CheckForErrors(sm, deltaTimeMs);

  if (currentFault != FAULT_NONE) {
    sm->currentState = STATE_FAULT;
    sm->outputs.activeFault = currentFault;
  }

  // Zamanlayıcıyı güncelle
  if (sm->isRtdTimerActive) {
    sm->rtdTimerMs += deltaTimeMs;
  }

  // Önceki durumu kaydet (Değişimleri yakalamak için)
  sm->previousState = sm->currentState;

  // 2. DURUM GEÇİŞLERİ (State Machine Mantığı)
  switch (sm->currentState) {

  case STATE_INIT:
    // Donanım testleri burada yapılır. Şimdilik direkt geçiyoruz.
    sm->currentState = STATE_LV_READY;
    break;

  case STATE_LV_READY:
    // Sadece Low Voltage (12V) sistemleri açık. Yüksek voltaj bekleniyor.
    // SDC (Güvenlik devresi) kapandığında TS Master anahtarı açılmış demektir.
    // Bu durumda Kontaktörleri kapatmak için PRECHARGE moduna geçiyoruz.
    if (sm->inputs.sdcClosed == true) {
      sm->prechargeTimerMs = 0; // Sayacı sıfırla
      sm->currentState = STATE_PRECHARGING;
    }
    break;

  case STATE_PRECHARGING:
    // FS KURALI: EV 4.11
    // Önce AIR- ve Precharge Rölesi kapatılır
    sm->outputs.contactorNegative = true;
    sm->outputs.contactorPrecharge = true;

    sm->prechargeTimerMs += deltaTimeMs;

    // Voltaj hedefi: İnverter voltajı (TS), Batarya voltajının (BMS) %90'ına
    // ulaşmalı
    uint16_t targetVoltage =
        (sm->inputs.bmsVoltage * CFG_PRECHARGE_SUCCESS_PERCENT) / 100;

    if (sm->inputs.tsVoltage >= targetVoltage &&
        sm->inputs.tsVoltage >= CFG_TS_MIN_VOLTAGE) {
      // Şarj başarılı! Artı kontaktörü kapat, precharge direncini devreden
      // çıkar
      sm->outputs.contactorPositive = true;
      sm->outputs.contactorPrecharge = false;
      sm->currentState = STATE_TS_ACTIVE;
    } else if (sm->prechargeTimerMs > CFG_PRECHARGE_TIMEOUT_MS) {
      // Zaman aşımı (Örn: 2 saniyede şarj olamadı -> Kaçak veya kısa devre var)
      sm->outputs.activeFault = FAULT_PRECHARGE_FAIL;
      sm->currentState = STATE_FAULT;
    }
    break;

  case STATE_TS_ACTIVE:
    // Yüksek Gerilim sistemi aktif (400V - 600V hatta var). İnverter beklemede.

    // SDC koptuysa (Örn: TS Master kapatıldıysa veya E-Stop basıldıysa)
    // LV_READY moduna güvenli bir şekilde geri dön.
    if (sm->inputs.sdcClosed == false) {
      sm->outputs.contactorNegative = false;
      sm->outputs.contactorPositive = false;
      sm->outputs.contactorPrecharge = false;
      sm->currentState = STATE_LV_READY;
    }

    // =========================================================================
    // FS KURALI: EV 4.12.1 (SÜRÜŞE HAZIR OLMA - READY TO DRIVE SARTLARI)
    // Araba sadece ve sadece şu şartlar aynı anda sağlanırsa çalışır:
    // 1) Sürücü frene belirli bir güçle basıyor olmalı (> %15)
    // 2) Start butonuna basılmış olmalı.
    // =========================================================================
    if (sm->inputs.brakePressure > CFG_BRAKE_RTD_THRESHOLD &&
        sm->inputs.startButtonPressed) {
      sm->currentState = STATE_RTD_TRANSITION;

      // Zamanlayıcıyı sıfırla ve Buzzer'ı öttür!
      sm->rtdTimerMs = 0;
      sm->isRtdTimerActive = true;
      sm->outputs.rtdBuzzerOn = true;
    }
    break;

  case STATE_RTD_TRANSITION:
    // =========================================================================
    // FS KURALI: EV 4.12.3 (RTD SESLİ UYARI - BUZZER)
    // Sürüş moduna geçmeden hemen önce, etraftaki mekanikerleri uyarmak için
    // 1 saniye ile 3 saniye arası kesintisiz zil (buzzer) çalmalıdır.
    // Biz burada tam 2 saniye (2000 ms) çalacak şekilde ayarladık.
    // =========================================================================
    if (sm->rtdTimerMs >= CFG_RTD_BUZZER_DURATION_MS) {
      sm->outputs.rtdBuzzerOn = false;  // Sesi kapat
      sm->isRtdTimerActive = false;     // Sayacı durdur
      sm->currentState = STATE_DRIVING; // ARTIK MOTOR DÖNEBİLİR!
    }
    break;

  case STATE_DRIVING:
    // SÜRÜŞ MODU! Inverter aktif edilir ve APPS değerine göre tork verilir.
    sm->outputs.inverterEnable = true;

    // Tork hesaplama (Torque Control modülünden gelir)
    // Bu fonksiyon; deadzone, regen ve sıcaklık limitlerini otomatik uygular.
    sm->outputs.torqueCommand = TC_CalculateTorque(&sm->inputs);

    // Sürücü tekrar Start butonuna basarsa aracı kapat (TS_ACTIVE'e dön)
    if (sm->inputs.startButtonPressed) {
      sm->currentState = STATE_TS_ACTIVE;
    }
    break;

  case STATE_FAULT:
    // HATA DURUMU! Aracı kilitle.
    sm->outputs.inverterEnable = false;
    sm->outputs.torqueCommand = 0;

    // FS KURALI: Hata durumunda yüksek voltaj kontaktörleri (Tractive System)
    // DERHAL açılmalıdır (Güç kesilmelidir).
    sm->outputs.contactorNegative = false;
    sm->outputs.contactorPositive = false;
    sm->outputs.contactorPrecharge = false;

    sm->rtdTimerMs = 0;
    sm->isRtdTimerActive = false;

    // Hata giderildiyse ve Reset butonuna basıldıysa geri dön
    // CheckForErrors çağrısına deltaTimeMs=0 gönderiyoruz çünkü
    // sadece hatanın geçip geçmediğini (anlık olarak) soruyoruz, sayaç
    // arttırmak istemiyoruz.
    if (CheckForErrors(sm, 0) == FAULT_NONE && sm->inputs.resetButtonPressed) {
      sm->outputs.activeFault = FAULT_NONE;
      sm->currentState = STATE_LV_READY;
    }
    break;
  }

  // Son durumu çıktılara yansıt (Bu veriler CAN üzerinden HMI'a basılacak)
  sm->outputs.currentState = sm->currentState;
}
