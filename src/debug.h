#pragma once

#include <stdint.h>

/// Flags for debug_print().
typedef enum debug_print_t
{
	k_print_info = 1 << 0,///< Flag for printing simple information.
	k_print_warning = 1 << 1,///< Flag for printing out any warnings.
	k_print_error = 1 << 2,///< Flag for printing errors.
} debug_print_t;

/// Install unhandled exception handler.
/// When unhandled exceptions are caught, will log an error and capture a memory dump.
void debug_install_exception_handler();

/// Set mask of which types of prints will actually fire.
/// @param mask 32 bit integer which is a mask to be passed.
/// @see debug_print().
void debug_set_print_mask(uint32_t mask);

/// Log a message to the console.
/// Message may be dropped if type is not in the active mask.
/// @param type Type to check against the set debug mask(s).
/// @param format String of characters of which to print. May be formmated.
/// @see debug_set_print_mask().
void debug_print(uint32_t type, _Printf_format_string_ const char* format, ...);

/// Capture a list of addresses that make up the current function callstack.
/// On return, stack contains at most stack_capacity addresses.
/// The number of addresses captured is the return value.
/// @param stack Pointer to the call stack list.
/// @param stack_capacity Maximum size for the given stack to be traced to.
int debug_backtrace(void** stack, int stack_capacity);
