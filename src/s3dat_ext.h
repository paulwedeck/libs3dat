#ifndef S3DAT_EXT_H
#define S3DAT_EXT_H

// header for custom extract_handlers, ref types, whatever

// <yourprefix>_free_func must ignore NULL pointers
// <yourprefix>_read_func and _write_func should not care about endianness

#include "s3dat.h"

#define S3DAT_ATTRIBUTE_INDEX 0x200
#define S3DAT_ATTRIBUTE_SEQ 0x201
#define S3DAT_ATTRIBUTE_SONG 0x20

#define S3DAT_EXCEPTION_WRONG_RESTYPE 0x109
#define S3DAT_EXCEPTION_IOERROR 0x101
#define S3DAT_EXCEPTION_HEADER 0x102
#define S3DAT_EXCEPTION_CONFLICTING_DATA 0x103
#define S3DAT_EXCEPTION_INDEXTYPE 0x104
#define S3DAT_EXCEPTION_OUT_OF_RANGE 0x105
#define S3DAT_EXCEPTION_ICONV_ERROR 0x106
#define S3DAT_EXCEPTION_OPEN 0x107
#define S3DAT_EXCEPTION_IOSET 0x108
#define S3DAT_EXCEPTION_WRONG_RESTYPE 0x109
#define S3DAT_EXCEPTION_OUT_OF_MEMORY 0x200

#define S3DAT_SEEK_CUR 0x20
#define S3DAT_SEEK_SET 0x21

#define S3DAT_INTERNAL_ADD_ATTR(handle, throws, attr, value)  \
	if(*throws != NULL) { \
		s3dat_add_attr(handle, throws, attr, value); \
	}

#define S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws) \
	s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0)


#define S3DAT_EXHANDLER_CALL(me, res, throws, file, func, line) \
	do { \
		me->before->call(me->before, res, throws); \
		S3DAT_HANDLE_EXCEPTION(handle, throws, file, func, line); \
	} while(0);

#define S3DAT_HANDLE_EXCEPTION(handle, throws, file, function, line)  \
	if(*throws != NULL) { \
		s3dat_add_to_stack(handle, throws, file, function, line); \
		return; \
	}

#define S3DAT_CHECK_TYPE(handle, res_arg, wanted_type, throws, file, func, line) \
	if(strncmp(res_arg->res->type->name, wanted_type, strlen(wanted_type)) != 0) { \
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_WRONG_RESTYPE, file, func, line); \
		return; \
	}



struct s3dat_ioset_t {
	bool (*read_func) (void*, void*, size_t);
	bool (*write_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*, bool);
	void (*close_func) (void*);
	void* (*fork_func) (void*);
	bool available;
};

struct s3dat_restype_t {
	char* name;
	void (*deref) (void*);
	void* (*alloc) (void*);
};

struct s3dat_ref_t {
	s3dat_restype_t* type;
	uint32_t refs;
	s3dat_t* src;

	union {
		s3dat_animation_t* ani;
		s3dat_bitmap_t* bmp;
		s3dat_string_t* str;
		s3dat_sound_t* snd;
		s3dat_packed_t* pkd;
		void* raw;
	} data;
};

struct s3dat_packed_t {
	s3dat_t* parent;
	uint32_t len;
	void* data;
};

struct s3dat_extracthandler_t {
	void (*call) (s3dat_extracthandler_t*, s3dat_res_t*, s3dat_exception_t**);
	void* arg;
	void (*arg_deref) (void*);
	s3dat_t* parent;

	s3dat_extracthandler_t* before;
};

struct s3dat_res_t {
	uint16_t first_index;
	uint32_t second_index;
	s3dat_content_type type;
	s3dat_ref_t* res;
};

void s3dat_writefile_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*),
	s3dat_exception_t** throws);

void s3dat_readfile_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*),
	s3dat_exception_t** throws);

void s3dat_init_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*));


void s3dat_free_func(s3dat_t* handle, void* data);
void* s3dat_alloc_func(s3dat_t* handle, size_t size, s3dat_exception_t** throws);

void s3dat_pack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);

void s3dat_delete_exhandler(s3dat_extracthandler_t* exhandler);
void s3dat_delete_exhandlers(s3dat_extracthandler_t* exhandlers, uint32_t count);

void s3dat_add_attr(s3dat_t* handle, s3dat_exception_t** throws, uint32_t name, uint32_t value);
void s3dat_add_to_stack(s3dat_t* handle, s3dat_exception_t** throws, uint8_t* file, const uint8_t* function, uint32_t line);
void s3dat_throw(s3dat_t* handle, s3dat_exception_t** throws, uint32_t exception, uint8_t* file, const uint8_t* function, uint32_t line);

void s3dat_add_extracthandler(s3dat_t* handle, s3dat_extracthandler_t* exhandler);

void s3dat_extract(s3dat_t* handle, s3dat_res_t* res, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_arg(s3dat_t* handle, uint16_t first_index, uint32_t second_index, s3dat_content_type type, s3dat_exception_t** throws);

s3dat_ref_t* s3dat_new_animation(s3dat_t* parent, uint32_t frames);
s3dat_ref_t* s3dat_new_bitmap(s3dat_t* parent, uint16_t width, uint16_t height);
s3dat_ref_t* s3dat_new_string(s3dat_t* parent, uint32_t strlen);
s3dat_ref_t* s3dat_new_packed(s3dat_t* parent);
s3dat_ref_t* s3dat_new_sound(s3dat_t* parent, uint32_t freq, uint16_t len);

#endif /*S3DAT_EXT_H*/
