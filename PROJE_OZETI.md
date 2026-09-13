# FS2026 VCU - PROJE DURUM RAPORU (AI AGENT İÇİN)
# Bu dosya insan okuyucu için değil, AI agent'ın proje durumunu hızla kavraması içindir.
# Her görev tamamlandığında veya yeni modül eklendiğinde güncellenmelidir.

## TAMAMLANAN GÖREVLER
- [DONE] Bütçe & Satın Alma Optimizasyonu tamamlandı.
- [DONE] Donanım Pin Haritası tamamlandı.
- [DONE] CAN Mesaj Sözlüğü Exceli oluşturuldu.
- [DONE] Ham CAN Frame Rehberi hazırlandı.
- [DONE] Donanım Testi 1 (CAN Haberleşme) NUCLEO-F446ZE ile DOĞRULANDI.
- [DONE] FS Kuralları (T 11.8.8, EV 5.7, EV 4.12.1, EV 5.5) kodlara entegre edildi.
- [DONE] Masaüstü testleri (Unity tabanlı) 87 testin tamamını geçerek 100% başarı oranına ulaştı.
- [DONE] REPO REFACTOR: Proje yapısı 3-Node (Front, Mid, Rear) şeklinde STM32 projeleri olarak ayrıldı ve ana dizine (FS2026_VCU_Software) yerleştirildi. Kopya `Src/` ve `Inc/` dosyaları silindi.

## AKTİF GÖREV
- [YOK] Yazılım, C tabanlı testlerden %100 oranında başarılı bir şekilde geçerek araca/donanıma yüklenmeye (Deployment) hazır hale geldi. Masaüstü simülasyon evresi kapandı.

## BEKLEYEN GÖREVLER (DONANIM GEREKLİ)
- Fiziksel Nextion ekran tasarımı (Ekran donanımı kesinleşmedi)
- Gerçek BMS ve İnverter ile CAN Bus entegrasyon testi
- Gerçek donanımlar üzerinde entegrasyon testleri.

## DOSYA HARİTASI (SINGLE SOURCE OF TRUTH MİMARİSİ)
```text
FS2026_VCU_Software/
├── FS2026_MidNode/      (VCU / Ana Beyin Projesi)
│   ├── Core/Inc/        CAN_Dictionary, vehicle_config, state_machine vb. (Referans noktası)
│   └── Core/Src/        state_machine.c, torque_control.c, telemetry.c vb.
├── FS2026_FrontNode/    (Ön Sensör Beyni)
│   └── Core/Src/        Pedalları okuyup CAN ile iletir.
├── FS2026_RearNode/     (Arka Sistem)
│   └── Core/Src/        İnverter/BMS simülasyonu ve eyleyiciler.
├── README.md            Genel proje özeti
├── PROJE_HARITASI.md    Satır numaralı detaylı içindekiler
├── HABERLESME_AGI.md    3-Node fiziksel ve mantıksal CAN akış diyagramı
└── MASA_TEST_SEMASI.md  (ARŞİV) Eski masaüstü test yapısı
```

## KRİTİK BAĞIMLILIKLAR
- Kök dizinde (root) artık kod bulunmamaktadır. Tüm kodlar STM32CubeIDE projelerinin (MidNode, FrontNode, RearNode) içine gömülmüştür.

## GİT DURUMU
- Remote: https://github.com/birbucukformula-Embedded/FS2026_VCU_Software.git
- Branch: master
- Push durumu: GÜNCEL

## ELİMİZDEKİ DONANIM VE REVİZE EDİLEN LİSTE
- [SİPARİŞ] 5x STM32 NUCLEO-F446ZE — 3-Node mimarisi için
- [SİPARİŞ] 4.3" Nextion Dokunmatik (NX8048P050-011C)
- [SİPARİŞ] 2x EBYTE E22-900T22D LoRa Modülü
- [SİPARİŞ] PC817 Optokuplör Modülleri
- [SİPARİŞ] SN65HVD230 CAN Modülleri
- [SİPARİŞ] 32GB SD Kart + Modülü
- [YOK] Motor & İnverter
- [YOK] BMS / LTC6811
- [YOK] IMU Sensörü
- [YOK] APPS ve Fren Basınç Sensörü
