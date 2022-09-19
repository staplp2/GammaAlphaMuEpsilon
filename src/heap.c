#include "heap.h"

#include "debug.h"
#include "tlsf/tlsf.h"

#include <stddef.h>
#include <stdio.h>


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

typedef struct arena_t
{
	pool_t pool;
	struct arena_t* next;
} arena_t;


//Structure to track memory accesses and look out for leaks
typedef struct mem_leak_tracker_t
{
	void* address; //address of memory access
	int free_state; //whether or not the memory at the address is currently freed
	size_t block_size; //size of memory being used at address
	void* call_stack [64]; //call stack from where memory was initially allocated
	int call_stack_size; //number of items on call stack

} mem_leak_tracker_t;

typedef struct heap_t
{
	tlsf_t tlsf;
	size_t grow_increment;
	arena_t* arena;
	mem_leak_tracker_t leak_tracker_list[64];
} heap_t;


heap_t* heap_create(size_t grow_increment)
{
	heap_t* heap = VirtualAlloc(NULL, sizeof(heap_t) + tlsf_size(),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!heap)
	{
		debug_print(
			k_print_error,
			"OUT OF MEMORY!\n");
		return NULL;
	}

	heap->grow_increment = grow_increment;
	heap->tlsf = tlsf_create(heap + 1);
	heap->arena = NULL;

	return heap;
}

void* heap_alloc(heap_t* heap, size_t size, size_t alignment)
{
	void* address = tlsf_memalign(heap->tlsf, alignment, size);
	if (!address)
	{
		size_t arena_size =
			__max(heap->grow_increment, size * 2) +
			sizeof(arena_t);
		arena_t* arena = VirtualAlloc(NULL,
			arena_size + tlsf_pool_overhead(),
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!arena)
		{
			debug_print(
				k_print_error,
				"OUT OF MEMORY!\n");
			return NULL;
		}

		arena->pool = tlsf_add_pool(heap->tlsf, arena + 1, arena_size);

		arena->next = heap->arena;
		heap->arena = arena;

		address = tlsf_memalign(heap->tlsf, alignment, size);
	}
	mem_leak_tracker_t leak_tracker = { .address = address, .block_size = size, .free_state = 1, };
	unsigned short frames = CaptureStackBackTrace(1, 63, leak_tracker.call_stack, NULL);
	leak_tracker.call_stack_size = frames;

	for (int i = 0; i < 64; i++) {
		if (heap->leak_tracker_list[i].free_state == 1) {
			continue;
		}
		else {
			heap->leak_tracker_list[i] = leak_tracker;
			break;
		}
	}

	return address;
}

void heap_free(heap_t* heap, void* address)
{
	tlsf_free(heap->tlsf, address);
	for (int i = 0; i < 64; i++) {
		if (heap->leak_tracker_list[i].address == address) {
			heap->leak_tracker_list[i].block_size = 0;
			heap->leak_tracker_list[i].free_state = 0;
			return;
		}
	}
}

void heap_destroy(heap_t* heap)
{
	for (int i = 0; i < 64; i++) {
		if (heap->leak_tracker_list[i].free_state == 1) {

			debug_print(
				k_print_error,
				"Memory leak of size %d bytes with callstack:\n",
				heap->leak_tracker_list[i].block_size);

			SymInitialize(GetCurrentProcess(), NULL, TRUE);
			
			SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
			symbol->MaxNameLen = 255;
			symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

			for (int j = 0; j < (heap->leak_tracker_list[i].call_stack_size - 6); j++) {
				if (SymFromAddr(GetCurrentProcess(), (DWORD64)(heap->leak_tracker_list[i].call_stack[j]), 0, symbol)) {
					debug_print(
						k_print_error,
						"[%d] %s\n",
						j, symbol->Name);
				}
				else {
					debug_print(
						k_print_error,
						"Broken");
					DWORD lasterror = GetLastError();
					printf("\n    Error! - Error code %d\n", lasterror);
				}
				
			}
			free(symbol);
			SymCleanup(GetCurrentProcess());
		}
	}

	tlsf_destroy(heap->tlsf);

	arena_t* arena = heap->arena;
	while (arena)
	{
		arena_t* next = arena->next;
		VirtualFree(arena, 0, MEM_RELEASE);
		arena = next;
	}

	VirtualFree(heap, 0, MEM_RELEASE);
}
