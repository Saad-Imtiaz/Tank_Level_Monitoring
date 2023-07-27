#ifndef CONFIG
#define CONFIG

#include <Arduino.h>

#define DEBUG true
#define RESTART_TIME 7200000 // 1 hr - 3600000 | 2 hr - 7200000


#define TRIGGER_PIN 5
#define ECHO_PIN 18

#define TOTAL_DEPTH 0.711; // Total depth of the tank in m
#define TANK_LENGHT 2.463; // Length of the tank in m
#define TANK_WIDTH  2.438;

#endif