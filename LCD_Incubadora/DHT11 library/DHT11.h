#ifndef DHT11_H_
#define DHT11_H_

#include <avr/io.h>

#define DHT11_OK            0
#define DHT11_ERROR_CHECKSUM       1
#define DHT11_ERROR_TIMEOUT        2

void DHT11_init(uint8_t pin);
uint8_t DHT11_read(void);
float DHT11_get_temperature(void);
float DHT11_get_humidity(void);

#endif /* DHT11_H_ */
