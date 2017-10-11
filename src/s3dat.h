#ifndef S3DAT_H
#define S3DAT_H

#include <stddef.h>
#include <stdbool.h>
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

#define S3DAT_EXHANDLER_CALL(me, res, throws, file, func, line) \
	do { \
		me->before->call(me->before, res, throws); \
		S3DAT_HANDLE_EXCEPTION(mem, throws, file, func, line); \
	} while(0);

typedef struct s3dat_exception_t s3dat_exception_t;

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
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*);
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

	uint16_t xoff;
	uint16_t yoff;

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
	bool (*read_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (void*);
	void (*close_func) (void*);
	void* (*fork_func) (void*);
	bool available;
} s3dat_ioset_t;

typedef struct {
	uint16_t first_index;
	uint32_t second_index;
	s3dat_content_type type;
	void* resdata;
} s3dat_res_t;


struct  s3dat_extracthandler_t {
	void (*call) (s3dat_extracthandler_t*, s3dat_res_t*, s3dat_exception_t**);
	void* arg;
	void (*arg_deref) (void*);
	s3dat_t* parent;

	s3dat_extracthandler_t* before;
};

void s3dat_readfile_name(s3dat_t* mem, char* name, s3dat_exception_t** throws);

void s3dat_readfile_fd(s3dat_t* mem, uint32_t* file, s3dat_exception_t** throws);

void s3dat_readfile_ioset(s3dat_t* mem, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func, s3dat_exception_t** throws);

void s3dat_readfile_func(s3dat_t* mem, void* arg,
	bool (*read_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*),
	void (*close_func) (void*),
	void* (*fork_func) (void*),
	s3dat_exception_t** throws_out);

void s3dat_add_extracthandler(s3dat_t* mem, s3dat_extracthandler_t* exhandler);
bool s3dat_remove_extracthandler(s3dat_t* mem, uint32_t steps_back);
bool s3dat_remove_last_extracthandler(s3dat_t* mem);

void s3dat_extract(s3dat_t* mem, s3dat_res_t* res, s3dat_exception_t** throws);

s3dat_bitmap_t* s3dat_extract_settler(s3dat_t* mem, uint16_t settler, uint8_t frame, s3dat_exception_t** throws);
s3dat_bitmap_t* s3dat_extract_shadow(s3dat_t* mem, uint16_t shadow, uint8_t frame, s3dat_exception_t** throws);
s3dat_bitmap_t* s3dat_extract_torso(s3dat_t* mem, uint16_t torso, uint8_t frame, s3dat_exception_t** throws);

s3dat_bitmap_t* s3dat_extract_landscape(s3dat_t* mem, uint16_t landscape, s3dat_exception_t** throws);
s3dat_bitmap_t* s3dat_extract_gui(s3dat_t* mem, uint16_t gui, s3dat_exception_t** throws);

s3dat_animation_t* s3dat_extract_animation(s3dat_t* mem, uint16_t animation, s3dat_exception_t** throws);
s3dat_string_t* s3dat_extract_string(s3dat_t* mem, uint16_t text, uint16_t language, s3dat_exception_t** throws);
s3dat_bitmap_t* s3dat_extract_palette(s3dat_t* mem, uint16_t palette, s3dat_exception_t** throws);
s3dat_string_t* s3dat_extract_sound(s3dat_t* mem, uint16_t soundtype, uint32_t altindex, s3dat_exception_t** throws);


s3dat_color_t s3dat_extract_palette_color(s3dat_t* mem, uint16_t palette, uint8_t brightness, uint32_t x, s3dat_exception_t** throws);

void s3dat_add_utf8_encoding(s3dat_t* mem);
void s3dat_add_landscape_blending(s3dat_t* mem);

s3dat_t* s3dat_fork(s3dat_t* mem);
void s3dat_delete_fork(s3dat_t* mem);

//linux
void* s3dat_linux_open_func(void* arg);
void s3dat_linux_close_func(void* arg);
bool s3dat_linux_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_linux_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_linux_pos_func(void* arg);
size_t s3dat_linux_size_func(void* arg);

void* s3dat_mmf_linux_fd_open_func(void* arg);
void* s3dat_mmf_linux_name_open_func(void* arg);
void s3dat_mmf_linux_close_func(void* arg);

//windows
void* s3dat_win32_open_func(void* arg);
void s3dat_win32_close_func(void* arg);
bool s3dat_win32_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_win32_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_win32_pos_func(void* arg);
size_t s3dat_win32_size_func(void* arg);

void* s3dat_mmf_win32_handle_open_func(void* arg);
void* s3dat_mmf_win32_name_open_func(void* arg);
void s3dat_mmf_win32_close_func(void* arg);

void* s3dat_libc_open_func(void* arg);
void s3dat_libc_close_func(void* arg);
bool s3dat_libc_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_libc_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_libc_pos_func(void* arg);
size_t s3dat_libc_size_func(void* arg);

bool s3dat_mmf_read_func(void* arg, void* bfr, size_t len);
bool s3dat_mmf_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_mmf_pos_func(void* arg);
size_t s3dat_mmf_size_func(void* arg);
void* s3dat_mmf_fork_func(void* arg);

s3dat_ioset_t* s3dat_get_default_ioset(uint32_t type);

void* s3dat_default_alloc_func(void* arg, size_t size);
void s3dat_default_free_func(void* arg, void* mem); // NULL pointers must be ignored

s3dat_t* s3dat_new_malloc();
s3dat_t* s3dat_new_func(void* arg, void* (*alloc_func) (void*, size_t), void (*free_func) (void*, void*));

s3dat_animation_t* s3dat_new_animation(s3dat_t* parent);
s3dat_animation_t* s3dat_new_animations(s3dat_t* parent, uint32_t count);

s3dat_bitmap_t* s3dat_new_bitmap(s3dat_t* parent);
s3dat_bitmap_t* s3dat_new_bitmaps(s3dat_t* parent, uint32_t count);

s3dat_sound_t* s3dat_new_sound(s3dat_t* parent);
s3dat_sound_t* s3dat_new_sounds(s3dat_t* parent, uint32_t count);

s3dat_string_t* s3dat_new_string(s3dat_t* parent);
s3dat_string_t* s3dat_new_strings(s3dat_t* parent, uint32_t count);

s3dat_extracthandler_t* s3dat_new_exhandler(s3dat_t* parent);
s3dat_extracthandler_t* s3dat_new_exhandlers(s3dat_t* parent, uint32_t count);

void s3dat_delete(s3dat_t* mem);


void s3dat_delete_animation(s3dat_animation_t* ani);
void s3dat_delete_animations(s3dat_animation_t* anis, uint32_t count);
void s3dat_delete_animation_array(s3dat_animation_t** anis, uint32_t count);

void s3dat_delete_frame(s3dat_animation_t* ani);
void s3dat_delete_frames(s3dat_animation_t* anis, uint32_t count);
void s3dat_delete_frame_array(s3dat_animation_t** anis, uint32_t count);


void s3dat_delete_bitmap(s3dat_bitmap_t* bmp);
void s3dat_delete_bitmaps(s3dat_bitmap_t* bmps, uint32_t count);
void s3dat_delete_bitmap_array(s3dat_bitmap_t** bmps, uint32_t count);

void s3dat_delete_pixdata(s3dat_bitmap_t* bmp);
void s3dat_delete_pixdatas(s3dat_bitmap_t* bmps, uint32_t count);
void s3dat_delete_pixdata_array(s3dat_bitmap_t** bmps, uint32_t count);


void s3dat_delete_sound(s3dat_sound_t* sound);
void s3dat_delete_sounds(s3dat_sound_t* sounds, uint32_t count);
void s3dat_delete_sound_array(s3dat_sound_t** sounds, uint32_t count);

void s3dat_delete_snddata(s3dat_sound_t* sound);
void s3dat_delete_snddatas(s3dat_sound_t* sounds, uint32_t count);
void s3dat_delete_snddata_array(s3dat_sound_t** sounds, uint32_t count);


void s3dat_delete_string(s3dat_string_t* string);
void s3dat_delete_strings(s3dat_string_t* strings, uint32_t count);
void s3dat_delete_string_array(s3dat_string_t** strings, uint32_t count);

void s3dat_delete_stringdata(s3dat_string_t* string);
void s3dat_delete_stringdatas(s3dat_string_t* strings, uint32_t count);
void s3dat_delete_stringdata_array(s3dat_string_t** strings, uint32_t count);


void s3dat_delete_exhandler(s3dat_extracthandler_t* exhandler);
void s3dat_delete_exhandlers(s3dat_extracthandler_t* exhandlers, uint32_t count);

void s3dat_print_exception(s3dat_exception_t* ex); // debugging only
void s3dat_delete_exception(s3dat_t* mem, s3dat_exception_t* ex);
void s3dat_throw(s3dat_t* mem, s3dat_exception_t** throws, uint32_t exception, char* file, const char* function, int line);
bool s3dat_catch_exception(s3dat_exception_t** throws, s3dat_t* from);
#endif /*S3DAT_H*/

