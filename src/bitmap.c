#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "bitmap.c"
#endif

uint8_t s3dat_image_header[4] = {12, 0, 0, 0};

void s3dat_internal_extract_bitmap(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;

	uint32_t from = s3dat_internal_seek_to(handle, res, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	void* header;
	int header_size = 12;
	if(res->type == s3dat_gui) header_size = 6;
	if(res->type == s3dat_landscape) header_size = 5;

	if(((from+header_size) % 2) == 1) header_size++;

	header = s3dat_internal_alloc_func(handle, header_size, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(!handle->read_func(handle->io_arg, header, header_size)) {
		handle->free_func(handle->mem_arg, header);
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	uint16_t* img_meta;

	if(header_size == 12 || header_size == 12+1) {
		if(memcmp(header, s3dat_image_header, 4) != 0) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
			return;
		}

		img_meta = header+4;
	} else {
		img_meta = header;
	}

	if(img_meta[0] == 0 || img_meta[1] == 0) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	int pixel_size = 2;
	if(res->type == s3dat_shadow) pixel_size = 0;
	if(res->type == s3dat_torso) pixel_size = 1;

	uint16_t width = le16(img_meta[0]);
	uint16_t height = le16(img_meta[1]);

	uint32_t bfr_size = pixel_size*width*height+header_size;
	void* bfr = s3dat_internal_alloc_func(handle, bfr_size, throws);

	if(*throws != NULL) {
		handle->free_func(handle->mem_arg, header);
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	memcpy(bfr, header, header_size);
	handle->free_func(handle->mem_arg, header);

	uint32_t read_size = header_size;

	uint16_t x = 0;
	uint16_t y = 0;

	bool end = false;

	while(!end) {
		uint16_t meta = s3dat_internal_read16LE(handle, throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			end = true;
		} else {
			x += (meta & 0xFF);
			x += ((meta >> 8) & 0x7F);
			uint16_t data_len = ((meta & 0xFF)*pixel_size)+2;

			uint8_t data_bfr[data_len];
			*((uint16_t*)data_bfr) = le16(meta);
			if(handle->read_func(handle->io_arg, data_bfr+2, data_len-2)) {
				if(read_size+data_len > bfr_size) {
					void* bfr2 = s3dat_internal_alloc_func(handle, bfr_size*2, throws);
					if(*throws != NULL) {
						end = true;
						s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
					} else {
						memcpy(bfr2, bfr, read_size);
						handle->free_func(handle->mem_arg, bfr);
						bfr = bfr2;
						bfr_size *= 2;
					}
				}

				if(*throws == NULL) {
					memcpy(bfr+read_size, data_bfr, data_len);
					read_size += data_len;
				}
			} else {
				s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
				end = true;
			}
		}

		if((meta >> 15) & 1) {
			x = 0;
			y++;
		}

		if(y == height) {
			end = true;
		}

		if((x > width || y > height) && *throws == NULL) {
			end = true;
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		}
	}

	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		handle->free_func(handle->mem_arg, bfr);
		return;
	}

	if(bfr_size != read_size) {
		void* bfr2 = handle->alloc_func(handle->mem_arg, read_size);
		if(bfr2) {
			memcpy(bfr2, bfr, read_size);
			handle->free_func(handle->mem_arg, bfr);
			bfr = bfr2;
		}
	}

	s3dat_packed_t* pack = handle->alloc_func(handle->mem_arg, sizeof(s3dat_packed_t));
	if(pack) {
		pack->data = bfr;
		pack->len = read_size;
		res->resdata = pack;
	} else {
		handle->free_func(handle->mem_arg, bfr);
		S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws);
	}
}

s3dat_color_t s3dat_internal_ex(void* addr, s3dat_color_type type) {
	s3dat_color_t color = {0, 0, 0, 0};
	if(type == s3dat_alpha1) return color;
	color.alpha = 0xFF;

	uint16_t raw = le16p(addr);
	
	double d58 = 255.0/31.0;
	double d68 = 255.0/63.0;

	if(type == s3dat_gray5) {
		color.red = color.green = color.blue = (int)((raw & 0x1F)*d58);
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

s3dat_color_t s3dat_internal_error_color = {0, 0, 0, 0};

s3dat_color_t s3dat_extract_palette_color(s3dat_t* handle, uint16_t palette, uint8_t brightness, uint32_t x, s3dat_exception_t** throws) {
	if(palette > handle->palette_index->len) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return s3dat_internal_error_color;
	}

	s3dat_internal_seek_func(handle, handle->palette_index->pointers[palette]+brightness*handle->palette_line_length+x, S3DAT_SEEK_SET, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return s3dat_internal_error_color;
	}

	uint16_t color = s3dat_internal_read16LE(handle, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return s3dat_internal_error_color;
	}

	return s3dat_internal_ex(&color, handle->green_6b ? s3dat_rgb565 : s3dat_rgb555);
}

