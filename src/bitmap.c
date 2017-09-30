#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "bitmap.c"
#endif

uint8_t s3dat_image_header[4] = {12, 0, 0, 0};

void s3dat_internal_read_bitmap_header(s3dat_t* mem, s3dat_content_type type, int from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, from, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	bool offset = (type == s3dat_settler || type == s3dat_torso || type == s3dat_shadow);

	if(offset) {

		uint8_t header[4];
		if(!mem->read_func(mem->io_arg, header, 4)) {
			s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
			return;
		}

		if(memcmp(header, s3dat_image_header, 4) != 0) {
			s3dat_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
			return;
		}
	}

	if(width != NULL) {
		*width = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	} else {
		s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}

	if(height != NULL) {
		*height = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	} else {
		s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}

	if(offset) {

		if(xoff != NULL) {
			*xoff = s3dat_internal_read16LE(mem, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		} else {
			s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		}
		if(yoff != NULL) {
			*yoff = s3dat_internal_read16LE(mem, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		} else {
			s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		}
	} else if(type == s3dat_gui) {
		s3dat_internal_seek_func(mem, 2, S3DAT_SEEK_CUR, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	} else {
		s3dat_internal_seek_func(mem, 1, S3DAT_SEEK_CUR, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}

	if(mem->pos_func(mem->io_arg) % 2 == 1) {
		s3dat_internal_seek_func(mem, 1, S3DAT_SEEK_CUR, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_read_bitmap_data(s3dat_t* mem, s3dat_color_type type, uint16_t width, uint16_t height, s3dat_color_t** re_pixdata, s3dat_exception_t** throws) {
	if(width == 0 || height == 0) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_color_t trans_color = {0, 0, 0, type == s3dat_alpha1 ? 0xFF : 0};

	s3dat_color_t* pixdata = NULL;
	if(re_pixdata) {
		pixdata = s3dat_internal_alloc_func(mem, width*height*sizeof(s3dat_color_t), throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		*re_pixdata = pixdata;
	}

	uint16_t x = 0;
	uint16_t y = 0;

	do {
		uint16_t meta = s3dat_internal_read16LE(mem, throws);
		if(*throws != NULL) {
			if(pixdata) mem->free_func(mem->mem_arg, pixdata);
				*re_pixdata = NULL;
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			return;
		}

		uint8_t skip = (meta >> 8) & 0x7F;
		while(skip > 0) {
			if(pixdata) pixdata[y*width+x] = trans_color;
			skip--;
			if(x == width) {
				if(pixdata) mem->free_func(mem->mem_arg, pixdata);
				*re_pixdata = NULL;
				s3dat_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
				return;
			}
			x++;
		}

		uint8_t datalen = meta & 0xFF;
		while(datalen > 0) {
			if(pixdata) {
				pixdata[y*width+x] = s3dat_internal_ex(mem, type, throws);
			} else {
				s3dat_internal_ex(mem, type, throws);
			}
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

			datalen--;
			if(x == width) {
				if(pixdata) mem->free_func(mem->mem_arg, pixdata);
				*re_pixdata = NULL;
				s3dat_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
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
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			return color;
		}

		color.red = color.green = color.blue = (int)((raw & 0x1F)*d58);
		return color;
	}

	raw = s3dat_internal_read16LE(mem, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
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

void s3dat_internal_extract_palette(s3dat_t* mem, uint16_t palette, s3dat_bitmap_t* to, s3dat_exception_t** throws) {
	if(palette > mem->palette_index->len) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(mem, mem->palette_index->pointers[palette], S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t colors = mem->palette_line_length*8;
	s3dat_color_type type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;

	s3dat_color_t* bmp_data = s3dat_internal_alloc_func(mem, sizeof(s3dat_color_t)*colors, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	for(uint32_t color = 0;color != colors;color++) {
		bmp_data[color] = s3dat_internal_ex(mem, type, throws);

		if(*throws != NULL) {
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			mem->free_func(mem->mem_arg, bmp_data);
			return;
		}
	}

	to->width = mem->palette_line_length;
	to->height = 8;

	to->src = mem;
	to->type = type;

	to->data = bmp_data;
}

s3dat_color_t s3dat_internal_error_color = {0, 0, 0, 0};

s3dat_color_t s3dat_extract_palette_color(s3dat_t* mem, uint16_t palette, uint8_t brightness, uint32_t x, s3dat_exception_t** throws) {
	if(palette > mem->palette_index->len) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return s3dat_internal_error_color;
	}

	s3dat_internal_seek_func(mem, mem->palette_index->pointers[palette]+brightness*mem->palette_line_length+x, S3DAT_SEEK_SET, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
		return s3dat_internal_error_color;
	}

	s3dat_color_t color = s3dat_internal_ex(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
		return s3dat_internal_error_color;
	}

	return color;
}

void s3dat_internal_extract_animation(s3dat_t* mem, uint16_t animation, s3dat_animation_t* to, s3dat_exception_t** throws) {
	if(mem->animation_index->len <= animation) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(mem, mem->animation_index->pointers[animation], S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t entries = s3dat_internal_read32LE(mem, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->src = mem;
	to->len = entries;
	to->frames = s3dat_internal_alloc_func(mem, entries*sizeof(s3dat_frame_t), throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	for(uint32_t i = 0;i != entries;i++) {
		to->frames[i].posx = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].posx = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].settler_id = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].settler_file = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].torso_id = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].torso_file = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].shadow_id = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].shadow_file = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].settler_frame = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].torso_frame = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].flag1 = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].flag2 = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}
}

