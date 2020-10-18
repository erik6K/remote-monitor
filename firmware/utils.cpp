/*
MIT License

NTP library for Arduino framework
Copyright (c) 2018 Stefan Staub

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "utils.h"

/*
// implementation of printf for use in Arduino sketch
void Serial_printf(char* fmt, ...) {
    char buf[256]; // resulting string limited to 128 chars
    va_list args;
    va_start (args, fmt );
    vsnprintf(buf, 256, fmt, args);
    va_end (args);
    Serial.print(buf);
}*/

// simple URL encoder
String urlEncode(const char* msg)
{
    static const char *hex PROGMEM = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
            || ('A' <= *msg && *msg <= 'Z')
            || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    return encodedMsg;
}

int32_t indexOf(const char* buffer, size_t length, const char* look_for, size_t look_for_length, int32_t start_index) {
    if (look_for_length > length) {
        return -1;
    }

    for (size_t pos = start_index; pos < length; pos++) {
        if (length - pos < look_for_length) {
            return -1;
        }

        if (buffer[pos] == *look_for) {
            size_t sub = 1;
            for (; sub < look_for_length; sub++) {
                if (buffer[pos + sub] != look_for[sub]) break;
            }

            if (sub == look_for_length) {
                return pos;
            }
        }
    }

    return -1;
}