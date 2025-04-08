#include "PicoHal.h"
#include "hardware/pio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <RadioLib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardware/uart.h"
#include <stdarg.h>

// Definir pines de SPI para la Raspberry Pi Pico
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_RST 3
#define PIN_IRQ 2
#define PIN_GPIO_IRQ 10

#define UART0_ID uart0
#define UART1_ID uart1
#define BAUD_RATE 9600

#define UART0_TX_PIN 1
#define UART0_RX_PIN 0

#define UART1_TX_PIN 8
#define UART1_RX_PIN 9

#define UART2_TX_PIN 12
#define UART2_RX_PIN 13

#define UART3_TX_PIN 4
#define UART3_RX_PIN 5

#define BUF_LEN 128

char rx_buffer[BUF_LEN];
char str[BUF_LEN]; // Buffer para la cadena

int rx_index = 0;

int transmissionState = RADIOLIB_ERR_NONE;
volatile bool transmittedFlag = false;

volatile bool last_mes = 0;

// Crear la instancia de PicoHal con los parámetros correctos
PicoHal hal(spi0, PIN_MISO, PIN_MOSI, PIN_SCK);

// Crear el módulo SX1262 usando la instancia de PicoHal
SX1262 radio = SX1262(new Module(&hal, PIN_CS, PIN_IRQ, PIN_RST, PIN_GPIO_IRQ));

void append_to_string(char *dest, size_t dest_size, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  size_t len = strlen(dest);
  if (len < dest_size - 1)
  {
    vsnprintf(dest + len, dest_size - len, format, args);
  }

  va_end(args);
}

void procesarCadena(const char *input)
{
  char buffer[BUF_LEN];
  strncpy(buffer, input, sizeof(buffer));
  buffer[sizeof(buffer) - 1] = '\0'; // aseguramos terminación

  char *token = strtok(buffer, ",");
  if (token != NULL)
  {
    // printf("Primer dato: %s\n", token);
    append_to_string(str, sizeof(str), "%s,", token);
  }

  token = strtok(NULL, ",");
  if (token != NULL)
  {
    while (*token == ' ')
      token++; // limpiar espacios
    int segundoDato = atoi(token);
    // printf("Segundo dato: %d\n", segundoDato);
    append_to_string(str, sizeof(str), "%d,", segundoDato);
  }
}

void uart0_rx_callback()
{
  while (uart_is_readable(UART0_ID))
  {
    char c = uart_getc(UART0_ID);
    if (c == '\n')
    {
      rx_buffer[rx_index] = '\0'; // cerrar la cadena
      printf("%s\n", rx_buffer);
      if (last_mes == 1)
      {
        procesarCadena(rx_buffer);
      }
      rx_index = 0; // reiniciar para el próximo mensaje
    }
    else
    {
      if (rx_index < BUF_LEN - 1)
      {
        rx_buffer[rx_index++] = c;
      }
    }
  }
}

void uart1_rx_callback()
{
  while (uart_is_readable(UART1_ID))
  {
    char c = uart_getc(UART1_ID);
    if (c == '\n')
    {
      rx_buffer[rx_index] = '\0'; // cerrar la cadena
      printf("%s\n", rx_buffer);
      if (last_mes == 1)
      {
        procesarCadena(rx_buffer);
      }
      rx_index = 0; // reiniciar para el próximo mensaje
    }
    else
    {
      if (rx_index < BUF_LEN - 1)
      {
        rx_buffer[rx_index++] = c;
      }
    }
  }
}

void cambiar_uart(int newTx, int newRx, int oldTx, int oldRx, uart_inst_t *uart_iden)
{

  gpio_set_function(oldRx, GPIO_FUNC_NULL); // Liberar
  gpio_set_function(oldTx, GPIO_FUNC_NULL); // Liberar
  gpio_set_dir(oldRx, GPIO_IN);
  gpio_set_dir(oldTx, GPIO_IN);
  gpio_pull_up(oldRx);
  gpio_pull_up(oldTx);

  gpio_set_function(newTx, UART_FUNCSEL_NUM(uart_iden, newTx));
  gpio_set_function(newRx, UART_FUNCSEL_NUM(uart_iden, newRx));
}

// Función para setear todas las salidas de uart de Tx para dormir los sensores
void DGS2_dormirlos()
{
  // Es necesario definir diferente a este pin, para que luego tome al resto correctamente
  //  UART0 tiene problemas para usar dos pines en modo TX (lo cual es lógico....)
  cambiar_uart(UART0_TX_PIN, UART0_RX_PIN, UART2_TX_PIN, UART2_RX_PIN, UART0_ID);
  gpio_set_function(UART1_TX_PIN, UART_FUNCSEL_NUM(UART1_ID, UART1_TX_PIN));
  gpio_set_function(UART2_TX_PIN, UART_FUNCSEL_NUM(UART0_ID, UART2_TX_PIN));
  gpio_set_function(UART3_TX_PIN, UART_FUNCSEL_NUM(UART1_ID, UART3_TX_PIN));
  sleep_ms(50);
  // Se envía el comando para que duerman todos los conectados a UART0 y UART1
  uart_puts(UART0_ID, "s");
  uart_puts(UART1_ID, "s");

  // hard_assert(rc == PICO_OK);
  // hard_assert(false); // should never get here!
  // return 0;
}

void setFlag(void)
{
  // Bandera para determinar si se envió un paquete
  transmittedFlag = true;
}

void configurar_uart(void)
{
  uart_init(UART0_ID, BAUD_RATE);
  uart_init(UART1_ID, BAUD_RATE);
  gpio_set_function(UART0_TX_PIN, UART_FUNCSEL_NUM(UART0_ID, UART0_TX_PIN));
  gpio_set_function(UART0_RX_PIN, UART_FUNCSEL_NUM(UART0_ID, UART0_RX_PIN));
  gpio_set_function(UART1_TX_PIN, UART_FUNCSEL_NUM(UART1_ID, UART1_TX_PIN));
  gpio_set_function(UART1_RX_PIN, UART_FUNCSEL_NUM(UART1_ID, UART1_RX_PIN));
  uart_set_irq_enables(UART0_ID, true, false);
  irq_set_exclusive_handler(UART0_IRQ, uart0_rx_callback);
  irq_set_enabled(UART0_IRQ, true);
  uart_set_irq_enables(UART1_ID, true, false);
  irq_set_exclusive_handler(UART1_IRQ, uart1_rx_callback);
  irq_set_enabled(UART1_IRQ, true);
}

void configurar_radio()
{
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
}

int main()
{
  stdio_init_all();
  configurar_uart();
  configurar_radio();
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
      for (int i = 0; i < 4; i++)
      {
        if (i == 3)
        {
          last_mes = true;
        }
        cambiar_uart(UART2_TX_PIN, UART2_RX_PIN, UART0_TX_PIN, UART0_RX_PIN, UART0_ID);
        cambiar_uart(UART3_TX_PIN, UART3_RX_PIN, UART1_TX_PIN, UART1_RX_PIN, UART1_ID);
        uart_puts(UART0_ID, "\r");
        sleep_ms(500);
        uart_puts(UART1_ID, "\r");
        sleep_ms(500);
        cambiar_uart(UART0_TX_PIN, UART0_RX_PIN, UART2_TX_PIN, UART2_RX_PIN, UART0_ID);
        cambiar_uart(UART1_TX_PIN, UART1_RX_PIN, UART3_TX_PIN, UART3_RX_PIN, UART1_ID);
        uart_puts(UART0_ID, "\r");
        sleep_ms(500);
        uart_puts(UART1_ID, "\r");
        sleep_ms(500);
      }
      sleep_ms(2000);
      // Le quito la última coma que no sirve de nada
      int index_2 = strlen(str);
      str[index_2 - 1] = '\0';
      // sprintf(str, "Sensores. Paquete #%d", contador++);
      transmissionState = radio.startTransmit(str);
      // Limpio el buffer luego de la transmisión
      // Previamente necesito que haya pasado un tiempo para que el paquete se haya enviado
      sleep_ms(1000);
      radio.finishTransmit();
      str[0] = '\0';
      last_mes = 0;
      DGS2_dormirlos();
      for (int j = 0; j < 10; j++)
      {
        sleep_ms(1000);
      }
    }
  }
}