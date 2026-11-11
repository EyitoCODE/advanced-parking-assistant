#include "Ultrasonic.h"
#include <avr/interrupt.h>
#include <util/delay.h>

// Mapped perfectly to your Tinkercad layout
#define ECHO_PIN PB0 // Digital 8 (Mandatory for ICP1 hardware timer)
#define TRIG_PIN PB1 // Digital 9

// Global variable for the professor's timer math
volatile int TimerOverflow = 0;

// Timer1 overflow interrupt service routine (From Professor's Code)
ISR(TIMER1_OVF_vect)
{
    TimerOverflow++;
}

void Ultrasonic_init(void)
{
    // Configure data direction for PORTB
    DDRB |= (1 << TRIG_PIN);    // Set Trigger pin as output
    DDRB &= ~(1 << ECHO_PIN);   // Set Echo pin as input
    PORTB |= (1 << ECHO_PIN);   // Enable internal pull-up resistor on Echo pin

    // Enable Timer1 overflow interrupts and set Normal operation
    TIMSK1 |= (1 << TOIE1);
    TCCR1A = 0;
}

uint16_t Ultrasonic_get_distance(void)
{
    long count;
    double distance;

    // 1. Fire the 10 microsecond trigger pulse
    PORTB |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTB &= ~(1 << TRIG_PIN);

    // 2. Prepare to capture the rising edge of the echo
    TCNT1 = 0;
    TCCR1B = 0x41;        // Capture rising edge, No pre-scaler
    TIFR1 = (1 << ICF1) | (1 << TOV1); // Clear Input Capture and Overflow flags
    TimerOverflow = 0;    // Reset overflow counter

    // 3. Wait for the rising edge (Added a timeout to prevent Tinkercad freezing)
    uint16_t timeout = 0;
    while ((TIFR1 & (1 << ICF1)) == 0)
    {
        timeout++;
        if (timeout > 50000) return 999; // Return out-of-bounds value if disconnected
    }

    // 4. Prepare to capture the falling edge of the echo
    TCNT1 = 0;
    TCCR1B = 0x01;        // Capture falling edge, No pre-scaler
    TIFR1 = (1 << ICF1) | (1 << TOV1); // Clear flags again
    TimerOverflow = 0;

    // 5. Wait for the falling edge
    timeout = 0;
    while ((TIFR1 & (1 << ICF1)) == 0)
    {
        timeout++;
        if (timeout > 50000) return 999;
    }

    // 6. The Professor's exact distance calculation
    count = ICR1 + (65535 * TimerOverflow); // Calculate total timer ticks

    // 16MHz Timer freq, sound speed = 343 m/s
    // 17150 x Timer value * 0.0625 x 10^-6 = Timer value / 932.8
    distance = (double)count / 932.8;

    return (uint16_t)distance; // Cast back to an integer for clean LCD display
}
