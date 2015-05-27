//scope.c
// PD0 - TxD
// PD1 - RxD
#include <avr/io.h>
#include <avr/interrupt.h>  

/* define CPU frequency in Mhz here if not defined in Makefile */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#define BAUD 57600 //dla 57600 przy 8MHz i wiecej warto stosować U2X w UCSRA ze wzgledu na błąd
//#define MYUBRR F_CPU/16/BAUD-1
#define MYUBRR F_CPU/8/BAUD-1 //8 bo U2X

volatile uint8_t adcnum = 0; //zaczynamy od pc0
volatile uint8_t data[2];
volatile uint8_t activech = 0xFF;

//przerwanie z przetwornika ADC
ISR(ADC_vect)                        
{
/*When ADCL is read, the ADC Data Register is not updated until ADCH is read. Consequently, if
the result is left adjusted and no more than 8-bit precision is required, it is sufficient to read
ADCH. Otherwise, ADCL must be read first, then ADCH.*/
    data[1] = ADCL;
    data[0] = (ADCH << 1) | 0b10000000; //dwa najmniej znaczace bity z adch sa używane i przesuniete o jeden w lewo by zrobic miejsce na bit 7 z adcl, bit 7 zawsze 1

    data[0] = data[0] | ((data[1] & 0b10000000) >> 7); //bit 7 przerzucony do data[0]
    data[0] = data[0] | ((adcnum & 0b00000111) << 4); //bity 6:4 maja byc id
    data[1] = data[1] & 0b01111111; // data[1]zawsze rozpoczynany 0
    
    adcnum++;
    if (adcnum > 7) adcnum = 0;
    
    while (!(activech & (1 << adcnum)))
    {
        adcnum++; /*przeskocz dalej jesli kanału nie ma na liscie aktywnych*/
        if (adcnum > 7) adcnum = 0;
    }

    ADMUX = adcnum | (1<<REFS0); //REFS0 odniesienie do napiecia zasilanie
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1);
} 

void USART_Init(uint8_t ubrr)
{
    /* ustawienie predkosci - datasheet strona 132 */
    UBRRH = (uint8_t)(ubrr>>8);
    UBRRL = (uint8_t)ubrr;
    /* Włączenie odbiornika i nadajnika */
    UCSRA = _BV(U2X); //ustawienie podwojnej predkosci dla zmiejszenia bledow
    UCSRB = (1<<RXEN)|(1<<TXEN);
    /* Ustawienie typu transmisji (sync/async, parzystość, bity stopu)
    (3<<UCSZ0) to jak (1<<UCSZ1) | (1<<UCSZ0)
    8 bitów danych, jeden parzystości
    Bit 7 – URSEL: Register Select
    This bit selects between accessing the UCSRC or the UBRRH Register. It is read as one when
    reading UCSRC. The URSEL must be one when writing the UCSRC
     */
    UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

void USART_Transmit(uint8_t part)
{
    /* Oczekiwanie na opróżnienie bufora trnsmisji */
    while (!(UCSRA & (1<<UDRE)));
    /* Umieszczenie danych w buforze, wysylanie danych */
    UDR = part;
}

void USART_TryReceive(void)
{
    if (UCSRA & (1<<RXC))
    {
        activech = UDR; /*odbierz aktywne kanały*/
    }

}


int main(void)
{
    //ADC
    ADMUX = adcnum | (1<<REFS0); //REFS0 odniesienie do napiecia zasilanie
    //ADMUX  = (1<<REFS1) | (1<<REFS0) | (1<<ADLAR) | adcnum; //REFS1 i 2 to wewnętrze 2.56V jako vref, gdy ADLAR=1 to w ADCH jest 8 najstarszych bitów
    ADCSRA = _BV(ADEN)|_BV(ADIE)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1); //ustawnienie preskalera na 64
    sei();     //Przerwania wlaczone
    USART_Init (MYUBRR); //inicjacja UART
    while (1)
    {   
        uint8_t d;
        cli();
        for (d = 0; d < 2; ++d)
        { 
            USART_Transmit(data[d]);
        }
        USART_TryReceive();
        sei();
    }	
    return 0;
}
