#include <LCD.h>
#include <util/delay.h>

void LCD_init(void)
{
    // Set RS(PC0), EN(PC1), and D4-D7(PC2-PC5) as outputs
    DDRC |= (1 << RS) | (1 << EN) | (1 << D4) | (1 << D5) | (1 << D6) | (1 << D7);
    PORTC &= ~(1 << EN);
    _delay_ms(20);

    LCD_command(0x33);
    LCD_command(0x32);
    LCD_command(0x28);
    LCD_command(0x0C);
    LCD_command(0x06);
    LCD_command(0x01);
    LCD_command(0x80);
    _delay_ms(2);
}

void LCD_command(uint8_t command)
{
    PORTC &= ~(1 << RS); // RS = 0 for command

    // Clear PC2-PC5 (0xC3 is 11000011 in binary)
    PORTC &= 0xC3;
    // Shift top 4 bits left by 2 so they land on PC2-PC5
    PORTC |= (((command >> 4) & 0x0F) << 2);

    PORTC |= (1 << EN);
    _delay_us(1);
    PORTC &= ~(1 << EN);
    _delay_ms(2);

    PORTC &= 0xC3; // Clear PC2-PC5 again
    // Shift bottom 4 bits left by 2
    PORTC |= ((command & 0x0F) << 2);

    PORTC |= (1 << EN);
    _delay_us(1);
    PORTC &= ~(1 << EN);
    _delay_ms(2);
}

void LCD_data(uint8_t data)
{
    PORTC |= (1 << RS);  // RS = 1 for data

    PORTC &= 0xC3;
    PORTC |= (((data >> 4) & 0x0F) << 2);

    PORTC |= (1 << EN);
    _delay_us(1);
    PORTC &= ~(1 << EN);
    _delay_ms(1);

    PORTC &= 0xC3;
    PORTC |= ((data & 0x0F) << 2);

    PORTC |= (1 << EN);
    _delay_us(1);
    PORTC &= ~(1 << EN);
    _delay_ms(1);
}

void LCD_string(char *str)
{
  uint8_t i;
  for(i=0; str[i] != 0; i++)
  {
    LCD_data(str[i]);
  }
}
