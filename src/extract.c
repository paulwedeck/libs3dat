#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "extract.c"
#endif

void s3dat_add_extracthandler(s3dat_t* handle, s3dat_extracthandler_t* exhandler) {
	exhandler->before = handle->last_handler;
	handle->last_handler = exhandler;
}

bool s3dat_remove_last_extracthandler(s3dat_t* handle) {
	return s3dat_remove_extracthandler(handle, 0);
}

bool s3dat_remove_extracthandler(s3dat_t* handle, uint32_t steps_back) {

	if(steps_back == 0) {
		s3dat_extracthandler_t* exhandler = handle->last_handler;
		if(exhandler == NULL) return false;

		handle->last_handler = exhandler->before;
		s3dat_delete_exhandler(exhandler);
		return true;
	} else {
		s3dat_extracthandler_t* lower = handle->last_handler;

		for(uint32_t i = 1;i < steps_back;i++) {
			if(lower->before != NULL) lower = lower->before;
				else return false;
		}

		handle->last_handler = lower;
		s3dat_extracthandler_t* re_handler = lower->before;

		if(re_handler != NULL) {
			lower->before = re_handler->before;
			s3dat_delete_exhandler(re_handler);
			return false;
		}
	}
	return true;
}

void s3dat_extract(s3dat_t* handle, s3dat_res_t* res, s3dat_exception_t** throws) {
	if(handle->last_handler) handle->last_handler->call(handle->last_handler, res, throws);
}

void s3dat_read_packed_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;

	if(res->type != s3dat_snd && res->type != s3dat_animation && res->type != s3dat_palette) {
		if(res->type == s3dat_string) {
			s3dat_internal_extract_string(handle, res->first_index, (uint16_t)(res->second_index & 0xFFFFF), &res->res, throws);
		} else {
			s3dat_internal_extract_bitmap(me, res, throws);
		}
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t raw_len;
	uint32_t read_len;
	void* data;

	uint32_t offset = res->type == s3dat_palette ? 0 : 4;


	s3dat_internal_seek_to(handle, res, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(res->type != s3dat_palette) {
		raw_len = s3dat_internal_read32LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}

	if(res->type == s3dat_snd) read_len = raw_len-4;
	else if(res->type == s3dat_animation) read_len = raw_len*24;
	else read_len = handle->palette_line_length*8;

	data = s3dat_alloc_func(handle, read_len+offset, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	if(res->type != s3dat_palette) *(uint32_t*)data = le32(raw_len);

	if(!handle->read_func(handle->io_arg, data+offset, read_len)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		handle->free_func(handle->mem_arg, data);
		return;
	}

	res->res = s3dat_new_packed(handle);
	res->res->data.pkd->len = read_len+offset;
	res->res->data.pkd->data = data;
}


void s3dat_unpack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	S3DAT_CHECK_TYPE(handle, res, "s3dat_packed_t", throws, __FILE__, __func__, __LINE__);

	s3dat_packed_t* package = res->res->data.pkd;
	s3dat_ref_t* new_ref = NULL;

	if(res->type == s3dat_snd) {
		s3dat_ref_t* sound = s3dat_new_sound(handle, le32p(package->data+12), (package->len-16)/2);
		uint16_t* ptr16 = package->data+16;
		for(uint16_t i = 0;i != (package->len-16)/2;i++) {
			sound->data.snd->data[i] = le16(ptr16[i]);
		}
		new_ref = sound;
	} else if(res->type == s3dat_animation) {
		s3dat_ref_t* animation_ref = s3dat_new_animation(handle, le32p(package->data));
		s3dat_animation_t* animation = animation_ref->data.ani;

		uint16_t* ptr16 = package->data+4;
		for(uint32_t i = 0;i != animation->len;i++) {
			animation->frames[i].posx = le16(*(ptr16++));
			animation->frames[i].posy = le16(*(ptr16++));
			animation->frames[i].settler_id = le16(*(ptr16++));
			animation->frames[i].settler_file = le16(*(ptr16++));
			animation->frames[i].torso_id = le16(*(ptr16++));
			animation->frames[i].torso_file = le16(*(ptr16++));
			animation->frames[i].shadow_id = le16(*(ptr16++));
			animation->frames[i].shadow_file = le16(*(ptr16++));
			animation->frames[i].settler_frame = le16(*(ptr16++));
			animation->frames[i].torso_frame = le16(*(ptr16++));
			animation->frames[i].flag1 = le16(*(ptr16++));
			animation->frames[i].flag2 = le16(*(ptr16++));
		}
		new_ref = animation_ref;
	} else if(res->type == s3dat_palette) {
		s3dat_ref_t* palette = s3dat_new_bitmap(handle, handle->palette_line_length, 8);

		palette->data.bmp->type = handle->green_6b ? s3dat_rgb565 : s3dat_rgb555;

		uint32_t colors = handle->palette_line_length*8;
		uint16_t* color_addr = package->data;
		for(uint32_t i = 0;i != colors;i++) {
			palette->data.bmp->data[i] = s3dat_internal_ex(color_addr++, palette->data.bmp->type);
		}
		new_ref = palette;
	} else if(res->type == s3dat_string) {
		s3dat_ref_t* string = s3dat_new_string(handle, package->len);

		string->data.str->original_encoding = true;
		string->data.str->language = res->second_index & 0xFFFF;
		memcpy(string->data.str->string_data, package->data, package->len);
		new_ref = string;
	} else if(res->type == s3dat_gui || res->type == s3dat_torso || res->type == s3dat_shadow ||
		res->type == s3dat_settler || res->type == s3dat_landscape) {

		uint16_t width = 0;
		uint16_t height = 0;
		int16_t xoff = 0;
		int16_t yoff = 0;
		uint16_t landscape_type = 0;
		uint32_t gui_type = 0;

		s3dat_color_type color_type;
		uint32_t pixel_size;

		if(res->type == s3dat_torso) {
			color_type = s3dat_gray5;
			pixel_size = 1;
		} else if(res->type == s3dat_shadow) {
			color_type = s3dat_alpha1;
			pixel_size = 0;
		} else {
			color_type = handle->green_6b ? s3dat_rgb565 : s3dat_rgb555;
			pixel_size = 2;
		}

		void* data_ptr = package->data;

		if(res->type == s3dat_settler || res->type == s3dat_torso || res->type == s3dat_shadow) {
			data_ptr += 4;

			width = le16p(data_ptr);
			data_ptr += 2;

			height = le16p(data_ptr);
			data_ptr += 2;

			xoff = le16p(data_ptr);
			data_ptr += 2;

			yoff = le16p(data_ptr);
			data_ptr += 2;
		} else if(res->type == s3dat_gui) {
			width = le16p(data_ptr);
			data_ptr += 2;

			height = le16p(data_ptr);
			data_ptr += 2;

			gui_type = le32p(data_ptr);
			data_ptr += 4;
		} else {
			width = le16p(data_ptr);
			data_ptr += 2;

			height = le16p(data_ptr);
			data_ptr += 2;

			landscape_type = le16p(data_ptr);
			data_ptr += 2;
		}

		s3dat_ref_t* image = s3dat_new_bitmap(handle, width, height);
		image->data.bmp->landscape_type = landscape_type;
		image->data.bmp->gui_type = gui_type;
		image->data.bmp->xoff = xoff;
		image->data.bmp->yoff = yoff;
		image->data.bmp->type = color_type;

		uint16_t x = 0;
		uint16_t y = 0;

		s3dat_color_t trans_color = {0, 0, 0, 0};

		while(y < height) {
			uint16_t meta = le16p(data_ptr);
			data_ptr += 2;

			uint8_t skip = (meta >> 8) & 0x7F;
			while(skip > 0) {
				image->data.bmp->data[y*width+x] = trans_color;
				x++;
				skip--;
			}

			uint8_t datalen = meta & 0xFF;
			while(datalen > 0) {
				image->data.bmp->data[y*width+x] = s3dat_internal_ex(data_ptr, color_type);
				data_ptr += pixel_size;
				datalen--;
				x++;
			}

			if((meta >> 15) & 1) {
				y++;
				x = 0;
			}
		}
		new_ref = image;
	} else {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
	}

	s3dat_unref(res->res);
	res->res = new_ref;
}

s3dat_ref_t* s3dat_extract_arg(s3dat_t* handle, uint16_t first_index, uint32_t second_index, s3dat_content_type type, s3dat_exception_t** throws) {
	s3dat_res_t res = {first_index, second_index, type, NULL};

	s3dat_extract(handle, &res, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
	}

	return res.res;
}

s3dat_ref_t* s3dat_extract_settler(s3dat_t* handle, uint16_t settler, uint8_t frame, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, settler, frame, s3dat_settler, throws);
}

s3dat_ref_t* s3dat_extract_shadow(s3dat_t* handle, uint16_t shadow, uint8_t frame, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, shadow, frame, s3dat_shadow, throws);
}

s3dat_ref_t* s3dat_extract_torso(s3dat_t* handle, uint16_t torso, uint8_t frame, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, torso, frame, s3dat_torso, throws);
}


s3dat_ref_t* s3dat_extract_landscape(s3dat_t* handle, uint16_t landscape, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, landscape, 0, s3dat_landscape, throws);
}

s3dat_ref_t* s3dat_extract_gui(s3dat_t* handle, uint16_t gui, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, gui, 0, s3dat_gui, throws);
}


s3dat_ref_t* s3dat_extract_animation(s3dat_t* handle, uint16_t animation, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, animation, 0, s3dat_animation, throws);
}

s3dat_ref_t* s3dat_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, text, language, s3dat_string, throws);
}

s3dat_ref_t* s3dat_extract_palette(s3dat_t* handle, uint16_t palette, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, palette, 0, s3dat_palette, throws);
}

s3dat_ref_t* s3dat_extract_sound(s3dat_t* handle, uint16_t soundtype, uint32_t altindex, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, soundtype, altindex, s3dat_snd, throws);
}

void s3dat_add_utf8_encoding(s3dat_t* handle) {
	s3dat_extracthandler_t* exhandler = s3dat_new_exhandler(handle);

	exhandler->call = s3dat_utf8_encoding_handler;

	s3dat_add_extracthandler(handle, exhandler);
}

void s3dat_blend_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;
	
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	S3DAT_CHECK_TYPE(handle, res, "s3dat_bitmap_t", throws, __FILE__, __func__, __LINE__);

	if(res->type != s3dat_landscape) return;

	s3dat_bitmap_t* bmp = res->res->data.bmp;

	uint32_t pixel_count = bmp->width*bmp->height;
	for(uint32_t i = 0;i != pixel_count;i++) {
		if(bmp->data[i].red == 0
			&& bmp->data[i].green == 0xce
			&& (bmp->data[i].blue == 0xff || bmp->data[i].blue == 0xee)
			&& bmp->data[i].alpha == 0xff) bmp->data[i].alpha = 0;
	}
}

void s3dat_add_landscape_blending(s3dat_t* handle) {
	s3dat_extracthandler_t* exhandler = s3dat_new_exhandler(handle);

	exhandler->call = s3dat_blend_handler;

	s3dat_add_extracthandler(handle, exhandler);
}

