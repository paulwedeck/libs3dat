#ifndef S3DAT_H
#define S3DAT_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

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
	s3dat_language_count = 8,
	s3dat_unknown_language = 9,
} s3dat_language;

typedef enum {
	s3dat_landscape_big = 0x101,
	s3dat_landscape_little = 0x201,
	s3dat_landscape_t3 = 0x301,
	s3dat_landscape_t4 = 0x401,
	s3dat_landscape_big2 = 0x501,
} s3dat_landscape_type;

typedef enum {
	s3dat_alpha1,
	s3dat_rgb565,
	s3dat_rgb555,
	s3dat_gray5,
	s3dat_unknown_color, // s3dat_new_bitmap in s3dat_ext.h
} s3dat_color_type;


//internal types
typedef struct s3dat_animation_t s3dat_animation_t;
typedef struct s3dat_exception_t s3dat_exception_t;
typedef struct s3dat_string_t s3dat_string_t;
typedef struct s3dat_bitmap_t s3dat_bitmap_t;
typedef struct s3dat_sound_t s3dat_sound_t;
typedef struct s3dat_t s3dat_t;


//ext types
typedef struct s3dat_extracthandler_t s3dat_extracthandler_t;
typedef struct s3dat_restype_t s3dat_restype_t;
typedef struct s3dat_packed_t s3dat_packed_t;
typedef struct s3dat_ioset_t s3dat_ioset_t;
typedef struct s3dat_ref_t s3dat_ref_t;
typedef struct s3dat_res_t s3dat_res_t;

typedef struct s3dat_frame_t s3dat_frame_t;
typedef struct s3dat_color_t s3dat_color_t;


struct s3dat_frame_t {
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
};

struct s3dat_color_t {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
};

void s3dat_writefile_name(s3dat_t* handle, uint8_t* name, s3dat_exception_t** throws);
void s3dat_writefile_fd(s3dat_t* handle, uint32_t* file, s3dat_exception_t** throws);
void s3dat_writefile_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func, s3dat_exception_t** throws);


void s3dat_readfile_name(s3dat_t* handle, uint8_t* name, s3dat_exception_t** throws);
void s3dat_readfile_fd(s3dat_t* handle, uint32_t* file, s3dat_exception_t** throws);
void s3dat_readfile_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func, s3dat_exception_t** throws);


void s3dat_init_name(s3dat_t* handle, uint8_t* name); //name must be vaild until s3dat_readfile/s3dat_writefile has ended
void s3dat_init_fd(s3dat_t* handle, uint32_t* file); //file must be vaild until s3dat_readfile/s3dat_writefile has ended
bool s3dat_init_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func); // io_arg mst be valid until s3dat_readfile/s3dat_writefile has ended

void s3dat_writefile(s3dat_t* handle, s3dat_exception_t** throws);
void s3dat_readfile(s3dat_t* handle, s3dat_exception_t** throws);

bool s3dat_remove_extracthandler(s3dat_t* handle, uint32_t steps_back);
bool s3dat_remove_last_extracthandler(s3dat_t* handle);

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

void s3dat_add_cache(s3dat_t* parent, s3dat_exception_t** throws);
void s3dat_add_utf8_encoding(s3dat_t* handle, s3dat_exception_t** throws);
void s3dat_add_landscape_blending(s3dat_t* handle, s3dat_exception_t** throws);

s3dat_t* s3dat_fork(s3dat_t* handle, s3dat_exception_t** throws);
s3dat_t* s3dat_writeable_fork(s3dat_t* handle, void* io_arg, s3dat_exception_t** throws);

void s3dat_delete_fork(s3dat_t* handle);

s3dat_ioset_t* s3dat_get_default_ioset(uint32_t type);

s3dat_t* s3dat_new_malloc();
s3dat_t* s3dat_new_malloc_monitor(void* arg, s3dat_ioset_t* ioset, bool open);
s3dat_t* s3dat_new_func(void* arg, void* (*alloc_func) (void*, size_t), void (*free_func) (void*, void*));

s3dat_extracthandler_t* s3dat_new_exhandler(s3dat_t* parent, s3dat_exception_t** throws);
s3dat_extracthandler_t* s3dat_new_exhandlers(s3dat_t* parent, uint32_t count, s3dat_exception_t** throws);

void s3dat_delete(s3dat_t* handle);

void s3dat_ref(s3dat_ref_t* ref);
void s3dat_unref(s3dat_ref_t* ref);

void s3dat_ref_array(s3dat_ref_t** refs, uint32_t count);
void s3dat_unref_array(s3dat_ref_t** refs, uint32_t count);

bool s3dat_catch_exception(s3dat_exception_t** throws);

//bitmap
bool s3dat_is_bitmap(s3dat_ref_t* bmp);

uint16_t s3dat_width(s3dat_ref_t* bmp);
uint16_t s3dat_height(s3dat_ref_t* bmp);
int16_t* s3dat_xoff(s3dat_ref_t* bmp);
int16_t* s3dat_yoff(s3dat_ref_t* bmp);
uint16_t* s3dat_landscape_meta(s3dat_ref_t* bmp);
uint32_t* s3dat_gui_meta(s3dat_ref_t* bmp);
s3dat_color_t* s3dat_bmpdata(s3dat_ref_t* bmp);

//animation
bool s3dat_is_animation(s3dat_ref_t* ani);

uint32_t s3dat_anilen(s3dat_ref_t* ani);
s3dat_frame_t* s3dat_frame(s3dat_ref_t* ani, uint32_t frame);

//string
bool s3dat_is_string(s3dat_ref_t* str);

bool s3dat_utf8(s3dat_ref_t* str);
uint8_t* s3dat_strdata(s3dat_ref_t* str);

//sound
bool s3dat_is_sound(s3dat_ref_t* snd);

uint32_t s3dat_freq(s3dat_ref_t* snd);
uint16_t s3dat_samples(s3dat_ref_t* snd);
uint16_t* s3dat_snddata(s3dat_ref_t* snd);


//s3dat_t
uint16_t s3dat_indexlen(s3dat_t* handle, s3dat_content_type type);
uint32_t s3dat_seqlen(s3dat_t* handle, uint16_t seq, s3dat_content_type type); //for GFX .dat files: return_value <= UINT16_MAX

uint32_t s3dat_indexaddr(s3dat_t* handle, uint16_t index, s3dat_content_type type);
uint32_t s3dat_seqaddr(s3dat_t* handle, uint16_t seq, uint32_t index, s3dat_content_type type);

// not writeable, because this would break palette extracting
uint32_t s3dat_palette_width(s3dat_t* handle);


#endif /*S3DAT_H*/
