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

void s3dat_readfile_fd(s3dat_t* handle, uint32_t* file, s3dat_exception_t** throws) {
	s3dat_init_fd(handle, file);
	s3dat_readfile(handle, throws);
}

void s3dat_init_fd(s3dat_t* handle, uint32_t* file) {
	s3dat_init_ioset(handle, file, s3dat_get_default_ioset(S3DAT_IOSET_DEFAULT), false);
}


void s3dat_readfile_name(s3dat_t* handle, uint8_t* name, s3dat_exception_t** throws) {
	s3dat_init_name(handle, name);
	s3dat_readfile(handle, throws);
}

void s3dat_init_name(s3dat_t* handle, uint8_t* name) {
	s3dat_init_ioset(handle, name, s3dat_get_default_ioset(S3DAT_IOSET_DEFAULT), true);
}


void s3dat_readfile_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func,  s3dat_exception_t** throws) {
	if(!s3dat_init_ioset(handle, io_arg, ioset, use_openclose_func)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOSET, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_readfile(handle, throws);
}

bool s3dat_init_ioset(s3dat_t* handle, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func) {
	if(ioset == NULL || ioset->available == false) return false;

	s3dat_init_func(handle, io_arg, ioset->read_func, ioset->write_func, ioset->size_func, ioset->pos_func, ioset->seek_func, (use_openclose_func ? ioset->open_func : NULL), (use_openclose_func ? ioset->close_func : NULL), ioset->fork_func);

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
	handle->io_arg = arg;
	handle->read_func = read_func;
	handle->write_func = write_func;
	handle->size_func = size_func;
	handle->pos_func = pos_func;
	handle->seek_func = seek_func;
	handle->open_func = open_func;
	handle->close_func = close_func;
	handle->fork_func = fork_func;
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
	s3dat_exception_t** throws) {
	s3dat_init_func(handle, arg, read_func, write_func, size_func, pos_func, seek_func, open_func, close_func, fork_func);
	s3dat_readfile(handle, throws);
}

void s3dat_readfile(s3dat_t* handle, s3dat_exception_t** throws) {
	handle->last_handler = s3dat_new_exhandler(handle);
	handle->last_handler->call = s3dat_unpack_handler;
	handle->last_handler->before = s3dat_new_exhandler(handle);
	handle->last_handler->before->call = s3dat_read_packed_handler;

	if(handle->open_func != NULL) handle->io_arg = handle->open_func(handle->io_arg, false);

	if(handle->io_arg == NULL) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_OPEN, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part1[33];
	if(!handle->read_func(handle->io_arg, header_part1, 16)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part1, s3dat_internal_snd_header_read_c, 16) == 0) {
		s3dat_internal_readsnd(handle, throws);
		if(*throws != NULL) s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	if(!handle->read_func(handle->io_arg, header_part1+16, 17)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}
	if(memcmp(header_part1, s3dat_header_start_part1, 33) != 0) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_filetype[5];
	if(!handle->read_func(handle->io_arg, header_filetype, 5)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_filetype, s3dat_header_rgb5, 5) == 0) {
		handle->green_6b = false;
	} else if(memcmp(header_filetype, s3dat_header_rgb565, 5) == 0) {
		handle->green_6b = true;
	} else {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part2[10];
	if(!handle->read_func(handle->io_arg, header_part2, 10)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part2, s3dat_header_start_part2, 10) != 0) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t file_size = s3dat_internal_read32LE(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(file_size != handle->size_func(handle->io_arg)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t sequence_pointers[8];
	for(uint32_t i = 0;i != 8;i++) {
		sequence_pointers[i] = s3dat_internal_read32LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}

	for(uint32_t i = 0;i != 8;i++) {
		s3dat_internal_read_index(handle, sequence_pointers[i], throws);
		S3DAT_INTERNAL_ADD_ATTR(handle, throws, S3DAT_ATTRIBUTE_INDEX, i);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_read_index(s3dat_t* handle, uint32_t index, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(handle, index, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint32_t index_type = s3dat_internal_read32LE(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(index_type == s3dat_string) {
		uint32_t index_size = s3dat_internal_read32LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		uint16_t texts = s3dat_internal_read16LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		uint16_t languages = s3dat_internal_read16LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		if(languages*texts*4+12 != index_size) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
		}

		if(texts == 0 || languages == 0) return;

		handle->string_index->len = texts;
		handle->string_index->type = s3dat_string;
		handle->string_index->sequences = s3dat_internal_alloc_func(handle, texts*sizeof(s3dat_index_t), throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			handle->string_index->type = 0;
			return;
		}

		for(uint16_t t = 0;t != texts;t++) {
			handle->string_index->sequences[t].len = languages;
			handle->string_index->sequences[t].type = s3dat_string;
			handle->string_index->sequences[t].pointers = s3dat_internal_alloc_func(handle, languages*4, throws);
			if(*throws != NULL) {
				s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);

				handle->free_func(handle->mem_arg, handle->string_index->sequences);
				for(uint16_t i = 0;i != (texts-1);i++) {
					handle->free_func(handle->mem_arg, handle->string_index->sequences[t].pointers);
				}

				handle->string_index->type = 0;
				return;
			}
		}

		for(uint16_t l = 0;l != languages;l++) {
			for(uint16_t t = 0;t != texts;t++) {
				handle->string_index->sequences[t].pointers[l] = s3dat_internal_read32LE(handle, throws);

				if(*throws != NULL) {
					s3dat_internal_delete_seq(handle, handle->string_index);
					return;
				}
			}
		}
	} else {

		uint16_t index_size = s3dat_internal_read16LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		uint16_t index_len = s3dat_internal_read16LE(handle, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		if((index_type != s3dat_palette && index_len*4+8 != index_size) ||
			(index_type == s3dat_palette && index_len*4+12 != index_size)) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
			return;
		}

		if(index_type != s3dat_settler && index_type != s3dat_torso &&
			index_type != s3dat_shadow && index_type != s3dat_landscape &&
			index_type != s3dat_gui && index_type != s3dat_animation &&
			index_type != s3dat_palette) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
			return;
		}

		if(index_type == s3dat_palette) {
			handle->palette_line_length = s3dat_internal_read32LE(handle, throws);
			S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
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
				s3dat_throw(handle, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				return;
			}

			uint32_t* pointers = s3dat_internal_alloc_func(handle, 4*index_len, throws);
			S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

			for(uint16_t i = 0;i != index_len;i++) {
				pointers[i] = s3dat_internal_read32LE(handle, throws);

				if(*throws != NULL) {
					s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
					handle->free_func(handle->mem_arg, pointers);
				}
			}

			s3dat_index_t* dead_indices = s3dat_internal_alloc_func(handle, index_len*sizeof(s3dat_index_t), throws);
			if(*throws != NULL) {
				s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
				handle->free_func(handle->mem_arg, pointers);
				return;
			}
			uint16_t real_count = 0;

			for(uint16_t i = 0;i != index_len;i++) {
				s3dat_internal_read_seq(handle, pointers[i], dead_indices+real_count, throws);
				S3DAT_INTERNAL_ADD_ATTR(handle, throws, S3DAT_ATTRIBUTE_SEQ, i);

				if(*throws != NULL) {
					s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
					s3dat_internal_delete_indices(handle, dead_indices, index_len);
					handle->free_func(handle->mem_arg, dead_indices);
					handle->free_func(handle->mem_arg, pointers);
					return;
				} else {
					dead_indices[real_count].type = index_type;
					real_count++;
				}
			}
			index->sequences = s3dat_internal_alloc_func(handle, real_count*sizeof(s3dat_index_t), throws);

			if(*throws != NULL) {
				s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			} else {
				index->type = index_type;
				index->len = real_count;
				memcpy(index->sequences, dead_indices, real_count*sizeof(s3dat_index_t));
			}


			handle->free_func(handle->mem_arg, dead_indices);
			handle->free_func(handle->mem_arg, pointers);
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
					s3dat_throw(handle, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
					return;
				break;
			}

			if(index->type == index_type) {
				s3dat_throw(handle, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				return;
			}

			index->pointers = s3dat_internal_alloc_func(handle, 4*index_len, throws);
			S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

			for(uint16_t i = 0;i != index_len;i++) {
				index->pointers[i] = s3dat_internal_read32LE(handle, throws);

				if(*throws != NULL) {
					handle->free_func(handle->mem_arg, index->pointers);
					s3dat_add_to_stack(handle, throws, __FILE__ , __func__, __LINE__);
					return;
				}
			}

			index->type = index_type;
			index->len = index_len;
		}
	}
}

void s3dat_internal_read_seq(s3dat_t* handle, uint32_t from, s3dat_index_t* to, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(handle, from, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint8_t seq_start[7];

	if(!handle->read_func(handle->io_arg, seq_start, 7)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(seq_start, s3dat_seq_start, 7) != 0) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t frames = s3dat_internal_read8(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	to->len = frames;
	to->pointers = s3dat_internal_alloc_func(handle, 4*frames, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	for(uint8_t i = 0;i != frames;i++) {
		to->pointers[i] = s3dat_internal_read32LE(handle, throws)+from;

		if(*throws != NULL) {
			handle->free_func(handle->mem_arg, to->pointers);

			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		}
	}
}


void s3dat_internal_seek_func(s3dat_t* handle, uint32_t pos, int whence, s3dat_exception_t** throws) {
	if(!handle->seek_func(handle->io_arg, pos, whence)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	}
}

uint32_t le32(uint32_t le32_int) {
	#ifdef _WIN32
		#ifdef IS_BE
		return ((le32_int & 0xFF) << 24) | ((le32_int & 0xFF00) << 8) |
		((le32_int & 0xFF0000) >> 8) | ((le32_int & 0xFF000000) >> 24);
		#else
		return le32_int;
		#endif
	#else
	return le32toh(le32_int);
	#endif
}

uint32_t le32p(uint32_t* le32_int) {
	return le32(*((uint32_t*)le32_int));
}

uint32_t s3dat_internal_read32LE(s3dat_t* handle, s3dat_exception_t** throws) {
	uint32_t dat;
	if(!handle->read_func(handle->io_arg, &dat, 4)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return le32(dat);

}

void s3dat_internal_write32LE(s3dat_t* handle, uint32_t b32_int, s3dat_exception_t** throws) {
	uint32_t le32_int = le32(b32_int);
	if(!handle->write_func(handle->io_arg, &le32_int, 4)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
}

uint16_t le16(uint16_t le16_int) {
	#ifdef _WIN32
		#ifdef IS_BE
		return ((le16_int & 0xFF) << 24) | ((le16_int & 0xFF00) << 8);
		#else
		return le16_int;
		#endif
	#else
	return le16toh(le16_int);
	#endif
}

uint16_t le16p(uint16_t* le16_int) {
	return le16(*((uint16_t*)le16_int));
}


uint16_t s3dat_internal_read16LE(s3dat_t* handle, s3dat_exception_t** throws) {
	uint16_t dat;
	if(!handle->read_func(handle->io_arg, &dat, 2)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return le16(dat);
}

void s3dat_internal_write16LE(s3dat_t* handle, uint16_t b16_int, s3dat_exception_t** throws) {
	uint16_t le16_int = le16(b16_int);
	if(!handle->write_func(handle->io_arg, &le16_int, 2)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
}

uint8_t s3dat_internal_read8(s3dat_t* handle, s3dat_exception_t** throws) {
	uint8_t dat;
	if(!handle->read_func(handle->io_arg, &dat, 1)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return dat;
}

void s3dat_internal_write8(s3dat_t* handle, uint8_t b8_int, s3dat_exception_t** throws) {
	if(!handle->write_func(handle->io_arg, &b8_int, 1)) s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
}

uint32_t s3dat_internal_seek_to(s3dat_t* handle, s3dat_res_t* res, s3dat_exception_t** throws) {
	uint32_t from;

	if(res->type == s3dat_snd) {
		if(handle->sound_index->len <= res->first_index || handle->sound_index->sequences[res->first_index].len <= res->second_index) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
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
				s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
				return 0;
			}

			index = seq_index->sequences+res->first_index;
		}

		if(index->len <= index_ptr) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
			return 0;
		}

		from = index->pointers[index_ptr];
	}


	s3dat_internal_seek_func(handle, from, S3DAT_SEEK_SET, throws);
	if(*throws != NULL) s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
	return from;
}

s3dat_restype_t s3dat_internal_animation_type = {"s3dat_animation_t", (void (*) (void*)) s3dat_delete_animation, (void* (*) (void*)) s3dat_new_raw_animation};
s3dat_restype_t s3dat_internal_bitmap_type = {"s3dat_bitmap_t", (void (*) (void*)) s3dat_delete_bitmap, (void* (*) (void*)) s3dat_new_raw_bitmap};
s3dat_restype_t s3dat_internal_packed_type = {"s3dat_packed_t", (void (*) (void*)) s3dat_delete_packed, (void* (*) (void*)) s3dat_new_raw_packed};
s3dat_restype_t s3dat_internal_string_type = {"s3dat_string_t", (void (*) (void*)) s3dat_delete_string, (void* (*) (void*)) s3dat_new_raw_string};
s3dat_restype_t s3dat_internal_sound_type = {"s3dat_sound_t", (void (*) (void*)) s3dat_delete_sound, (void* (*) (void*)) s3dat_new_raw_sound};

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

