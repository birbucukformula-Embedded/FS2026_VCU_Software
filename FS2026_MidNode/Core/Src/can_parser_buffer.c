#include "can_parser_buffer.h"
#include "sd_file_system.h"
#include "FS2026_CAN_Dictionary.h"
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

void CAN_Parse_Message(uint32_t canId, const uint8_t* data, uint8_t dlc, VCU_Inputs_t* inputs) {
    if (inputs == NULL || data == NULL || dlc < 8) return;

    if (canId == 0x110) {
        // Front Node Mesajı (0x110) - Kritik Sensör Verileri
        CAN_Front_Sensors_t* frontData = (CAN_Front_Sensors_t*)data;
        
        // Checksum doğrulaması (Byte 5, T 11.9.2.d kuralı)
        uint8_t calculated_crc = frontData->apps1Percent ^ 
                                 frontData->apps2Percent ^ 
                                 frontData->brakePressure ^ 
                                 frontData->startButton ^ 
                                 frontData->resetButton;
                                 
        // Checksum yanlışsa paketi reddet (Zamanla Timeout hatasına düşer)
        if (calculated_crc != frontData->checksum) return;

        inputs->apps1Percent = frontData->apps1Percent;
        inputs->apps2Percent = frontData->apps2Percent;
        inputs->appsPercent = (inputs->apps1Percent + inputs->apps2Percent) / 2;
        
        inputs->brakePressure = frontData->brakePressure;
        inputs->startButtonPressed = (frontData->startButton == 1);
        inputs->resetButtonPressed = (frontData->resetButton == 1);

    } else if (canId == 0x200) {
        // BMS Mesajı (0x200)
        // Byte 0-1: Voltaj (0.1V çözünürlük, Big-Endian varsayalım)
        // Byte 2-3: Akım (0.1A çözünürlük)
        uint16_t bmsVolts = (data[0] << 8) | data[1];
        uint16_t bmsAmps  = (data[2] << 8) | data[3];

        inputs->bmsVoltage = bmsVolts / 10; // 0.1V -> V
        inputs->tsCurrent  = bmsAmps / 10;  // 0.1A -> A

    } else if (canId == 0x300) {
        CAN_INV_Dynamics_t* pInv = (CAN_INV_Dynamics_t*)data;
        inputs->tsVoltage = pInv->tsVoltage;
        inputs->vehicleSpeedKmh = (pInv->motorRPM * 60 * 157) / 400000; 
    }
}

void CAN_Generate_Mock_RearNode_Data(uint8_t* bmsData, uint8_t* invData) {
    if (bmsData == NULL || invData == NULL) return;

    // BMS Verisi Simülasyonu (ID: 0x200)
    // 395.5 V -> 3955 = 0x0F73
    bmsData[0] = 0x0F; bmsData[1] = 0x73;
    // 120.5 A -> 1205 = 0x04B5
    bmsData[2] = 0x04; bmsData[3] = 0xB5;
    // Diğer bytelar şimdilik boş
    bmsData[4] = 0; bmsData[5] = 0; bmsData[6] = 0; bmsData[7] = 0;

    // Inverter Verisi Simülasyonu (ID: 0x300)
    // TS Voltage: 395 V -> 0x018B
    invData[0] = 0x01; invData[1] = 0x8B;
    // Motor RPM: 2000 -> 0x07D0
    invData[2] = 0x07; invData[3] = 0xD0;
    // Diğer bytelar şimdilik boş
    invData[4] = 0; invData[5] = 0; invData[6] = 0; invData[7] = 0;
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
