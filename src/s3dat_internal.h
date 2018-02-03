#include "s3dat_ext.h"

#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef S3DAT_INTERNAL_H
#define S3DAT_INTERNAL_H

#define S3DAT_INTERNAL_READ(type, handle, throws) S3UTIL_INTERNAL_READ(type, s3dat_ioset(handle), s3dat_memset(handle), throws);
#define S3DAT_INTERNAL_WRITE(type, handle, to, throws) S3UTIL_INTERNAL_WRITE(type, s3dat_ioset(handle), s3dat_memset(handle), to, throws);

struct s3dat_sound_t {
	s3dat_t* src;

	uint32_t freq;
	uint16_t len;
	int16_t* data;
};

struct s3dat_bitmap_t {
	uint16_t width;
	uint16_t height;

	uint16_t landscape_type;
	uint32_t gui_type;

	int16_t xoff;
	int16_t yoff;

	s3dat_t* src;
	s3util_color_type type;

	s3util_color_t* data;

};

struct s3dat_string_t {
	s3dat_t* src;

	bool original_encoding;
	s3dat_language language;
	char* string_data;
};

struct s3dat_animation_t {
	s3dat_t* src;

	uint32_t len;
	s3dat_frame_t* frames;
};


typedef struct s3dat_cache_t s3dat_cache_t;

struct s3dat_cache_t {
	s3dat_t* parent;
	s3dat_res_t res;
	s3dat_cache_t* next;
};


typedef struct s3dat_mmf_t s3dat_mmf_t;

struct s3dat_mmf_t {
	void* addr;
	uint32_t pos;
	uint32_t len;
	bool fork;

	void* additional_data; // win32 handles
};


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

typedef struct {
	s3dat_content_type type;
	uint16_t len;
	uint32_t* pointers;
} s3dat_index_t;

typedef struct {
	s3dat_content_type type;
	uint32_t len;
	uint32_t* pointers;
} s3dat_index32_t;

typedef struct {
	s3dat_content_type type;
	uint16_t len;
	s3dat_index_t* sequences;
} s3dat_seq_index_t;

typedef struct {
	s3dat_content_type type;
	uint16_t len;
	s3dat_index32_t* sequences;
} s3dat_seq_index32_t;


struct s3dat_t {
	s3util_memset_t memset;
	s3util_ioset_t ioset;

	bool green_6b;
	uint32_t palette_line_length;
	s3dat_extracthandler_t* last_handler;

	s3dat_seq_index_t* settler_index;
	s3dat_seq_index_t* shadow_index;
	s3dat_seq_index_t* torso_index;
	s3dat_seq_index_t* string_index;
	s3dat_seq_index32_t* sound_index; // SND .dat files only
	s3dat_index_t* landscape_index;
	s3dat_index_t* gui_index;
	s3dat_index_t* animation_index;
	s3dat_index_t* palette_index;

};

void s3dat_internal_read_index(s3dat_t* handle, uint32_t index, s3util_exception_t** throws);
void s3dat_internal_read_seq(s3dat_t* handle, uint32_t from, s3dat_index_t* to, s3util_exception_t** throws);

void s3dat_internal_delete_index(s3dat_t* handle, s3dat_index_t* index);
void s3dat_internal_delete_indices(s3dat_t* handle, s3dat_index_t* indices, uint32_t count);
void s3dat_internal_delete_index32(s3dat_t* handle, s3dat_index32_t* index);
void s3dat_internal_delete_seq(s3dat_t* handle, s3dat_seq_index_t* seq);
void s3dat_internal_delete_seq32(s3dat_t* handle, s3dat_seq_index32_t* seq);

void s3dat_internal_read_bitmap_data(s3dat_t* handle, s3util_color_type type, uint16_t width, uint16_t height, s3util_color_t** re_pixdata, s3util_exception_t** throws);
void s3dat_internal_read_bitmap_header(s3dat_t* handle, s3dat_content_type type, uint32_t from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff, s3util_exception_t** throws);

void s3dat_internal_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, s3dat_ref_t** to, s3util_exception_t** throws);
void s3dat_internal_extract_bitmap(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws);

void s3dat_pack_animation(s3dat_t* handle, s3dat_animation_t* animation, s3dat_packed_t* packed, s3util_exception_t** throws);
void s3dat_pack_palette(s3dat_t* handle, s3dat_bitmap_t* palette, s3dat_packed_t* packed, s3util_exception_t** throws);
void s3dat_pack_bitmap(s3dat_t* handle, s3dat_bitmap_t* bitmap, s3dat_content_type type, s3dat_packed_t* packed, s3util_exception_t** throws);
void s3dat_pack_string(s3dat_t* handle, s3dat_string_t* string, s3dat_packed_t* packed, s3util_exception_t** throws);
void s3dat_pack_sound(s3dat_t* handle, s3dat_sound_t* sound, s3dat_packed_t* packed, s3util_exception_t** throws);

void s3dat_unpack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws);
void s3dat_read_packed_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws);
void s3dat_utf8_encoding_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws);

void s3dat_internal_readsnd_index(s3dat_t* handle, uint32_t from, s3dat_index32_t* to, s3util_exception_t** throws);

void s3dat_internal_seek_func(s3dat_t* handle, uint32_t pos, int whence, s3util_exception_t** throws);

s3dat_restype_t* s3dat_internal_get_restype(s3dat_ref_type type);

uint32_t s3dat_internal_seek_to(s3dat_t* handle, s3dat_res_t* res, s3util_exception_t** throws);

void s3dat_internal_readsnd(s3dat_t* handle, s3util_exception_t** throws);



void s3dat_delete_ref(s3dat_ref_t* ref);
void s3dat_delete_ref_array(s3dat_ref_t** refs, uint32_t count);

s3dat_animation_t* s3dat_new_raw_animation(s3dat_t* parent, s3util_exception_t** throws);
s3dat_bitmap_t* s3dat_new_raw_bitmap(s3dat_t* parent, s3util_exception_t** throws);
s3dat_sound_t* s3dat_new_raw_sound(s3dat_t* parent, s3util_exception_t** throws);
s3dat_string_t* s3dat_new_raw_string(s3dat_t* parent, s3util_exception_t** throws);
s3dat_packed_t* s3dat_new_raw_packed(s3dat_t* parent, s3util_exception_t** throws);

void s3dat_delete_animation(s3dat_animation_t* ani);
void s3dat_delete_bitmap(s3dat_bitmap_t* bmp);
void s3dat_delete_sound(s3dat_sound_t* sound);
void s3dat_delete_string(s3dat_string_t* string);
void s3dat_delete_packed(s3dat_packed_t* package);

s3dat_cache_t* s3dat_new_cache(s3dat_t* handle, s3util_exception_t** throws);
void s3dat_delete_cache_r(s3dat_cache_t* cache);

#endif /*S3DAT_INTERNAL_H*/

