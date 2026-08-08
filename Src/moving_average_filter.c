#include "moving_average_filter.h"
#include <string.h>

void MovingAverage_Init(MovingAverageFilter_t* filter) {
    if (filter == NULL) {
        return;
    }

    /* Diziyi ve sayaçları sıfırla. memset kullanmak elle for döngüsü yazmaktan daha hızlı/temiz. */
    memset(filter->buffer, 0, sizeof(filter->buffer));
    filter->index = 0;
    filter->count = 0;
    filter->sum   = 0.0f;
}

float MovingAverage_Update(MovingAverageFilter_t* filter, float new_sample) {
    if (filter == NULL) {
        return 0.0f;
    }

    /* DAİRESEL (CIRCULAR) BUFFER MANTIĞI:
     * Dizi hep ayni boyutta kalir (MA_FILTER_WINDOW_SIZE). Yeni ornek geldiginde,
     * o an "en eski" olan elemanin (filter->index'in gosterdigi yer) uzerine yaziyoruz.
     * Boylece dizinin tamamini her seferinde kaydirmaya gerek kalmiyor (kaydirma O(N)
     * olurdu, bizim yontemimiz O(1)). */

    /* Once o hucrede duran ESKI degeri toplamdan cikar (artik gecerli degil) */
    filter->sum -= filter->buffer[filter->index];

    /* Yeni degeri o hucreye yaz ve toplama ekle */
    filter->buffer[filter->index] = new_sample;
    filter->sum += new_sample;

    /* Indexi bir sonraki hucreye tasi; sona gelince basa don (dairesel) */
    filter->index = (uint8_t)((filter->index + 1) % MA_FILTER_WINDOW_SIZE);

    /* Buffer henuz tam dolmadiysa (baslangicta) count'u arttir, doluysa N'de sabit kalir */
    if (filter->count < MA_FILTER_WINDOW_SIZE) {
        filter->count++;
    }

    /* Ortalama = toplam / dolu eleman sayisi (baslangicta N'den az olabilecegi icin count kullaniyoruz) */
    return filter->sum / (float)filter->count;
}
