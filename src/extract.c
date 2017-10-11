#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "bitmap.c"
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

void s3dat_default_extract(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;

	if(res->type == s3dat_snd) {
		s3dat_sound_t* sound = s3dat_new_sound(handle);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
		s3dat_internal_extract_sound(handle, res->first_index, res->second_index, sound, throws);
		if(*throws != NULL) {
			s3dat_delete_sound(sound);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		} else {
			res->resdata = sound;
		}
	} else if(res->type == s3dat_animation) {
		s3dat_animation_t* animation = s3dat_new_animation(handle);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		s3dat_internal_extract_animation(handle, res->first_index, animation, throws);
		if(*throws != NULL) {
			s3dat_delete_animation(animation);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		} else {
			res->resdata = animation;
		}
	} else if(res->type == s3dat_palette) {
		s3dat_bitmap_t* palette = s3dat_new_bitmap(handle);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		s3dat_internal_extract_palette(handle, res->first_index, palette, throws);
		if(*throws != NULL) {
			s3dat_delete_bitmap(palette);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		} else {
			res->resdata = palette;
		}
	} else if(res->type == s3dat_string) {
		s3dat_string_t* string = s3dat_new_string(handle);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		s3dat_internal_extract_string(handle, res->first_index, (uint16_t)(res->second_index & 0xFFFFF), string, throws);
		if(*throws != NULL) {
			s3dat_delete_string(string);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		} else {
			res->resdata = string;
		}
	} else if(res->type == s3dat_gui || res->type == s3dat_torso || res->type == s3dat_shadow ||
		res->type == s3dat_settler || res->type == s3dat_landscape) {
		s3dat_index_t* index = NULL;
		s3dat_seq_index_t* seq_index = NULL;
		if(res->type == s3dat_landscape) index = handle->landscape_index;
		else if(res->type == s3dat_settler) seq_index = handle->settler_index;
		else if(res->type == s3dat_shadow) seq_index = handle->shadow_index;
		else if(res->type == s3dat_torso) seq_index = handle->torso_index;
		else if(res->type == s3dat_gui) index = handle->gui_index;

		uint16_t index_ptr = res->first_index;

		if(seq_index != NULL) {
			if(res->first_index < seq_index->len) {
				index = seq_index->sequences+res->first_index;
				index_ptr = res->second_index;
			} else {
				s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
				return;
			}
		}

		if(index_ptr >= index->len) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
			return;
		}

		uint16_t width;
		uint16_t height;
		uint16_t xoff;
		uint16_t yoff;

		s3dat_internal_read_bitmap_header(handle, res->type, index->pointers[index_ptr], &width, &height, &xoff, &yoff, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		s3dat_color_type color_type = handle->green_6b ? s3dat_rgb565 : s3dat_rgb555;

		if(res->type == s3dat_torso) color_type = s3dat_gray5;
			else if(res->type == s3dat_alpha1) color_type = s3dat_alpha1;

		s3dat_color_t* color_data;

		s3dat_internal_read_bitmap_data(handle, color_type, width, height, &color_data, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		s3dat_bitmap_t* image = s3dat_new_bitmap(handle);

		image->width = width;
		image->height = height;
		image->xoff = xoff;
		image->yoff = yoff;
		image->type = color_type;
		image->data = color_data;

		res->resdata = image;
	} else {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
	}
}

void* s3dat_extract_arg(s3dat_t* handle, uint16_t first_index, uint32_t second_index, s3dat_content_type type, s3dat_exception_t** throws) {
	s3dat_res_t res = {first_index, second_index, type, NULL};

	s3dat_extract(handle, &res, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
	}

	return res.resdata;
}

s3dat_bitmap_t* s3dat_extract_settler(s3dat_t* handle, uint16_t settler, uint8_t frame, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, settler, frame, s3dat_settler, throws);
}

s3dat_bitmap_t* s3dat_extract_shadow(s3dat_t* handle, uint16_t shadow, uint8_t frame, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, shadow, frame, s3dat_shadow, throws);
}

s3dat_bitmap_t* s3dat_extract_torso(s3dat_t* handle, uint16_t torso, uint8_t frame, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, torso, frame, s3dat_torso, throws);
}


s3dat_bitmap_t* s3dat_extract_landscape(s3dat_t* handle, uint16_t landscape, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, landscape, 0, s3dat_landscape, throws);
}

s3dat_bitmap_t* s3dat_extract_gui(s3dat_t* handle, uint16_t gui, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, gui, 0, s3dat_gui, throws);
}


s3dat_animation_t* s3dat_extract_animation(s3dat_t* handle, uint16_t animation, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, animation, 0, s3dat_animation, throws);
}

s3dat_string_t* s3dat_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, text, language, s3dat_string, throws);
}

s3dat_bitmap_t* s3dat_extract_palette(s3dat_t* handle, uint16_t palette, s3dat_exception_t** throws) {
	return s3dat_extract_arg(handle, palette, 0, s3dat_palette, throws);
}

s3dat_string_t* s3dat_extract_sound(s3dat_t* handle, uint16_t soundtype, uint32_t altindex, s3dat_exception_t** throws) {
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

	if(res->type != s3dat_landscape) return;

	s3dat_bitmap_t* bmp = res->resdata;

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

