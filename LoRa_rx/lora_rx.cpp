#include "PicoHal.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <RadioLib.h>
#include <stdio.h>

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
volatile bool receivedFlag = false;

// Crear la instancia de PicoHal con los parámetros correctos
PicoHal hal(spi0, PIN_MISO, PIN_MOSI, PIN_SCK);

// Crear el módulo SX1262 usando la instancia de PicoHal
SX1262 radio = SX1262(new Module(&hal, PIN_CS, PIN_IRQ, PIN_RST, PIN_GPIO_IRQ));



void setFlag(void)
{
  // Cuando se recibe, se genera una interrupción
  receivedFlag = true;
}

int main()
{
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
    printf("La radio no pudo conectarse. Reintentar más tarde.\n");
    while (true)
    {
      sleep_ms(50);
    }
  }
  // Si hay una interrupción porque llega algo
  radio.setPacketReceivedAction(setFlag);
  printf("Empezando a escuchar...");
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
    if (receivedFlag)
    {
      size_t tamanio = radio.getPacketLength(true);
      char buffer[tamanio + 1];
      buffer[tamanio] = '\0';
      int state = radio.readData((uint8_t *)buffer, tamanio);
      receivedFlag = false;
      if (state == RADIOLIB_ERR_NONE)
      {
        printf("Paquete recibido: %s\n", buffer);
        printf("RSSI recibido: %f dBm\t", radio.getRSSI());
        printf("Relación SNR: %f dB\t", radio.getSNR());
        printf("Error en frecuencia: %f Hz\n", radio.getFrequencyError());
      }
      else if (state == RADIOLIB_ERR_CRC_MISMATCH)
      {
        // el paquete se recibió pero incorrecto
        printf("CRC error!");
      }
      else
      {
        printf("Falló, codigo: %i", state);
      }
    }
  }
}