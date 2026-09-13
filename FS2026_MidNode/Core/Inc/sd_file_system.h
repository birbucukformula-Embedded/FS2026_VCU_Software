/**
 * @file      sd_file_system.h
 * @brief     1. KİŞİNİN GÖREVİ: SD Kart ve FATFS Dosya Sistemi Yönetimi (Backend Modülü)
 * @details   Bu kütüphane, gömülü sistemlerde SD kart üzerinde fiziksel dosya işlemlerini (FATFS)
 *            sarmalayan (wrapper) yapıları içerir.
 * 
 *            TAKIM İÇİ GÖREV DAĞILIMI:
 *            - 1. KİŞİ (BU MODÜL): SD kartın donanımsal başlatılması (f_mount), dosyanın açılması (f_open),
 *              verilerin fiziki olarak yazılması (f_write/f_sync) ve hata durumlarının denetlenmesinden sorumludur.
 *            - 2. KİŞİ (can_parser_buffer): CAN verisini üretir, CSV string formata çevirir ve 
 *              SD kart ömrünü korumak için 512 baytlık RAM buffer'da biriktirdikten sonra
 *              buradaki SD_Logger_Write() fonksiyonunu çağırır.
 * 
 * @note      Bu arayüz (Interface), 1. ve 2. kişi arasında yapılan prototip anlaşmasıdır.
 */

#ifndef SD_FILE_SYSTEM_H
#define SD_FILE_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SD Kart İşlem Durumu / Hata Yönetim Kodları
 * @details 1. Kişinin görevlerinden olan "Hata Yönetimi" için tanımlanmış durum bildirimleridir.
 *          SD kartın takılı olmaması, disk doluluğu veya dosya açma hataları buradan tespit edilir.
 */
typedef enum {
    SD_LOGGER_OK = 0,               /**< İşlem başarıyla tamamlandı */
    SD_LOGGER_ERR_NOT_MOUNTED,      /**< SD kart takılı değil veya mount (bağlama) başarısız oldu */
    SD_LOGGER_ERR_FILE_NOT_OPENED,  /**< CSV dosyası açılamadı veya oluşturulamadı */
    SD_LOGGER_ERR_DISK_FULL,        /**< SD kart diski doldu (Boş alan kalmadı) */
    SD_LOGGER_ERR_WRITE_FAILED,     /**< f_write işlemi sırasında fiziksel yazma hatası oluştu */
    SD_LOGGER_ERR_SYNC_FAILED       /**< f_sync işlemi başarısız (Kart çıkarılmış olabilir) */
} SD_Logger_Status_t;

/* =========================================================================================
 * 1. KİŞİNİN TEMEL SARMALAYICI (WRAPPER) FONKSİYON ARABİRİMİ
 * ========================================================================================= */

/**
 * @brief  SD kartı sistemde mount eder ve donanım denetimlerini yapar.
 * @note   Sistem açılışında (main.c içinde) ilk çağrılacak fonksiyondur.
 * 
 * @return true  : SD kart algılandı ve başarıyla mount edildi.
 * @return false : SD kart algılanamadı veya dosya sistemi bozuk (SD_LOGGER_ERR_NOT_MOUNTED).
 */
bool SD_Logger_Init(void);

/**
 * @brief  Belirtilen dosya adıyla .csv dosyası açar; dosya yoksa yeni oluşturur.
 * @details Dosya imleci dosyanın sonuna konumlandırılır (FA_OPEN_ALWAYS | FA_WRITE | FA_APPEND).
 * 
 * @param  filename Açılacak/oluşturulacak dosya adı (Örn: "telemetry_2026.csv").
 * @return true     : Dosya başarıyla açıldı/oluşturuldu ve yazmaya hazır.
 * @return false    : Dosya açılamadı veya disk tam kapasiteye ulaştı.
 */
bool SD_Logger_OpenCSV(const char* filename);

/**
 * @brief  Ham (raw) karakter verisini açık olan dosyaya yazar (f_write).
 * @details Bu fonksiyonu doğrudan her CAN mesajında çağırmak yerine, 2. Kişinin yazdığı
 *          buffer sistemi dolduğunda (örn. 512 byte olunca) blok yazma yapmak için kullanılır.
 * 
 * @param  data Yazılacak karakter verisi (String/CSV tamponu).
 * @param  len  Yazılacak bayt sayısı.
 * @return true  : Tüm baytlar başarıyla dosyaya yazıldı.
 * @return false : Yazma hatası veya disk dolma hatası meydana geldi.
 */
bool SD_Logger_Write(const char* data, size_t len);

/**
 * @brief  RAM'deki dosya sistemi önbelleğini (cache) fiziksel SD karta işler (f_sync).
 * @details Ani güç kesilmelerinde veri kaybını önlemek için belirli aralıklarla (örn. her 10 
 *          sektör yazımında veya tur sonunda) çağrılmalıdır.
 * 
 * @return true  : Veriler başarıyla fiziksel flash hücrelerine kaydedildi.
 * @return false : Eşzamanlama sırasında hata oluştu.
 */
bool SD_Logger_Sync(void);

/**
 * @brief  Açık dosyayı güvenli biçimde kapatır (f_close) ve işlemleri sonlandırır.
 * 
 * @return true  : Dosya güvenle kapatıldı.
 * @return false : Dosya kapatılırken hata oluştu.
 */
bool SD_Logger_Close(void);

/* =========================================================================================
 * YARDIMCI HATA YÖNETİM FONKSİYONLARI (1. KİŞİ EKLENTİSİ)
 * ========================================================================================= */

/**
 * @brief  Gerçekleşen son işlemin detaylı hata durumunu döndürür.
 * @return SD_Logger_Status_t tipinde son hata kodu.
 */
SD_Logger_Status_t SD_Logger_GetLastStatus(void);

/**
 * @brief  Hata kodunu okunabilir string mesaja çevirir (GUI, UART veya Debug çıktısı için).
 * @param  status Çevrilecek durum/hata kodu.
 * @return Hatanın metin açıklaması (Örn: "ERR: SD Card is not mounted!").
 */
const char* SD_Logger_GetStatusString(SD_Logger_Status_t status);

#ifdef __cplusplus
}
#endif

#endif /* SD_FILE_SYSTEM_H */
