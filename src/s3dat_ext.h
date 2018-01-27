#ifndef S3DAT_EXT_H
#define S3DAT_EXT_H

// header for custom extract_handlers, ref types, whatever

// <yourprefix>_free_func must ignore NULL pointers
// <yourprefix>_read_func and _write_func should not care about endianness

#include "s3dat.h"

#define S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws) \
	s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0)


#define S3DAT_EXHANDLER_CALL(me, res, throws, file, func, line) \
	do { \
		me->before->call(me->before, res, throws); \
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, file, func, line); \
	} while(0);

#define S3DAT_CHECK_TYPE(handle, res_arg, wanted_type, throws, file, func, line) \
	if(strncmp(res_arg->res->type->name, wanted_type, strlen(wanted_type)) != 0) { \
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_WRONG_RESTYPE, file, func, line); \
		return; \
	}

struct s3dat_restype_t {
	char* name;
	void (*deref) (void*);
	void* (*alloc) (void*,s3util_exception_t**);
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
	void (*call) (s3dat_extracthandler_t*, s3dat_res_t*, s3util_exception_t**);
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
	s3util_exception_t** throws);

void s3dat_readfile_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*),
	s3util_exception_t** throws);

void s3dat_init_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*));


void s3dat_pack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws);

void s3dat_delete_exhandler(s3dat_extracthandler_t* exhandler);
void s3dat_delete_exhandlers(s3dat_extracthandler_t* exhandlers, uint32_t count);

void s3dat_add_extracthandler(s3dat_t* handle, s3dat_extracthandler_t* exhandler);

void s3dat_extract(s3dat_t* handle, s3dat_res_t* res, s3util_exception_t** throws);
s3dat_ref_t* s3dat_extract_arg(s3dat_t* handle, uint16_t first_index, uint32_t second_index, s3dat_content_type type, s3util_exception_t** throws);

s3dat_ref_t* s3dat_new_animation(s3dat_t* parent, uint32_t frames, s3util_exception_t** throws);
s3dat_ref_t* s3dat_new_bitmap(s3dat_t* parent, uint16_t width, uint16_t height, s3util_exception_t** throws);
s3dat_ref_t* s3dat_new_string(s3dat_t* parent, uint32_t strlen, s3util_exception_t** throws);
s3dat_ref_t* s3dat_new_packed(s3dat_t* parent, s3util_exception_t** throws);
s3dat_ref_t* s3dat_new_sound(s3dat_t* parent, uint32_t freq, uint16_t len, s3util_exception_t** throws);

#endif /*S3DAT_EXT_H*/
