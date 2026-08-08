#include "can_parser_buffer.h"
#include "sd_file_system.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* =========================================================================================
 * 2. KİŞİ GÖREVİ: CAN VERİ SİMÜLASYONU, CSV FORMATLAMA VE SEKTÖR BUFFER YÖNETİMİ
 * ========================================================================================= */

/* 512 Baytlık Sektör Buffer (RAM) ve imleç indisi */
static char     s_sector_buffer[CAN_BUFFER_SECTOR_SIZE];
static uint32_t s_buffer_idx = 0;

/* Simülasyon durum değişkenleri */
static uint32_t s_sim_timestamp_ms = 0;

void CAN_SimulateData(TelemetryPacket_t* packet) {
    if (packet == NULL) return;

    /* Zaman damgası her çağrıda 10 ms (100 Hz CAN paketi) ilerlesin */
    s_sim_timestamp_ms += 10;
    packet->uptimeMs = s_sim_timestamp_ms;

    /* Örnek test simülasyon değerleri */
    packet->vehicleState = 5; // DRIVING
    packet->faultCode = 0;
    
    // Gaz pedalı rampa simülasyonu
    static int apps = 0;
    apps = (apps + 1) % 100;
    packet->appsPercent = (uint8_t)apps;
    
    packet->brakePressure = 0;
    packet->torqueCommand = 1500;
    packet->motorRPM = 2000;
    packet->batteryVoltage = 4000;
    packet->batteryCurrent = -10;
    packet->batterySOC = 95;
    packet->motorTemp = 65;
    packet->inverterTemp = 55;
    packet->maxCellTemp = 45;
    packet->systemFlags = 0x0F;
}

int CAN_FormatCSV(const TelemetryPacket_t* packet, char* out_str, size_t max_len) {
    if (packet == NULL || out_str == NULL || max_len == 0) {
        return -1;
    }

    int len = snprintf(out_str, max_len, "%lu,%u,%u,%u,%u,%d,%d,%u,%d,%u,%u,%u,%u,%u\r\n",
                       (unsigned long)packet->uptimeMs,
                       packet->vehicleState,
                       packet->faultCode,
                       packet->appsPercent,
                       packet->brakePressure,
                       packet->torqueCommand,
                       packet->motorRPM,
                       packet->batteryVoltage,
                       packet->batteryCurrent,
                       packet->batterySOC,
                       packet->motorTemp,
                       packet->inverterTemp,
                       packet->maxCellTemp,
                       packet->systemFlags);

    if (len < 0 || (size_t)len >= max_len) {
        return -1; /* Formatlama hatası veya tampon taşması */
    }

    return len;
}

const char* CAN_GetCSVHeader(void) {
    return CAN_CSV_HEADER_STRING;
}

void CAN_Buffer_Init(void) {
    memset(s_sector_buffer, 0, sizeof(s_sector_buffer));
    s_buffer_idx = 0;
    s_sim_timestamp_ms = 0;
}

bool CAN_Buffer_Push(const TelemetryPacket_t* packet) {
    if (packet == NULL) {
        return false;
    }

    char line_buf[128];
    int line_len = CAN_FormatCSV(packet, line_buf, sizeof(line_buf));
    if (line_len <= 0) {
        return false;
    }

    /* Yeni satır eklendiğinde 512 baytı aşacaksa mevcut tamponu 1. Kişinin modülüyle SD karta yaz */
    if (s_buffer_idx + (uint32_t)line_len > CAN_BUFFER_SECTOR_SIZE) {
        bool write_ok = SD_Logger_Write(s_sector_buffer, s_buffer_idx);
        if (!write_ok) {
            return false;
        }
        /* Tampon diske yazıldığı için sıfırlıyoruz */
        s_buffer_idx = 0;
    }

    /* Yeni CSV satırını RAM Sektör Buffer'a kopyala */
    memcpy(&s_sector_buffer[s_buffer_idx], line_buf, (size_t)line_len);
    s_buffer_idx += (uint32_t)line_len;

    /* Eğer tampon tam 512 bayt olduysa anında diske yaz */
    if (s_buffer_idx == CAN_BUFFER_SECTOR_SIZE) {
        bool write_ok = SD_Logger_Write(s_sector_buffer, s_buffer_idx);
        if (!write_ok) {
            return false;
        }
        s_buffer_idx = 0;
    }

    return true;
}

bool CAN_Buffer_Flush(void) {
    bool ok = true;

    /* Eğer tamponda henüz 512 bayta ulaşmamış veri varsa zorla diske yaz */
    if (s_buffer_idx > 0) {
        ok = SD_Logger_Write(s_sector_buffer, s_buffer_idx);
        s_buffer_idx = 0;
    }

    /* Fiziksel senkronizasyon (f_sync) tetiklenir */
    if (ok) {
        ok = SD_Logger_Sync();
    }

    return ok;
}

uint32_t CAN_Buffer_GetPendingBytes(void) {
    return s_buffer_idx;
}
