#ifndef S3DAT_H
#define S3DAT_H

//#define S3DAT_COMPATIBILITY_MODE 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define S3DAT_READ_SUCCESSFUL 0
#define S3DAT_ERROR_CORRUPT_HEADER 0x100
#define S3DAT_ERROR_FILE_TOO_SHORT 0x101
#define S3DAT_UNKNOWN_FILETYPE 0x102
#define S3DAT_ERROR_CORRUPT_INDEX 0x103
#define S3DAT_ERROR_INDEX_TYPE_COLLISION 0x104
#define S3DAT_ERROR_CORRUPT_SEQUENCE 0x105
#define S3DAT_ERROR_CORRUPT_IMAGE 0x106
#define S3DAT_ERROR_NYI 0x107
#define S3DAT_ERROR_CORRUPT_IMAGEDATA 0x108
#define S3DAT_ERROR_NULL_IMAGES_ARE_NULL 0x109
#define S3DAT_ERROR_VALUE_HIGHER_THAN_MAX 0x110

#define S3DAT_WRITE_SUCCESSFUL 0
#define S3DAT_DATA_VALID 0
#define S3DAT_ERROR_INVALID_INPUT 0x200
#define S3DAT_ERROR_WRONG_COLORTYPE 0x201
#define S3DAT_ERROR_CORRUPT_WIDTH_HEIGHT 0x202
#define S3DAT_ERROR_CORRUPT_PIXEL 0x203

typedef enum {
	s3dat_snd = 0xFFFF, // SND .dat files only
	s3dat_settler = 0x106,
	s3dat_torso = 0x3112,
	s3dat_shadow = 0x5982,
	s3dat_landscape = 0x2412,
	s3dat_gui = 0x11306,
	s3dat_animation = 0x21702,
	s3dat_palette = 0x2607,
} s3dat_content_type;

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

typedef struct {
	uint32_t mem_arg;
	uint32_t io_arg;
	void* (*alloc_func) (uint32_t, size_t);
	void (*free_func) (uint32_t, void*);
	void (*read_func) (uint32_t, void*, size_t);
	size_t (*size_func) (uint32_t);
	size_t (*pos_func) (uint32_t);
	void (*seek_func) (uint32_t, uint32_t, int);

	bool green_6b;

	s3dat_seq_index_t settler_index;
	s3dat_seq_index_t shadow_index;
	s3dat_seq_index_t torso_index;
	s3dat_seq_index32_t sound_index; // SND .dat files only
	s3dat_index_t landscape_index;
	s3dat_index_t gui_index;
	s3dat_index_t animation_index;
	s3dat_index_t palette_index;

} s3dat_t;

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

	uint32_t len;
	s3dat_frame_t* frames;
} s3dat_animation_t;

uint32_t s3dat_readfile_fd(s3dat_t* mem, uint32_t file);

uint32_t s3dat_readfile_func(s3dat_t* mem, uint32_t arg,
	void (*read_func) (uint32_t, void*, size_t),
	size_t (*size_func) (uint32_t),
	size_t (*pos_func) (uint32_t),
	void (*seek_func) (uint32_t, uint32_t, int));

uint32_t s3dat_internal_readsnd(s3dat_t* mem); // io functions must be defined

uint32_t s3dat_extract_settler(s3dat_t* mem, uint16_t settler, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff);
uint32_t s3dat_extract_shadow(s3dat_t* mem, uint16_t shadow, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff);
uint32_t s3dat_extract_torso(s3dat_t* mem, uint16_t torso, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff);
uint32_t s3dat_extract_sound(s3dat_t* mem, uint16_t soundtype, uint32_t altindex, s3dat_sound_t* to);
uint32_t s3dat_extract_landscape2(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to, bool blend);
uint32_t s3dat_extract_landscape(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to);
uint32_t s3dat_extract_gui(s3dat_t* mem, uint16_t gui, s3dat_bitmap_t* to);
uint32_t s3dat_extract_animation(s3dat_t* mem, uint16_t animation, s3dat_animation_t* to);

void s3dat_default_read_func(uint32_t arg, void* bfr, size_t len); // system endianness
void s3dat_default_write_func(uint32_t arg, void* bfr, size_t len); // system endianness
void s3dat_default_seek_func(uint32_t arg, uint32_t pos, int whence);
size_t s3dat_default_pos_func(uint32_t arg);
size_t s3dat_default_size_func(uint32_t arg);
void* s3dat_default_alloc_func(uint32_t arg, size_t size);
void s3dat_default_free_func(uint32_t arg, void* mem);

s3dat_t* s3dat_new_malloc();
s3dat_t* s3dat_new_func(uint32_t arg, void* (*alloc_func) (uint32_t, size_t), void (*free_func) (uint32_t, void*));

s3dat_animation_t* s3dat_new_animation(s3dat_t* parent);
s3dat_animation_t* s3dat_new_animations(s3dat_t* parent, uint32_t count);

s3dat_bitmap_t* s3dat_new_bitmap(s3dat_t* parent);
s3dat_bitmap_t* s3dat_new_bitmaps(s3dat_t* parent, uint32_t count);

s3dat_sound_t* s3dat_new_sound(s3dat_t* parent);
s3dat_sound_t* s3dat_new_sounds(s3dat_t* parent, uint32_t count);

uint32_t s3dat_internal_read32LE(s3dat_t* mem);
uint16_t s3dat_internal_read16LE(s3dat_t* mem);
uint16_t s3dat_internal_read8(s3dat_t* mem);

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

#endif /*S3DAT_H*/

