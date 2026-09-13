# FS2026 VCU - PROJE DURUM RAPORU (AI AGENT İÇİN)
# Bu dosya insan okuyucu için değil, AI agent'ın proje durumunu hızla kavraması içindir.
# Her görev tamamlandığında veya yeni modül eklendiğinde güncellenmelidir.

## TAMAMLANAN GÖREVLER
- [DONE] Bütçe & Satın Alma Optimizasyonu → `FS2026_Revize_Alim_Listesi.xlsx` tamamlandı (Tüm Node, HMI, Sensör, Kablolama).
- [DONE] Donanım Pin Haritası → `FS2026_Donanim_Pin_Baglantilari.xlsx` tamamlandı (Front, Mid, Rear için siyah header tasarımı).
- [DONE] CAN Mesaj Sözlüğü Exceli → `FS2026_CAN_Sozlugu.xlsx` oluşturuldu (Tüm CAN ID ve Byte tanımları).
- [DONE] Ham CAN Frame Rehberi → `CAN_Frame_Okuma_Rehberi.md` hazırlandı (Candump okumak için görsel kılavuz).
- [DONE] Donanım Testi 1 (CAN Haberleşme) → NUCLEO-F446ZE + SN65HVD230 + Avioni UCAN ile Linux candump üzerinden 500kbps canlı haberleşme BAŞARIYLA DOĞRULANDI.
- [DONE] Görev 7: CAN Frame Sözlüğü → Inc/FS2026_CAN_Dictionary.h (8 mesaj ID, 6 struct)
- [DONE] Görev 1: Araç Durum Makinesi → Inc/state_machine.h + Src/state_machine.c (6 state, 7 transition)
- [DONE] vehicle_config.h oluşturuldu → Tüm ayarlanabilir parametreler merkeze alındı
- [DONE] Görev 2: Tork İsteği & Sürüş Algoritmaları → torque_control.c (Deadzone, Regen, Safety Limits)
- [DONE] Görev 4: Plausibility → EV 5.7 (Fren-Gaz) ve T 11.8.8 (APPS %10 Sapma) koda eklendi
- [DONE] Görev 9: Nodelar arası mesaj yönetimi → can_manager.c (Zamanlayıcı ve mesaj paketleme eklendi)
- [DONE] Görev 13: Precharge & Kontaktör kontrolü → state_machine'e PRECHARGING ve kontaktörler eklendi
- [DONE] Görev 16: Telemetri TX veri paketi → telemetry.c (25 Byte Frame, XOR Checksum, 100ms periyot)
- [DONE] Görev 3 & Görev 5: Sensör Filtreleri ve SD Datalogger → Subsystems'den VCU'ya entegre edildi. Tek veri tipi (TelemetryPacket_t) kullanılıyor.
- [DONE] Görev 24: Unit Test yazımı → Test/test_all.c (87 test, 6 süit: SM, FS Kuralları, Tork, Telemetri, Filtreler, SD Mock, CAN Parse)
- [DONE] Görev 6: Fault Handling → CheckForErrors() yazıldı, BMS ve İnverter CAN Parsing ile simülasyonu eklendi.
- [DONE] Yazılım Uygulamaları → Front Node (GPIO, Checksum, 10ms Kuralı), Rear Node (CAN Mocking, R2D Buzzer, I/O) ve VCU entegrasyonu tamamen tamamlandı. Testlerden başarıyla geçti.

## AKTİF GÖREV
- [YOK] Tüm FS2026 yazılım zorunlulukları (FS2026_Yazilimsal_Zorunluluklar_Takip.xlsx) koda döküldü, birim testleri (87 test) %100 başarıyla geçti. Fiziksel donanım testine hazırız!

## BEKLEYEN GÖREVLER (DONANIM GEREKLİ)
- Fiziksel Nextion ekran tasarımı (Ekran donanımı kesinleşmedi)
- Gerçek BMS ve İnverter ile CAN Bus entegrasyon testi
- Pazar (veya gerçek test günü) yapılacak donanım "Masa Testi"

## DOSYA HARİTASI
```
FS2026_VCU_Software/
├── Inc/
│   ├── FS2026_CAN_Dictionary.h  [DONE] CAN mesaj ID'leri ve paket struct'ları
│   ├── can_manager.h            [DONE] CAN zamanlayıcıları ve şablonu
│   ├── state_machine.h          [DONE] Durum enum'ları, hata kodları, I/O struct'ları, Filtreler
│   ├── telemetry.h              [DONE] Telemetri TX veri paketi (21 Byte + Frame)
│   ├── torque_control.h         [DONE] Tork hesaplama prototipleri
│   ├── vehicle_config.h         [DONE] Tüm ayarlanabilir parametreler (CFG_)
│   ├── moving_average_filter.h  [DONE] Sensör filtreleri
│   ├── low_pass_filter.h        [DONE] Sensör filtreleri
│   ├── can_parser_buffer.h      [DONE] SD Kart Datalogger (RAM Tamponu)
│   └── sd_file_system.h         [DONE] SD Kart Mock Sistemi
├── Src/
│   ├── can_manager.c            [DONE] CAN periyodik mesaj gönderimi
│   ├── state_machine.c          [DONE] Ana durum makinesi algoritması (switch-case)
│   ├── telemetry.c              [DONE] Telemetri paketi oluşturma ve gönderimi
│   ├── torque_control.c         [DONE] Tork haritası, regen ve güvenlik limitleri
│   ├── moving_average_filter.c  [DONE] Filtre implementasyonu
│   ├── low_pass_filter.c        [DONE] Filtre implementasyonu
│   ├── can_parser_buffer.c      [DONE] TelemetryPacket_t formatlama ve buffer
│   └── sd_file_system.c         [DONE] fopen/fwrite mock dosya sistemi
├── Test/
│   ├── test_framework.h         [DONE] Minimal test çerçevesi (assert makroları)
│   └── test_all.c               [DONE] 75 test (SM, FS kuralları, Tork, Telemetri, Filtreler, SD)
├── README.md                    [DONE] Klasör yapısı açıklaması
├── PROJE_HARITASI.md            [DONE] Satır numaralı içindekiler
└── HABERLESME_AGI.md            [DONE] İletişim akış diyagramı
```

## KRİTİK BAĞIMLILIKLAR
- state_machine.c → vehicle_config.h (tüm eşik değerleri config'den gelir)
- state_machine.c → state_machine.h (enum ve struct tanımları)
- Gelecekteki torque_control.c → vehicle_config.h (motor parametreleri)
- Gelecekteki torque_control.c → state_machine.h (VCU_Inputs_t kullanacak)

## GİT DURUMU
- Remote: https://github.com/birbucukformula-Embedded/Fs2026_vcu.git
- Branch: master
- Push durumu: BAŞARILI (Tüm yerel commitler GitHub'a gönderildi)

## ELİMİZDEKİ DONANIM VE REVİZE EDİLEN LİSTE (FS2026_Revize_Alim_Listesi)
- [SİPARİŞ] 5x STM32 NUCLEO-F446ZE — 3-Node mimarisi için (Ön, Orta, Arka)
- [SİPARİŞ] 4.3" Nextion Dokunmatik (NX8048P050-011C) — Sürücü ekranı (UART, Rezistif).
- [SİPARİŞ] 2x EBYTE E22-900T22D LoRa Modülü — Telemetri (Araç ve Pit).
- [SİPARİŞ] PC817 Optokuplör Modülleri — Sinyal izolasyonu (TSAL, Fren, Butonlar).
- [SİPARİŞ] SN65HVD230 CAN Modülleri — Node haberleşmesi.
- [SİPARİŞ] 32GB SD Kart + Modülü — Orta Node veri kaydı için.
- [YOK] Motor & İnverter — Marka/model belli değil. CFG_MOTOR_MAX_TORQUE_NM = 230 (varsayılan).
- [YOK] BMS / LTC6811 — Henüz yok. BMS firmware yazılamıyor.
- [YOK] IMU Sensörü — Torque Vectoring için gerekli.
- [YOK] APPS (Gaz Pedalı Sensörü) — Front Node: PA3 (APPS1), PC0 (APPS2).
- [YOK] Fren Basınç Sensörü — Front Node: PC3 (Sinyal çakışması nedeniyle A2 iptal edildi, PC3'e taşındı).
- [DONANIM] Pasif Buzzer (RTD Ses) — Rear Node: PA5 (D13) pinine taşındı (Yazılımsal 500Hz PWM).
