# FS2026 VCU (Araç Beyni) Yazılım Projesi

Hoş geldiniz! Bu klasör, takımımızın 2026 yarış aracı için yazdığı tüm **beyin (VCU) kodlarını** içerir. 

Eğer koda yeni bakıyorsanız kaybolmamanız için aşağıda bir "Proje Haritası" çıkardık. Hangi dosyanın nerede olduğunu ve ne işe yaradığını buradan görebilirsiniz.

## 🗺️ PROJE HARİTASI (KLASÖR YAPISI)

```text
FS2026_VCU_Software/
│
├── Inc/   (INCLUDE Klasörü - Sadece Tanımlamalar ve Kurallar)
│   │
│   ├── FS2026_CAN_Dictionary.h  ---> [ARACIN DİLİ] Arabadaki tüm cihazların birbiriyle nasıl 
│   │                                 konuştuğunu, Hangi CAN ID'sinin ne işe yaradığını içerir.
│   │
│   ├── moving_average_filter.h  ---> [SENSÖR FİLTRESİ] Gürültülü sensör verilerini temizler.
│   │
│   ├── can_parser_buffer.h      ---> [SD KART BUFFER] Verileri diske yazmadan önce RAM'de biriktirir.
│   │
│   └── state_machine.h          ---> [DURUM KİTAPÇIĞI] Aracın hangi durumlarda (INIT, DRIVING vb.)
│                                     olabileceğini listeleyen sözlüktür.
│
├── Src/   (SOURCE Klasörü - Asıl Algoritmalar ve Düşünen Kodlar)
│   │
│   ├── state_machine.c          ---> [BEYİN / KARAR MEKANİZMASI] Aracın kontağını çevirdiğiniz andan 
│   │                                 itibaren FS kurallarını (EV 4.12 vb.) denetleyen, 
│   │                                 Buzzer öttüren ve arabayı süren asıl yerdir.
│   │
│   ├── moving_average_filter.c  ---> [FİLTRE KODLARI] Matematiksel hareketli ortalama algoritması.
│   │
│   └── can_parser_buffer.c      ---> [SD FORMATLAYICI] Telemetri paketlerini CSV formatına çevirir.
│
└── README.md                    ---> (Şu an okuduğunuz harita)
```

---

## 🔍 KOD OKUMA REHBERİ (Yeni Başlayanlar İçin)

Takıma yeni katılan bir mühendisseniz, kodların içinde devasa bloklar halinde **FS KURALLARI** yorum satırları göreceksiniz. Biz kod yazarken yarışma kitapçığındaki kuralları doğrudan kodun içine gömdük.

Örneğin `Src/state_machine.c` dosyasını açarsanız, şöyle bloklar göreceksiniz:

```c
// =========================================================================
// FS KURALI: EV 4.12.3 (RTD SESLİ UYARI - BUZZER)
// Sürüş moduna geçmeden hemen önce, etraftaki mekanikerleri uyarmak için
// 1 saniye ile 3 saniye arası kesintisiz zil (buzzer) çalmalıdır.
// =========================================================================
```

Bu sayede kodun sadece bir "yazılım" olmadığını, aslında arabanın fiziksel bir kuralını işlettiğini rahatça anlayabilirsiniz.

Aracı geliştirirken her yeni özellik eklendiğinde (Tork Hesaplama, Torque Vectoring vb.) önce `Inc/` klasörüne kuralı, sonra `Src/` klasörüne asıl matematiğini yazacağız.
