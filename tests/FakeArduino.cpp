/*
 * FakeArduino.cpp
 *
 *  Created on: 16 Oct 2012
 *      Author: jack
 */

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "FakeArduino.h"

Level Logger::log_threshold = DEBUG;

void log(const Level& level, const char *__fmt, ...)
{
    if (Logger::log_threshold <= level) {
        std::cout << " ";
        print_log_level(level);
        std::cout << " ";

        char buf[64];

        va_list vl;
        va_start(vl, __fmt);
        vsprintf(buf, __fmt, vl);
        va_end(vl);

        std::cout << buf << std::endl;
    }
}


void print_log_level(const Level& level)
{
    std::cout << level;

    switch (level) {
    case DEBUG: std::cout << "(DEBUG)"; break;
    case INFO:  std::cout << "(INFO)"; break;
    case WARN:  std::cout << "(WARN)"; break;
    case ERROR: std::cout << "(ERROR)"; break;
    case FATAL: std::cout << "(FATAL)"; break;
    }

}


void print_log_levels()
{
    for (uint8_t level=DEBUG; level<FATAL; level+=1) {
        print_log_level((Level)level);
        std::cout << ", ";
    }
    print_log_level(FATAL);
    std::cout << std::endl;
}


void FakeSerial::print(const char* str, const PrintFormat print_format)
{
    format(print_format);
    std::cout << str;
}

void FakeSerial::print(const int& str, const PrintFormat print_format)
{
    format(print_format);
    std::cout << str;
}

void FakeSerial::print(const unsigned int& str, const PrintFormat print_format)
{
    format(print_format);
    std::cout << (unsigned int)str;
}


void FakeSerial::println(const char* str, const PrintFormat print_format)
{
    format(print_format);
    std::cout << str << std::endl;
}


void FakeSerial::print(const float& str, const PrintFormat print_format)
{
    format(print_format);
    std::cout << str;
}

void FakeSerial::println(const float& str, const PrintFormat print_format)
{
    format(print_format);
    std::cout << str << std::endl;
}


bool FakeSerial::available()
{
    return false; // TODO
}

char FakeSerial::read()
{
    return 'a'; // TODO
}

void FakeSerial::write(const char value)
{
    std::cout << value;
}

void FakeSerial::flush()
{
    std::cout.flush();
}

void FakeSerial::format(const PrintFormat print_format)
{
    switch (print_format) {
    case DEC: std::cout << std::dec; break;
    case HEX: std::cout << std::hex; break;
    }
}


millis_t millis()
{
    //TODO: stub
    return 0;
}

millis_t micros()
{
    // TODO: stub
    return 0;
}

void FakeSerial::begin(uint32_t baudrate)
{
}

void FakeSerial::end()
{
    flush();
}

int analogRead(uint8_t pin)
{
    return 0; // TODO could read from a data file.
}
