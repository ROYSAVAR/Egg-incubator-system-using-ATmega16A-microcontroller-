#include "DHT11.h"
#include <util/delay.h>

#define DHT11_PIN 2 // Cambia esto al pin al que has conectado el DHT11

static uint8_t DHT11_receive_data(void);

void DHT11_init(uint8_t pin) {
    // Configura el pin como salida
    DDRD |= (1 << pin);
}

uint8_t DHT11_read(void) {
    // Envía señal de inicio
    PORTD |= (1 << DHT11_PIN);
    _delay_ms(18);

    // Cambia el pin a entrada
    DDRD &= ~(1 << DHT11_PIN);

    // Espera respuesta
    _delay_us(20);

    // Verifica si el sensor responde correctamente
    if (!(PIND & (1 << DHT11_PIN))) {
        _delay_us(80);
        if (PIND & (1 << DHT11_PIN)) {
            _delay_us(80);
            return DHT11_OK;
        }
    }

    return DHT11_ERROR_TIMEOUT;
}

float DHT11_get_temperature(void) {
    // Leer los datos del sensor
    uint8_t data[5];
    uint8_t i;
    for (i = 0; i < 5; i++) {
        data[i] = DHT11_receive_data();
    }

    // Calcular y devolver la temperatura
    return (float)data[2];
}

float DHT11_get_humidity(void) {
    // Leer los datos del sensor
    uint8_t data[5];
    uint8_t i;
    for (i = 0; i < 5; i++) {
        data[i] = DHT11_receive_data();
    }

    // Calcular y devolver la humedad
    return (float)data[0];
}

static uint8_t DHT11_receive_data(void) {
    uint8_t result = 0;
    uint8_t i;
    for (i = 0; i < 8; i++) {
        // Espera que el pin se ponga en alto
        while (!(PIND & (1 << DHT11_PIN)));

        // Espera un tiempo para determinar el bit
        _delay_us(30);

        // Si el pin sigue siendo alto, es un 1
        if (PIND & (1 << DHT11_PIN)) {
            result |= (1 << (7 - i));
        }

        // Espera que el pin vuelva a ser bajo
        while (PIND & (1 << DHT11_PIN));
    }
    return result;
}
