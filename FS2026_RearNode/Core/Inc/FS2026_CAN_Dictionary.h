#ifndef FS2026_CAN_DICTIONARY_H
#define FS2026_CAN_DICTIONARY_H

#include <stdint.h>
#include <stdbool.h>

/*===========================================================================*
 * FS2026 CAN BUS DICTIONARY (DBC EQUIVALENT)
 * Baud Rate: 500 kbps
 * Format: Standard 11-bit ID
 *===========================================================================*/

/* --- CAN MESSAGE IDs --- */
#define CAN_ID_VCU_CONTROL   0x100  // VCU -> INV
#define CAN_ID_VCU_STATUS    0x101  // VCU -> HMI, STM
#define CAN_ID_VCU_FAULTS    0x102  // VCU -> All
#define CAN_ID_FRONT_SENSORS 0x110  // FRONT -> VCU (APPS, Brake, Start)
#define CAN_ID_BMS_VOLT_CURR 0x200  // BMS -> VCU
#define CAN_ID_BMS_STATUS    0x201  // BMS -> VCU, HMI
#define CAN_ID_INV_DYNAMICS  0x300  // INV -> VCU, HMI
#define CAN_ID_INV_FAULTS    0x301  // INV -> VCU
#define CAN_ID_STM_IMU       0x400  // STM -> VCU

/*===========================================================================*
 * CAN PAYLOAD STRUCTURES
 * (Mapped directly to 8-byte CAN data frames)
 *===========================================================================*/

// ID 0x110: Front Sensors (Front Node to VCU)
typedef struct __attribute__((packed)) {
    uint8_t  apps1Percent;  // Byte 0: APPS 1 % (0-100)
    uint8_t  apps2Percent;  // Byte 1: APPS 2 % (0-100)
    uint8_t  brakePressure; // Byte 2: Brake pressure (0-255 bar)
    uint8_t  startButton;   // Byte 3: Start Button Status (1=Pressed)
    uint8_t  resetButton;   // Byte 4: Reset Button Status (1=Pressed)
    uint8_t  checksum;      // Byte 5: XOR checksum of Bytes 0-4
    uint8_t  reserved[2];   // Byte 6-7: Not used
} CAN_Front_Sensors_t;

// ID 0x100: VCU_Control (VCU to Inverter)
typedef struct __attribute__((packed)) {
    int16_t  torqueRequest; // Byte 0-1: Torque request (-32000 to +32000)
    uint8_t  inverterEnable;// Byte 2: 1 = Enable, 0 = Disable
    uint8_t  direction;     // Byte 3: 0 = Forward, 1 = Reverse
    uint32_t reserved;      // Byte 4-7: Not used
} CAN_VCU_Control_t;

// ID 0x101: VCU_Status (VCU to Network)
typedef struct __attribute__((packed)) {
    uint8_t stateMachine;   // Byte 0: 0=INIT, 1=LV_READY, 2=TS_ACTIVE, 3=RTD, 4=DRIVE, 6=FAULT
    uint8_t appsPercent;    // Byte 1: APPS % (0-100)
    uint8_t brakePressure;  // Byte 2: Brake pressure (0-255 bar)
    int8_t  steeringAngle;  // Byte 3: SAS Angle (-128 to 127 deg)
    uint8_t sdcStatus;      // Byte 4: SDC Closed (1=OK, 0=Broken)
    uint8_t activeFault;    // Byte 5: Fault Code (0=None, 1=Plausibility, etc)
    uint8_t reserved[2];    // Byte 6-7: Not used
} CAN_VCU_Status_t;

// ID 0x200: BMS_Volt_Curr (BMS to VCU)
typedef struct __attribute__((packed)) {
    uint16_t totalVoltage;  // Byte 0-1: HV Voltage (e.g. 600V * 10 = 6000)
    int16_t  totalCurrent;  // Byte 2-3: HV Current (Amps)
    uint8_t  reserved[4];   // Byte 4-7
} CAN_BMS_VoltCurr_t;

// ID 0x201: BMS_Status (BMS to VCU/HMI)
typedef struct __attribute__((packed)) {
    uint8_t socPercent;     // Byte 0: State of Charge (0-100%)
    uint8_t maxCellTemp;    // Byte 1: Max Cell Temp (Celsius)
    uint8_t bmsState;       // Byte 2: BMS Internal State
    uint8_t bmsFaultCode;   // Byte 3: Fault Code (0 = None)
    uint8_t reserved[4];    // Byte 4-7
} CAN_BMS_Status_t;

// ID 0x300: INV_Dynamics (Inverter to VCU/HMI)
typedef struct __attribute__((packed)) {
    int16_t  motorRPM;      // Byte 0-1: RPM
    uint8_t  motorTemp;     // Byte 2: Motor Temperature (C)
    uint8_t  inverterTemp;  // Byte 3: Inverter Temperature (C)
    uint16_t tsVoltage;     // Byte 4-5: TS Voltage (Inverter input voltage)
    uint8_t  reserved[2];   // Byte 6-7
} CAN_INV_Dynamics_t;

// ID 0x400: STM_IMU (Telemetry to VCU)
typedef struct __attribute__((packed)) {
    int16_t accelX;         // Byte 0-1: X Acceleration
    int16_t accelY;         // Byte 2-3: Y Acceleration
    int16_t yawRate;        // Byte 4-5: Z Yaw Rate
    uint8_t gpsFix;         // Byte 6: GPS Fix status (1=OK)
    uint8_t reserved;       // Byte 7
} CAN_STM_IMU_t;

#endif // FS2026_CAN_DICTIONARY_H
