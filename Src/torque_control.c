#include "torque_control.h"
#include "vehicle_config.h"

// 1. Tork Haritası (Deadzone Kontrolü ve Yüzde Dönüşümü)
static int16_t TC_ApplyTorqueMap(uint8_t appsPercent) {
    if (appsPercent <= CFG_APPS_DEADZONE_PERCENT) {
        return 0; // Ayak titremesi / pedala hafif dokunma bölgesi
    }
    
    // Yüzdeyi 0-100'den torka çevir (Doğrusal Harita)
    // Örnek: CFG_MOTOR_MAX_TORQUE_NM = 230 Nm
    // (appsPercent - Deadzone) kullanarak ölü bölge bittiği an yavaşça artmasını sağlıyoruz.
    uint8_t activeRange = 100 - CFG_APPS_DEADZONE_PERCENT;
    uint8_t effectiveApps = appsPercent - CFG_APPS_DEADZONE_PERCENT;
    
    int16_t requestedTorque = (effectiveApps * CFG_MOTOR_MAX_TORQUE_NM) / activeRange;
    return requestedTorque;
}

// 2. Rejeneratif Frenleme (Motoru jeneratör olarak kullanarak pili şarj etme)
static int16_t TC_ApplyRegen(const VCU_Inputs_t *inputs) {
    if (!CFG_REGEN_ENABLED) {
        return 0;
    }

    // Regen Şartları: 
    // 1. Gaz pedalı ölü bölgede olmalı (Ayak gazdan çekilmiş)
    // 2. Araç hareket halinde olmalı (Şu an IMU/Hız verimiz yok, o yüzden sadece gaza bakıyoruz)
    // Not: Gerçekte araç hızı < 5 km/h ise regen yapılmaz, motor kilitlenmesin diye.
    
    if (inputs->appsPercent <= CFG_APPS_DEADZONE_PERCENT) {
        // Maksimum regen gücünü hesapla (Örn: 230 Nm'nin %20'si = 46 Nm frenleme)
        int16_t maxRegenTorque = (CFG_MOTOR_MAX_TORQUE_NM * CFG_REGEN_MAX_PERCENT) / 100;
        
        // Şimdilik sabit bir regen uyguluyoruz, ileride fren basıncına orantılı (Brake Blending) yapılabilir.
        return -maxRegenTorque; // Eksi değer motoru yavaşlatır
    }
    
    return 0;
}

// 3. Güvenlik ve Limit Kontrolleri (Sıcaklık ve SOC)
static int16_t TC_ApplySafetyLimits(int16_t rawTorque, const VCU_Inputs_t *inputs) {
    (void)inputs; // Henüz kullanılmıyor, ileride eklenecek
    int16_t limitedTorque = rawTorque;
    
    // NOT: Bu blok BMS'ten sıcaklık ve SOC verisi gelmeye başladığında doldurulacaktır.
    // Şimdilik örnek bir kısıtlama gösterimi (Eğer veriler dışarıdan geliyorsa):
    
    /* 
    // 1. Pil çok sıcaksa gücü yarıya indir (Derating)
    if (inputs->bmsMaxCellTemp > CFG_BATTERY_TEMP_LIMIT) {
        limitedTorque = limitedTorque / 2;
    }
    
    // 2. Pil bitmek üzereyse aracı süründür (Limp Mode)
    if (inputs->bmsSocPercent < CFG_BATTERY_SOC_CRITICAL) {
        limitedTorque = (limitedTorque * 25) / 100; // Gücü %25'e düşür
    }
    */
    
    return limitedTorque;
}

// =========================================================================
// ANA TORK HESAPLAMA FONKSİYONU
// =========================================================================
int16_t TC_CalculateTorque(const VCU_Inputs_t *inputs) {
    int16_t finalTorque = 0;
    
    // 1. Gaza basılıyorsa Tork Haritasını çalıştır
    if (inputs->appsPercent > CFG_APPS_DEADZONE_PERCENT) {
        finalTorque = TC_ApplyTorqueMap(inputs->appsPercent);
    } 
    // 2. Gaza basılmıyorsa Rejeneratif Frenlemeyi çalıştır
    else {
        finalTorque = TC_ApplyRegen(inputs);
    }
    
    // 3. En son elde edilen torku güvenlik limitlerinden geçir (Sıcaklık/Pil kısıtlamaları)
    finalTorque = TC_ApplySafetyLimits(finalTorque, inputs);
    
    // 4. Son Torku Inverter'e gönder
    return finalTorque;
}
