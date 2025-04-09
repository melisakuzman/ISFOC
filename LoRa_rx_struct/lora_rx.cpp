#include "PicoHal.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <RadioLib.h>
#include <stdio.h>
#include "lora_tx.h"

// Definir pines de SPI para la Raspberry Pi Pico y la Radio
// Gitlab oficial:  https://github.com/jgromes/RadioLib
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_RST 3
#define PIN_IRQ 2
#define PIN_GPIO_IRQ 10

int transmissionState = RADIOLIB_ERR_NONE;
// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// Crear la instancia de PicoHal con los parámetros correctos
PicoHal hal(spi0, PIN_MISO, PIN_MOSI, PIN_SCK);

// Crear el módulo SX1262 usando la instancia de PicoHal
SX1262 radio = SX1262(new Module(&hal, PIN_CS, PIN_IRQ, PIN_RST, PIN_GPIO_IRQ));

// Rx vars
int i = 0;
int numBytes ;
int state;
int last_seq = -1;
// message
msg_uplink message;
uint8_t buffer[sizeof(msg_uplink)];

void setFlag(void)
{
  // Cuando se recibe, se genera una interrupción
  // we got a packet, set the flag
  receivedFlag = true;
}

int main()
{
  stdio_init_all();
  // Se incia la radio. Si no enciende adecuadamente, se bloquea...
  printf("Iniciando radio...\n");
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    printf("Radio encendida\n");
  }
  else
  {
    printf("La radio no pudo conectarse. Reintentar más tarde.\n");
    while (true) {sleep_ms(50); }
  }
  
  // set the function that will be called when new packet is received
  radio.setPacketReceivedAction(setFlag);
  printf("Empezando a escuchar...\n");
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE)
  {
    printf("Escuchando correctamente\n");
  }
  else
  {
    printf("La radio no pudo conectarse. Reintentar más tarde.\n");
    while (true)
    {
      sleep_ms(50);
    }
  }

  while (true)
  {
     // Check if the flag is set
    if(receivedFlag) {
      receivedFlag = false;
    }
 
    // Leer los datos
    numBytes = radio.getPacketLength(true);
    state = radio.readData (buffer, numBytes);
    if (state == RADIOLIB_ERR_NONE) 
    {
     
      memcpy(&message,buffer,numBytes); //#sizeof(msg_uplink)
      if (message.hdr.sequence == last_seq) { continue; }
      printf("Num bytes recibidos: %d\n", numBytes);  

      // Mostrar los datos
      printf("Node ID: %d\n", message.hdr.devEUI );
      printf("\tSecuencia: %d\n", last_seq = message.hdr.sequence);
      printf("\tTipo: %d\n", message.hdr.type);
      printf("\tSensor CO: %s, %u, %u, %u, %d\n", message.payload.CO_sensor_id, message.payload.CO_data, message.payload.CO_temp,message.payload.CO_hum,message.payload.CO_ppb);
      printf("\tSensor NO2: %s, %u, %u, %u, %d\n", message.payload.NO2_sensor_id, message.payload.NO2_data, message.payload.NO2_temp,message.payload.NO2_hum,message.payload.NO2_ppb);
      printf("\tSensor O3: %s, %u, %u, %u, %d\n", message.payload.O3_sensor_id, message.payload.O3_data, message.payload.O3_temp,message.payload.O3_hum,message.payload.O3_ppb);
      printf("\tSensor SO2: %s, %u, %u, %u, %d\n", message.payload.SO2_sensor_id, message.payload.SO2_data, message.payload.SO2_temp,message.payload.SO2_hum,message.payload.SO2_ppb);
      printf("\tchecksum:%d\n",message.tail.mic);
      printf("Payload (hex): ");
      for (int i = 0; i < numBytes; i++) {
        printf("%02X ", buffer[i]);
      }
      printf("\n");
      
      // Imprimir parámetros calidad
      printf("RSSI recibido: %f dBm\t", radio.getRSSI());
      printf("Relación SNR: %f dB\t", radio.getSNR());
      printf("Error en frecuencia: %f Hz\n", radio.getFrequencyError());

      bzero(buffer,sizeof(msg_uplink));
      numBytes = 0;
    }
    else{
      if (state == RADIOLIB_ERR_CRC_MISMATCH)
      {
        // el paquete se recibió pero incorrecto
        printf("CRC error!\n");
      }else
      {
         printf("Falló, codigo: %i\n", state);
      }
    }
  }
}