#include <Arduino.h>

// Defines Variables
#define RS PC0 // Register Select is connected to PC0
#define EN PC1 // Enable pin is connected to PC1

#define D4 PC2 // D4 is connected to PC2
#define D5 PC3 // D5 is connected to PC3
#define D6 PC4 // D6 is connected to PC4
#define D7 PC5 // D7 is connected to PC5
// Function prototypes
void LCD_init(void);
void LCD_command(uint8_t command);
void LCD_data(uint8_t data);
void LCD_string(char *str);
