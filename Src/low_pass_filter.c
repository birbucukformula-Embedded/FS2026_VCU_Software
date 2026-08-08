#include "low_pass_filter.h"
#include <stddef.h>

void LowPass_Init(LowPassFilter_t* filter, float alpha) {
    if (filter == NULL) {
        return;
    }

    filter->alpha       = alpha;
    filter->last_output = 0.0f;
    filter->initialized = false; /* henuz ilk ornek gelmedi */
}

float LowPass_Update(LowPassFilter_t* filter, float new_sample) {
    if (filter == NULL) {
        return 0.0f;
    }

    if (!filter->initialized) {
        /* Ilk ornekte "Y_eski" diye bir sey yok, o yuzden ilk ciktiyi direkt
         * ilk ham ornege esitliyoruz. Aksi halde filtre 0'dan baslayip
         * gercek degere ulasana kadar yanlis bir sicrama/gecikme yasardi. */
        filter->last_output = new_sample;
        filter->initialized = true;
        return filter->last_output;
    }

    /* README'deki formulun birebir kod karsiligi: Y_yeni = (A * X_yeni) + ((1-A) * Y_eski) */
    float new_output = (filter->alpha * new_sample) + ((1.0f - filter->alpha) * filter->last_output);

    filter->last_output = new_output; /* bir sonraki cagri icin Y_eski'yi guncelle */
    return new_output;
}
