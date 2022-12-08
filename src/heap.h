#pragma once

#include <stdlib.h>


/// Main object, heap_t, represents a dynamic memory heap.
/// Once created, memory can be allocated and free from the heap.
typedef struct heap_t heap_t;

/// Creates a new memory heap.
/// @param grow_increment The default size with which the heap grows.
/// Should be a multiple of OS page size.
/// @return Handle to created heap.
heap_t* heap_create(size_t grow_increment);

/// Destroy a previously created heap.
/// @param heap Handle to heap to be destroyed.
void heap_destroy(heap_t* heap);

/// Allocate memory from a heap.
/// @param heap Handle to heap for more memory to be allocated to.
/// @param size Size of the memory allocation within the heap
/// @param alignment Offset within the heap where memory allocations should begin at.
/// @return Pointer to allocated memory.
void* heap_alloc(heap_t* heap, size_t size, size_t alignment);

/// Free memory previously allocated from a heap.
/// @param heap Handle to heap where memory is to be freed from.
/// @param address Pointer to memory within the given heap which is to be freed.
void heap_free(heap_t* heap, void* address);
