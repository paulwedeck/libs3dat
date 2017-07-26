#include "s3dat.h"

#include <stdio.h>
#include <string.h>
#include <endian.h>

uint8_t s3dat_image_header[4] = {12, 0, 0, 0};

uint32_t s3dat_internal_read_bitmap_data(s3dat_t* mem, s3dat_color_type type, uint16_t width, uint16_t height, s3dat_color_t** re_pixdata);

uint32_t s3dat_internal_read_bitmap_header(s3dat_t* mem, s3dat_content_type type, int from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff);

s3dat_color_t s3dat_internal_ex(s3dat_t* mem, s3dat_color_type type);

uint32_t s3dat_internal_read_bitmap_header(s3dat_t* mem, s3dat_content_type type, int from, uint16_t* width, uint16_t* height, uint16_t* xoff, uint16_t* yoff) {
	if(type == s3dat_animation) return S3DAT_ERROR_INVALID_INPUT;
	mem->seek_func(mem->io_arg, from, SEEK_SET);
	uint32_t p = from;
	uint32_t size = mem->size_func(mem->io_arg);

	bool offset = (type == s3dat_settler || type == s3dat_torso || type == s3dat_shadow);

	if(offset) {
		if(p+4 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

		uint8_t header[4];
		mem->read_func(mem->io_arg, header, 4);

		if(memcmp(header, s3dat_image_header, 4) != 0) return S3DAT_ERROR_CORRUPT_IMAGE;
		p += 4;
	}

	if(p+5 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

	if(width != NULL) {
		*width = s3dat_internal_read16LE(mem);
	} else {
		mem->seek_func(mem->io_arg, 2, SEEK_CUR);
	}

	if(height != NULL) {
		*height = s3dat_internal_read16LE(mem);
	} else {
		mem->seek_func(mem->io_arg, 2, SEEK_CUR);
	}
	p += 4;

	if(offset) {
		if(p+4 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

		if(xoff != NULL) {
			*xoff = s3dat_internal_read16LE(mem);
		} else {
			mem->seek_func(mem->io_arg, 2, SEEK_CUR);
		}
		if(yoff != NULL) {
			*yoff = s3dat_internal_read16LE(mem);
		} else {
			mem->seek_func(mem->io_arg, 2, SEEK_CUR);
		}
		p += 4;
	} else if(type == s3dat_gui) {
		if(p+2 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

		mem->seek_func(mem->io_arg, 2, SEEK_CUR);
		p += 2;
	} else {
		if(p+1 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

		mem->seek_func(mem->io_arg, 1, SEEK_CUR);
		p++;
	}

	if(p % 2 == 1) {
		if(p+1 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

		mem->seek_func(mem->io_arg, 1, SEEK_CUR);
		p++;
	}

	return S3DAT_READ_SUCCESSFUL;
}

uint32_t s3dat_internal_read_bitmap_data(s3dat_t* mem, s3dat_color_type type, uint16_t width, uint16_t height, s3dat_color_t** re_pixdata) {
	if(width == 0 && height == 0) return S3DAT_ERROR_NULL_IMAGES_ARE_NULL;

	uint32_t pos = mem->pos_func(mem->io_arg);
	uint32_t size = mem->size_func(mem->io_arg);

	s3dat_color_t trans_color = {0, 0, 0, type == s3dat_alpha1 ? 0xFF : 0};

	s3dat_color_t* pixdata = NULL;
	if(re_pixdata) pixdata = mem->alloc_func(mem->mem_arg, width*height*sizeof(s3dat_color_t));
	memset(pixdata, 0, width*height*4);
	*re_pixdata = pixdata;

	uint16_t meta = 0;
	uint16_t x = 0;
	uint16_t y = 0;

	do {
		if(pos+2 > size) return S3DAT_ERROR_FILE_TOO_SHORT;
			else pos += 2;
		meta = s3dat_internal_read16LE(mem);

		uint8_t skip = (meta >> 8) & 0x7F;
		while(skip > 0) {
			if(pixdata) pixdata[y*width+x] = trans_color;
			skip--;
			if(x == width) {
				mem->free_func(mem->mem_arg, pixdata);
				return S3DAT_ERROR_CORRUPT_IMAGEDATA;
			}
			x++;
		}

		uint8_t datalen = meta & 0xFF;
		while(datalen > 0) {
			if((type == s3dat_rgb555 || type == s3dat_rgb565) && pos+2 > size) return S3DAT_ERROR_FILE_TOO_SHORT;
				else pos += 2;
			if(type == s3dat_gray5 && pos+1 > size) return S3DAT_ERROR_FILE_TOO_SHORT;
				else pos++;

			if(pixdata) {
				pixdata[y*width+x] = s3dat_internal_ex(mem, type);
			} else {
				s3dat_internal_ex(mem, type);
			}
			datalen--;
			if(x == width) {
				mem->free_func(mem->mem_arg, pixdata);
				return S3DAT_ERROR_CORRUPT_IMAGEDATA;
			}
			x++;
		}
		if(meta >> 15 & 1) {
			y++;
			x = 0;
		}
	} while(y < height);

	return S3DAT_READ_SUCCESSFUL;
}

s3dat_color_t s3dat_internal_ex(s3dat_t* mem, s3dat_color_type type) {
	s3dat_color_t color = {0, 0, 0, 0};
	if(type == s3dat_alpha1) return color;
	color.alpha = 0xFF;

	uint16_t raw;
	
	double d58 = 255.0/31.0;
	double d68 = 255.0/63.0;

	if(type == s3dat_gray5) {
		raw = s3dat_internal_read8(mem);
		color.red = color.green = color.blue = (int)((raw & 0x1F)*d58);
		return color;
	}
	raw = s3dat_internal_read16LE(mem);

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

uint32_t s3dat_extract_settler(s3dat_t* mem, uint16_t settler, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff) {
	if(settler > mem->settler_index.len || frame > mem->settler_index.sequences[settler].len) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;

	uint16_t  w, h;
	uint32_t re = s3dat_internal_read_bitmap_header(mem, s3dat_settler, mem->settler_index.sequences[settler].pointers[frame], &w, &h, xoff, yoff);

	if(re != S3DAT_READ_SUCCESSFUL) return re;

	to->type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;
	to->width = w;
	to->height = h;

	return s3dat_internal_read_bitmap_data(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, w, h, &to->data);
}

uint32_t s3dat_extract_torso(s3dat_t* mem, uint16_t torso, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff) {
	if(torso > mem->torso_index.len || frame > mem->torso_index.sequences[torso].len) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;
	uint16_t w, h;
	uint32_t re = s3dat_internal_read_bitmap_header(mem, s3dat_torso, mem->torso_index.sequences[torso].pointers[frame], &w, &h, xoff, yoff);

	if(re != S3DAT_READ_SUCCESSFUL) return re;

	to->type = s3dat_gray5;
	to->width = w;
	to->height = h;

	return s3dat_internal_read_bitmap_data(mem, s3dat_gray5, w, h, &to->data);
}

uint32_t s3dat_extract_shadow(s3dat_t* mem, uint16_t shadow, uint8_t frame, s3dat_bitmap_t* to, uint16_t* xoff, uint16_t* yoff) {
	if(shadow > mem->shadow_index.len || frame > mem->shadow_index.sequences[shadow].len) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;
	uint16_t w, h;
	uint32_t re = s3dat_internal_read_bitmap_header(mem, s3dat_shadow, mem->shadow_index.sequences[shadow].pointers[frame], &w, &h, xoff, yoff);

	if(re != S3DAT_READ_SUCCESSFUL) return re;

	to->type = s3dat_alpha1;
	to->width = w;
	to->height = h;

	return s3dat_internal_read_bitmap_data(mem, s3dat_alpha1, w, h, &to->data);
}

uint32_t s3dat_extract_landscape2(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to, bool blend) {
	uint32_t error_code = s3dat_extract_landscape(mem, landscape, to);

	if(error_code != S3DAT_READ_SUCCESSFUL) return error_code;

	if(blend) {

		uint32_t pixel_count = to->width*to->height;

		for(uint32_t i = 0;i != pixel_count;i++) {
			if(to->data[i].red == 0
				&& to->data[i].green == 0xce
				&& (to->data[i].blue == 0xff || to->data[i].blue == 0xee)
				&& to->data[i].alpha == 0xff) to->data[i].alpha = 0;
		}
	}

	return S3DAT_READ_SUCCESSFUL;
}


uint32_t s3dat_extract_landscape(s3dat_t* mem, uint16_t landscape, s3dat_bitmap_t* to) {
	if(landscape > mem->landscape_index.len) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;
	uint16_t xoff, yoff, w, h;
	uint32_t re = s3dat_internal_read_bitmap_header(mem, s3dat_landscape, mem->landscape_index.pointers[landscape], &w, &h, &xoff, &yoff);

	if(re != S3DAT_READ_SUCCESSFUL) return re;

	to->type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;
	to->width = w;
	to->height = h;

	return s3dat_internal_read_bitmap_data(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, w, h, &to->data);
	
}

uint32_t s3dat_extract_gui(s3dat_t* mem, uint16_t gui, s3dat_bitmap_t* to) {
	if(gui > mem->gui_index.len) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;
	uint16_t xoff, yoff, w, h;
	uint32_t re = s3dat_internal_read_bitmap_header(mem, s3dat_gui, mem->gui_index.pointers[gui], &w, &h, &xoff, &yoff);

	if(re != S3DAT_READ_SUCCESSFUL) return re;

	to->type = mem->green_6b ? s3dat_rgb565 : s3dat_rgb555;
	to->width = w;
	to->height = h;

	return s3dat_internal_read_bitmap_data(mem, mem->green_6b ? s3dat_rgb565 : s3dat_rgb555, w, h, &to->data);
}

