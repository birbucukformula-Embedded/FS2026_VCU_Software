# FS2026 ARAÇ İÇİ HABERLEŞME AĞI (SİSTEM MİMARİSİ)
# ══════════════════════════════════════════════════════════════
# Bu dosya, aracın yazılımı ve donanımı geliştikçe aşama aşama güncellenmiştir.
# Son Güncelleme: Eylül 2026 (Masaüstü Testleri %100 Tamamlandı)
# ══════════════════════════════════════════════════════════════

Aşağıdaki şemalar, arabadaki 3 ana beyin (Front, Mid, Rear) ve diğer cihazların birbirlerine HANGİ KABLO/PROTOKOL ile bağlandığını ve HANGİ VERİLERİ gönderdiğini gösterir.

## 1. DONANIM BAĞLANTI ŞEMASI (Fiziksel)

```text
  [ GAZ PEDALI 1 & 2 ] ---(Analog Voltaj)---┐
                                             │
  [ FREN SENSÖRÜ ] ------(Analog Voltaj)-----┤
                                             │
  [ START / RESET ] -----(Dijital IN)--------┤
                                             ▼
                                ╔══════════════════════╗
                                ║    ÖN BEYİN (Front)  ║ (Pedalları Okur ve XOR
                                ║   (FS2026_FrontNode) ║  Checksum ile CAN'e basar)
                                ╚══════════════════════╝
                                             │
                                             │ (CAN ID: 0x110)
                                             ▼
                                ╔══════════════════════╗
  [ AIR- Kontaktör ] <---(OUT)--║  ANA BEYİN (VCU/Mid) ║ (Tüm güvenlik kurallarını
  [ AIR+ Kontaktör ] <---(OUT)--║   (FS2026_MidNode)   ║  denetler, torku hesaplar)
  [ Precharge Röle ] <---(OUT)--║                      ║
  [ Gösterge LED'leri] <-(OUT)--╚══════════════════════╝
                                             │
                                             │ (CAN ID: 0x100, 0x101)
                                             ▼
                                ╔══════════════════════╗
  [ BUZZER (RTD) ] <----(PWM)---║  ARKA BEYİN (Rear)   ║ (İnverter ve BMS'ten gelen
  [ FAN & POMPALAR ] <---(OUT)--║   (FS2026_RearNode)  ║  verileri işler, soğutmayı açar)
                                ╚══════════════════════╝
                                             │
                          ┌──────────────────┼──────────────────┐
                          │ CAN BUS          │                  │ UART
                          ▼                  ▼                  ▼
              [ İNVERTER (0x300) ]     [ BMS (0x200) ]     [ STM32 TELEMETRİ ]
              [ MOTOR            ]     [ Batarya     ]     [   NODE (LoRa)   ]
                                                                    │
                                                                    │ LoRa (Telsiz)
                                                                    ▼
                                                           [ YER İSTASYONU ]
```

## 2. CAN BUS VERİ AKIŞI (Mantıksal)

### 🚗 ÖN BEYİN → VCU (CAN ID: `0x110`, Her 10ms'de)
**Paket İçeriği (`CAN_Front_Sensors_t`):**
- **APPS 1 & 2:** Gaz pedalı yüzdeleri (0-100).
- **Fren:** Fren basıncı yüzdesi (0-100).
- **Butonlar:** Start ve Reset.
- **Güvenlik (Byte 7):** XOR Checksum (Bu değer yanlış gelirse VCU veriyi reddeder).

### 🚗 VCU → İnverter & Arka Node (CAN ID: `0x100`, Her 10ms'de)
**Paket İçeriği (`CAN_VCU_Control_t`):**
- **Tork Komutu:** Gaz pedalı yüzdesine, güvenlik limitlerine (Brake Plausibility) ve Regen durumuna göre hesaplanır.
- **İnverter İzni:** STATE_DRIVING moduna geçmeden izin verilmez.
- **Yön:** İleri/Geri
- **Buzzer Komutu (Byte 4):** Arka Node'a "Buzzer Çal" (1/0) komutunu gönderir.

### 🚗 VCU → Ağdaki Herkese (CAN ID: `0x101`, Her 20ms'de)
**Paket İçeriği (`CAN_VCU_Status_t`):**
- Araç Durumu, Gaz %, Fren Basıncı, Direksiyon Açısı, SDC Durumu, Hata Kodları.

## 3. TELEMETRİ VERİ AKIŞI (VCU → Pit Alanı)

```text
 ╔═══════════════════╗     UART/CAN      ╔═════════════╗      LoRa      ╔══════════════╗
 ║   VCU (MidNode)   ║ ──── 100ms ─────> ║   STM32     ║ ──── RF ────> ║ Yer İstasyonu║
 ║  (telemetry.c)    ║   25 Byte/paket   ║ (Telemetri) ║   (Telsiz)   ║  (Bilgisayar)║
 ╚═══════════════════╝                   ╚═════════════╝              ╚══════════════╝
```

**Paket Formatı (25 Byte):**
```text
┌────────┬────────┬──────────────────────┬──────────┐
│ 0xAA   │ 0x55   │  21 Byte Veri        │ Checksum │
│ Header │ Header │  (TelemetryPacket_t) │ (XOR)    │
└────────┴────────┴──────────────────────┴──────────┘
```

## 4. KONTAKTÖR KONTROL SIRASI (Precharge - FS EV 4.11)

```text
  Adım 1: LV_READY → sdcClosed=true → PRECHARGING moduna geç
                        │
  Adım 2: PRECHARGING   ├── AIR- = KAPAT ✓
                        ├── Precharge = KAPAT ✓
                        ├── Bekleniyor... (İnverter şarj oluyor)
                        │
  Adım 3: İnverter V.  ├── tsVoltage >= %90 × bmsVoltage ?
           ≥ %90 BMS    │     EVET → AIR+ = KAPAT, Precharge = AÇ → TS_ACTIVE ✓
                        │     2sn Timeout → FAULT_PRECHARGE_FAIL ✗
```

## 5. İÇ KARAR MEKANİZMASI (Internal Flow - Multi-Node Watchdog)

```text
 1. GİRDİLER           2. KARAR (state_machine.c)          3. ÇIKTILAR
 ───────────           ──────────────────────────          ──────────────────
 - Node Timeout    →   Front/BMS/Inv koptu mu?   →        CAN_TIMEOUT → FAULT (Tork = 0)
 - Gaz Yüzdesi     →   Fren+Gaz çakışma?         →        HATA VER (Tork = 0)
 - Fren Basıncı    →   Fren>15 + Start basılı?   →        BUZZER ÇAL → DRIVING
 - TS Voltajı      →   DRIVING modundaysa        →        TORK HESAPLA → CAN'e gönder
 - BMS Voltajı     →   PRECHARGING modunda?      →        KONTAKTÖR SIRASI (EV 4.11)
 - SDC Durumu      →   SDC koptu?                →        TÜMÜNÜ KAPAT → FAULT
```
