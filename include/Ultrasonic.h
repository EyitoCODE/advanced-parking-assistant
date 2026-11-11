#pragma once
#include <Arduino.h>

// Function prototypes for the Ultrasonic Sensor
void Ultrasonic_init(void);
uint16_t Ultrasonic_get_distance(void);
