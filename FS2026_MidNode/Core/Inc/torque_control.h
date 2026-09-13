#ifndef TORQUE_CONTROL_H
#define TORQUE_CONTROL_H

#include <stdint.h>
#include "state_machine.h"

/*===========================================================================*
 * FS2026 TORQUE CONTROL & DRIVING ALGORITHMS
 *===========================================================================*/

// Ana Tork Hesaplama Fonksiyonu
// Parametreler: VCU sensör verileri
// Dönüş: İnvertere gönderilecek nihai tork komutu (Nm)
int16_t TC_CalculateTorque(const VCU_Inputs_t *inputs);

#endif // TORQUE_CONTROL_H
