# FS2026 VCU (Araç Beyni) Yazılım Projesi

Hoş geldiniz! Bu klasör, takımımızın 2026 yarış aracı için yazdığı tüm **beyin (VCU) kodlarını** ve tüm düğüm (Node) projelerini içerir. Masaüstü testleri %100 başarı oranıyla tamamlanmıştır.

## 🏆 PROJE DURUMU: TÜM TESTLER %100 BAŞARILI
Tüm haberleşme (CAN), güvenlik (SDC, IMD), sensör doğrulaması (APPS Plausibility, Deadzone, Regen) ve sistem durum makineleri (RTD, Precharge) yarışma kurallarına tamamen uyumlu çalışmaktadır.

## 🗺️ PROJE HARİTASI (KLASÖR YAPISI)

```text
FS2026_VCU_Software/
│
├── FS2026_MidNode/   (VCU / Ana Beyin Projesi - SINGLE SOURCE OF TRUTH)
│   ├── Core/Inc/
│   │   ├── FS2026_CAN_Dictionary.h  ---> [ARACIN DİLİ] Tüm cihazların birbiriyle nasıl konuştuğunu içerir.
│   │   ├── vehicle_config.h         ---> [AYARLAR] Tüm eşik değerler, limitler ve konfigürasyon.
│   │   └── state_machine.h          ---> [DURUM KİTAPÇIĞI] Aracın hangi durumlarda olabileceği.
│   └── Core/Src/     
│       ├── state_machine.c          ---> [BEYİN / KARAR MEKANİZMASI] Tüm güvenlik kural motoru.
│       └── torque_control.c         ---> [TORK KONTROL / REGEN] İnvertere giden gücün hesabı.
│
├── FS2026_FrontNode/ (Ön Sensör Beyni)
│   └── Core/Src/     Gaz, Fren ve Start butonu verilerini okuyup XOR Checksum'ı ile VCU'ya iletir.
│
└── FS2026_RearNode/  (Arka Sistem / İnverter Simülasyon Beyni)
    └── Core/Src/     Batarya ve İnverter mesajlarını üreten, pompaları/fanları açan beyin.
```

## 🔍 KOD OKUMA REHBERİ (Yeni Başlayanlar İçin)

Takıma yeni katılan bir mühendisseniz, kodların içinde devasa bloklar halinde **FS KURALLARI** yorum satırları göreceksiniz. Biz kod yazarken yarışma kitapçığındaki kuralları doğrudan kodun içine gömdük.

Örneğin `FS2026_MidNode/Core/Src/state_machine.c` dosyasını açarsanız, şöyle bloklar göreceksiniz:

```c
// =========================================================================
// FS KURALI: EV 4.12.3 (RTD SESLİ UYARI - BUZZER)
// Sürüş moduna geçmeden hemen önce, etraftaki mekanikerleri uyarmak için
// 1 saniye ile 3 saniye arası kesintisiz zil (buzzer) çalmalıdır.
// =========================================================================
```

Bu sayede kodun sadece bir "yazılım" olmadığını, aslında arabanın fiziksel bir kuralını işlettiğini rahatça anlayabilirsiniz.

## 🛡️ YARIŞMA GÜVENLİK KURALLARI (TAMAMLANANLAR)
- **T 11.8.8**: APPS %10 Sapma kontrolü ve 100ms kuralı.
- **EV 5.7**: Fren-Gaz Çakışması (Brake Plausibility).
- **EV 4.12.1**: RTD (Ready To Drive) Güvenli geçiş koşulları.
- **EV 5.5**: SDC (Shutdown Circuit) İzolasyon ve Kapanma Testleri.
- **T 11.9.4**: CAN Node Timeout (Kopan beyin durumunda sistemi kapatma).
