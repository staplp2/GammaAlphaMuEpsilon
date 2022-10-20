#include "fs.h"

#include "event.h"
#include "heap.h"
#include "queue.h"
#include "thread.h"

#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct fs_t
{
	heap_t* heap;
	queue_t* file_queue;
	queue_t* compression_queue; //separate queue for compression and decompression
	thread_t* file_thread;
	thread_t* compression_thread;//thread in order to call compression and decompression function
} fs_t;

typedef enum fs_work_op_t
{
	k_fs_work_op_read,
	k_fs_work_op_write,
} fs_work_op_t;

typedef struct fs_work_t
{
	heap_t* heap;
	fs_work_op_t op;
	char path[1024];
	bool null_terminate;
	bool use_compression;
	void* buffer;
	size_t size;
	event_t* done;
	int result;
} fs_work_t;

static int file_thread_func(void* user);

static int compression_thread_func(void* user);//function which operates on the compression thread

fs_t* fs_create(heap_t* heap, int queue_capacity)
{
	fs_t* fs = heap_alloc(heap, sizeof(fs_t), 8);
	fs->heap = heap;
	fs->file_queue = queue_create(heap, queue_capacity);
	fs->compression_queue = queue_create(heap, queue_capacity);
	fs->file_thread = thread_create(file_thread_func, fs);
	fs->compression_thread = thread_create(compression_thread_func, fs);//create the thread calling the compression function
	return fs;
}

void fs_destroy(fs_t* fs)
{
	queue_push(fs->file_queue, NULL);
	queue_push(fs->compression_queue, NULL);
	thread_destroy(fs->file_thread);
	thread_destroy(fs->compression_thread);
	queue_destroy(fs->file_queue);
	queue_destroy(fs->compression_queue);//make sure to properly destory new queue and thread
	heap_free(fs->heap, fs);
}

fs_work_t* fs_read(fs_t* fs, const char* path, heap_t* heap, bool null_terminate, bool use_compression)
{
	fs_work_t* work = heap_alloc(fs->heap, sizeof(fs_work_t), 8);
	work->heap = heap;
	work->op = k_fs_work_op_read;
	strcpy_s(work->path, sizeof(work->path), path);
	work->buffer = NULL;
	work->size = 0;
	work->done = event_create();
	work->result = 0;
	work->null_terminate = null_terminate;
	work->use_compression = use_compression;
	queue_push(fs->file_queue, work);
	return work;
}

fs_work_t* fs_write(fs_t* fs, const char* path, const void* buffer, size_t size, bool use_compression)
{
	fs_work_t* work = heap_alloc(fs->heap, sizeof(fs_work_t), 8);
	work->heap = fs->heap;
	work->op = k_fs_work_op_write;
	strcpy_s(work->path, sizeof(work->path), path);
	work->buffer = (void*)buffer;
	work->size = size;
	work->done = event_create();
	work->result = 0;
	work->null_terminate = false;
	work->use_compression = use_compression;

	if (use_compression)
	{
		// HOMEWORK 2: Queue file write work on compression queue!
		queue_push(fs->compression_queue, work);
	}
	else
	{
		queue_push(fs->file_queue, work);
	}

	return work;
}

bool fs_work_is_done(fs_work_t* work)
{
	return work ? event_is_raised(work->done) : true;
}

void fs_work_wait(fs_work_t* work)
{
	if (work)
	{
		event_wait(work->done);
	}
}

int fs_work_get_result(fs_work_t* work)
{
	fs_work_wait(work);
	return work ? work->result : -1;
}

void* fs_work_get_buffer(fs_work_t* work)
{
	fs_work_wait(work);
	return work ? work->buffer : NULL;
}

size_t fs_work_get_size(fs_work_t* work)
{
	fs_work_wait(work);
	return work ? work->size : 0;
}

void fs_work_destroy(fs_work_t* work)
{
	if (work)
	{
		event_wait(work->done);
		event_destroy(work->done);
		heap_free(work->heap, work);
	}
}

static void file_read(fs_work_t* work, fs_t* fs) //Added fs so that I could call queue_psuh with the compression queue
{
	wchar_t wide_path[1024];
	if (MultiByteToWideChar(CP_UTF8, 0, work->path, -1, wide_path, sizeof(wide_path)) <= 0)
	{
		work->result = -1;
		event_signal(work->done);
		return;
	}

	HANDLE handle = CreateFile(wide_path, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		work->result = GetLastError();
		event_signal(work->done);
		return;
	}

	if (!GetFileSizeEx(handle, (PLARGE_INTEGER)&work->size))
	{
		work->result = GetLastError();
		CloseHandle(handle);
		event_signal(work->done);
		return;
	}

	work->buffer = heap_alloc(work->heap, work->null_terminate ? work->size + 1 : work->size, 8);

	DWORD bytes_read = 0;
	if (!ReadFile(handle, work->buffer, (DWORD)work->size, &bytes_read, NULL))
	{
		work->result = GetLastError();
		CloseHandle(handle);
		event_signal(work->done);
		return;
	}

	work->size = bytes_read;
	if (work->null_terminate)
	{
		((char*)work->buffer)[bytes_read] = 0;
	}

	CloseHandle(handle);

	if (work->use_compression)
	{
		// HOMEWORK 2: Queue file read work on decompression queue!
		queue_push(fs->compression_queue, work);
	}
	else
	{
		event_signal(work->done);
	}
}

int get_hash(void* address, int bucket_count)
{
	return (intptr_t)address % bucket_count;
}


static void file_write(fs_work_t* work)
{
	wchar_t wide_path[1024];
	if (MultiByteToWideChar(CP_UTF8, 0, work->path, -1, wide_path, sizeof(wide_path)) <= 0)
	{
		work->result = -1;
		event_signal(work->done);
		return;
	}

	HANDLE handle = CreateFile(wide_path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		work->result = GetLastError();
		event_signal(work->done);
		return;
	}

	DWORD bytes_written = 0;
	if (!WriteFile(handle, work->buffer, (DWORD)work->size, &bytes_written, NULL))
	{
		work->result = GetLastError();
		CloseHandle(handle);
		event_signal(work->done);
		return;
	}

	work->size = bytes_written;

	CloseHandle(handle);

	event_signal(work->done);
}

static int file_thread_func(void* user)
{
	fs_t* fs = user;
	while (true)
	{
		fs_work_t* work = queue_pop(fs->file_queue);
		if (work == NULL)
		{
			break;
		}
		
		switch (work->op)
		{
		case k_fs_work_op_read:
			file_read(work, fs);
			break;
		case k_fs_work_op_write:
			file_write(work);
			break;
		}

	}
	return 0;
}

#include "lz4/lz4.h"

static int compression_thread_func(void* user)
{
	fs_t* fs = user;
	while (true)
	{
		fs_work_t* work = queue_pop(fs->compression_queue);//look for work on the compression queue
		if (work == NULL)
		{
			break;
		}

		switch (work->op)
		{
		case k_fs_work_op_read:
		{
			//when needing to read you need to decompress
			int dat_buffer_size = 10000000;//estimate that most likely size will be smaller than this
			void* dat_buffer = heap_alloc(work->heap, dat_buffer_size, 8);//allocate space for the decompressed string
			//call decompress from LZ4 using the information passed from file_read in the data of work
			int decomp_size = LZ4_decompress_safe(work->buffer, dat_buffer, (int)work->size, dat_buffer_size);
			work->buffer = dat_buffer;//assign decompressed data to work to be passed back

			event_signal(work->done);//signal finished with reading
			break;
		}
		case k_fs_work_op_write:
		{
			//write => compress data into file
			int dat_buffer_size = LZ4_compressBound((int)work->size);//find the maximum compressed size given the data size
			void* dat_buffer = heap_alloc(work->heap, dat_buffer_size, 8);//allocate buffer for compressed data
			//compress the given data into new form using LZ4
			int comp_size = LZ4_compress_default(work->buffer, dat_buffer, (int)work->size, dat_buffer_size);
			//pass compressed data back into the buffer for work
			work->buffer = dat_buffer;
			queue_push(fs->file_queue, work);//finish the write operation with normal file operations
			break; 
		}
		}
	}
	return 0;
}