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

uint8_t s3dat_header_end[12] = {4, 25, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0};

uint8_t s3dat_header_rgb5[5] = {124, 0, 0, 224, 3};
uint8_t s3dat_header_rgb565[5] = {248, 0, 0, 224, 7};

uint8_t s3dat_seq_start[7] = {2, 20, 0, 0, 8, 0, 0};

void s3dat_readfile_fd(s3dat_t* mem, uint32_t* file, s3dat_exception_t** throws) {
	s3dat_readfile_ioset(mem, file, s3dat_get_default_ioset(S3DAT_IOSET_DEFAULT), false, throws);
}


void s3dat_readfile_name(s3dat_t* mem, char* name, s3dat_exception_t** throws) {
	s3dat_readfile_ioset(mem, name, s3dat_get_default_ioset(S3DAT_IOSET_DEFAULT), true, throws);
}

void s3dat_readfile_ioset(s3dat_t* mem, void* io_arg, s3dat_ioset_t* ioset, bool use_openclose_func,  s3dat_exception_t** throws) {
	if(ioset == NULL || ioset->available == false) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOSET, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_readfile_func(mem, io_arg, ioset->read_func, ioset->size_func, ioset->pos_func, ioset->seek_func, (use_openclose_func ? ioset->open_func : NULL), (use_openclose_func ? ioset->close_func : NULL), ioset->fork_func, throws);
}

void s3dat_readfile_func(s3dat_t* mem, void* arg,
	bool (*read_func) (void*, void*, size_t),
	size_t (*size_func) (void*),
	size_t (*pos_func) (void*),
	bool (*seek_func) (void*, uint32_t, int),
	void* (*open_func) (void*),
	void (*close_func) (void*),
	void* (*fork_func) (void*),
	s3dat_exception_t** throws) {
	mem->io_arg = arg;
	mem->read_func = read_func;
	mem->size_func = size_func;
	mem->pos_func = pos_func;
	mem->seek_func = seek_func;
	mem->open_func = open_func;
	mem->close_func = close_func;
	mem->fork_func = fork_func;

	mem->last_handler = s3dat_new_exhandler(mem);
	mem->last_handler->call = s3dat_default_extract;

	if(open_func != NULL) mem->io_arg = open_func(mem->io_arg);

	if(mem->io_arg == NULL) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_OPEN, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part1[33];
	if(!read_func(mem->io_arg, header_part1, 16)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part1, s3dat_internal_snd_header_read_c, 16) == 0) {
		s3dat_internal_readsnd(mem, throws);
		if(*throws != NULL) s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
		return;
	}

	if(!read_func(mem->io_arg, header_part1+16, 17)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}
	if(memcmp(header_part1, s3dat_header_start_part1, 33) != 0) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_filetype[5];
	if(!read_func(mem->io_arg, header_filetype, 5)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_filetype, s3dat_header_rgb5, 5) == 0) {
		mem->green_6b = false;
	} else if(memcmp(header_filetype, s3dat_header_rgb565, 5) == 0) {
		mem->green_6b = true;
	} else {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part2[10];
	if(!read_func(mem->io_arg, header_part2, 10)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part2, s3dat_header_start_part2, 10) != 0) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t file_size = s3dat_internal_read32LE(mem, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	if(file_size != mem->size_func(mem->io_arg)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t sequence_pointers[8];
	for(uint32_t i = 0;i != 8;i++) {
		sequence_pointers[i] = s3dat_internal_read32LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}

	for(uint32_t i = 0;i < 8;i++) {
		s3dat_internal_read_index(mem, sequence_pointers[i], throws);
		S3DAT_INTERNAL_ADD_ATTR(mem, throws, S3DAT_ATTRIBUTE_INDEX, i);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_read_index(s3dat_t* mem, uint32_t index, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, index, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t index_type = s3dat_internal_read32LE(mem, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	if(index_type == s3dat_string) {
		uint32_t index_size = s3dat_internal_read32LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		uint16_t texts = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		uint16_t languages = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		if(languages*texts*4+12 != index_size) {
			s3dat_throw(mem, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
		}

		mem->string_index->len = texts;
		mem->string_index->type = s3dat_string;
		mem->string_index->sequences = s3dat_internal_alloc_func(mem, texts*sizeof(s3dat_index_t), throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			mem->string_index->type = 0;
			return;
		}

		for(uint16_t t = 0;t != texts;t++) {
			mem->string_index->sequences[t].len = languages;
			mem->string_index->sequences[t].type = s3dat_string;
			mem->string_index->sequences[t].pointers = s3dat_internal_alloc_func(mem, languages*4, throws);
			if(*throws != NULL) {
				s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);

				mem->free_func(mem->mem_arg, mem->string_index->sequences);
				for(uint16_t i = 0;i != (texts-1);i++) {
					mem->free_func(mem->mem_arg, mem->string_index->sequences[t].pointers);
				}

				mem->string_index->type = 0;
				return;
			}
		}

		for(uint16_t l = 0;l != languages;l++) {
			for(uint16_t t = 0;t != texts;t++) {
				mem->string_index->sequences[t].pointers[l] = s3dat_internal_read32LE(mem, throws);

				if(*throws != NULL) {
					s3dat_internal_delete_seq(mem, mem->string_index);
					return;
				}
			}
		}
	} else {

		uint16_t index_size = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		uint16_t index_len = s3dat_internal_read16LE(mem, throws);
		S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		if((index_type != s3dat_palette && index_len*4+8 != index_size) ||
			(index_type == s3dat_palette && index_len*4+12 != index_size)) {
			s3dat_throw(mem, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
			return;
		}

		if(index_type != s3dat_settler && index_type != s3dat_torso &&
			index_type != s3dat_shadow && index_type != s3dat_landscape &&
			index_type != s3dat_gui && index_type != s3dat_animation &&
			index_type != s3dat_palette) {
			s3dat_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
			return;
		}

		if(index_type == s3dat_palette) {
			mem->palette_line_length = s3dat_internal_read32LE(mem, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		}

		if(index_type == s3dat_settler || index_type == s3dat_torso || index_type == s3dat_shadow) {
			s3dat_seq_index_t* index;
			switch(index_type) {
				case s3dat_settler:
					index = mem->settler_index;
				break;
				case s3dat_torso:
					index = mem->torso_index;
				break;
				case s3dat_shadow:
					index = mem->shadow_index;
				break;
			}
			if(index->type == index_type) {
				s3dat_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				return;
			}

			uint32_t* pointers = s3dat_internal_alloc_func(mem, 4*index_len, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

			for(uint16_t i = 0;i != index_len;i++)  {
				pointers[i] = s3dat_internal_read32LE(mem, throws);

				if(*throws != NULL) {
					s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
					mem->free_func(mem->mem_arg, pointers);
				}
			}

			s3dat_index_t* dead_indices = s3dat_internal_alloc_func(mem, index_len*sizeof(s3dat_index_t), throws);
			if(*throws != NULL) {
				s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
				mem->free_func(mem->mem_arg, pointers);
				return;
			}
			uint16_t real_count = 0;

			for(uint16_t i = 0;i != index_len;i++) {
				s3dat_internal_read_seq(mem, pointers[i], dead_indices+real_count, throws);
				S3DAT_INTERNAL_ADD_ATTR(mem, throws, S3DAT_ATTRIBUTE_SEQ, i);

				if(*throws != NULL) {
					s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
					s3dat_internal_delete_indices(mem, dead_indices, index_len);
					mem->free_func(mem->mem_arg, dead_indices);
					mem->free_func(mem->mem_arg, pointers);
					return;
				} else {
					dead_indices[real_count].type = index_type;
					real_count++;
				}
			}
			index->sequences = s3dat_internal_alloc_func(mem, real_count*sizeof(s3dat_index_t), throws);

			if(*throws != NULL) {
				s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			} else {
				index->type = index_type;
				index->len = real_count;
				memcpy(index->sequences, dead_indices, real_count*sizeof(s3dat_index_t));
			}


			mem->free_func(mem->mem_arg, dead_indices);
			mem->free_func(mem->mem_arg, pointers);
		} else {
			s3dat_index_t* index;
			switch(index_type) {
				case s3dat_gui:
					index = mem->gui_index;
				break;
				case s3dat_landscape:
					index = mem->landscape_index;
				break;
				case s3dat_animation:
					index = mem->animation_index;
				break;
				case s3dat_palette:
					index = mem->palette_index;
				break;
				default:
					s3dat_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				break;
			}

			if(index->type == index_type) {
				s3dat_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
				return;
			}

			index->pointers = s3dat_internal_alloc_func(mem, 4*index_len, throws);
			S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

			for(uint16_t i = 0;i != index_len;i++) {
				index->pointers[i] = s3dat_internal_read32LE(mem, throws);

				if(*throws != NULL) {
					mem->free_func(mem->mem_arg, index->pointers);
					s3dat_add_to_stack(mem, throws, __FILE__ , __func__, __LINE__);
					return;
				}
			}

			index->type = index_type;
			index->len = index_len;
		}
	}
}

void s3dat_internal_read_seq(s3dat_t* mem, uint32_t from, s3dat_index_t* to, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, from, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint8_t seq_start[7];

	if(!mem->read_func(mem->io_arg, seq_start, 7)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(seq_start, s3dat_seq_start, 7) != 0) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t frames = s3dat_internal_read8(mem, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->len = frames;
	to->pointers = s3dat_internal_alloc_func(mem, 4*frames, throws);
	S3DAT_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	for(uint8_t i = 0;i != frames;i++) {
		to->pointers[i] = s3dat_internal_read32LE(mem, throws)+from;

		if(*throws != NULL) {
			mem->free_func(mem->mem_arg, to->pointers);

			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			return;
		}
	}
}


void s3dat_internal_seek_func(s3dat_t* mem, uint32_t pos, int whence, s3dat_exception_t** throws) {
	if(!mem->seek_func(mem->io_arg, pos, whence)) {
		s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	}
}

uint32_t s3dat_internal_read32LE(s3dat_t* mem, s3dat_exception_t** throws) {
	uint32_t dat;
	if(!mem->read_func(mem->io_arg, &dat, 4)) s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	#ifdef _WIN32
		#ifdef IS_BE
		return ((dat & 0xFF) << 24) | ((dat & 0xFF00) << 8) |
		((dat & 0xFF0000) >> 8) | ((dat & 0xFF000000) >> 24);
		#else
		return dat;
		#endif
	#else
	return le32toh(dat);
	#endif
}

uint16_t s3dat_internal_read16LE(s3dat_t* mem, s3dat_exception_t** throws) {
	uint16_t dat;
	if(!mem->read_func(mem->io_arg, &dat, 2)) s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	#ifdef _WIN32
		#ifdef IS_BE
		return ((dat & 0xFF) << 24) | ((dat & 0xFF00) << 8);
		#else
		return dat;
		#endif
	#else
	return le16toh(dat);
	#endif
}

uint16_t s3dat_internal_read8(s3dat_t* mem, s3dat_exception_t** throws) {
	uint8_t dat;
	if(!mem->read_func(mem->io_arg, &dat, 1)) s3dat_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return dat;
}

