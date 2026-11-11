#include <Arduino.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "LCD.h"
#include "USART.h"
#include "ShiftReg.h"
#include "Segment.h"
#include "Ultrasonic.h"

// define variables
#define BUZZER_PIN PB2 // Digital 10
#define LED_GREEN  PB3 // Digital 11
#define LED_YELLOW PB4 // Digital 12
#define LED_RED    PB5 // Digital 13

// The Button on PD2 (Digital 2) toggles this variable via INT0
volatile uint8_t sound_enabled = 1;

// Hardware Interrupt for the Mute Button
ISR(INT0_vect)
{
    sound_enabled = !sound_enabled; // Toggle between 1 and 0
}

void init_hardware()
{
    // Set LEDs and Buzzer as outputs on PORTB
    DDRB |= (1 << LED_GREEN) | (1 << LED_YELLOW) | (1 << LED_RED) | (1 << BUZZER_PIN);

    // Set Button pin as input with pull-up resistor on PORTD
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);

    // Configure External Interrupt 0 (INT0) on falling edge
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);

    // Initialize all modules
    USART_init();
    LCD_init();
    init_shift(PD4, PD6, PD5); // Data, Clock, Latch
    Ultrasonic_init();

    sei(); // Enable global interrupts
}

int main()
{
    // Initialize the Arduino timebase used by millis() and the warning cadence.
    init();
    init_hardware();
    char buffer[40];
    byte digits[10] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6};

    const uint16_t threshold_cm = 30; // Hardcoded danger threshold

    USART_send_string("Parking System Initialized...\r\n");

    while (1)
    {
        uint16_t distance = Ultrasonic_get_distance();

        // Clear all visual/audio outputs before updating
        PORTB &= ~((1 << LED_GREEN) | (1 << LED_YELLOW) | (1 << LED_RED) | (1 << BUZZER_PIN));
        uint8_t danger_level = 0;


        // Evaluate Distance
        if (distance <= threshold_cm)
        {
            PORTB |= (1 << LED_RED);
            danger_level = 9;
            if (sound_enabled) tone(10, 1000); // 1000 Hz loud tone on Pin 10
            else noTone(10);
        }
        else if (distance <= (threshold_cm * 2))
        {
            PORTB |= (1 << LED_YELLOW);
            danger_level = 5;

            // Rapid Beep: Toggle tone on and off
            if (sound_enabled) {
                 if (millis() % 500 < 250) tone(10, 1000);
                 else noTone(10);
            } else noTone(10);
        }
        else
        {
            PORTB |= (1 << LED_GREEN);
            danger_level = 0;
            noTone(10); // Silence in the green zone
        }

        // // Evaluate Distance
        // if (distance <= threshold_cm)
        // {
        //     PORTB |= (1 << LED_RED);
        //     danger_level = 9;
        //     if (sound_enabled) PORTB |= (1 << BUZZER_PIN);
        // }
        // else if (distance <= (threshold_cm * 2))
        // {
        //     PORTB |= (1 << LED_YELLOW);
        //     danger_level = 5;
        //     if (sound_enabled) PORTB ^= (1 << BUZZER_PIN); // Toggle buzzer for beeping
        // }
        // else
        // {
        //     PORTB |= (1 << LED_GREEN);
        //     danger_level = 0;
        // }

        // Update Outputs
        displyValue(digits[danger_level]);

        LCD_command(0x01); // Clear display
        _delay_ms(2);

        LCD_command(0x80); // Line 1
        sprintf(buffer, "Dist: %d cm", distance);
        LCD_string(buffer);

        LCD_command(0xC0); // Line 2
        sprintf(buffer, "Limit: %d cm", threshold_cm);
        LCD_string(buffer);

        sprintf(buffer, "Dist: %d | Thresh: %d | Sound: %s\r\n", distance, threshold_cm, sound_enabled ? "ON" : "OFF");
        USART_send_string(buffer);

        _delay_ms(150);
    }
    return 0;
}
