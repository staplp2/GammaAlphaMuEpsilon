#pragma once

#include <stdbool.h>

/// Handle to file system.
typedef struct fs_t fs_t;

/// Handle to file work.
typedef struct fs_work_t fs_work_t;

/// Handle to heap for memory allocation.
typedef struct heap_t heap_t;

/// Create a new file system.
/// @param heap will be used to allocate space for queue and work buffers.
/// @param queue_capacity defines number of in-flight file operations.
fs_t* fs_create(heap_t* heap, int queue_capacity);

/// Destroy a previously created file system.
/// @param fs Handle to file system to destroy.
void fs_destroy(fs_t* fs);

/// Queue a file read.
/// @param fs File at the specified path will be read in full.
/// @param heap Memory for the file will be allocated out of the provided heap.
/// @param null_terminate Bool to signify whether to null terminate when reading in file.
/// @param use_compression Bool to signify if the file being read should be compressed.
/// @return a work object.
fs_work_t* fs_read(fs_t* fs, const char* path, heap_t* heap, bool null_terminate, bool use_compression);

/// Queue a file write.
/// @param fs File at the specified path will be written.
/// @param buffer What is to be written into the given file.
/// @param size The size of how much is to be written.
/// @param use_compression Bool to signify if the file being read should be compressed.
/// @return a work object.
fs_work_t* fs_write(fs_t* fs, const char* path, const void* buffer, size_t size, bool use_compression);

/// If true, the file work is complete.
/// @param work Handle to a work object.
/// @return True when the work is finished. False elsewise.
bool fs_work_is_done(fs_work_t* work);

/// Block for the file work to complete.
/// @param work Handle to a work object.
void fs_work_wait(fs_work_t* work);

/// Get the error code for the file work.
/// A value of zero generally indicates success.
/// @param work Handle to a work object.
/// @return Error code for the file work.
int fs_work_get_result(fs_work_t* work);

/// Get the buffer associated with the file operation.
/// @param work Handle to a work object.
/// @return Buffer associated with given file work operation.
void* fs_work_get_buffer(fs_work_t* work);

/// Get the size associated with the file operation.
/// @param work Handle to a work object.
/// @return Size associated with given file work operation.
size_t fs_work_get_size(fs_work_t* work);

/// Free a file work object.
/// @param work Handle to a work object.
void fs_work_destroy(fs_work_t* work);
