#include "s3dat_internal.h"

#include <stdio.h>
#include <string.h>
#include <endian.h>

uint8_t s3dat_image_header[4] = {12, 0, 0, 0};

void s3dat_internal_read_bitmap_header(s3dat_t* mem, s3dat_content_type type, int from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws) {
	if(type == s3dat_animation || type == s3dat_palette) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(mem, from, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	bool offset = (type == s3dat_settler || type == s3dat_torso || type == s3dat_shadow);

	if(offset) {

		uint8_t header[4];
		if(!mem->read_func(mem->io_arg, header, 4)) {
			s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __LINE__);
			return;
		}

		if(memcmp(header, s3dat_image_header, 4) != 0) {
			s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __LINE__);
			return;
		}
	}

	if(width != NULL) {
		*width = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	} else {
		s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	}

	if(height != NULL) {
		*height = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	} else {
		s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	}

	if(offset) {

		if(xoff != NULL) {
			*xoff = s3dat_internal_read16LE(mem, throws);
			S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
		} else {
			s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
			S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
		}
		if(yoff != NULL) {
			*yoff = s3dat_internal_read16LE(mem, throws);
			S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
		} else {
			s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
			S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
		}
	} else if(type == s3dat_gui) {
		s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	} else {
		s3dat_internal_seek_func(mem, 1, S3DAT_SEEK_CUR, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	}

	if(mem->pos_func(mem->io_arg) % 2 == 1) {
		s3dat_internal_seek_func(mem, 1, S3DAT_SEEK_CUR, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
	}
}

void s3dat_internal_read_bitmap_data(s3dat_t* mem, s3dat_color_type type, uint16_t width, uint16_t height, s3dat_color_t** re_pixdata, s3dat_exception_t** throws) {
	if(width == 0 || height == 0) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
		return;
	}

	s3dat_color_t trans_color = {0, 0, 0, type == s3dat_alpha1 ? 0xFF : 0};

	s3dat_color_t* pixdata = NULL;
	if(re_pixdata) {
		pixdata = mem->alloc_func(mem->mem_arg, width*height*sizeof(s3dat_color_t));
		memset(pixdata, 0, width*height*4);
		*re_pixdata = pixdata;
	}

	uint16_t meta = 0;
	uint16_t x = 0;
	uint16_t y = 0;

	do {
		meta = s3dat_internal_read16LE(mem, throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(mem, throws, __FILE__, __LINE__);
			mem->free_func(mem->mem_arg, pixdata);
			return;
		}

		uint8_t skip = (meta >> 8) & 0x7F;
		while(skip > 0) {
			if(pixdata) pixdata[y*width+x] = trans_color;
			skip--;
			if(x == width) {
				mem->free_func(mem->mem_arg, pixdata);
				s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
				return;
			}
			x++;
		}

		uint8_t datalen = meta & 0xFF;
		while(datalen > 0) {
			if(pixdata) {
				pixdata[y*width+x] = s3dat_internal_ex(mem, type,throws);
			} else {
				s3dat_internal_ex(mem, type, throws);
			}
			S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

			datalen--;
			if(x == width) {
				mem->free_func(mem->mem_arg, pixdata);
				s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
				return;
			}
			x++;
		}
		if(meta >> 15 & 1) {
			y++;
			x = 0;
		}
	} while(y < height);
}

s3dat_color_t s3dat_internal_ex(s3dat_t* mem, s3dat_color_type type, s3dat_exception_t** throws) {
	s3dat_color_t color = {0, 0, 0, 0};
	if(type == s3dat_alpha1) return color;
	color.alpha = 0xFF;

	uint16_t raw;
	
	double d58 = 255.0/31.0;
	double d68 = 255.0/63.0;

	if(type == s3dat_gray5) {
		raw = s3dat_internal_read8(mem, throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(mem, throws, __FILE__, __LINE__);
			return color;
		}

		color.red = color.green = color.blue = (int)((raw & 0x1F)*d58);
		return color;
	}

	raw = s3dat_internal_read16LE(mem, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(mem, throws, __FILE__, __LINE__);
		return color;
	}

	if(type == s3dat_rgb555) {
		color.red = (uint8_t)(((raw >> 10) & 0x1F)*d58);
		color.green = (uint8_t)(((raw >> 5) & 0x1F)*d58);
	} else {
		color.red = (uint8_t)(((raw >> 11)& 0x1F)*d58);
		color.green = (uint8_t)(((raw >> 5) & 0x3F)*d68);
	}
	color.blue = (uint8_t)((raw & 0x1F)*d58);

	return color;
}

void s3dat_extract_settler(s3dat_t* mem, uint16_t settler, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws) {
	if(settler > mem->settler_index.len || frame > mem->settler_index.sequences[settler].len) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
		return;
	}

	uint16_t  w, h;
	s3dat_internal_read_bitmap_header(mem, s3dat_settler, mem->settler_index.sequences[settler].pointers[frame], &w, &h, xoff, yoff, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	to->type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;
	to->width = w;
	to->height = h;

	s3dat_internal_read_bitmap_data(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, w, h, &to->data, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
}

void s3dat_extract_torso(s3dat_t* mem, uint16_t torso, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws) {
	if(torso > mem->torso_index.len || frame > mem->torso_index.sequences[torso].len) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
		return;
	}

	uint16_t w, h;
	s3dat_internal_read_bitmap_header(mem, s3dat_torso, mem->torso_index.sequences[torso].pointers[frame], &w, &h, xoff, yoff, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	to->type = s3dat_gray5;
	to->width = w;
	to->height = h;

	s3dat_internal_read_bitmap_data(mem, s3dat_gray5, w, h, &to->data, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
}

void s3dat_extract_shadow(s3dat_t* mem, uint16_t shadow, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws) {
	if(shadow > mem->shadow_index.len || frame > mem->shadow_index.sequences[shadow].len) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
		return;
	}

	uint16_t w, h;
	s3dat_internal_read_bitmap_header(mem, s3dat_shadow, mem->shadow_index.sequences[shadow].pointers[frame], &w, &h, xoff, yoff, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	to->type = s3dat_alpha1;
	to->width = w;
	to->height = h;

	s3dat_internal_read_bitmap_data(mem, s3dat_alpha1, w, h, &to->data, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
}

void s3dat_extract_landscape2(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to, bool blend, s3dat_exception_t** throws) {
	s3dat_extract_landscape(mem, landscape, to, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	if(blend) {

		uint32_t pixel_count = to->width*to->height;

		for(uint32_t i = 0;i != pixel_count;i++) {
			if(to->data[i].red == 0
				&& to->data[i].green == 0xce
				&& (to->data[i].blue == 0xff || to->data[i].blue == 0xee)
				&& to->data[i].alpha == 0xff) to->data[i].alpha = 0;
		}
	}
}


void s3dat_extract_landscape(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to, s3dat_exception_t** throws) {
	if(landscape > mem->landscape_index.len) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
		return;
	}

	uint16_t xoff, yoff, w, h;
	s3dat_internal_read_bitmap_header(mem, s3dat_landscape, mem->landscape_index.pointers[landscape], &w, &h, &xoff, &yoff, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	to->type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;
	to->width = w;
	to->height = h;

	s3dat_internal_read_bitmap_data(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, w, h, &to->data, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
}

void s3dat_extract_gui(s3dat_t* mem, uint16_t gui, s3dat_bitmap_t* to, s3dat_exception_t** throws) {
	if(gui > mem->gui_index.len) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __LINE__);
		return;
	}

	uint16_t xoff, yoff, w, h;
	s3dat_internal_read_bitmap_header(mem, s3dat_gui, mem->gui_index.pointers[gui], &w, &h, &xoff, &yoff, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);

	to->type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;
	to->width = w;
	to->height = h;

	s3dat_internal_read_bitmap_data(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, w, h, &to->data, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __LINE__);
}

