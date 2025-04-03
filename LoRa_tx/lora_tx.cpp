#include "PicoHal.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <RadioLib.h>
#include <stdio.h>

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

void setFlag(void)
{
  // Bandera para determinar si se envió un paquete
  transmittedFlag = true;
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
    printf("La radio no pudo conectarse. Reintentar más tarde. Código: %i \n", state);
    while (true)
    {
      sleep_ms(50);
    }
  }
  
  // Si hay una interrupción porque llega algo
  radio.setPacketSentAction(setFlag);
  printf("Enviando primer paquete...\n");
  transmissionState = radio.startTransmit("Primer paquete");
  //Enviando primer paquete
  int contador = 0;


  while (true)
  {
      if (transmittedFlag)
      { 
        transmittedFlag = false;
        if (transmissionState == RADIOLIB_ERR_NONE)
        {
          printf("Paquete enviado\n");
        }
        else
        {
          printf("Algo falló, código: %i", transmissionState);
        }

        sleep_ms(2000);
        char str[50];  // Buffer para la cadena
        sprintf(str, "Paquete #%d", contador++);
        transmissionState = radio.startTransmit(str);
        // Limpio el buffer luego de la transmisión
        // Previamente necesito que haya pasado un tiempo para que el paquete se haya enviado
        sleep_ms(1000);
        radio.finishTransmit();
        
      }
  }
}