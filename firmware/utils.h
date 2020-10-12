#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// implementation of printf for use in Arduino sketch
//void Serial_printf(char* fmt, ...);

// simple URL encoder
String urlEncode(const char* msg);

int32_t indexOf(const char* buffer, size_t length, const char* look_for, size_t look_for_length, int32_t start_index);

#endif