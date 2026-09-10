# FS2026 ARAÇ İÇİ HABERLEŞME AĞI (SİSTEM MİMARİSİ)
# ══════════════════════════════════════════════════════════════
# Bu dosya aracın yazılımı geliştikçe aşama aşama güncellenecektir.
# Şu an sadece koda döktüğümüz ve aktif olan kısımları içermektedir.
# Son Güncelleme: 21 Temmuz 2026
# ══════════════════════════════════════════════════════════════

Aşağıdaki şemalar, arabadaki cihazların birbirlerine HANGİ KABLO/PROTOKOL ile bağlandığını ve HANGİ VERİLERİ gönderdiğini gösterir.


## 1. DONANIM BAĞLANTI ŞEMASI (Fiziksel)

```text
  [ GAZ PEDALI (APPS) ] ---------(Analog Voltaj)--------┐
                                                         │
  [ FREN SENSÖRÜ ] -------------(Analog Voltaj)---------┤
                                                         │
  [ START / RESET BUTONU ] -----(Dijital 1/0)-----------┤
                                                         ▼
                                            ╔══════════════════════╗
                                            ║      ANA BEYİN       ║
  [ AIR- Kontaktör ] <-----(Dijital OUT)---║        (VCU)         ║
  [ AIR+ Kontaktör ] <-----(Dijital OUT)---║   (state_machine.c)  ║
  [ Precharge Röle ] <-----(Dijital OUT)---║  (torque_control.c)  ║
                                            ║   (telemetry.c)     ║
                                            ╚══════════════════════╝
                                                         │
  [ BUZZER (RTD Ses) ] <-------(PWM)------- [ ARKA NODE (Rear) ] 
                                                         │
                                          ┌──────────────┼──────────────┐
                                          │ CAN BUS      │              │ UART
                                          ▼              ▼              ▼
                              [ İNVERTER ]    [ BMS ]         [ STM32 TELEMETRİ ]
                              [ MOTOR     ]   [ Batarya ]     [   NODE (LoRa)   ]
                                                                       │
                                                                       │ LoRa (Telsiz)
                                                                       ▼
                                                              [ YER İSTASYONU ]
                                                              [ (Pit Bilgisayar) ]
```


## 2. CAN BUS VERİ AKIŞI (Mantıksal)

### 🚗 VCU (Mid Node) → İnverter & Arka Node (CAN ID: `0x100`, Her 10ms'de)

**Paket İçeriği (`CAN_VCU_Control_t`):**
- **Tork Komutu:** Gaz pedalı yüzdesine, güvenlik limitlerine ve Regen durumuna göre hesaplanır.
- **İnverter İzni:** STATE_DRIVING moduna geçmeden izin verilmez.
- **Yön:** İleri/Geri
- **Reserved Baytları (Byte 4 ve 5):** Byte 4 (En düşük bayt) Arka Node'a "Buzzer Çal" (1/0) komutunu gönderir. Byte 5 ise hata ayıklama (diagnostic) amacıyla mevcut "Durum Makinesi" (State Machine) durumunu tutar.

### 🚗 VCU → Ağdaki Herkese (CAN ID: `0x101`, Her 20ms'de)

**Paket İçeriği (`CAN_VCU_Status_t`):**
- Araç Durumu, Gaz %, Fren Basıncı, Direksiyon Açısı, SDC Durumu


## 3. TELEMETRİ VERİ AKIŞI (VCU → Pit Alanı)

```text
 ╔═══════════════════╗     UART/CAN      ╔═════════════╗      LoRa      ╔══════════════╗
 ║   VCU             ║ ──── 100ms ─────> ║   STM32     ║ ──── RF ────> ║ Yer İstasyonu║
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

**Paket İçeriği:**
| Byte     | Veri             | Açıklama |
|----------|------------------|----------|
| 0        | vehicleState     | Araç durumu (INIT, DRIVING, FAULT vb.) |
| 1        | faultCode        | Aktif hata kodu |
| 2        | appsPercent      | Gaz pedalı yüzdesi |
| 3        | brakePressure    | Fren basıncı |
| 4-5      | torqueCommand    | Tork komutu |
| 6-7      | motorRPM         | Motor devri |
| 8-9      | batteryVoltage   | Batarya voltajı (V × 10) |
| 10-11    | batteryCurrent   | Batarya akımı |
| 12       | batterySOC       | Şarj durumu (%) |
| 13       | motorTemp        | Motor sıcaklığı (°C) |
| 14       | inverterTemp     | İnverter sıcaklığı (°C) |
| 15       | maxCellTemp      | En sıcak hücre (°C) |
| 16       | systemFlags      | Kontaktörler + SDC + İnverter (Bit bazlı) |
| 17-20    | uptimeMs         | Çalışma süresi (ms) |


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


## 5. İÇ KARAR MEKANİZMASI (Internal Flow)

```text
 1. GİRDİLER           2. KARAR (state_machine.c)          3. ÇIKTILAR
 ───────────           ──────────────────────────          ──────────────────
 - Gaz Yüzdesi     →   Fren+Gaz çakışma?         →        HATA VER (Tork = 0)
 - Fren Basıncı    →   Fren>15 + Start basılı?   →        BUZZER ÇAL → DRIVING
 - TS Voltajı      →   DRIVING modundaysa        →        TORK HESAPLA → CAN'e gönder
 - BMS Voltajı     →   PRECHARGING modunda?      →        KONTAKTÖR SIRASI (EV 4.11)
 - SDC Durumu      →   SDC koptu?                →        TÜMÜNÜ KAPAT → FAULT
```


---
> **Not:** BMS firmware, Nextion ekran ve IMU sensörü entegre edildikçe bu ağ şeması genişleyecektir.
