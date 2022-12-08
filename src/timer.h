#pragma once

#include <stdint.h>

/// Perform one-time initialization of the timer.
void timer_startup();

/// Get the number of OS-defined ticks that have elapsed since startup.
/// @return Number of ticks since start.
uint64_t timer_get_ticks();

/// Get the OS-defined tick frequency.
/// @return Tick Frequency
uint64_t timer_get_ticks_per_second();

/// Convert a number of OS-defined ticks to microseconds.
/// @param Ticks to be converted
/// @return Time in microseconds of given ticks
uint64_t timer_ticks_to_us(uint64_t t);

/// Convert a number of OS-defined ticks to milliseconds.
/// @param Ticks to be converted
/// @return Time in milliseconds of given ticks
uint32_t timer_ticks_to_ms(uint64_t t);
