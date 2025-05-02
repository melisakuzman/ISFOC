#include <RadioLib.h>
#include <hal/RPi/PiHal.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include "simple_paquete.h"

#define PIN_CS 8
#define PIN_IRQ 17 //DIO
#define PIN_RST 22
#define PIN_GPIO_IRQ 27 //BUSY

int transmissionState = RADIOLIB_ERR_NONE;
volatile bool receivedFlag = false;
volatile bool running = true;


// Handler para SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nSe recibió Ctrl+C (SIGINT). Saliendo con seguridad...\n");
    running = false;
}

// Canal SPI0
PiHal *hal = new PiHal(0);

// Crear el módulo SX1262 usando la instancia de PiHal
// now we can create the radio module
// pinout corresponds to the Waveshare LoRaWAN Hat
// NSS pin:   8 //CEO_N
// DIO1 pin:  2
// NRST pin:  4
// BUSY pin:  3

SX1262 radio = SX1262(new Module(hal, PIN_CS, PIN_IRQ, PIN_RST, PIN_GPIO_IRQ));

void setFlag(void)
{
  // Cuando se recibe, se genera una interrupción
  receivedFlag = true;
}

int main(int argc, char **argv)
{
  
  size_t len = 0;
  sensor_packet_t pkt;
  FILE *flog;
  char timestamp[20];  // Espacio para el timestamp
    
  if (argc != 2) {
    printf("Uso: lora_rx [LOG FILE]\n");
    return 2;
  }
  
  // Crear y abrir el fichero
  flog = fopen(argv[1], "w");
  if (flog == NULL) {
        perror("No se pudo crear el fichero");
        return 3;
  }
  
  signal(SIGINT, handle_sigint);

  
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
   fclose(flog);
   return(1);
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
    fclose(flog);
    return(1);
  }

  while (running)
  {
    if (receivedFlag)
    {
      len = radio.getPacketLength(true);
      uint8_t buffer[len];
      int state = radio.readData(buffer, len);
      receivedFlag = false;
      if (state != RADIOLIB_ERR_NONE)
      {
        printf("Paquete recibido erróneo\n");
        if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            printf("CRC error!");
        }
        else {
            printf("Falló, codigo: %i", state);
        }
        continue;
      }
      if (len != SIMPLE_PKT_LEN) {
          printf("Tamaño difiere del esperado, se descarta. %d\n",len);
          continue;
      }
      
      parse_sensor_packet(buffer,&pkt);

      fprintf(flog,"%ld;%u;%u;%u;%d;%d;%d;%d;%u;%u;%u;%u;%u;%u\n",time(NULL),pkt.sensor_id,pkt.packet_number,pkt.temperature,pkt.co,pkt.so,pkt.o3,pkt.no2,pkt.battery_1_raw,pkt.battery_2_raw,pkt.pm_1,pkt.pm_2,pkt.pm_3,pkt.valid);
      fflush(flog);
      
      printf("Sensor ID: %u\nNúmero de paquete: %u\n",pkt.sensor_id,pkt.packet_number);
    
    	printf("Tem: %.2f ºC RAW: %u \n", (float)pkt.temperature/100.0f, pkt.temperature);
          
    	printf("CO: %d\n", pkt.co);
    	printf("SO: %d\n", pkt.so);
    	printf("O3: %d\n", pkt.o3);
    	printf("NO2: %d\n", pkt.no2);

    	printf("Batería 1: %.2f V RAW: %u\n", pkt.battery_1_raw / 4095.0f * 3.3f,pkt.battery_1_raw);
    	printf("Batería 2: %.2f V RAW: %u\n", pkt.battery_2_raw / 4095.0f * 3.3f,pkt.battery_2_raw);
    	printf("PM1: %u\n", pkt.pm_1);
      printf("PM2: %u\n", pkt.pm_2);
      printf("PM3: %u\n", pkt.pm_3);
      printf("Valid: %u\n", pkt.valid);
 
      printf("RSSI recibido: %f dBm\t", radio.getRSSI());
      printf("Relación SNR: %f dB\t", radio.getSNR());
      printf("Error en frecuencia: %f Hz\n", radio.getFrequencyError());
   }
 }
 fclose(flog);
 return 0;
}

