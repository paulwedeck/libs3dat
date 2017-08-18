#ifndef S3DAT_H
#define S3DAT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define S3DAT_SEEK_CUR 0x20
#define S3DAT_SEEK_SET 0x21

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
} s3dat_language;

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

struct s3dat_t {
	void* mem_arg;
	void* io_arg;
	void* (*alloc_func) (void*, size_t);
	void (*free_func) (void*, void*);
	bool (*read_func) (void*, void*, size_t);
	size_t (*size_func) (void*);
	size_t (*pos_func) (void*);
	bool (*seek_func) (void*, uint32_t, int);
	void* (*open_func) (s3dat_t*);
	void (*close_func) (s3dat_t*);

	bool green_6b;
	uint32_t palette_line_length;

	s3dat_seq_index_t settler_index;
	s3dat_seq_index_t shadow_index;
	s3dat_seq_index_t torso_index;
	s3dat_seq_index_t string_index;
	s3dat_seq_index32_t sound_index; // SND .dat files only
	s3dat_index_t landscape_index;
	s3dat_index_t gui_index;
	s3dat_index_t animation_index;
	s3dat_index_t palette_index;

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
	char* string_data;
} s3dat_string_t;

typedef struct {
	s3dat_t* src;

	uint32_t len;
	s3dat_frame_t* frames;
} s3dat_animation_t;

void s3dat_readfile_name(s3dat_t* mem, char* name, s3dat_exception_t** throws);

void s3dat_readfile_fd(s3dat_t* mem, uint32_t* file, s3dat_exception_t** throws);

void s3dat_readfile_func(s3dat_t* mem, void* arg,
	bool (*read_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (s3dat_t*),
	void (*close_func) (s3dat_t*),
	s3dat_exception_t** throws_out);

void s3dat_extract_settler(s3dat_t* mem, uint16_t settler, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws);
void s3dat_extract_shadow(s3dat_t* mem, uint16_t shadow, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws);
void s3dat_extract_torso(s3dat_t* mem, uint16_t torso, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws);
void s3dat_extract_sound(s3dat_t* mem, uint16_t soundtype, uint32_t altindex, s3dat_sound_t* to, s3dat_exception_t** throws);
void s3dat_extract_landscape2(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to, bool blend, s3dat_exception_t** throws);
void s3dat_extract_landscape(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to, s3dat_exception_t** throws);
void s3dat_extract_gui(s3dat_t* mem, uint16_t gui, s3dat_bitmap_t* to, s3dat_exception_t** throws);
void s3dat_extract_animation(s3dat_t* mem, uint16_t animation, s3dat_animation_t* to, s3dat_exception_t** throws);
void s3dat_extract_string(s3dat_t* mem, uint16_t text, s3dat_language language, s3dat_string_t* to, bool utf8, s3dat_exception_t** throws);
void s3dat_extract_palette(s3dat_t* mem, uint16_t palette, s3dat_bitmap_t* to, s3dat_exception_t** throws);
s3dat_color_t s3dat_extract_palette_color(s3dat_t* mem, uint16_t palette, uint8_t brightness, uint32_t x, s3dat_exception_t** throws);

//linux
void* s3dat_linux_open_func(s3dat_t* from);
void s3dat_linux_close_func(s3dat_t* from);
bool s3dat_linux_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_linux_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_linux_pos_func(void* arg);
size_t s3dat_linux_size_func(void* arg);

void* s3dat_libc_open_func(s3dat_t* from);
void s3dat_libc_close_func(s3dat_t* from);
bool s3dat_libc_read_func(void* arg, void* bfr, size_t len); // system endianness
bool s3dat_libc_seek_func(void* arg, uint32_t pos, int whence);
size_t s3dat_libc_pos_func(void* arg);
size_t s3dat_libc_size_func(void* arg);


void* s3dat_default_alloc_func(void* arg, size_t size);
void s3dat_default_free_func(void* arg, void* mem);

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

void s3dat_delete(s3dat_t* mem);
void s3dat_delete_animation(s3dat_animation_t* mem);
void s3dat_delete_animations(s3dat_animation_t* mem, uint32_t count);
void s3dat_delete_frame(s3dat_animation_t* mem);
void s3dat_delete_frames(s3dat_animation_t* mem, uint32_t count);

void s3dat_delete_bitmap(s3dat_bitmap_t* mem);
void s3dat_delete_bitmaps(s3dat_bitmap_t* mem, uint32_t count);
void s3dat_delete_pixdata(s3dat_bitmap_t* mem);
void s3dat_delete_pixdatas(s3dat_bitmap_t* mem, uint32_t count);

void s3dat_delete_sound(s3dat_sound_t* mem);
void s3dat_delete_sounds(s3dat_sound_t* mem, uint32_t count);
void s3dat_delete_snddata(s3dat_sound_t* mem);
void s3dat_delete_snddatas(s3dat_sound_t* mem, uint32_t count);

void s3dat_delete_string(s3dat_string_t* string);
void s3dat_delete_strings(s3dat_string_t* strings, uint32_t count);
void s3dat_delete_stringdata(s3dat_string_t* string);
void s3dat_delete_stringdatas(s3dat_string_t* strings, uint32_t count);

void s3dat_print_exception(s3dat_exception_t* ex); // debuging only
void s3dat_delete_exception(s3dat_t* mem, s3dat_exception_t* ex);

#endif /*S3DAT_H*/

