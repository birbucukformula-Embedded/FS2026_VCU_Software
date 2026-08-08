#include "sd_file_system.h"
#include "can_parser_buffer.h" /* dosyayi ilk kez actigimizda CSV basligini buradan alacagiz */

#include <stdio.h>
#include <string.h>

/* =========================================================================================
 * GERCEK DONANIMDA (STM32 + FatFs) BU KATMAN NASIL OLACAKTI?
 * -----------------------------------------------------------------------------------------
 * Su an elimizde gercek bir SD kart / SPI hatti / FatFs kutuphanesi (ff.c, ff.h, diskio.c)
 * yok. O yuzden asagidaki fonksiyonlari, PC'de test edebilmek icin standart C dosya
 * fonksiyonlariyla (fopen/fwrite/fclose) SAHTE (mock) bir "SD kart" gibi calisacak sekilde
 * yazdim. Amac, FATFS API'sinin akisini (mount -> open -> write -> sync -> close) birebir
 * ayni sirayla ogrenmek. Kart gercekten takildiginda asagidaki karsilik gelen satirlar
 * degisecek, fonksiyonlarin disaridan gorunumu (imzalari) AYNI kalacak:
 *
 *   Bizim mock kod                  ->  Gercek STM32 HAL + FatFs karsiligi
 *   -------------------------------------------------------------------------------
 *   SD_Logger_Init()                ->  HAL_SPI_Init(&hspi1) + f_mount(&fs, "", 1)
 *                                        (diskio.c icinde disk_initialize() SPI ile
 *                                        SD karta CMD0/CMD8/CMD58 komutlarini gonderir)
 *   SD_Logger_OpenCSV(filename)     ->  f_open(&file, filename, FA_OPEN_ALWAYS | FA_WRITE)
 *                                        + f_lseek(&file, f_size(&file))  // dosya sonuna git
 *   SD_Logger_Write(data, len)      ->  f_write(&file, data, len, &bytesWritten)
 *                                        (diskio.c icinde disk_write() CS pinini LOW yapip
 *                                        HAL_SPI_TransmitReceive ile 512 baytlik blok gonderir)
 *   SD_Logger_Sync()                ->  f_sync(&file)
 *   SD_Logger_Close()               ->  f_close(&file)
 *
 * SPI pinleri (arastirma notlarinda da yazdigim gibi): MOSI/MISO/SCK veri hatti, CS
 * (Chip Select) ise SD karta "simdi seninle konusuyorum" demek icin LOW'a cekilen pin.
 * ========================================================================================= */

/* Mock "SD kart" aslinda bu isimde bir dosya olacak (calisma dizininde) */
static FILE* s_file_handle = NULL;
static bool  s_mounted     = false;

/* Egitim amacli: gercek bir SD kart gibi "dolabilecek" bir kapasite simule ediyoruz.
 * Gercek kartta bu deger GB seviyesindedir, biz test icin kucuk tuttuk. */
#define MOCK_SD_MAX_BYTES   (1024UL * 1024UL) /* 1 MB */
static uint32_t s_total_bytes_written = 0;

static SD_Logger_Status_t s_last_status = SD_LOGGER_OK;

bool SD_Logger_Init(void) {
    /* Gercekte burada SPI baslatilir ve karta CMD0 gonderilip cevap beklenir.
     * Bizde "mount" islemi = calisma dizinine dosya acip yazabildigimizi test etmek. */
    FILE* test_fp = fopen("sd_mount_test.tmp", "w");
    if (test_fp == NULL) {
        s_mounted    = false;
        s_last_status = SD_LOGGER_ERR_NOT_MOUNTED;
        return false;
    }

    fclose(test_fp);
    remove("sd_mount_test.tmp"); /* test dosyasini temizle, sadece yazilabilirlik kontroluydu */

    s_mounted    = true;
    s_last_status = SD_LOGGER_OK;
    return true;
}

bool SD_Logger_OpenCSV(const char* filename) {
    if (!s_mounted) {
        s_last_status = SD_LOGGER_ERR_NOT_MOUNTED;
        return false;
    }

    if (filename == NULL) {
        s_last_status = SD_LOGGER_ERR_FILE_NOT_OPENED;
        return false;
    }

    /* Dosya daha once var miydi diye once "r" ile deniyoruz. Bu bilgiyi, birazdan
     * CSV basligini yazip yazmayacagimizi anlamak icin kullanacagiz. */
    bool file_already_exists = false;
    FILE* check_fp = fopen(filename, "r");
    if (check_fp != NULL) {
        file_already_exists = true;
        fclose(check_fp);
    }

    /* "a" = append modu: dosya yoksa olusturur, varsa imleci sona koyar.
     * Gercek FATFS'te bu FA_OPEN_ALWAYS | FA_WRITE + f_lseek(sona git) ile yapiliyor. */
    s_file_handle = fopen(filename, "a");
    if (s_file_handle == NULL) {
        s_last_status = SD_LOGGER_ERR_FILE_NOT_OPENED;
        return false;
    }

    /* Dosya ilk kez olusturuluyorsa CSV basligini en basa yaz (2. Kisinin fonksiyonu) */
    if (!file_already_exists) {
        const char* header = CAN_GetCSVHeader();
        fwrite(header, sizeof(char), strlen(header), s_file_handle);
    }

    s_last_status = SD_LOGGER_OK;
    return true;
}

bool SD_Logger_Write(const char* data, size_t len) {
    if (s_file_handle == NULL) {
        s_last_status = SD_LOGGER_ERR_FILE_NOT_OPENED;
        return false;
    }

    if (data == NULL || len == 0) {
        s_last_status = SD_LOGGER_ERR_WRITE_FAILED;
        return false;
    }

    /* Disk dolu senaryosunu simule ediyoruz, gercek kartta bu f_write'in FR_DENIED /
     * disk doluluk hatasi donmesine karsilik gelir. */
    if (s_total_bytes_written + (uint32_t)len > MOCK_SD_MAX_BYTES) {
        s_last_status = SD_LOGGER_ERR_DISK_FULL;
        return false;
    }

    size_t written = fwrite(data, sizeof(char), len, s_file_handle);
    if (written != len) {
        s_last_status = SD_LOGGER_ERR_WRITE_FAILED;
        return false;
    }

    s_total_bytes_written += (uint32_t)len;
    s_last_status = SD_LOGGER_OK;
    return true;
}

bool SD_Logger_Sync(void) {
    if (s_file_handle == NULL) {
        s_last_status = SD_LOGGER_ERR_SYNC_FAILED;
        return false;
    }

    /* fflush = FATFS'teki f_sync karsiligi: RAM'deki dosya tamponunu fiziksel diske iter */
    if (fflush(s_file_handle) != 0) {
        s_last_status = SD_LOGGER_ERR_SYNC_FAILED;
        return false;
    }

    s_last_status = SD_LOGGER_OK;
    return true;
}

bool SD_Logger_Close(void) {
    if (s_file_handle == NULL) {
        /* Zaten kapali sayilir, hata degil */
        return true;
    }

    int result = fclose(s_file_handle);
    s_file_handle = NULL;

    if (result != 0) {
        s_last_status = SD_LOGGER_ERR_SYNC_FAILED;
        return false;
    }

    s_last_status = SD_LOGGER_OK;
    return true;
}

SD_Logger_Status_t SD_Logger_GetLastStatus(void) {
    return s_last_status;
}

const char* SD_Logger_GetStatusString(SD_Logger_Status_t status) {
    switch (status) {
        case SD_LOGGER_OK:
            return "OK: Islem basarili.";
        case SD_LOGGER_ERR_NOT_MOUNTED:
            return "HATA: SD kart mount edilemedi (kart takili degil).";
        case SD_LOGGER_ERR_FILE_NOT_OPENED:
            return "HATA: CSV dosyasi acilamadi/olusturulamadi.";
        case SD_LOGGER_ERR_DISK_FULL:
            return "HATA: SD kartta bos alan kalmadi.";
        case SD_LOGGER_ERR_WRITE_FAILED:
            return "HATA: Fiziksel yazma islemi basarisiz oldu.";
        case SD_LOGGER_ERR_SYNC_FAILED:
            return "HATA: f_sync/f_close basarisiz oldu (kart cikarilmis olabilir).";
        default:
            return "HATA: Bilinmeyen durum kodu.";
    }
}
