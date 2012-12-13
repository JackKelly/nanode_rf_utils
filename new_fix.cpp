/*
 * new.cpp
 *
 *  Created on: 16 Oct 2012
 *      Author: jack
 */

#include <Arduino.h>
#include "new_fix.h"

void * operator new(size_t size)
{
  return malloc(size);
}


void operator delete(void * ptr)
{
  free(ptr);
}

void * operator new[](size_t size)
{
    return malloc(size);
}


void operator delete[](void * ptr)
{
    free(ptr);
}

void __cxa_pure_virtual(void) {}; 
