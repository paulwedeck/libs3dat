#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "read.c"
#endif

uint8_t s3dat_internal_snd_header_read_c[16] = { 68, 21, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 0 };

uint8_t s3dat_header_start_part1[33] = {
	4, 19, 4, 0, 12, 0, 0, 0, 0, 0, 0, 0, 84, 0, 0, 0,
	32, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0
};

uint8_t s3dat_header_start_part2[10] = {0, 0, 31, 0, 0, 0, 0, 0, 0, 0};

uint8_t s3dat_header_rgb5[5] = {124, 0, 0, 224, 3};
uint8_t s3dat_header_rgb565[5] = {248, 0, 0, 224, 7};

uint8_t s3dat_seq_start[7] = {2, 20, 0, 0, 8, 0, 0};

void s3dat_readfile_fd(s3dat_t* handle, uint32_t* file, s3util_exception_t** throws) {
	s3dat_init_fd(handle, file);
	s3dat_readfile(handle, throws);
}

void s3dat_init_fd(s3dat_t* handle, uint32_t* file) {
	s3dat_init_ioset(handle, file, s3util_get_default_ioset(S3UTIL_IOSET_DEFAULT), false);
}


void s3dat_readfile_name(s3dat_t* handle, char* name, s3util_exception_t** throws) {
	s3dat_init_name(handle, name);
	s3dat_readfile(handle, throws);
}

void s3dat_init_name(s3dat_t* handle, char* name) {
	s3dat_init_ioset(handle, name, s3util_get_default_ioset(S3UTIL_IOSET_DEFAULT), true);
}


void s3dat_readfile_ioset(s3dat_t* handle, void* io_arg, s3util_ioset_t* ioset, bool use_openclose_func,  s3util_exception_t** throws) {
	if(!s3dat_init_ioset(handle, io_arg, ioset, use_openclose_func)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOSET, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_readfile(handle, throws);
}

bool s3dat_init_ioset(s3dat_t* handle, void* io_arg, s3util_ioset_t* ioset, bool use_openclose_func) {
	if(ioset == NULL || ioset->available == false) return false;

	handle->ioset = *ioset;
	s3dat_ioset(handle)->arg = io_arg;
	if(!use_openclose_func) {
		s3dat_ioset(handle)->open_func = NULL;
		s3dat_ioset(handle)->close_func = NULL;
	}
	return true;
}


void s3dat_init_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*)) {
	s3dat_ioset(handle)->arg = arg;
	s3dat_ioset(handle)->read_func = read_func;
	s3dat_ioset(handle)->write_func = write_func;
	s3dat_ioset(handle)->size_func = size_func;
	s3dat_ioset(handle)->pos_func = pos_func;
	s3dat_ioset(handle)->seek_func = seek_func;
	s3dat_ioset(handle)->open_func = open_func;
	s3dat_ioset(handle)->close_func = close_func;
	s3dat_ioset(handle)->fork_func = fork_func;
}

void s3dat_readfile_func(s3dat_t* handle, void* arg,
	bool (*read_func) (void*, void*, size_t),
	bool (*write_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*, bool),
	void (*close_func) (void*),
	void* (*fork_func) (void*),
	s3util_exception_t** throws) {
	s3dat_init_func(handle, arg, read_func, write_func, size_func, pos_func, seek_func, open_func, close_func, fork_func);
	s3dat_readfile(handle, throws);
}

void s3dat_readfile(s3dat_t* handle, s3util_exception_t** throws) {
	handle->last_handler = s3dat_new_exhandler(handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	handle->last_handler->call = s3dat_unpack_handler;
	handle->last_handler->before = s3dat_new_exhandler(handle, throws);
	if(*throws != NULL) {
		s3dat_delete_exhandler(handle->last_handler);
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return;
	}

	handle->last_handler->before->call = s3dat_read_packed_handler;

	if(s3dat_ioset(handle)->open_func != NULL) s3dat_ioset(handle)->arg = s3dat_ioset(handle)->open_func(s3dat_ioset(handle)->arg, false);

	if(s3dat_ioset(handle)->arg == NULL) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_OPEN, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part1[33];
	if(!s3dat_ioset(handle)->read_func(s3dat_ioset(handle)->arg, header_part1, 16)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part1, s3dat_internal_snd_header_read_c, 16) == 0) {
		s3dat_internal_readsnd(handle, throws);
		if(*throws != NULL) s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return;
	}

	if(!s3dat_ioset(handle)->read_func(s3dat_ioset(handle)->arg, header_part1+16, 17)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}
	if(memcmp(header_part1, s3dat_header_start_part1, 33) != 0) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_filetype[5];
	if(!s3dat_ioset(handle)->read_func(s3dat_ioset(handle)->arg, header_filetype, 5)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_filetype, s3dat_header_rgb5, 5) == 0) {
		handle->green_6b = false;
	} else if(memcmp(header_filetype, s3dat_header_rgb565, 5) == 0) {
		handle->green_6b = true;
	} else {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part2[10];
	if(!s3dat_ioset(handle)->read_func(s3dat_ioset(handle)->arg, header_part2, 10)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part2, s3dat_header_start_part2, 10) != 0) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t file_size = S3DAT_INTERNAL_READ(32LE, handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	if(file_size != s3dat_ioset(handle)->size_func(s3dat_ioset(handle)->arg)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t sequence_pointers[8];
	for(uint32_t i = 0;i != 8;i++) {
		sequence_pointers[i] = S3DAT_INTERNAL_READ(32LE, handle, throws);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
	}

	for(uint32_t i = 0;i != 8;i++) {
		s3dat_internal_read_index(handle, sequence_pointers[i], throws);
		s3util_add_attr(s3dat_memset(handle), throws, S3UTIL_ATTRIBUTE_INDEX, i);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_read_index(s3dat_t* handle, uint32_t index, s3util_exception_t** throws) {
	s3dat_internal_seek_func(handle, index, S3UTIL_SEEK_SET, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint32_t index_type = S3DAT_INTERNAL_READ(32LE, handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	if(index_type == s3dat_string) {
		uint32_t index_size = S3DAT_INTERNAL_READ(32LE, handle, throws);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

		uint16_t texts = S3DAT_INTERNAL_READ(16LE, handle, throws);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

		uint16_t languages = S3DAT_INTERNAL_READ(16LE, handle, throws);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

		if(languages*texts*4+12 != index_size) {
			s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
		}

		if(texts == 0 || languages == 0) return;

		handle->string_index->len = texts;
		handle->string_index->sequences = s3util_alloc_func(s3dat_memset(handle), texts*sizeof(s3dat_index_t), throws);
		if(*throws != NULL) {
			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
			return;
		}

		for(uint16_t t = 0;t != texts;t++) {
			handle->string_index->sequences[t].len = languages;
			handle->string_index->sequences[t].type = s3dat_string;
			handle->string_index->sequences[t].pointers = s3util_alloc_func(s3dat_memset(handle), languages*4, throws);
			if(*throws != NULL) {
				s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

				s3util_free_func(s3dat_memset(handle), handle->string_index->sequences);
				for(uint16_t i = 0;i != (texts-1);i++) {
					s3util_free_func(s3dat_memset(handle), handle->string_index->sequences[t].pointers);
				}

				return;
			}
		}

		for(uint16_t l = 0;l != languages;l++) {
			for(uint16_t t = 0;t != texts;t++) {
				handle->string_index->sequences[t].pointers[l] = S3DAT_INTERNAL_READ(32LE, handle, throws);

				if(*throws != NULL) {
					s3dat_internal_delete_seq(handle, handle->string_index);
					return;
				}
			}
		}
		handle->string_index->type = s3dat_string;
	} else {

		uint16_t index_size = S3DAT_INTERNAL_READ(16LE, handle, throws);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

		uint16_t index_len = S3DAT_INTERNAL_READ(16LE, handle, throws);
		S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

		if((index_type != s3dat_palette && index_len*4+8 != index_size) ||
			(index_type == s3dat_palette && index_len*4+12 != index_size)) {
			s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
			return;
		}

		if(index_type != s3dat_settler && index_type != s3dat_torso &&
			index_type != s3dat_shadow && index_type != s3dat_landscape &&
			index_type != s3dat_gui && index_type != s3dat_animation &&
			index_type != s3dat_palette) {
			s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
			return;
		}

		if(index_type == s3dat_palette) {
			handle->palette_line_length = S3DAT_INTERNAL_READ(32LE, handle, throws);
			S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		}

		if(index_type == s3dat_settler || index_type == s3dat_torso || index_type == s3dat_shadow) {
			s3dat_seq_index_t* index;
			switch(index_type) {
				case s3dat_settler:
					index = handle->settler_index;
				break;
				case s3dat_torso:
					index = handle->torso_index;
				break;
				case s3dat_shadow:
					index = handle->shadow_index;
				break;
			}
			if(index->type == index_type) {
				s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				return;
			}

			uint32_t* pointers = s3util_alloc_func(s3dat_memset(handle), 4*index_len, throws);
			S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

			for(uint16_t i = 0;i != index_len;i++) {
				pointers[i] = S3DAT_INTERNAL_READ(32LE, handle, throws);

				if(*throws != NULL) {
					s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
					s3util_free_func(s3dat_memset(handle), pointers);
				}
			}

			s3dat_index_t* indices = s3util_alloc_func(s3dat_memset(handle), index_len*sizeof(s3dat_index_t), throws);
			if(*throws != NULL) {
				s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
				s3util_free_func(s3dat_memset(handle), pointers);
				return;
			}

			for(uint16_t i = 0;i != index_len;i++) {
				s3dat_internal_read_seq(handle, pointers[i], &indices[i], throws);
				s3util_add_attr(s3dat_memset(handle), throws, S3UTIL_ATTRIBUTE_SEQ, i);

				if(*throws != NULL) {
					s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
					s3dat_internal_delete_indices(handle, indices, index_len);
					s3util_free_func(s3dat_memset(handle), indices);
					s3util_free_func(s3dat_memset(handle), pointers);
					return;
				}
				indices[i].type = index_type;
			}
			s3util_free_func(s3dat_memset(handle), pointers);

			index->type = index_type;
			index->len = index_len;
			index->sequences = indices;
		} else {
			s3dat_index_t* index;
			switch(index_type) {
				case s3dat_gui:
					index = handle->gui_index;
				break;
				case s3dat_landscape:
					index = handle->landscape_index;
				break;
				case s3dat_animation:
					index = handle->animation_index;
				break;
				case s3dat_palette:
					index = handle->palette_index;
				break;
				default:
					s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
					return;
				break;
			}

			if(index->type == index_type) {
				s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				return;
			}

			index->pointers = s3util_alloc_func(s3dat_memset(handle), 4*index_len, throws);
			S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

			for(uint16_t i = 0;i != index_len;i++) {
				index->pointers[i] = S3DAT_INTERNAL_READ(32LE, handle, throws);

				if(*throws != NULL) {
					s3util_free_func(s3dat_memset(handle), index->pointers);
					s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__ , __func__, __LINE__);
					return;
				}
			}

			index->type = index_type;
			index->len = index_len;
		}
	}
}

void s3dat_internal_read_seq(s3dat_t* handle, uint32_t from, s3dat_index_t* to, s3util_exception_t** throws) {
	s3dat_internal_seek_func(handle, from, S3UTIL_SEEK_SET, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint8_t seq_start[7];

	if(!s3dat_ioset(handle)->read_func(s3dat_ioset(handle)->arg, seq_start, 7)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(seq_start, s3dat_seq_start, 7) != 0) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t frames = S3DAT_INTERNAL_READ(8, handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	to->len = frames;
	to->pointers = s3util_alloc_func(s3dat_memset(handle), 4*frames, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	for(uint8_t i = 0;i != frames;i++) {
		to->pointers[i] = S3DAT_INTERNAL_READ(32LE, handle, throws);
		to->pointers[i] += from;

		if(*throws != NULL) {
			s3util_free_func(s3dat_memset(handle), to->pointers);

			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
			return;
		}
	}
}


void s3dat_internal_seek_func(s3dat_t* handle, uint32_t pos, int whence, s3util_exception_t** throws) {
	if(!s3dat_ioset(handle)->seek_func(s3dat_ioset(handle)->arg, pos, whence)) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	}
}

uint32_t s3dat_internal_seek_to(s3dat_t* handle, s3dat_res_t* res, s3util_exception_t** throws) {
	uint32_t from;

	if(res->type == s3dat_snd) {
		if(handle->sound_index->len <= res->first_index || handle->sound_index->sequences[res->first_index].len <= res->second_index) {
			s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
			return 0;
		}

		from = handle->sound_index->sequences[res->first_index].pointers[res->second_index];
	} else {
		s3dat_seq_index_t* seq_index = NULL;
		s3dat_index_t* index = NULL;

		uint16_t index_ptr = (res->type == s3dat_settler || res->type == s3dat_shadow || res->type == s3dat_torso) ? res->second_index : res->first_index;

		if(res->type == s3dat_settler) seq_index = handle->settler_index;
		if(res->type == s3dat_shadow) seq_index = handle->shadow_index;
		if(res->type == s3dat_string) seq_index = handle->string_index;
		if(res->type == s3dat_torso) seq_index = handle->torso_index;

		if(res->type == s3dat_animation) index = handle->animation_index;
		if(res->type == s3dat_landscape) index = handle->landscape_index;
		if(res->type == s3dat_palette) index = handle->palette_index;
		if(res->type == s3dat_gui) index = handle->gui_index;

		if(seq_index) {
			if(seq_index->len <= res->first_index) {
				s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
				return 0;
			}

			index = seq_index->sequences+res->first_index;
		}

		if(index->len <= index_ptr) {
			s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
			return 0;
		}

		from = index->pointers[index_ptr];
	}


	s3dat_internal_seek_func(handle, from, S3UTIL_SEEK_SET, throws);
	if(*throws != NULL) s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
	return from;
}

s3dat_restype_t s3dat_internal_animation_type = {"s3dat_animation_t", (void (*) (void*)) s3dat_delete_animation, (void* (*) (void*,s3util_exception_t**)) s3dat_new_raw_animation};
s3dat_restype_t s3dat_internal_bitmap_type = {"s3dat_bitmap_t", (void (*) (void*)) s3dat_delete_bitmap, (void* (*) (void*,s3util_exception_t**)) s3dat_new_raw_bitmap};
s3dat_restype_t s3dat_internal_packed_type = {"s3dat_packed_t", (void (*) (void*)) s3dat_delete_packed, (void* (*) (void*,s3util_exception_t**)) s3dat_new_raw_packed};
s3dat_restype_t s3dat_internal_string_type = {"s3dat_string_t", (void (*) (void*)) s3dat_delete_string, (void* (*) (void*,s3util_exception_t**)) s3dat_new_raw_string};
s3dat_restype_t s3dat_internal_sound_type = {"s3dat_sound_t", (void (*) (void*)) s3dat_delete_sound, (void* (*) (void*,s3util_exception_t**)) s3dat_new_raw_sound};

s3dat_restype_t* s3dat_internal_get_restype(s3dat_ref_type type) {
	switch(type) {
		case s3dat_pkd_ref:
			return &s3dat_internal_packed_type;

		case s3dat_bmp_ref:
			return &s3dat_internal_bitmap_type;

		case s3dat_ani_ref:
			return &s3dat_internal_animation_type;

		case s3dat_str_ref:
			return &s3dat_internal_string_type;

		case s3dat_snd_ref:
			return &s3dat_internal_sound_type;
	}
	return NULL;
}

bool s3dat_is_bitmap(s3dat_ref_t* bmp) {
	return bmp->type == &s3dat_internal_bitmap_type;
}

bool s3dat_is_animation(s3dat_ref_t* ani) {
	return ani->type == &s3dat_internal_animation_type;
}

bool s3dat_is_string(s3dat_ref_t* str) {
	return str->type == &s3dat_internal_string_type;
}

bool s3dat_is_sound(s3dat_ref_t* snd) {
	return snd->type == &s3dat_internal_sound_type;
}


