#include "PicoHal.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <RadioLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

#include "lora_tx.h"
#include <strings.h>

// Definir pines de SPI para la Raspberry Pi Pico
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_RST 3
#define PIN_IRQ 2
#define PIN_GPIO_IRQ 10

int transmissionState = RADIOLIB_ERR_NONE;
volatile bool transmittedFlag = false;

// Crear la instancia de PicoHal con los parámetros correctos
PicoHal hal(spi0, PIN_MISO, PIN_MOSI, PIN_SCK);

// Crear el módulo SX1262 usando la instancia de PicoHal
SX1262 radio = SX1262(new Module(&hal, PIN_CS, PIN_IRQ, PIN_RST, PIN_GPIO_IRQ));

// message
msg_uplink message;
uint8_t buffer[sizeof(msg_uplink)];
int sequence = 0;

void setFlag(void)
{
  // Bandera para determinar si se envió un paquete
  transmittedFlag = true;
}


int main()
{ 
  // Initial seed
  srand (time(NULL));

  // SF, CR, ETC
  //configure_radio()
  
  // Fill the header message
  message.hdr.sequence =  sequence++;
  message.hdr.devEUI = 12345678;
  message.hdr.type = TYPE_MSG_UPLINK_UNCONFIRMED;

  // Read the sensor CO
  strcpy(message.payload.CO_sensor_id,"hola1");
  message.payload.CO_sensor_id[5] = '\0';
  message.payload.CO_data = rand() % (MAX_SHORT-1);
  message.payload.CO_hum =  rand() % (MAX_SHORT-1);
  message.payload.CO_temp = rand() % (MAX_SHORT-1);
  message.payload.CO_ppb =  MIN_INT + rand() / (RAND_MAX / (MAX_INT - (MIN_INT) + 1) + 1);

  // Read the sensor NO2
  strcpy(message.payload.NO2_sensor_id,"hola2");
  message.payload.NO2_sensor_id[5] = '\0';
  message.payload.NO2_data = rand() % (MAX_SHORT-1);
  message.payload.NO2_hum =  rand() % (MAX_SHORT-1);
  message.payload.NO2_temp = rand() % (MAX_SHORT-1);
  message.payload.NO2_ppb = MIN_INT + rand() / (RAND_MAX / (MAX_INT - (MIN_INT) + 1) + 1);
  
  // Read the sensor O3
  strcpy(message.payload.O3_sensor_id,"hola3");
  message.payload.O3_sensor_id[5] = '\0';
  message.payload.O3_data = rand() % (MAX_SHORT-1);
  message.payload.O3_hum =  rand() % (MAX_SHORT-1);
  message.payload.O3_temp = rand() % (MAX_SHORT-1);
  message.payload.O3_ppb =  MIN_INT + rand() / (RAND_MAX / (MAX_INT - (MIN_INT) + 1) + 1);
  
  // Read the sensor SO2
  strcpy(message.payload.SO2_sensor_id,"hola4");    
  message.payload.SO2_sensor_id[5] = '\0';
  message.payload.SO2_data = rand() % (MAX_SHORT-1);
  message.payload.SO2_hum =  rand() % (MAX_SHORT-1);
  message.payload.SO2_temp = rand() % (MAX_SHORT-1);
  message.payload.SO2_ppb =  MIN_INT + rand() / (RAND_MAX / (MAX_INT - (MIN_INT) + 1) + 1);

  message.tail.mic = rand() % MAX_INT;
  stdio_init_all();
  // Se incia la radio. Si no enciende adecuadamente, se bloquea...
  printf("iniciando radio...");
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    printf("Radio encendida\n");
  }
  else
  {
    printf("La radio no pudo conectarse. Reintentar más tarde. Código: %i \n", state);
    while (true)
    {
      sleep_ms(50);
    }
  }
  
  // Si hay una interrupción porque llega algo
  radio.setPacketSentAction(setFlag);
  memcpy(buffer,&message,sizeof(message));
  printf("Transmitiendo %zu bytes\n", sizeof(message));
  for (int i = 0; i < sizeof(message); i++) {
    printf("%02X ", buffer[i]);
  }
  transmissionState = radio.startTransmit(buffer, sizeof(message));
  
  while (true)
  {
      if (transmittedFlag)
      { 
        transmittedFlag = false;
        if (transmissionState == RADIOLIB_ERR_NONE)
        {
          printf("Paquete: %d enviado\n",  message.hdr.sequence =  sequence++);
        }
        else
        {
          printf("Algo falló, código: %i", transmissionState);
        }
          
        // Mostrar los datos
        printf("Node ID: %d\n", message.hdr.devEUI );
        printf("\tSecuencia: %d\n", message.hdr.sequence);
        printf("\tTipo: %d\n", message.hdr.type);
        printf("\tSensor CO: %s, %u, %u, %u, %d\n", message.payload.CO_sensor_id, message.payload.CO_data, message.payload.CO_temp,message.payload.CO_hum,message.payload.CO_ppb);
        printf("\tSensor NO2: %s, %u, %u, %u, %d\n", message.payload.NO2_sensor_id, message.payload.NO2_data, message.payload.NO2_temp,message.payload.NO2_hum,message.payload.NO2_ppb);
        printf("\tSensor O3: %s, %u, %u, %u, %d\n", message.payload.O3_sensor_id, message.payload.O3_data, message.payload.O3_temp,message.payload.O3_hum,message.payload.O3_ppb);
        printf("\tSensor SO2: %s, %u, %u, %u, %d\n", message.payload.SO2_sensor_id, message.payload.SO2_data, message.payload.SO2_temp,message.payload.SO2_hum,message.payload.SO2_ppb);
        printf("\tchecksum:%d\n",message.tail.mic);

        memcpy(buffer,&message,sizeof(message));
        printf("Payload (hex): ");
        for (int i = 0; i < sizeof(message); i++) {
          printf("%02X ", buffer[i]);
        }
        printf("\n");
        printf("sizeof(message)=%d\n",sizeof(message));
        printf("Tamaño real de estructura: %zu\n", sizeof(msg_uplink));
        transmissionState = radio.startTransmit(buffer, sizeof(message));

        // Asegurar que la tx ha finalizado
        sleep_ms(2000);
        radio.finishTransmit();

        // Esperar para tx el siguiente paquete
        sleep_ms(1000);    
        bzero(buffer,sizeof(msg_uplink));
        
      }
  }
}