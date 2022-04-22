#define F_CPU 16000000UL

#define aref_mode 0
#define avcc_mode (1<<REFS0)
#define int_mode (1<<REFS0)|(1<<REFS1)
#define choose_type aref_mode

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/sfr_defs.h>

#define RS PINC1
#define RW PINC2
#define RE PINC3

//https://www.electronicsforu.com/technology-trends/learn-electronics/16x2-lcd-pinout-diagram

void setup_lcd(unsigned char x)
{
	PORTD = x; 
	PORTC &=~ (1 << RS);	// RS = 0, su dung thanh ghi lenh IR
	PORTC &=~ (1 << RW);	// RW = 0, su dung LCD o che do ghi
	PORTC |= (1 << RE);	// RE = 1, bat dau truyen
	_delay_us(1);
	PORTC &=~ (1 << RE);	// RE = 0, ket thuc truyen
	_delay_us(3);
}

void display_char(unsigned char x)
{
	PORTD= x;
	PORTC |= ( 1 << RS );	// RS = 1, su dung thanh ghi du lieu DR
	PORTC &=~ ( 1 << RW);	// RW = 0, su dung LCD o che do ghi
	PORTC |= ( 1 << RE);	// RE = 1, bat dau truyen
	_delay_us(1);
	PORTC &=~ (1 << RE);	// RE = 0, ket thuc truyen
	_delay_us(2);
	
}

void khoitao(void)
{
	DDRD = 0xFF;	// Set port D lam output
	DDRC = 0xFF;	// Set port C lam output
	_delay_ms(20);
	setup_lcd(0x38); //2 lines and 5×7 matrix
	setup_lcd(0x0C); //Display on, cursor off
	setup_lcd(0x01); //Clear display screen
	setup_lcd(0x80); //Force cursor to beginning ( 1st line)
}

void display_string (char* str)
{
	for (int i = 0; str[i] != '\0'; i++)
	{
		display_char(str[i]);
	}
}

void display_stringxy(char row, char col, char* str)
{
	if (row == 0 && col < 16)
	{
		setup_lcd((col & 0x0F)|0x80); // Display on, cursor blinking and Force cursor to beginning ( 1st line)
	}
	else if (row == 1 && col < 16)
	{
		setup_lcd((col & 0x0F)|0xC0); // // Display on, cursor blinking and Force cursor to beginning ( 2nd line)
	}
	display_string(str);
}

void clear_lcd()
{
	setup_lcd(0x01); // Clear display screen
	setup_lcd(0x80); // Force cursor to beginning ( 1st line)
}

uint16_t read_adc (unsigned int kenh_adc)
{
	ADMUX = kenh_adc | (1 << REFS0);  // Chon dien ap tham chieu là AVCC
	ADCSRA |= (1 << ADSC); // Bat dau quá trình chuyen doi
	loop_until_bit_is_set(ADCSRA, ADIF); // Cho den khi ket thuc chuyen doi
	return ADCW; // Tra ve ket qua chuyen doi
}
uint16_t adc_val;

int main(void)
{
   khoitao();
   ADMUX |= (1 << REFS0); // Chon dien ap tham chieu là AVCC
   ADCSRA |= (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0); // Cho phép module ADC hoat dong, he so chia xung nhip = 8
   int celcius;
   char nhietdo[7];
  
   DDRB = 0xff;
   PORTB = 0x00;

   TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);
   TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS10);
   OCR1A = 8000;
   ICR1 = 10000;
   ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS0);
   ADMUX = choose_type;
   while (1){
	   adc_val= (float) read_adc(0);
	   _delay_ms(10);
	   if(adc_val > 30){
		   PORTB = 0x08;//bat chan enA
		   if(OCR1A == 6000) OCR1A = 8000;// doi goc quay
		   else if(OCR1A == 8000) OCR1A = 6000;// doi goc quay
	   }
	   else
	   PORTB = 0x00;
	   
	   display_stringxy(0, 0, "Nhiet do: ");
	  
	   celcius = adc_val * 75 / 64; // adc_val * 12 / 1024 * 100;
	   
	   sprintf (nhietdo, "%d oC",(int)celcius);
	   display_stringxy(1,11, nhietdo);
   }
}
