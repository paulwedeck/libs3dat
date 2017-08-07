#include "s3dat_internal.h"
#line __LINE__ "read.c"

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

void s3dat_readfile_fd(s3dat_t* mem, uint32_t file, s3dat_exception_t** throws) {
	s3dat_readfile_func(mem, file, s3dat_default_read_func,
		s3dat_default_size_func, s3dat_default_pos_func, s3dat_default_seek_func, throws);
}

void s3dat_readfile_func(s3dat_t* mem, uint32_t arg,
	bool (*read_func) (uint32_t, void*, size_t),
	size_t (*size_func) (uint32_t),
	size_t (*pos_func) (uint32_t),
	bool (*seek_func) (uint32_t, uint32_t, int),
	s3dat_exception_t** throws) {
	mem->io_arg = arg;
	mem->read_func = read_func;
	mem->size_func = size_func;
	mem->pos_func = pos_func;
	mem->seek_func = seek_func;

	uint8_t header_part1[33];
	if(!read_func(arg, header_part1, 16)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part1, s3dat_internal_snd_header_read_c, 16) == 0) {
		s3dat_internal_readsnd(mem, throws);
		if(*throws != NULL) s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
		return;
	}

	if(!read_func(arg, header_part1+16, 17)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}
	if(memcmp(header_part1, s3dat_header_start_part1, 33) != 0) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_filetype[5];
	if(!read_func(arg, header_filetype, 5)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_filetype, s3dat_header_rgb5, 5) == 0) {
		mem->green_6b = false;
	} else if(memcmp(header_filetype, s3dat_header_rgb565, 5) == 0) {
		mem->green_6b = true;
	} else {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t header_part2[10];
	if(!read_func(arg, header_part2, 10)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(header_part2, s3dat_header_start_part2, 10) != 0) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t file_size = s3dat_internal_read32LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	if(file_size != mem->size_func(mem->io_arg)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t sequence_pointers[8];
	for(uint32_t i = 0;i != 8;i++) {
		sequence_pointers[i] = s3dat_internal_read32LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}

	for(uint32_t i = 0;i < 8;i++) {
		s3dat_internal_read_index(mem, sequence_pointers[i], throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_read_index(s3dat_t* mem, uint32_t index, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, index, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t index_type = s3dat_internal_read32LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	if(index_type == s3dat_string) {
		uint32_t index_size = s3dat_internal_read32LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		uint16_t texts = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		uint16_t languages = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		if(languages*texts*4+12 != index_size) {
			s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
		}

		mem->string_index.len = texts;
		mem->string_index.type = s3dat_string;
		mem->string_index.sequences = mem->alloc_func(mem->mem_arg, texts*sizeof(s3dat_index_t));

		for(uint16_t t = 0;t != texts;t++) {
			mem->string_index.sequences[t].len = languages;
			mem->string_index.sequences[t].type = s3dat_string;
			mem->string_index.sequences[t].pointers = mem->alloc_func(mem->mem_arg, languages*4);
		}

		for(uint16_t l = 0;l != languages;l++) {
			for(uint16_t t = 0;t != texts;t++) {
				mem->string_index.sequences[t].pointers[l] = s3dat_internal_read32LE(mem, throws);

				if(*throws != NULL) {
					s3dat_internal_delete_seq(mem, &mem->string_index);
				}
			}
		}

		return;
	}

	uint16_t index_size = s3dat_internal_read16LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint16_t index_len = s3dat_internal_read16LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	if((index_type != s3dat_palette && index_len*4+8 != index_size) ||
		(index_type == s3dat_palette && index_len*4+12 != index_size)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
		return;
	}

	if(index_type != s3dat_settler && index_type != s3dat_torso &&
		index_type != s3dat_shadow && index_type != s3dat_landscape &&
		index_type != s3dat_gui && index_type != s3dat_animation &&
		index_type != s3dat_palette) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
		return;
	}

	if(index_type == s3dat_palette) {
		mem->palette_line_length = s3dat_internal_read32LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}

	if(index_type == s3dat_settler || index_type == s3dat_torso || index_type == s3dat_shadow) {
		s3dat_seq_index_t* index;
		switch(index_type) {
			case s3dat_settler:
				index = &mem->settler_index;
			break;
			case s3dat_torso:
				index = &mem->torso_index;
			break;
			case s3dat_shadow:
				index = &mem->shadow_index;
			break;
		}
		if(index->type == index_type) {
			s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
			return;
		}

		uint32_t* pointers = mem->alloc_func(mem->mem_arg, 4*index_len);
		for(uint16_t i = 0;i != index_len;i++)  {
			pointers[i] = s3dat_internal_read32LE(mem, throws);

			if(*throws != NULL) {
				s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
				mem->free_func(mem->mem_arg, pointers);
			}
		}

		s3dat_index_t* dead_indices = mem->alloc_func(mem->mem_arg, index_len*sizeof(s3dat_index_t));
		uint16_t real_count = 0;

		for(uint16_t i = 0;i != index_len;i++) {
			s3dat_internal_read_seq(mem, pointers[i], dead_indices+real_count, throws);

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
		index->type = index_type;
		index->len = real_count;
		index->sequences = mem->alloc_func(mem->mem_arg, real_count*sizeof(s3dat_index_t));
		memcpy(index->sequences, dead_indices, real_count*sizeof(s3dat_index_t));

		mem->free_func(mem->mem_arg, dead_indices);
		mem->free_func(mem->mem_arg, pointers);
	} else {
		s3dat_index_t* index;
		switch(index_type) {
			case s3dat_gui:
				index = &mem->gui_index;
			break;
			case s3dat_landscape:
				index = &mem->landscape_index;
			break;
			case s3dat_animation:
				index = &mem->animation_index;
			break;
			case s3dat_palette:
				index = &mem->palette_index;
			break;
			default:
				s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
			break;
		}

		if(index->type == index_type) {
			s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_INDEXTYPE, __FILE__, __func__, __LINE__);
			return;
		}

		index->type = index_type;
		index->len = index_len;

		index->pointers = mem->alloc_func(mem->mem_arg, 4*index_len);
		for(uint16_t i = 0;i != index_len;i++) {
			index->pointers[i] = s3dat_internal_read32LE(mem, throws);
			S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
		}
	}
}

void s3dat_internal_read_seq(s3dat_t* mem, uint32_t from, s3dat_index_t* to, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, from, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint8_t seq_start[7];

	if(!mem->read_func(mem->io_arg, seq_start, 7)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(memcmp(seq_start, s3dat_seq_start, 7) != 0) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint8_t frames = s3dat_internal_read8(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->len = frames;
	to->pointers = mem->alloc_func(mem->mem_arg, 4*frames);

	for(uint8_t i = 0;i != frames;i++) {
		to->pointers[i] = s3dat_internal_read32LE(mem, throws)+from;

		if(*throws != NULL) {
			mem->free_func(mem->mem_arg, to->pointers);

			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			return;
		}
	}
}

bool s3dat_default_read_func(uint32_t arg, void* bfr, size_t len) {
	return read(arg, bfr, len) == len;
}

size_t s3dat_default_size_func(uint32_t arg) {
	struct stat file_stat;
	fstat(arg, &file_stat);
	return file_stat.st_size;
}

size_t s3dat_default_pos_func(uint32_t arg) {
	return lseek(arg, 0, SEEK_CUR);
}

void s3dat_internal_seek_func(s3dat_t* mem, uint32_t pos, int whence, s3dat_exception_t** throws) {
	if(!mem->seek_func(mem->io_arg, pos, whence)) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	}
}

bool s3dat_default_seek_func(uint32_t arg, uint32_t pos, int whence) {
	int lseek_whence = whence == S3DAT_SEEK_CUR ? SEEK_CUR : SEEK_SET;
	return lseek(arg, pos,  lseek_whence) != (off_t)-1;
}

uint32_t s3dat_internal_read32LE(s3dat_t* mem, s3dat_exception_t** throws) {
	uint32_t dat;
	if(!mem->read_func(mem->io_arg, &dat, 4)) s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
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
	if(!mem->read_func(mem->io_arg, &dat, 2)) s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
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
	if(!mem->read_func(mem->io_arg, &dat, 1)) s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	return dat;
}

void s3dat_extract_animation(s3dat_t* mem, uint16_t animation, s3dat_animation_t* to, s3dat_exception_t** throws) {
	if(mem->animation_index.len <= animation) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(mem, mem->animation_index.pointers[animation], S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t entries = s3dat_internal_read32LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->src = mem;
	to->len = entries;
	to->frames = mem->alloc_func(mem->mem_arg, entries*sizeof(s3dat_frame_t));

	for(uint32_t i = 0;i != entries;i++) {
 		to->frames[i].posx = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].posx = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].settler_id = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].settler_file = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].torso_id = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].torso_file = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].shadow_id = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].shadow_file = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].settler_frame = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].torso_frame = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);


		to->frames[i].flag1 = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

		to->frames[i].flag2 = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}
}

uint8_t* s3dat_internal_read_cstr(s3dat_t* mem, s3dat_exception_t** throws) {
	#define STRING_BUFFER 1024

	uint8_t* bfr = mem->alloc_func(mem->mem_arg, STRING_BUFFER);
	uint32_t bfr_size = STRING_BUFFER;
	uint32_t pos = 0;
	do {
		bfr[pos] = s3dat_internal_read8(mem, throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			mem->free_func(mem->mem_arg, bfr);
			return NULL;
		}

		if(pos+1 == bfr_size) {
			uint8_t* bfr2 = mem->alloc_func(mem->mem_arg, bfr_size+STRING_BUFFER);
			memcpy(bfr2, bfr, bfr_size);
			mem->free_func(mem->mem_arg, bfr);
			bfr = bfr2;
			bfr_size += STRING_BUFFER;
		}
	} while(bfr[pos++] != '\0');

	return bfr;
}

void s3dat_internal_short(s3dat_t* mem, uint8_t** str) {
	uint8_t* bfr = *str;

	uint8_t* bfr2 = mem->alloc_func(mem->mem_arg, strlen(bfr)+1);
	strcpy(bfr, bfr2);
	mem->free_func(mem->mem_arg, bfr);

	*str = bfr2;
}

uint16_t s3dat_internal_iso8859_2_to_utf8_map[96] = {0xA0, 0x104, 0x2D8, 0x141, 0xA4, 0x13D, 0x15A, 0xA7, 0xA8, 0x160, 0x15E, 0x164, 0x179, 0xAD, 0x17D, 0x17B, 0xB0, 0x105, 0x2DB, 0x142, 0xB4, 0x13E, 0x15B, 0x2C7, 0xB8, 0x161, 0x15F, 0x165, 0x17A, 0x2DD, 0x17E, 0x17C, 0x154, 0xC1, 0xC2, 0x102, 0xC4, 0x139, 0x106, 0xC7, 0x10C, 0xC9, 0x118, 0xCB, 0x11A, 0xCD, 0xCE, 0x10E, 0x110, 0x143, 0x147, 0xD3, 0xD4, 0x150, 0xD6, 0xD7, 0x158, 0x16E, 0xDA, 0x170, 0xDC, 0xDD, 0x162, 0xDF, 0x155, 0xE1, 0xE2, 0x103, 0xE4, 0x13A, 0x107, 0xE7, 0x10D, 0xE9, 0x119, 0xEB, 0x11B, 0xED, 0xEE, 0x10F, 0x111, 0x144, 0x148, 0xF3, 0xF4, 0x151, 0xF6, 0xF7, 0x159, 0x16F, 0xFA, 0x171, 0xFC, 0xFD, 0x163, 0x2D9};

void s3dat_internal_iso8859_to_utf8(s3dat_t* mem, uint8_t** str, uint32_t len, bool iso8859_2) {

	uint8_t* bfr = *str;

	uint32_t real_len = 0;
	for(uint32_t i = 0;i != len;i++) {
		if(bfr[i] == '\\' && i+1 != len && bfr[i+1] == 'n') { // \n is only one character
		} else if(iso8859_2 && bfr[i] > 126 && bfr[i] < 160) {
			real_len += 3; // unknown chars like some not iso8859-2 polish characters
		} else if(bfr[i] >= 128){
			real_len += 2;
		} else {
			real_len++;
		}
	}

	uint8_t* bfr2 = mem->alloc_func(mem->mem_arg, real_len);
	uint32_t bfr2_ptr = 0;
	for(uint32_t bfr_ptr = 0;bfr_ptr != len;bfr_ptr++) {
		if(bfr[bfr_ptr] == '\\' && bfr_ptr+1 != len && bfr[bfr_ptr+1] == 'n') {
			bfr2[bfr2_ptr] = '\n';
			bfr_ptr++;
		} else if(iso8859_2 && bfr[bfr_ptr] > 126 && bfr[bfr_ptr] < 160) {
			bfr2[bfr2_ptr] = 0xEF;
			bfr2[bfr2_ptr+1] = 0xBF;
			bfr2[bfr2_ptr+2] = 0xBD;
			bfr2_ptr += 2;
		} else if((iso8859_2 && bfr[bfr_ptr] > 0xA0) || (!iso8859_2 && bfr[bfr_ptr] >= 128)) {
			uint16_t character = iso8859_2 ? s3dat_internal_iso8859_2_to_utf8_map[bfr[bfr_ptr]-0xA0] : bfr[bfr_ptr];
			bfr2[bfr2_ptr+1] = 0x80 | (character & 0x3F);
			bfr2[bfr2_ptr] = 0xC0 | ((character & 0x7C0) >> 6);
			bfr2_ptr++;
		} else {
			bfr2[bfr2_ptr] = bfr[bfr_ptr];
		}

		bfr2_ptr++;
	}

	mem->free_func(mem->mem_arg, bfr);
	bfr2[real_len-1] = '\0';

	*str = bfr2;
}

#ifdef USE_ICONV
void s3dat_internal_iconv_dat_to_utf8(s3dat_t* mem, s3dat_language language, uint8_t* cstr, uint8_t** utf8_str, s3dat_exception_t** throws) {
	char* charset;

	switch(language) {
		case s3dat_german:
		case s3dat_english:
		case s3dat_spanish:
		case s3dat_italian:
		case s3dat_french:
		default:
			charset = "iso8859-1";
		break;
		case s3dat_polish:
			charset = "iso8859-2";
		break;
		case s3dat_korean:
			charset = "EUC-KR";
		break;
		case s3dat_japanese:
			charset = "SHIFT_JIS";
		break;
	}
	iconv_t iconv_s = iconv_open("UTF8", charset);

	if(iconv_s == (iconv_t)-1) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_ICONV_ERROR, __FILE__, __func__, __LINE__);
		return;
	}

	size_t inlen = strlen(cstr);
	size_t outlen = inlen*4+4;
	char* utf8s = mem->alloc_func(mem->mem_arg, outlen);
	*utf8_str = utf8s;
	char* instr = cstr;

	if(iconv(iconv_s, &instr, &inlen, &utf8s, &outlen) == (size_t)-1) {
		mem->free_func(mem->mem_arg, *utf8_str);
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_ICONV_ERROR, __FILE__, __func__, __LINE__);
	}
	iconv_close(iconv_s);
}
#endif

void s3dat_extract_string(s3dat_t* mem, uint16_t text, s3dat_language language, s3dat_string_t* to, bool utf8, s3dat_exception_t** throws) {
	if(text > mem->string_index.len || language > mem->string_index.sequences[text].len) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(mem, mem->string_index.sequences[text].pointers[language], S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint8_t* cstr = s3dat_internal_read_cstr(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->original_encoding = true;
	to->language = language;
	to->src = mem;

	if(utf8) {
		#ifdef USE_ICONV
		uint8_t* utf8_str = NULL;
		s3dat_internal_iconv_dat_to_utf8(mem, language, cstr, &utf8_str, throws);

		if(*throws != NULL) {
			s3dat_internal_short(mem, &cstr);
			to->string_data = cstr;
		} else {
			mem->free_func(mem->mem_arg, cstr);
			to->original_encoding = false;
			to->string_data = utf8_str;
		}
		#else
		if(language != s3dat_japanese && language != s3dat_korean) {
			s3dat_internal_iso8859_to_utf8(mem, &cstr, strlen(cstr), language == s3dat_polish);
		}

		to->string_data = cstr;
		#endif
	} else {
		s3dat_internal_short(mem, &cstr);
		to->string_data = cstr;
	}
}

