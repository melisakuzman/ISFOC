#include "simple_paquete.h"
#include <stdio.h>

void parse_sensor_packet(const uint8_t *data, sensor_packet_t *pkt) {

    int offset = 0;

    pkt->sensor_id = data[offset];
    offset += 1;

    pkt->packet_number = (data[offset] << 24) | (data[offset+1] << 16) |
                         (data[offset+2] << 8)  | data[offset+3];
    offset += 4;

    // Temperatura (float)
    uint16_t temp_raw = (data[offset] << 8) | (data[offset+1]);
    pkt->temperature = temp_raw;
    offset += 2;

    pkt->co = (data[offset] << 24) | (data[offset+1] << 16) |
              (data[offset+2] << 8)  | data[offset+3];
    offset += 4;

    pkt->so = (data[offset] << 24) | (data[offset+1] << 16) |
              (data[offset+2] << 8)  | data[offset+3];
    offset += 4;

    pkt->o3 = (data[offset] << 24) | (data[offset+1] << 16) |
              (data[offset+2] << 8)  | data[offset+3];
    offset += 4;

    pkt->no2 = (data[offset] << 24) | (data[offset+1] << 16) |
               (data[offset+2] << 8)  | data[offset+3];
    offset += 4;

    pkt->battery_1_raw = (data[offset] << 8) | data[offset+1];
    offset += 2;

    pkt->battery_2_raw = (data[offset] << 8) | data[offset+1];
    offset += 2;

    pkt->pm_1 =  (data[offset] << 8)  | data[offset+1];
    offset += 2;

    pkt->pm_2 =  (data[offset] << 8)  | data[offset+1];
    offset += 2;

    pkt->pm_3 =  (data[offset] << 8)  | data[offset+1];
    offset += 2;

    pkt->valid =  (data[offset] << 8)  | data[offset+1];
}

void serialize_sensor_packet(const sensor_packet_t *pkt, uint8_t *data_out) {
    int offset = 0;

    // sensor_id
    data_out[offset++] = pkt->sensor_id & 0xFF;

    // packet_number
    data_out[offset++] = (pkt->packet_number >> 24) & 0xFF;
    data_out[offset++] = (pkt->packet_number >> 16) & 0xFF;
    data_out[offset++] = (pkt->packet_number >> 8)  & 0xFF;
    data_out[offset++] = pkt->packet_number & 0xFF;

    // temperatura (float)
    //union { uint32_t i; float f; } conv;
    //conv.f = pkt->temperature;
    //data_out[offset++] = (conv.i >> 24) & 0xFF;
    //data_out[offset++] = (conv.i >> 16) & 0xFF;
    data_out[offset++] = (pkt->temperature >> 8)  & 0xFF;
    data_out[offset++] = pkt->temperature & 0xFF;

    // CO
    data_out[offset++] = (pkt->co >> 24) & 0xFF;
    data_out[offset++] = (pkt->co >> 16) & 0xFF;
    data_out[offset++] = (pkt->co >> 8)  & 0xFF;
    data_out[offset++] = pkt->co & 0xFF;

    // SO
    data_out[offset++] = (pkt->so >> 24) & 0xFF;
    data_out[offset++] = (pkt->so >> 16) & 0xFF;
    data_out[offset++] = (pkt->so >> 8)  & 0xFF;
    data_out[offset++] = pkt->so & 0xFF;

    // O3
    data_out[offset++] = (pkt->o3 >> 24) & 0xFF;
    data_out[offset++] = (pkt->o3 >> 16) & 0xFF;
    data_out[offset++] = (pkt->o3 >> 8)  & 0xFF;
    data_out[offset++] = pkt->o3 & 0xFF;

    // NO2
    data_out[offset++] = (pkt->no2 >> 24) & 0xFF;
    data_out[offset++] = (pkt->no2 >> 16) & 0xFF;
    data_out[offset++] = (pkt->no2 >> 8)  & 0xFF;
    data_out[offset++] = pkt->no2 & 0xFF;

    // Batería 1 (uint16_t)
    data_out[offset++] = (pkt->battery_1_raw >> 8) & 0xFF;
    data_out[offset++] = pkt->battery_1_raw & 0xFF;

    // Batería 2
    data_out[offset++] = (pkt->battery_2_raw >> 8) & 0xFF;
    data_out[offset++] = pkt->battery_2_raw & 0xFF;

    // PM1
    data_out[offset++] = (pkt->pm_1 >> 8)  & 0xFF;
    data_out[offset++] = pkt->pm_1 & 0xFF;

    // PM2
    data_out[offset++] = (pkt->pm_2 >> 8)  & 0xFF;
    data_out[offset++] = pkt->pm_2 & 0xFF;

    //PM3
    data_out[offset++] = (pkt->pm_3 >> 8)  & 0xFF;
    data_out[offset++] = pkt->pm_3 & 0xFF;    

    // Valid
    data_out[offset++] = (pkt->valid >> 8)  & 0xFF;
    data_out[offset++] = pkt->valid & 0xFF;
}

void print_sensor_packet(const sensor_packet_t* pkt) {
    printf("Sensor Packet:\n");
    printf("  Sensor ID: %u\n", pkt->sensor_id);
    printf("  Packet #: %u\n", pkt->packet_number);
    printf("  Temp: %.2f C\n", pkt->temperature/100.0f);
    printf("  CO: %d ppb\n", pkt->co);
    printf("  SO: %d ppb\n", pkt->so);
    printf("  O3: %d ppb\n", pkt->o3);
    printf("  NO2: %d ppb\n", pkt->no2);
    printf("  Battery 1: %u\n", pkt->battery_1_raw);
    printf("  Battery 2: %u\n", pkt->battery_2_raw);
    printf("  PM0.5: %u ug/m3\n", pkt->pm_1);
    printf("  PM1.0: %u ug/m3\n", pkt->pm_2);
    printf("  PM2.5: %u ug/m3\n", pkt->pm_3);
    printf("  Validez: %u\n", pkt->valid);
}

/*
sensor_packet_t pkt = {
    .sensor_id = 0x12345678,
    .packet_number = 42,
    .temperature = 23.5f,
    .co = 120,
    .so = 80,
    .o3 = 60,
    .no2 = 45,
    .battery_1_raw = 3500,
    .battery_2_raw = 3400,
    .pm = 215
    uint8_t buffer[36];
    serialize_sensor_packet(&pkt, buffer);
};*/


