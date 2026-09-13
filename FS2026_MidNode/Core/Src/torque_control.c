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
    // 3. Sert fren yapılmamalı (Tekerlek kilitlenmesini ve spin'i önlemek için)
    if (inputs->brakePressure > CFG_REGEN_BRAKE_CUTOFF_VAL) {
        return 0; // Çok sert fren yapılıyor, motor frenini iptal et sadece diskler tutsun
    }

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
    
    // Eğer anlık akım sınırın (Örn: 480A) üstüne çıktıysa torku agresifçe kıs
    if (inputs->tsCurrent > CFG_CURRENT_DERATE_THRESHOLD_A) {
        // Ne kadar aştıysak (Örn: 490 - 480 = 10A), torktan o kadar çok keselim
        int16_t overCurrent = inputs->tsCurrent - CFG_CURRENT_DERATE_THRESHOLD_A;
        int16_t reduction = overCurrent * CFG_CURRENT_DERATE_FACTOR; // Her 1A fazlalık için tork kıs
        limitedTorque -= reduction;
    }

    // 2. Güç Limiti (EV 2.2.1: 80kW)
    // Güç (Watt) = Voltaj (V) * Akım (A)
    uint32_t currentPowerW = (uint32_t)inputs->bmsVoltage * (uint32_t)inputs->tsCurrent;
    
    // Eğer anlık güç sınırın (Örn: 78.000W) üstüne çıktıysa torku kıs
    if (currentPowerW > CFG_POWER_DERATE_THRESHOLD_W) {
        // Ne kadar aştıysak (Örn: 79000 - 78000 = 1000W), torktan kes
        uint32_t overPower = currentPowerW - CFG_POWER_DERATE_THRESHOLD_W;
        int16_t reduction = overPower / CFG_POWER_DERATE_FACTOR; // Her belli güç fazlalığı için tork kıs
        limitedTorque -= reduction;
    }

    // Kısma işlemi negatif torka sebep olmasın (Araç geri gitmesin)
    if (limitedTorque < 0 && rawTorque > 0) {
        limitedTorque = 0;
    }
    
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
    
    // 3. Güvenlik ve Limit Kontrolleri (Güç, Akım, Pil Sıcaklığı)
    finalTorque = TC_ApplySafetyLimits(finalTorque, inputs);
    
    // =========================================================================
    // FS KURALI: EV 2.2.4 - Ters Dönüş (Geri Vites) Engeli
    // Motor sürücüye giden komut aracı geriye hareket ettirmemelidir.
    // Rejeneratif frenleme negatif tork üretir. Ancak araç dururken veya çok yavaşken
    // (Örn: < 5 km/h) negatif tork uygulanırsa araç geri geri gitmeye başlar.
    // Bunu engellemek için düşük hızlarda negatif tork (Regen) kapatılır.
    // =========================================================================
    if (finalTorque < 0 && inputs->vehicleSpeedKmh < 5) {
        finalTorque = 0; // Araç duruyorsa geri gitmesini engelle
    }

    // 4. Son Torku Inverter'e gönder
    return finalTorque;
}
