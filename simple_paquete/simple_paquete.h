#ifndef __LORA_RX_RPI_H__
#define __LORA_RX_RPI_H__
#include <stdint.h>

typedef struct {
    uint8_t sensor_id;
    uint32_t packet_number;
    int16_t temperature;
    int32_t co;
    int32_t so;
    int32_t o3;
    int32_t no2;
    uint16_t battery_1_raw;
    uint16_t battery_2_raw;
    uint16_t pm_1;
    uint16_t pm_2;
    uint16_t pm_3;
    uint16_t valid;
} sensor_packet_t;

#define SIMPLE_PKT_LEN 35

#define MASK_ERROR_TEMP 0x0080
#define MASK_ERROR_CO 0x0040
#define MASK_ERROR_SO 0x0020
#define MASK_ERROR_O3 0x0010
#define MASK_ERROR_NO2 0x0008
#define MASK_ERROR_BAT1 0x0004
#define MASK_ERROR_BAT2 0x0002
#define MASK_ERROR_PM1 0x0001
#define MASK_ERROR_PM2 0x0100
#define MASK_ERROR_PM3 0x0200

void parse_sensor_packet(const uint8_t *data, sensor_packet_t *pkt);

void serialize_sensor_packet(const sensor_packet_t *pkt, uint8_t *data_out);

void print_sensor_packet(const sensor_packet_t* pkt);

#endif

