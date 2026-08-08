/**
 * @file      moving_average_filter.h
 * @brief     GÖREV 3: Hareketli Ortalama (Moving Average) Filtresi
 * @details   Sensörden gelen gürültülü (titrek) ham verileri pürüzsüzleştirmek için
 *            son N örneği bir dizide tutup ortalamasını alan basit bir dijital filtre.
 *
 *            Neden işe yarar? Rastgele gürültü (noise) genelde gerçek değerin etrafında
 *            +/- rastgele sapar. Son N örneğin ortalamasını almak bu rastgele sapmaları
 *            birbirini götürecek şekilde (bir tanesi +2 sapmışsa diğeri -1.5 sapmış olabilir)
 *            iptal eder ve gerçek eğilimi (trend) ortaya çıkarır.
 */

#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pencere (window) boyutu: son kaç örneğin ortalaması alınacak.
 * @details README'de istenen "son 10 sensör verisi" kuralı buradan geliyor.
 */
#define MA_FILTER_WINDOW_SIZE   (10U)

/**
 * @brief Hareketli ortalama filtresinin durumunu (state) tutan yapı.
 * @details Bu struct'ı dairesel (circular) bir tampon gibi kullanıyoruz: dizi hep
 *          sabit boyutta (MA_FILTER_WINDOW_SIZE), yeni örnek geldiğinde en eski
 *          örnek olduğu yerin üzerine yazılır (index dönüp dolaşır).
 */
typedef struct {
    float    buffer[MA_FILTER_WINDOW_SIZE]; /**< Son N örneğin tutulduğu dairesel dizi */
    uint8_t  index;                          /**< Bir sonraki yazılacak dizi indeksi (0..N-1 arası döner) */
    uint8_t  count;                          /**< Şu ana kadar doldurulan örnek sayısı (baslangicta N'den az olabilir) */
    float    sum;                            /**< Buffer içindeki tüm elemanların toplamı (her seferinde topla-diye baştan
                                                   dönmemek icin guncel tutulur, boylece ortalama O(1) surede hesaplanir) */
} MovingAverageFilter_t;

/**
 * @brief  Filtreyi sıfırlar (ilk kullanımdan önce mutlaka çağrılmalı).
 * @param  filter Sıfırlanacak filtre yapısının adresi.
 * @note   Parametre olarak pointer (adres) alıyoruz, struct'ı kopyalamak yerine
 *         çağıranın kendi struct'ını doğrudan güncelliyoruz (embedded sistemlerde
 *         gereksiz kopyalama = gereksiz RAM/CPU kullanımı demek).
 */
void MovingAverage_Init(MovingAverageFilter_t* filter);

/**
 * @brief  Filtreye yeni bir ham örnek ekler ve güncel (pürüzsüzleştirilmiş) ortalamayı döndürür.
 * @param  filter     Güncellenecek filtre yapısının adresi.
 * @param  new_sample Sensörden okunan yeni ham değer (örn. voltaj, sıcaklık vb.).
 * @return Son (en fazla) MA_FILTER_WINDOW_SIZE örneğin ortalaması.
 */
float MovingAverage_Update(MovingAverageFilter_t* filter, float new_sample);

#ifdef __cplusplus
}
#endif

#endif /* MOVING_AVERAGE_FILTER_H */
