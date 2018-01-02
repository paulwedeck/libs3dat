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
	uint32_t header_size = 12;
	if(res->type == s3dat_gui) header_size = 8;
	if(res->type == s3dat_landscape) header_size = 6;


	header = s3dat_alloc_func(handle, header_size, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(!handle->read_func(handle->io_arg, header, header_size)) {
		s3dat_free_func(handle, header);
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	if(from % 2 == 1) {
		s3dat_internal_read8(handle, throws); // this fill byte can be dropped
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}

	uint16_t* img_meta;

	if(res->type != s3dat_gui && res->type != s3dat_landscape) {
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

	uint32_t pixel_size = 2;
	if(res->type == s3dat_shadow) pixel_size = 0;
	if(res->type == s3dat_torso) pixel_size = 1;

	uint16_t width = le16(img_meta[0]);
	uint16_t height = le16(img_meta[1]);

	uint32_t bfr_size = pixel_size*width*height+header_size;
	void* bfr = s3dat_alloc_func(handle, bfr_size, throws);

	if(*throws != NULL) {
		s3dat_free_func(handle, header);
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	memcpy(bfr, header, header_size);
	s3dat_free_func(handle, header);

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
					void* bfr2 = s3dat_alloc_func(handle, bfr_size*2, throws);
					if(*throws != NULL) {
						end = true;
						s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
					} else {
						memcpy(bfr2, bfr, read_size);
						s3dat_free_func(handle, bfr);
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
		s3dat_free_func(handle, bfr);
		return;
	}

	if(bfr_size != read_size) {
		void* bfr2 = s3dat_alloc_func(handle, read_size, NULL);
		if(bfr2) {
			memcpy(bfr2, bfr, read_size);
			s3dat_free_func(handle, bfr);
			bfr = bfr2;
		}
	}

	s3dat_ref_t* pack = s3dat_new_packed(handle, throws);
	if(*throws != NULL) {
		s3dat_free_func(handle, bfr);
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
	} else {
		pack->data.pkd->parent = handle;
		pack->data.pkd->data = bfr;
		pack->data.pkd->len = read_size;
		res->res = pack;
	}
}

void s3dat_pack_bitmap(s3dat_t* handle, s3dat_bitmap_t* bitmap, s3dat_content_type type, s3dat_packed_t* packed, s3dat_exception_t** throws) {
	uint32_t pixel_size = 2;
	if(type == s3dat_shadow) pixel_size = 0;
	if(type == s3dat_torso) pixel_size = 1;

	uint32_t header_size = 12;
	if(type == s3dat_gui) header_size = 8;
	if(type == s3dat_landscape) header_size = 6;

	uint32_t metas = 0;
	uint32_t datas = 0;

	uint8_t stat = 0;
	for(uint32_t y = 0;y != bitmap->height;y++) {
		s3dat_color_t* color;

		for(uint32_t x = 0;x != bitmap->width;x++) {
			color = bitmap->data+(y*bitmap->width+x);

			if(color->alpha > 127) {
				stat = 2;
				datas++;
			} else {
				if(stat == 2) {
					stat = 3;
				} else if(stat == 0) {
					stat = 1;
				}
			}

			if(stat == 3 || (x+1) == bitmap->width) {
				metas++;
				stat = 0;
			}
		}
	}

	packed->len = header_size+(metas*2)+(datas*pixel_size);
	packed->data = s3dat_alloc_func(handle, header_size+(metas*2)+(datas*pixel_size), throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint16_t* size_meta;

	if(type == s3dat_gui) {
		size_meta = packed->data;
		*((uint32_t*)packed->data+4) = bitmap->gui_type;
	} else if(type == s3dat_landscape) {
		size_meta = packed->data;
		*((uint16_t*)packed->data+4) = bitmap->landscape_type;
	} else {
		memcpy(packed->data, s3dat_image_header, 4);
		size_meta = packed->data+4;

		uint16_t* off_ptr = packed->data+8;
		off_ptr[0] = le16(bitmap->xoff);
		off_ptr[1] = le16(bitmap->yoff);
	}

	size_meta[0] = le16(bitmap->width);
	size_meta[1] = le16(bitmap->height);

	uint16_t* meta = packed->data+header_size;
	uint8_t* data = packed->data+header_size+2;

	uint8_t current_data = 0;
	uint8_t current_clear = 0;

	stat = 0;
	for(uint32_t y = 0;y != bitmap->height;y++) {
		for(uint32_t x = 0;x != bitmap->width;x++) {
			if(bitmap->data[y*bitmap->width+x].alpha > 127) {
				stat = 2;
				s3dat_internal_8b_to_native(bitmap->data+(y*bitmap->width+x), data, bitmap->type);
				data += pixel_size;
				current_data++;
			} else {
				if(stat == 1) {
					current_clear++;
				}else if(stat == 2) {
					stat = 3;
				} else if(stat == 0) {
					stat = 1;
				}
			}

			if(stat == 3 || (x+1) == bitmap->width) {
				uint16_t tmp_meta = current_data + ((current_clear & 0x7F)<<8);

				current_data = 0;
				current_clear = 0;

				if((x+1) == bitmap->width) tmp_meta |= (1<<15);
				*meta = le16(tmp_meta);

				meta = (uint16_t*)data;
				data += 2;

				stat = 0;
			}
		}
	}
}

s3dat_color_t s3dat_internal_ex(void* addr, s3dat_color_type type) {
	s3dat_color_t color = {0, 0, 0, 0xFF};
	if(type == s3dat_alpha1) return color;

	
	double d58 = 255.0/31.0;
	double d68 = 255.0/63.0;

	if(type == s3dat_gray5) {
		color.red = color.green = color.blue = (int)((*((uint8_t*)addr) & 0x1F)*d58);
		return color;
	}

	uint16_t raw = le16p(addr);

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

void s3dat_internal_8b_to_native(s3dat_color_t* color, void* to, s3dat_color_type type) {
	if(type == s3dat_alpha1) return;

	uint8_t* ptr8 = to;
	uint16_t* ptr16 = to;

	double d85 = 31.0/255.0;
	double d86 = 63.0/255.0;

	if(type == s3dat_gray5) {
		uint8_t gray5 = ((color->red+color->green+color->blue)/3)*d85;
		*ptr8 = (gray5 & 0x1F);
		return;
	}

	uint16_t red, green;

	uint16_t blue = (uint16_t)(color->blue*d85);

	if(type == s3dat_rgb555) {
		red = color->red*d85;
		green = color->green*d85;

		red = (red) << 10;
		green = (green) << 5;
	} else {
		red = color->red*d85;
		green = color->green*d86;

		red = (red) << 11;
		green = (green) << 5;
	}

	*ptr16 = le16(red+green+blue);
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

uint16_t s3dat_width(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return 0;
	return bmp->data.bmp->width;
}

uint16_t s3dat_height(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return 0;
	return bmp->data.bmp->height;
}


int16_t* s3dat_xoff(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return NULL;
	return &bmp->data.bmp->xoff;
}

int16_t* s3dat_yoff(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return NULL;
	return &bmp->data.bmp->yoff;
}

uint16_t* s3dat_landscape_meta(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return NULL;
	return &bmp->data.bmp->landscape_type;
}

uint32_t* s3dat_gui_meta(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return NULL;
	return &bmp->data.bmp->gui_type;
}

s3dat_color_t* s3dat_bmpdata(s3dat_ref_t* bmp) {
	if(!s3dat_is_bitmap(bmp)) return NULL;
	return bmp->data.bmp->data;
}

