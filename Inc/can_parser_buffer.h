/**
 * @file      can_parser_buffer.h
 * @brief     2. KİŞİNİN GÖREVİ: CAN Simülasyonu, CSV Formatlama ve Sector Buffer Yönetimi
 * @details   Bu kütüphane, sensör/CAN verilerinin oluşturulması, formatlanması ve SD kart ömrünün
 *            korunması için RAM tampon (buffer) mimarisini uygular.
 * 
 *            TAKIM İÇİ GÖREV DAĞILIMI:
 *            - 2. KİŞİ (BU MODÜL): 
 *              1) Hız, sıcaklık, hata kodu üreten sanal CAN verilerini tanımlar (CAN_DataFrame_t).
 *              2) Veriyi "Timestamp,Speed,Temp,ErrorCode\r\n" formatlı CSV satırına çevirir.
 *              3) Mikrodenetleyiciyi yormamak ve SD kart flash hücrelerini aşındırmamak için
 *                 veriyi her seferinde yazmaz; 512 Baytlık (1 sektör) RAM buffer'da biriktirir.
 *              4) Buffer dolunca 1. Kişinin yazdığı SD_Logger_Write() fonksiyonunu tetikler.
 *            - 1. KİŞİ (sd_file_system): FATFS dosya sistemiyle ve donanımsal yazma işleriyle ilgilenir.
 * 
 * @note      Bu dosya, 1. ve 2. kişinin entegrasyonu için ortak veri modellerini ve buffer API'sini sunar.
 */

#ifndef CAN_PARSER_BUFFER_H
#define CAN_PARSER_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "vehicle_config.h"
#include "telemetry.h"
#include "state_machine.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================================
 * SABİTLER VE YAPILANMA TANIMLARI (2. KİŞİNİN SORUMLULUK ALANI)
 * ========================================================================================= */

/**
 * @brief  SD Kart Sektör Boyutu (512 Byte) - Ömrü Koruma Stratejisi
 * @details Neden 512 Byte?
 *          SD kartların yerleşik Flash kontrolcüsü en küçük "sektör/blok" birimi olarak
 *          512 baytlık bloklarla çalışır. Her küçük mesajda SD karta yazmak, kartın 512 baytlık
 *          hücreyi silip tekrar yazmasına (Wear-Out) sebep olur ve SPI hattını meşgul eder.
 *          Bu sebeple verileri 512 Bayta ulaşana dek RAM'de biriktirip tek seferde yazarız.
 */
#define CAN_BUFFER_SECTOR_SIZE      (CFG_SD_SECTOR_SIZE)

/**
 * @brief  CSV Dosyası Başlık Satırı (Header)
 * @details İlk kez dosya oluşturulduğunda en üste eklenmesi gereken standart sütun başlıkları.
 */
#define CAN_CSV_HEADER_STRING       "Uptime(ms),State,Fault,Apps(%),Brake,Torque,RPM,Voltage,Current,SOC,MotorTemp,InvTemp,CellTemp,Flags\r\n"

/* =========================================================================================
 * 1. SANAL CAN VERİSİ ÜRETİMİ (SİMÜLASYON) FONKSİYONLARI
 * ========================================================================================= */

/**
 * @brief  Test ve geliştirme aşaması için sanal CAN telemetri verisi üretir.
 * @details Gerçek CAN hattı bağlı değilken sistemin test edilebilmesi için gerçekçi
 *          hız, sıcaklık ve arıza kodu verileri türetir (Rastgele veya rampa davranışı).
 * 
 * @param[out] packet Üretilen sanal verinin yazılacağı TelemetryPacket_t adresi.
 */
void CAN_SimulateData(TelemetryPacket_t* packet);

/**
 * @brief  Gerçek CAN hattından (veya simülatörden) gelen veriyi ayrıştırır.
 * @details CAN ID'ye göre gelen 8 baytlık veriyi (Payload) VCU_Inputs_t yapısına kaydeder.
 *          Böylece State Machine hep en güncel verilerle çalışır.
 * 
 * @param[in]  canId  Gelen mesajın CAN ID'si (Örn: 0x200 BMS, 0x300 Inverter)
 * @param[in]  data   Gelen 8 baytlık veri dizisi (Payload)
 * @param[in]  dlc    Veri uzunluğu (Data Length Code, genellikle 8)
 * @param[out] inputs VCU'nun ana girdi veri yapısı
 */
void CAN_Parse_Message(uint32_t canId, const uint8_t* data, uint8_t dlc, VCU_Inputs_t* inputs);

/**
 * @brief  Rear Node (Arka Düğüm) yerine geçerek sahte CAN verileri üretir.
 * @details Test ortamında (test_all.c veya masaüstü testinde) kullanılmak üzere
 *          BMS (0x200) ve Inverter (0x300) mesajlarını 8'er baytlık diziler halinde doldurur.
 * 
 * @param[out] bmsData 8 baytlık BMS veri dizisi
 * @param[out] invData 8 baytlık Inverter veri dizisi
 */
void CAN_Generate_Mock_RearNode_Data(uint8_t* bmsData, uint8_t* invData);

/* =========================================================================================
 * 2. CSV STRING FORMATLAMA FONKSİYONLARI
 * ========================================================================================= */

/**
 * @brief  CAN verisini CSV formatlı metin satırına çevirir (snprintf kullanarak).
 * @details Örnek Çıktı: "1050,5,0,25,40,1500,2000,4000,-10,95,65,55,45,15\r\n"
 * 
 * @param[in]  packet   Formatlanacak ham telemetri verisi.
 * @param[out] out_str  Oluşturulan CSV satırının yazılacağı karakter dizisi buffer'ı.
 * @param[in]  max_len  out_str tamponunun maksimum bayt uzunluğu (Taşmaları önlemek için).
 * @return     int      Oluşturulan string'in uzunluğu (< 0 ise formatlama hatası).
 */
int CAN_FormatCSV(const TelemetryPacket_t* packet, char* out_str, size_t max_len);

/**
 * @brief  CSV sütun başlığı (Header) satırını döndürür.
 * @details 1. Kişi dosyayı ilk açtığında en başa yazmak için bu fonksiyonu çağırabilir.
 * 
 * @return CSV başlığı karakter dizisi referansı.
 */
const char* CAN_GetCSVHeader(void);

/* =========================================================================================
 * 3. RAM SEKTÖR TAMPONU (BUFFER) YÖNETİM FONKSİYONLARI
 * ========================================================================================= */

/**
 * @brief  512 Baytlık RAM buffer sistemini sıfırlar ve başlatır.
 * @note   main() içindeki başlatma fazında bir kez çağrılmalıdır.
 */
void CAN_Buffer_Init(void);

/**
 * @brief  Yeni bir CAN paketini CSV satırına çevirip 512 baytlık RAM tamponuna ekler.
 * @details KRİTİK NOKTA (Entegrasyon):
 *          - Gelen veri RAM buffer'a (512 Baytlık alan) kopyalanır.
 *          - Eğer yeni satır eklendiğinde buffer dolarsa (veya 512 bayta ulaşılırsa),
 *            bu fonksiyon OTOMATİK OLARAK 1. Kişinin yazdığı `SD_Logger_Write()` fonksiyonunu 
 *            çağırıp buffer'ı boşaltır ve kart ömrünü korur.
 * 
 * @param[in] packet Tampona eklenecek telemetri verisi.
 * @return    true  : Veri tampona başarıyla eklendi (veya dolduysa SD karta başarıyla yazıldı).
 * @return    false : SD karta yazma aşamasında 1. Kişinin modülü hata döndürdü.
 */
bool CAN_Buffer_Push(const TelemetryPacket_t* packet);

/**
 * @brief  Tamponda (Buffer) bekleyen, henüz 512 bayta ulaşmamış kalan verileri zorla SD karta yazar.
 * @details Sürüş bittiğinde, kontak kapatılırken veya log dosyasını kapatmadan (SD_Logger_Close)
 *          hemen önce tamponda kalan son baytların kaybolmaması için çağrılmalıdır.
 * 
 * @return true  : Tampondaki tüm veriler başarıyla diske yazıldı ve tampon sıfırlandı.
 * @return false : SD karta yazma hatası meydana geldi.
 */
bool CAN_Buffer_Flush(void);

/**
 * @brief  Şu an RAM tamponunda kaç bayt verinin yazılmayı beklediğini döndürür (Debug/İstatistik için).
 * @return Tampondaki anlık bayt sayısı (0 ile 512 arası).
 */
uint32_t CAN_Buffer_GetPendingBytes(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_PARSER_BUFFER_H */
