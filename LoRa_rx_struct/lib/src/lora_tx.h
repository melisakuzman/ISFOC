#ifndef LORA_TX_H
#define LORA_TX_H

#define MAX_SEQUENCE 4294967296
#define MAX_INT      (2147483648-1)
#define MIN_INT      (-MAX_INT+1)
#define MAX_SHORT    65536

// Type of message
#define TYPE_MSG_UPLINK_CONFIRMED         0
#define TYPE_MSG_DOWNLINK_CONFIRMED       1
#define TYPE_MSG_UPLINK_UNCONFIRMED         2
#define TYPE_MSG_DOWNLINK_UNCONFIRMED       3
#define TYPE_MSG_JOINREQUEST                4
#define TYPE_MSG_JOINACCEPT                 5

struct header {
    uint32_t devEUI;     
    uint32_t sequence;
    uint8_t type;
};

struct payload_uplink  {
    //  CO sensor
    char CO_sensor_id[12];
    int32_t CO_ppb;         //[-2^31:2^31]
    uint16_t CO_data;       //[0:2^16]
    uint16_t CO_temp;       //[0:2^16]
    uint16_t CO_hum;        //[0:2^16]
    // NO2
    char NO2_sensor_id[12];
    int32_t NO2_ppb;        //[-2^31:2^31]
    uint16_t NO2_data;
    uint16_t NO2_temp;
    uint16_t NO2_hum;
    // O3 
    char O3_sensor_id[12];
    int32_t O3_ppb;         //[-2^31:2^31]
    uint16_t O3_data;
    uint16_t O3_temp;
    uint16_t O3_hum;
    // SO2
    char SO2_sensor_id[12];
    int32_t SO2_ppb;        //[-2^31:2^31]
    uint16_t SO2_data;
    uint16_t SO2_temp;
    uint16_t SO2_hum;
};

struct checksum {
    int32_t mic;
};

struct pkt_unconfirmed_uplink{
    struct header hdr;
    struct payload_uplink payload;
    struct checksum tail;
};
typedef struct pkt_unconfirmed_uplink msg_uplink;
#endif