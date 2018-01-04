#include "s3dat_ext.h"

#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

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
	s3dat_color_type type;

	s3dat_color_t* data;

};

struct s3dat_string_t {
	s3dat_t* src;

	bool original_encoding;
	s3dat_language language;
	uint8_t* string_data;
};

struct s3dat_animation_t {
	s3dat_t* src;

	uint32_t len;
	s3dat_frame_t* frames;
};


typedef struct s3dat_cache_t s3dat_cache_t;
typedef struct s3dat_monitor_t s3dat_monitor_t;

struct s3dat_cache_t {
	s3dat_t* parent;
	s3dat_res_t res;
	s3dat_cache_t* next;
};

struct s3dat_monitor_t {
	void* io_arg;
	bool close;
	s3dat_ioset_t* ioset;

	void* mem_arg;
	void* (*alloc_func) (void*,size_t);
	void (*free_func) (void*,void*);

	uint32_t last_state;
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

struct s3dat_exception_t {
	uint32_t type;
	s3dat_t* parent;

	s3dat_internal_stack_t* stack;
	s3dat_internal_attribute_t* attrs;
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
	void* mem_arg;
	void* io_arg;
	void* (*alloc_func) (void*, size_t);
	void (*free_func) (void*, void*);
	bool (*read_func) (void*, void*, size_t);
	bool (*write_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*, bool);
	void (*close_func) (void*);
	void* (*fork_func) (void*);

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

void s3dat_internal_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, s3dat_ref_t** to, s3dat_exception_t** throws);
void s3dat_internal_extract_bitmap(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);

void s3dat_pack_animation(s3dat_t* handle, s3dat_animation_t* animation, s3dat_packed_t* packed, s3dat_exception_t** throws);
void s3dat_pack_palette(s3dat_t* handle, s3dat_bitmap_t* palette, s3dat_packed_t* packed, s3dat_exception_t** throws);
void s3dat_pack_bitmap(s3dat_t* handle, s3dat_bitmap_t* bitmap, s3dat_content_type type, s3dat_packed_t* packed, s3dat_exception_t** throws);
void s3dat_pack_string(s3dat_t* handle, s3dat_string_t* string, s3dat_packed_t* packed, s3dat_exception_t** throws);
void s3dat_pack_sound(s3dat_t* handle, s3dat_sound_t* sound, s3dat_packed_t* packed, s3dat_exception_t** throws);

void s3dat_internal_8b_to_native(s3dat_color_t* color, void* to, s3dat_color_type type);

void s3dat_unpack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);
void s3dat_read_packed_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);
void s3dat_utf8_encoding_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);

void s3dat_internal_readsnd_index(s3dat_t* handle, uint32_t from, s3dat_index32_t* to, s3dat_exception_t** throws);

void s3dat_internal_seek_func(s3dat_t* handle, uint32_t pos, int whence, s3dat_exception_t** throws);

s3dat_restype_t* s3dat_internal_get_restype(s3dat_ref_type type);

uint32_t s3dat_internal_seek_to(s3dat_t* handle, s3dat_res_t* res, s3dat_exception_t** throws);

uint32_t s3dat_le32(uint32_t le32_int);
uint16_t s3dat_le16(uint16_t le16_int);

uint32_t s3dat_le32p(uint32_t* le32_int);
uint16_t s3dat_le16p(uint16_t* le16_int);

uint32_t s3dat_internal_read32LE(s3dat_t* handle, s3dat_exception_t** throws);
uint16_t s3dat_internal_read16LE(s3dat_t* handle, s3dat_exception_t** throws);
uint8_t s3dat_internal_read8(s3dat_t* handle, s3dat_exception_t** throws);

void s3dat_internal_write32LE(s3dat_t* handle, uint32_t b32_int, s3dat_exception_t** throws);
void s3dat_internal_write16LE(s3dat_t* handle, uint16_t b16_int, s3dat_exception_t** throws);
void s3dat_internal_write8(s3dat_t* handle, uint8_t b8_int, s3dat_exception_t** throws);

void s3dat_internal_readsnd(s3dat_t* handle, s3dat_exception_t** throws);



void s3dat_delete_ref(s3dat_ref_t* ref);
void s3dat_delete_ref_array(s3dat_ref_t** refs, uint32_t count);

s3dat_animation_t* s3dat_new_raw_animation(s3dat_t* parent, s3dat_exception_t** throws);
s3dat_bitmap_t* s3dat_new_raw_bitmap(s3dat_t* parent, s3dat_exception_t** throws);
s3dat_sound_t* s3dat_new_raw_sound(s3dat_t* parent, s3dat_exception_t** throws);
s3dat_string_t* s3dat_new_raw_string(s3dat_t* parent, s3dat_exception_t** throws);
s3dat_packed_t* s3dat_new_raw_packed(s3dat_t* parent, s3dat_exception_t** throws);

void s3dat_delete_animation(s3dat_animation_t* ani);
void s3dat_delete_bitmap(s3dat_bitmap_t* bmp);
void s3dat_delete_sound(s3dat_sound_t* sound);
void s3dat_delete_string(s3dat_string_t* string);
void s3dat_delete_packed(s3dat_packed_t* package);


void s3dat_monitor_print(s3dat_monitor_t* monitor);

s3dat_cache_t* s3dat_new_cache(s3dat_t* handle, s3dat_exception_t** throws);
void s3dat_delete_cache_r(s3dat_cache_t* cache);


void s3dat_print_exception(s3dat_exception_t* ex);
void s3dat_delete_exception(s3dat_t* handle, s3dat_exception_t* ex);

//ioset and memory functions

void* s3dat_monitor_alloc_func(void* arg, size_t size);
void s3dat_monitor_free_func(void* arg, void* mem);

void* s3dat_default_alloc_func(void* arg, size_t size);
void s3dat_default_free_func(void* arg, void* mem);

//linux
void* s3dat_linux_open_func(void* arg, bool write);
void s3dat_linux_close_func(void* arg);
bool s3dat_linux_read_func(void* arg, void* bfr, size_t len);
bool s3dat_linux_write_func(void* arg, void* bfr, size_t len);
bool s3dat_linux_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_linux_pos_func(void* arg);
size_t s3dat_linux_size_func(void* arg);

void* s3dat_mmf_linux_fd_open_func(void* arg, bool write);
void* s3dat_mmf_linux_name_open_func(void* arg, bool write);
void s3dat_mmf_linux_close_func(void* arg);

//windows
void* s3dat_win32_open_func(void* arg, bool write);
void s3dat_win32_close_func(void* arg);
bool s3dat_win32_read_func(void* arg, void* bfr, size_t len);
bool s3dat_win32_write_func(void* arg, void* bfr, size_t len);
bool s3dat_win32_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_win32_pos_func(void* arg);
size_t s3dat_win32_size_func(void* arg);

void* s3dat_mmf_win32_handle_open_func(void* arg, bool write);
void* s3dat_mmf_win32_name_open_func(void* arg, bool write);
void s3dat_mmf_win32_close_func(void* arg);

void* s3dat_libc_open_func(void* arg, bool write);
void s3dat_libc_close_func(void* arg);
bool s3dat_libc_read_func(void* arg, void* bfr, size_t len);
bool s3dat_libc_write_func(void* arg, void* bfr, size_t len);
bool s3dat_libc_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_libc_pos_func(void* arg);
size_t s3dat_libc_size_func(void* arg);

bool s3dat_mmf_read_func(void* arg, void* bfr, size_t len);
bool s3dat_mmf_write_func(void* arg, void* bfr, size_t len);
bool s3dat_mmf_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_mmf_pos_func(void* arg);
size_t s3dat_mmf_size_func(void* arg);
void* s3dat_mmf_fork_func(void* arg);

#endif /*S3DAT_INTERNAL_H*/

