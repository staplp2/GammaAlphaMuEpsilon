#pragma once


/// Increment a number atomically.
/// Performs the following operation atomically:
///   int old_value = *address; (*address)++; return old_value;
/// @param address a given integer which is to be incremented
/// @return the old value of the number.
int atomic_increment(int* address);

/// Decrement a number atomically.
/// Performs the following operation atomically:
///   int old_value = *address; (*address)--; return old_value;
/// @param address a given integer which is to be decremented
/// @return the old value of the number.
int atomic_decrement(int* address);

/// Compare two numbers atomically and assign if equal.
/// Performs the following operation atomically:
///   int old_value = *address; if (*address == compare) *address = exchange; return old_value;
/// @param dest a given integer that is the original value
/// @param compare a given integer which is to be compared to the original value
/// @param exchange when dest and compare are equal then return this given integer
/// @return the old value of the number.
int atomic_compare_and_exchange(int* dest, int compare, int exchange);

/// Reads an integer from an address.
/// All writes that occurred before the last atomic_store to this address are flushed.
/// @param address a given address from which to load an integer
/// @return Integer stored at given address.
/// @see atomic_store
int atomic_load(int* address);

/// Writes an integer.
/// Paired with an atomic_load, can guarantee ordering and visibility.
/// @param address a given integer which is to be incremented
/// @param value Integer value to be loaded at address
/// @see atomic_load
void atomic_store(int* address, int value);
