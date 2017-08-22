#include "s3dat.h"

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef _WIN32
#include <endian.h>
#endif

#ifdef USE_ICONV
#include <iconv.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef S3DAT_INTERNAL_H
#define S3DAT_INTERNAL_H

#define S3DAT_EXCEPTION_IOERROR 0x101
#define S3DAT_EXCEPTION_HEADER 0x102
#define S3DAT_EXCEPTION_CONFLICTING_DATA 0x103
#define S3DAT_EXCEPTION_INDEXTYPE 0x104
#define S3DAT_EXCEPTION_OUT_OF_RANGE 0x105
#define S3DAT_EXCEPTION_ICONV_ERROR 0x106
#define S3DAT_EXCEPTION_OPEN 0x107
#define S3DAT_EXCEPTION_IOSET 0x108
#define S3DAT_EXCEPTION_OUT_OF_MEMORY 0x200

#define S3DAT_ATTRIBUTE_INDEX 0x200
#define S3DAT_ATTRIBUTE_SEQ 0x201
#define S3DAT_ATTRIBUTE_SONG 0x202

#define S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, file, function, line)  \
	if(*throws != NULL) { \
		s3dat_add_to_stack(mem, throws, file, function, line); \
		return; \
	}


#define S3DAT_INTERNAL_ADD_ATTR(mem, throws, attr, value)  \
	if(*throws != NULL) { \
		s3dat_add_attr(mem, throws, attr, value); \
	}

#define S3DAT_INTERNAL_OUT_OF_MEMORY(mem, throws) \
	s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0)


typedef struct s3dat_internal_stack_t s3dat_internal_stack_t;
typedef struct s3dat_internal_attribute_t s3dat_internal_attribute_t;

struct s3dat_internal_stack_t {
	char* file;
	const char* function;
	uint32_t line;

	s3dat_internal_stack_t* down;
};

struct s3dat_internal_attribute_t {
	uint32_t name;
	uint32_t value;

	s3dat_internal_attribute_t* next;
};

struct s3dat_exception_t {
	uint32_t type;

	s3dat_internal_stack_t* stack;
	s3dat_internal_attribute_t* attrs;
};



void s3dat_internal_read_index(s3dat_t* mem, uint32_t index, s3dat_exception_t** throws);
void s3dat_internal_read_seq(s3dat_t* mem, uint32_t from, s3dat_index_t* to, s3dat_exception_t** throws);

void s3dat_internal_delete_index(s3dat_t* mem, s3dat_index_t* index);
void s3dat_internal_delete_indices(s3dat_t* mem, s3dat_index_t* indices, uint32_t count);
void s3dat_internal_delete_index32(s3dat_t* mem, s3dat_index32_t* index);
void s3dat_internal_delete_seq(s3dat_t* mem, s3dat_seq_index_t* seq);
void s3dat_internal_delete_seq32(s3dat_t* mem, s3dat_seq_index32_t* seq);

void s3dat_internal_read_bitmap_data(s3dat_t* mem, s3dat_color_type type, uint16_t width, uint16_t height, s3dat_color_t** re_pixdata, s3dat_exception_t** throws);
void s3dat_internal_read_bitmap_header(s3dat_t* mem, s3dat_content_type type, int from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws);
s3dat_color_t s3dat_internal_ex(s3dat_t* mem, s3dat_color_type type, s3dat_exception_t** throws);

void s3dat_internal_readsnd_index(s3dat_t* mem, uint32_t from, s3dat_index32_t* to, s3dat_exception_t** throws);

void s3dat_internal_seek_func(s3dat_t* mem, uint32_t pos, int whence, s3dat_exception_t** throws);

uint32_t s3dat_internal_read32LE(s3dat_t* mem, s3dat_exception_t** throws);
uint16_t s3dat_internal_read16LE(s3dat_t* mem, s3dat_exception_t** throws);
uint16_t s3dat_internal_read8(s3dat_t* mem, s3dat_exception_t** throws);
void s3dat_internal_readsnd(s3dat_t* mem, s3dat_exception_t** throws);

void s3dat_add_to_stack(s3dat_t* mem, s3dat_exception_t** throws, char* file, const char* function, int line);
void s3dat_internal_throw(s3dat_t* mem, s3dat_exception_t** throws, uint32_t exception, char* file, const char* function, int line);
void s3dat_add_attr(s3dat_t* mem, s3dat_exception_t** throws, uint32_t name, uint32_t value);

void* s3dat_internal_alloc_func(s3dat_t* mem, size_t size, s3dat_exception_t** throws);

#endif /*S3DAT_INTERNAL_H*/
