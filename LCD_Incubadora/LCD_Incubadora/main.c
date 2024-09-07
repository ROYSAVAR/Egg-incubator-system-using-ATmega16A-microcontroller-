#define  F_CPU 2000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "DHT11.h"

#define DHT11_PIN 2

//PROTOTIPADO DE FUNCIONES PARA PODER UTILIZARLAS DESDE CUALQUIER "LUGAR"
uint8_t cero_en_bit(volatile uint8_t *LUGAR, uint8_t BIT);
uint8_t uno_en_bit(volatile uint8_t *LUGAR, uint8_t BIT);
void EscribeEEPROM(uint16_t direccion, uint8_t datos);
uint8_t LeeEEPROM(uint16_t direccion);

void USART_init(uint16_t ubrr);
void USART_Transmit(uint8_t data);
uint8_t USART_Receive(void);

void inicializar();
void inicializar_reloj();
float regla_3(uint16_t adc);

void botones_funcionalidades();
void programar_veces(uint8_t recibidos);

void prender_ventilador();
void apagar_ventilador();
void prender_motor();
void apagar_motor();
void prender_foco();
void apagar_foco();

void valores_EEPROM();

// DECLARACION DE VARIABLES GLOBALES
volatile uint16_t res = 0;
char buffer_temp[6] = "";
char buffer[20];
char buffer_dias[20];

uint8_t contador = 0;
uint8_t contador2 = 0;

// SENSORES
uint8_t contador3 = 200;
float temperatura;
float humedad;
char printbuff[10];

uint8_t ya_paso1 = 0;

uint8_t veces; 

volatile uint8_t motor_encendido = 0;
volatile uint16_t contador_motor = 0;

volatile uint8_t horas = 23, minutos = 59, segundos = 30, dias;
uint8_t otra = 1;
volatile uint8_t datos_recibidos = 0; // Hacerlo volatile ya que se modifica en ISR
float res_float;

int main(void)
{
	
	inicializar();
	//EscribeEEPROM(0b11, 0b1);	
	//dias = LeeEEPROM(0b11); // Leer valor de dias desde EEPROM al inicio
	dias = 0b00000000; 
	
	while (1)
	{
		inicializar_reloj();
		
		/*if (datos_recibidos != 0) // Verifica si hay datos recibidos
		{
			// Aquí puedes manejar los datos recibidos como desees
			lcd_gotoxy(13,2);
			lcd_putc(datos_recibidos);
			// Resetea la variable para la próxima recepción
			//datos_recibidos = 0;
		}*/
		contador3++;
		if (contador3 >= 100)
		{
			contador3 = 0;
			
			uint8_t status = DHT11_read(&temperatura, &humedad);
			if (status)
			{
				lcd_gotoxy(0, 0);
				lcd_puts("Humed. ");
				dtostrf(humedad, 2, 2, printbuff);
				lcd_puts(printbuff);
				lcd_puts(" %");
				
				lcd_gotoxy(0, 1);
				lcd_puts("Temp. ");
				dtostrf(temperatura, 2, 2, printbuff);
				lcd_puts(printbuff);
				lcd_puts(" %");
			}
		}
/*
		
		botones_funcionalidades();
		
		/*
		// PRENDER CADA 30 MIN
		if((minutos == 30 && segundos == 0) || (minutos == 1 && segundos == 0))
		{
			prender_motor();
			prender_ventilador();
		}*/
		
		// APAGAR FOCO SI PASA DE LOS 37 GRADOS
		/*if(res_float > 38)
		{
			apagar_foco();
		}
		// SI NO PASA LO PRENDEMOS Y APAGAMOS VENTILADOR
		else
		{
			prender_foco();
		}*/
	}
	return 0;
}

// FUNCIONES
uint8_t cero_en_bit(volatile uint8_t *LUGAR, uint8_t BIT) { return (!(*LUGAR & (1 << BIT))); }
uint8_t uno_en_bit(volatile uint8_t *LUGAR, uint8_t BIT) { return (*LUGAR & (1 << BIT)); }

void inicializar()
{
	cli(); // Deshabilitar interrupciones globales
	sei(); // Habilitar interrupciones
	DHT11_init();
	
	lcd_init(LCD_DISP_ON); // Inicializar display, cursor off
	lcd_clrscr(); // Clear display and home cursor
	
	USART_init(MYUBRR);
	PORTD = 0b00000010; // Para el serial Y el sensor de hum
		
	DDRB = 255; // Puerto B como salida para puente H
	
	
	
	//lcd_gotoxy(5 , 0);
	//lcd_puts("00     00:00:00");
	//lcd_gotoxy(0,0);
	//lcd_puts("Dia:");
	/*lcd_gotoxy(0,0);
	lcd_puts("Temperatura: ");
	lcd_gotoxy(0,1);
	lcd_puts("Humedad: ");*/
	//lcd_gotoxy(0,1);
	//lcd_puts("Temp Config: 37 C");
	//lcd_gotoxy(0,2);
	//lcd_puts("Veces: 48");
	
	
	// Inicializar Timer0
	TCNT0 = 0; // Cuenta el número de ciclos de reloj
	TIFR = 0b00000011;
	TIMSK = 0b00000010; // Habilitar interrupción por comparación
	TCCR0 = 0b00001101; // Modo CTC, prescaler - 1024, 1MHz
	OCR0 = 250 - 1; //1925.125	//243
	
	prender_foco();
	prender_ventilador();
}

void inicializar_reloj()
{
	if (ya_paso1 == 1) // Si ya pasó un segundo
	{
		segundos++;
		if (segundos == 60)
		{
			segundos = 0;
			minutos++;
		}
		
		if (minutos == 60)
		{
			minutos = 0;
			horas++;
		}
		
		if (horas == 24)
		{
			horas = 0;
			
			//dias = LeeEEPROM(0b11);
			dias++;
			//EscribeEEPROM(0b11, dias);	// ESCRIBIMOS NUEVO VALOR DE DIA 
		}
		
		if(dias == 24)
		{
			dias = 0;					//RESETEAR DIAS DE EEPROM Y DE CONTADOR 
			//EscribeEEPROM(0b11, 0b0); 
		}
		
		/*
		// Imprimir en LCD hora, minuto, segundo
		sprintf(buffer, "%02u     %02u:%02u:%02u", dias, horas, minutos, segundos);
		lcd_gotoxy(5 , 0);
		lcd_puts(buffer);
	*/
		ya_paso1 = 0;
	}
	
}

float regla_3(uint16_t adc)
{
	float voltage = (adc * 5.0) / 1023.0; // Convierte el valor del ADC a voltaje
	float temperature = voltage * 100.0;  // Convierte el voltaje a grados Celsius
	return temperature;
}


// Timer para el reloj
ISR(TIMER0_COMP_vect)
{
	contador++;
	if (contador >= 8) // 25	4
	{
		ya_paso1 = 1;
		contador = 0;
	}
	
	
	// Control del motor
	if (motor_encendido == 1)
	{
		contador_motor++;
		if (contador_motor >= 40) // Aproximadamente 5 segundos (suponiendo que TIMER0_COMP_vect se ejecuta cada 125 ms)
		{
			apagar_motor();
			apagar_ventilador(); 
			motor_encendido = 0;
			contador_motor = 0;
		}
	}
}

void prender_ventilador()
{
	//PORTB = 0b00001010;
	PORTB |= (1 << 1);
	PORTB |= (1 << 3);
}

void apagar_ventilador()
{
	//PORTB = 0b00000000;
	PORTB &= ~(1 << 1);
	PORTB &= ~(1 << 3);
}

void prender_motor()
{
	PORTB |= (1 << 5);  // Establecer un Bit en 1
	motor_encendido = 1; // Marcar que el motor está encendido
	//contador_motor = 0;  // Reiniciar el contador del motor
}

void apagar_motor()
{
	datos_recibidos = '9';
	PORTB &= ~(1 << 5); // Limpiar un Bit (Ponerlo en 0)
}

void prender_foco()
{
	PORTB |= (1 << 7);  // Establecer un Bit en 1
}

void apagar_foco()
{
	PORTB &= ~(1 << 7); // Limpiar un Bit (Ponerlo en 0)
}

void USART_init(uint16_t ubrr)
{
	UCSRA=0b00100000;
	UCSRB=0b10011000;
	UCSRC=0b10001110;
	UBRRH=0;
	UBRRL=12; //Para que funcione a 9600bps
}
	
void USART_Transmit(uint8_t data)
{
	// Wait for empty transmit buffer
	while (!(UCSRA & (1 << UDRE))) {}
	// Put data in to buffer, sends the data
	UDR = data;
}

// ISR para manejar la recepción de datos
ISR(USART_RXC_vect)
{
	datos_recibidos = UDR; // Lee el byte recibido
}

void botones_funcionalidades()
{
	if( datos_recibidos == '1')
	{
		prender_ventilador();
	}
	
	if(datos_recibidos == '2' )
	{
		apagar_ventilador();
	}
	if(datos_recibidos == '3' )
	{
		//prender_foco();
	}
	if(datos_recibidos == '4' )
	{
		//apagar_foco();
	}
	
	if(datos_recibidos == '5' )
	{
		prender_motor();
	}
}

// FUNCION ESCRIBIR
void EscribeEEPROM(uint16_t direccion, uint8_t datos)
{
	while(uno_en_bit(&EECR, EEWE)) {}
	
	EEAR = direccion;
	EEDR = datos;
	cli();
	EECR|= (1<<EEMWE);
	EECR|= (1<<EEWE);
	sei();
}

// FUNCION PARA LEER
uint8_t LeeEEPROM(uint16_t direccion)
{
	while(uno_en_bit(&EECR, EEWE)) {}
	EEAR = direccion;
	EECR |= (1<<EERE);
	return EEDR;
}


void programar_veces(uint8_t recibidos)
{
	switch(recibidos)
	{
		case 2:
		
			lcd_gotoxy(8, 2);
			lcd_puts("02");
			if(horas == 12 || horas == 24)
			{
				prender_motor();
			}
			break;
		
		case 4:
			
			lcd_gotoxy(8, 2);
			lcd_puts("04");
			if(horas == 6 || horas == 12|| horas == 18 || horas == 24)
			{
				prender_motor();
			}
			break;
		
		case 5:
			
			lcd_gotoxy(8, 2);
			lcd_puts("05");
			
			if(horas == 5 || horas == 10|| horas == 15 || horas == 20 || horas == 24)
			{
				prender_motor();
			}
			
			break;
		
		case 48:
		
			lcd_gotoxy(8, 2);
			lcd_puts("48");
			
			if(minutos == 30)
			{
				prender_motor();
			}
			
			break;
	}
	
}