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

#ifdef __linux__
#include <sys/mman.h>
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

#define S3DAT_HANDLE_EXCEPTION(handle, throws, file, function, line)  \
	if(*throws != NULL) { \
		s3dat_add_to_stack(handle, throws, file, function, line); \
		return; \
	}


#define S3DAT_INTERNAL_ADD_ATTR(handle, throws, attr, value)  \
	if(*throws != NULL) { \
		s3dat_add_attr(handle, throws, attr, value); \
	}

#define S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws) \
	s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0)

#define S3DAT_SAFE_READ(to, len, file, function, line) \
		if(!handle->read_func(handle->io_arg, to, len)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, file, function, line);

typedef struct s3dat_internal_stack_t s3dat_internal_stack_t;
typedef struct s3dat_internal_attribute_t s3dat_internal_attribute_t;

struct s3dat_internal_stack_t {
	uint8_t* file;
	const uint8_t* function;
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



void s3dat_internal_read_index(s3dat_t* handle, uint32_t index, s3dat_exception_t** throws);
void s3dat_internal_read_seq(s3dat_t* handle, uint32_t from, s3dat_index_t* to, s3dat_exception_t** throws);

void s3dat_internal_delete_index(s3dat_t* handle, s3dat_index_t* index);
void s3dat_internal_delete_indices(s3dat_t* handle, s3dat_index_t* indices, uint32_t count);
void s3dat_internal_delete_index32(s3dat_t* handle, s3dat_index32_t* index);
void s3dat_internal_delete_seq(s3dat_t* handle, s3dat_seq_index_t* seq);
void s3dat_internal_delete_seq32(s3dat_t* handle, s3dat_seq_index32_t* seq);

void s3dat_internal_read_bitmap_data(s3dat_t* handle, s3dat_color_type type, uint16_t width, uint16_t height, s3dat_color_t** re_pixdata, s3dat_exception_t** throws);
void s3dat_internal_read_bitmap_header(s3dat_t* handle, s3dat_content_type type, uint32_t from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws);
s3dat_color_t s3dat_internal_ex(void* addr, s3dat_color_type type);

void s3dat_internal_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, void** to, s3dat_exception_t** throws);
void s3dat_internal_extract_bitmap(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);


void s3dat_unpack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);
void s3dat_read_packed_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);
void s3dat_utf8_encoding_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);

void s3dat_internal_readsnd_index(s3dat_t* handle, uint32_t from, s3dat_index32_t* to, s3dat_exception_t** throws);

void s3dat_internal_seek_func(s3dat_t* handle, uint32_t pos, int whence, s3dat_exception_t** throws);


uint32_t s3dat_internal_seek_to(s3dat_t* handle, s3dat_res_t* res, s3dat_exception_t** throws);

uint32_t le32(uint32_t le32_int);
uint16_t le16(uint16_t le16_int);

uint32_t le32p(uint32_t* le32_int);
uint16_t le16p(uint16_t* le16_int);

uint32_t s3dat_internal_read32LE(s3dat_t* handle, s3dat_exception_t** throws);
uint16_t s3dat_internal_read16LE(s3dat_t* handle, s3dat_exception_t** throws);
uint16_t s3dat_internal_read8(s3dat_t* handle, s3dat_exception_t** throws);
void s3dat_internal_readsnd(s3dat_t* handle, s3dat_exception_t** throws);

void s3dat_add_to_stack(s3dat_t* handle, s3dat_exception_t** throws, uint8_t* file, const uint8_t* function, uint32_t line);
void s3dat_add_attr(s3dat_t* handle, s3dat_exception_t** throws, uint32_t name, uint32_t value);

void* s3dat_internal_alloc_func(s3dat_t* handle, size_t size, s3dat_exception_t** throws);

#endif /*S3DAT_INTERNAL_H*/

