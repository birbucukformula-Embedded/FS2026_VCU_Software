/**
 * @file      low_pass_filter.h
 * @brief     GÖREV 3: Alçak Geçiren Filtre (Low-Pass Filter / Exponential Moving Average)
 * @details   Formül (README'de verilen): Y_yeni = (A * X_yeni) + ((1 - A) * Y_eski)
 *
 *            - X_yeni : sensörden gelen yeni ham örnek
 *            - Y_eski : filtrenin bir önceki çıktısı
 *            - A (alpha) : 0 ile 1 arasında bir katsayı, filtrenin "ne kadar hızlı tepki
 *              vereceğini" belirler.
 *              alpha 1'e yakınsa -> filtre yeni veriye çok güvenir, hızlı tepki verir ama
 *                                    gürültüyü daha az temizler.
 *              alpha 0'a yakınsa -> filtre eski değere çok güvenir, gürültüyü çok temizler
 *                                    ama gerçek değişikliklere de yavaş tepki verir (gecikme).
 *
 *            Moving Average'dan farkı: geçmiş TÜM örnekleri (üstel olarak azalan ağırlıkla)
 *            hesaba katar ama sadece TEK bir önceki değeri (Y_eski) saklaması yeterlidir —
 *            Moving Average gibi 10 elemanlık dizi tutmaya gerek yoktur, bu yüzden RAM
 *            kullanımı çok daha azdır.
 */

#ifndef LOW_PASS_FILTER_H
#define LOW_PASS_FILTER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Alçak geçiren filtrenin durumunu (state) tutan yapı.
 */
typedef struct {
    float alpha;        /**< Filtre katsayısı (0.0 - 1.0 arası) */
    float last_output;  /**< Bir önceki çıktı (Y_eski) */
    bool  initialized;  /**< İlk örnek henüz gelmediyse true/false (ilk örnekte Y_eski
                              tanımlı olmadığından, ilk çıktıyı direkt ilk örneğe eşitleriz) */
} LowPassFilter_t;

/**
 * @brief  Filtreyi verilen alpha katsayısıyla sıfırlar (ilk kullanımdan önce çağrılmalı).
 * @param  filter Sıfırlanacak filtre yapısının adresi (pointer).
 * @param  alpha  Filtre katsayısı, 0.0 ile 1.0 arasında olmalı.
 */
void LowPass_Init(LowPassFilter_t* filter, float alpha);

/**
 * @brief  Filtreye yeni bir ham örnek ekler ve güncel (pürüzsüzleştirilmiş) çıktıyı döndürür.
 * @param  filter     Güncellenecek filtre yapısının adresi.
 * @param  new_sample Sensörden okunan yeni ham değer (X_yeni).
 * @return Y_yeni = (alpha * X_yeni) + ((1 - alpha) * Y_eski)
 */
float LowPass_Update(LowPassFilter_t* filter, float new_sample);

#ifdef __cplusplus
}
#endif

#endif /* LOW_PASS_FILTER_H */
