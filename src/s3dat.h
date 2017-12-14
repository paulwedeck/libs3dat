#ifndef S3DAT_H
#define S3DAT_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define S3DAT_SEEK_CUR 0x20
#define S3DAT_SEEK_SET 0x21

#define S3DAT_IOSET_NATIVEOS 0x400
#define S3DAT_IOSET_LINUX 0x401
#define S3DAT_IOSET_WIN32 0x402

#define S3DAT_IOSET_DEFAULT 0x500
#define S3DAT_IOSET_LIBC 0x501

//memory mapped file
#define S3DAT_IOSET_NATIVEOS_MMF 0x600
#define S3DAT_IOSET_LINUX_MMF 0x601
#define S3DAT_IOSET_WIN32_MMF 0x602
#define S3DAT_IOSET_LINUX_MMF_FD 0x603
#define S3DAT_IOSET_WIN32_MMF_HANDLE 0x604

#define S3DAT_EXCEPTION_WRONG_RESTYPE 0x109

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



typedef struct s3dat_exception_t s3dat_exception_t;

typedef enum {
	s3dat_pkd_ref = 0x10FFFFFF,
	s3dat_bmp_ref = 0x11FFFFFF,
	s3dat_snd_ref = 0x12FFFFFF,
	s3dat_str_ref = 0x13FFFFFF,
	s3dat_ani_ref = 0x14FFFFFF,
} s3dat_ref_type;

typedef enum {
	s3dat_snd = 0xFFFF, // SND .dat files only
	s3dat_settler = 0x106,
	s3dat_torso = 0x3112,
	s3dat_shadow = 0x5982,
	s3dat_landscape = 0x2412,
	s3dat_gui = 0x11306,
	s3dat_animation = 0x21702,
	s3dat_palette = 0x2607,
	s3dat_string = 0x1904,
} s3dat_content_type;


typedef enum {
	s3dat_german = 0,
	s3dat_english = 1,
	s3dat_italian = 2,
	s3dat_french = 3,
	s3dat_polish = 4,
	s3dat_spanish = 5,
	s3dat_korean = 6,
	s3dat_japanese = 7,
	s3dat_language_count = 8
} s3dat_language;

typedef enum {
	s3dat_landscape_big = 0x101,
	s3dat_landscape_little = 0x201,
	s3dat_landscape_t3 = 0x301,
	s3dat_landscape_t4 = 0x401,
	s3dat_landscape_big2 = 0x501,
} s3dat_landscape_type;

typedef struct s3dat_extracthandler_t s3dat_extracthandler_t;
typedef struct s3dat_bitmap_t s3dat_bitmap_t;
typedef struct s3dat_sound_t s3dat_sound_t;

typedef enum {
	s3dat_alpha1,
	s3dat_rgb565,
	s3dat_rgb555,
	s3dat_gray5
} s3dat_color_type;

typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} s3dat_color_t;

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

typedef struct s3dat_t s3dat_t;

typedef struct {
	void* addr;
	uint32_t pos;
	uint32_t len;
	bool fork;

	void* additional_data; // win32 handles
} s3dat_mmf_t;

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

typedef struct {
	int16_t posx;
	int16_t posy;

	uint16_t settler_file;
	uint16_t settler_id;
	uint16_t settler_frame;

	uint16_t torso_file;
	uint16_t torso_id;
	uint16_t torso_frame;

	uint16_t shadow_file;
	uint16_t shadow_id;

	uint16_t flag1;
	uint16_t flag2;
} s3dat_frame_t;

typedef struct {
	s3dat_t* src;

	bool original_encoding;
	s3dat_language language;
	uint8_t* string_data;
} s3dat_string_t;

typedef struct {
	s3dat_t* src;

	uint32_t len;
	s3dat_frame_t* frames;
} s3dat_animation_t;


typedef struct {
	s3dat_t* parent;
	uint32_t len;
	void* data;
} s3dat_packed_t;

typedef struct {
	bool (*read_func) (void*, void*, size_t);
	bool (*write_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*, bool);
	void (*close_func) (void*);
	void* (*fork_func) (void*);
	bool available;
} s3dat_ioset_t;

typedef struct {
	char* name;
	void (*deref) (void*);
	void* (*alloc) (void*);
} s3dat_restype_t;


typedef struct {
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
} s3dat_ref_t;

typedef struct {
	uint16_t first_index;
	uint32_t second_index;
	s3dat_content_type type;
	s3dat_ref_t* res;
} s3dat_res_t;

typedef struct s3dat_cache_t s3dat_cache_t;

struct s3dat_cache_t {
	s3dat_t* parent;
	s3dat_res_t res;
	s3dat_cache_t* next;
};

struct s3dat_extracthandler_t {
	void (*call) (s3dat_extracthandler_t*, s3dat_res_t*, s3dat_exception_t**);
	void* arg;
	void (*arg_deref) (void*);
	s3dat_t* parent;

	s3dat_extracthandler_t* before;
};

void s3dat_writefile_name(s3dat_t* handle, uint8_t* name, s3dat_exception_t** throws);
void s3dat_writefile_fd(s3dat_t* handle, uint32_t* file, s3dat_exception_t** throws);
void s3dat_writefile_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func, s3dat_exception_t** throws);

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

void s3dat_readfile_name(s3dat_t* handle, uint8_t* name, s3dat_exception_t** throws);
void s3dat_readfile_fd(s3dat_t* handle, uint32_t* file, s3dat_exception_t** throws);
void s3dat_readfile_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func, s3dat_exception_t** throws);

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

void s3dat_init_name(s3dat_t* handle, uint8_t* name); //name must be vaild until s3dat_readfile/s3dat_writefile has ended
void s3dat_init_fd(s3dat_t* handle, uint32_t* file); //file must be vaild until s3dat_readfile/s3dat_writefile has ended
bool s3dat_init_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func);

void s3dat_init_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*));

void s3dat_writefile(s3dat_t* handle, s3dat_exception_t** throws);
void s3dat_readfile(s3dat_t* handle, s3dat_exception_t** throws);

void s3dat_add_extracthandler(s3dat_t* handle, s3dat_extracthandler_t* exhandler);
bool s3dat_remove_extracthandler(s3dat_t* handle, uint32_t steps_back);
bool s3dat_remove_last_extracthandler(s3dat_t* handle);

void s3dat_extract(s3dat_t* handle, s3dat_res_t* res, s3dat_exception_t** throws);

s3dat_ref_t* s3dat_extract_settler(s3dat_t* handle, uint16_t settler, uint8_t frame, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_shadow(s3dat_t* handle, uint16_t shadow, uint8_t frame, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_torso(s3dat_t* handle, uint16_t torso, uint8_t frame, s3dat_exception_t** throws);

s3dat_ref_t* s3dat_extract_landscape(s3dat_t* handle, uint16_t landscape, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_gui(s3dat_t* handle, uint16_t gui, s3dat_exception_t** throws);

s3dat_ref_t* s3dat_extract_animation(s3dat_t* handle, uint16_t animation, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_palette(s3dat_t* handle, uint16_t palette, s3dat_exception_t** throws);
s3dat_ref_t* s3dat_extract_sound(s3dat_t* handle, uint16_t soundtype, uint32_t altindex, s3dat_exception_t** throws);


s3dat_color_t s3dat_extract_palette_color(s3dat_t* handle, uint16_t palette, uint8_t brightness, uint32_t x, s3dat_exception_t** throws);

void s3dat_add_cache(s3dat_t* parent);
void s3dat_add_utf8_encoding(s3dat_t* handle);
void s3dat_add_landscape_blending(s3dat_t* handle);

s3dat_t* s3dat_fork(s3dat_t* handle);
void s3dat_delete_fork(s3dat_t* handle);

s3dat_t* s3dat_writeable_fork(s3dat_t* handle, void* io_arg);
void s3dat_pack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);

//linux
void* s3dat_linux_open_func(void* arg, bool write);
void s3dat_linux_close_func(void* arg);
bool s3dat_linux_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_linux_write_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_linux_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_linux_pos_func(void* arg);
size_t s3dat_linux_size_func(void* arg);

void* s3dat_mmf_linux_fd_open_func(void* arg, bool write);
void* s3dat_mmf_linux_name_open_func(void* arg, bool write);
void s3dat_mmf_linux_close_func(void* arg);

//windows
void* s3dat_win32_open_func(void* arg, bool write);
void s3dat_win32_close_func(void* arg);
bool s3dat_win32_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_win32_write_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_win32_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_win32_pos_func(void* arg);
size_t s3dat_win32_size_func(void* arg);

void* s3dat_mmf_win32_handle_open_func(void* arg, bool write);
void* s3dat_mmf_win32_name_open_func(void* arg, bool write);
void s3dat_mmf_win32_close_func(void* arg);

void* s3dat_libc_open_func(void* arg, bool write);
void s3dat_libc_close_func(void* arg);
bool s3dat_libc_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_libc_write_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_libc_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_libc_pos_func(void* arg);
size_t s3dat_libc_size_func(void* arg);

bool s3dat_mmf_read_func(void* arg, void* bfr, size_t len);
bool s3dat_mmf_write_func(void* arg, void* bfr, size_t len);
bool s3dat_mmf_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_mmf_pos_func(void* arg);
size_t s3dat_mmf_size_func(void* arg);
void* s3dat_mmf_fork_func(void* arg);

s3dat_ioset_t* s3dat_get_default_ioset(uint32_t type);

void* s3dat_default_alloc_func(void* arg, size_t size);
void s3dat_default_free_func(void* arg, void* mem); // NULL pointers must be ignored

void* s3dat_monitor_alloc_func(void* arg, size_t size);
void s3dat_monitor_free_func(void* arg, void* mem); // NULL pointers must be ignored

typedef struct {
	void* io_arg;
	bool close;
	s3dat_ioset_t* ioset;

	void* mem_arg;
	void* (*alloc_func) (void*,size_t);
	void (*free_func) (void*,void*);

	uint32_t last_state;
} s3dat_monitor_t;

s3dat_t* s3dat_new_malloc();
s3dat_t* s3dat_new_malloc_monitor(void* arg, s3dat_ioset_t* ioset, bool open);
s3dat_t* s3dat_new_func(void* arg, void* (*alloc_func) (void*, size_t), void (*free_func) (void*, void*));

s3dat_ref_t* s3dat_new_animation(s3dat_t* parent);
s3dat_ref_t* s3dat_new_bitmap(s3dat_t* parent);
s3dat_ref_t* s3dat_new_string(s3dat_t* parent);
s3dat_ref_t* s3dat_new_packed(s3dat_t* parent);
s3dat_ref_t* s3dat_new_sound(s3dat_t* parent);

s3dat_extracthandler_t* s3dat_new_exhandler(s3dat_t* parent);
s3dat_extracthandler_t* s3dat_new_exhandlers(s3dat_t* parent, uint32_t count);

void s3dat_delete(s3dat_t* handle);


void s3dat_delete_packed(s3dat_packed_t* package);


void s3dat_ref(s3dat_ref_t* ref);
void s3dat_unref(s3dat_ref_t* ref);

void s3dat_ref_array(s3dat_ref_t** refs, uint32_t count);
void s3dat_unref_array(s3dat_ref_t** refs, uint32_t count);

void s3dat_delete_exhandler(s3dat_extracthandler_t* exhandler);
void s3dat_delete_exhandlers(s3dat_extracthandler_t* exhandlers, uint32_t count);

void s3dat_print_exception(s3dat_exception_t* ex); // debugging only
void s3dat_delete_exception(s3dat_t* handle, s3dat_exception_t* ex);
void s3dat_throw(s3dat_t* handle, s3dat_exception_t** throws, uint32_t exception, uint8_t* file, const uint8_t* function, uint32_t line);
bool s3dat_catch_exception(s3dat_exception_t** throws, s3dat_t* from);
void s3dat_add_to_stack(s3dat_t* handle, s3dat_exception_t** throws, uint8_t* file, const uint8_t* function, uint32_t line);
#endif /*S3DAT_H*/

