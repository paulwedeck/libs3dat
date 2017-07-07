#include "s3dat.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>

uint32_t s3dat_internal_read_index(s3dat_t* mem, uint32_t index);
uint32_t s3dat_internal_read_seq(s3dat_t* mem, uint32_t from, s3dat_index_t* to);

uint8_t s3dat_header_start_part1[33] = {
	4, 19, 4, 0, 12, 0, 0, 0, 0, 0, 0, 0, 84, 0, 0, 0,
	32, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0
};

uint8_t s3dat_header_start_part2[10] = {0, 0, 31, 0, 0, 0, 0, 0, 0, 0};

uint8_t s3dat_header_end[12] = {4, 25, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0};

uint8_t s3dat_header_rgb5[5] = {124, 0, 0, 224, 3};
uint8_t s3dat_header_rgb565[5] = {248, 0, 0, 224, 7};

uint8_t s3dat_seq_start[7] = {2, 20, 0, 0, 8, 0, 0};

uint32_t s3dat_readfile_fd(s3dat_t* mem, uint32_t file) {
	return s3dat_readfile_func(mem, file, s3dat_default_read_func,
		s3dat_default_size_func, s3dat_default_pos_func, s3dat_default_seek_func);
}

uint32_t s3dat_readfile_func(s3dat_t* mem, uint32_t arg, 
	void (*read_func) (uint32_t, void*, size_t),
	size_t (*size_func) (uint32_t),
	size_t (*pos_func) (uint32_t),
	void (*seek_func) (uint32_t, uint32_t, int)) {
	mem->io_arg = arg;
	mem->read_func = read_func;
	mem->size_func = size_func;
	mem->pos_func = pos_func;
	mem->seek_func = seek_func;

	uint32_t p = 0;
	uint32_t size = size_func(arg);

	// read file header
	// this file may be a SND .dat file
	uint8_t header_part1[33];
	if(96 > size) return s3dat_internal_readsnd(mem);
	read_func(arg, header_part1, 33);
	if(memcmp(header_part1, s3dat_header_start_part1, 33) != 0) return s3dat_internal_readsnd(mem);
	p += 33;

	// it must be a GFX .dat file
	uint8_t header_filetype[5];
	read_func(arg, header_filetype, 5);
	if(memcmp(header_filetype, s3dat_header_rgb5, 5) == 0) {
		mem->green_6b = false;
	} else if(memcmp(header_filetype, s3dat_header_rgb565, 5) == 0) {
		mem->green_6b = true;
	} else {
		return S3DAT_UNKNOWN_FILETYPE;
	}
	p += 5;

	uint8_t header_part2[10];
	read_func(arg, header_part2, 10);
	if(memcmp(header_part2, s3dat_header_start_part2, 10) != 0) return S3DAT_ERROR_CORRUPT_HEADER;
	p += 10;

	uint32_t file_size = s3dat_internal_read32LE(mem);
	if(file_size != size) {
		#ifndef S3DAT_COMPATIBILITY_MODE
		return S3DAT_ERROR_CORRUPT_HEADER;
		#endif
	}
	seek_func(arg, 4, SEEK_CUR);
	p += 8;
	uint32_t sequence_pointers[6];
	for(uint32_t i = 0;i != 6;i++) sequence_pointers[i] = s3dat_internal_read32LE(mem);

	seek_func(arg, 4, SEEK_CUR);
	p += 28;

	uint8_t header_end[12];
	read_func(arg, header_end, 12);
	if(memcmp(header_end, s3dat_header_end, 12) != 0) return S3DAT_ERROR_CORRUPT_HEADER;
	p += 12;

	for(uint32_t i = 0;i < 6;i++) {
		uint32_t return_value = s3dat_internal_read_index(mem, sequence_pointers[i]);
		if(return_value != S3DAT_READ_SUCCESSFUL) {
			#ifndef S3DAT_COMPATIBILITY_MODE
			return return_value;
			#endif
		}
	}

	return S3DAT_READ_SUCCESSFUL;
}

uint32_t s3dat_internal_read_index(s3dat_t* mem, uint32_t index) {
	uint32_t size = mem->size_func(mem->io_arg);

	if(index > size) {
		return S3DAT_ERROR_CORRUPT_INDEX;
	}
	mem->seek_func(mem->io_arg, index, SEEK_SET);
	uint32_t p = index;
	if(p+8 > size) {
		return S3DAT_ERROR_CORRUPT_INDEX;
	}

	uint32_t index_type = s3dat_internal_read32LE(mem);
	uint16_t index_size = s3dat_internal_read16LE(mem);
	uint16_t index_len = s3dat_internal_read16LE(mem);

	if(index_len*4+8 != index_size || p+index_size > size || (index_type != s3dat_settler && index_type != s3dat_torso && index_type != s3dat_shadow && index_type != s3dat_landscape && index_type != s3dat_gui && index_type != s3dat_nyi)) {
		return S3DAT_ERROR_CORRUPT_INDEX;
	}
	p += 8;

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
		if(index->type == index_type) return S3DAT_ERROR_INDEX_TYPE_COLLISION;
		uint32_t* pointers = mem->alloc_func(mem->mem_arg, 4*index_len);
		for(uint16_t i = 0;i != index_len;i++) pointers[i] = s3dat_internal_read32LE(mem);
		index->type = index_type;
		s3dat_index_t* dead_index = mem->alloc_func(mem->mem_arg, index_len*sizeof(s3dat_index_t));
		uint16_t real_count = 0;

		for(uint16_t i = 0;i != index_len;i++) {
			dead_index[real_count].type = index_type;
			uint32_t return_value = s3dat_internal_read_seq(mem, pointers[i], dead_index+real_count);
			if(return_value != S3DAT_READ_SUCCESSFUL) {
				#ifndef S3DAT_COMPATIBILITY_MODE
				index->type = 0;
				index->len = 0;
				mem->free_func(mem->mem_arg, dead_index);
				mem->free_func(mem->mem_arg, pointers);
				return return_value;
				#endif
			} else {
				dead_index[real_count].type = index_type;
				real_count++;
			}
		}
		index->len = real_count;
		index->sequences = mem->alloc_func(mem->mem_arg, real_count*sizeof(s3dat_index_t));
		memcpy(index->sequences, dead_index, real_count*sizeof(s3dat_index_t)); 

		mem->free_func(mem->mem_arg, dead_index);
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
			case s3dat_nyi:
				index = &mem->nyi_index;
			break;
		}

		if(index->type == index_type) return S3DAT_ERROR_INDEX_TYPE_COLLISION;

		index->type = index_type;
		index->len = index_len;

		index->pointers = mem->alloc_func(mem->mem_arg, 4*index_len);
		for(uint16_t i = 0;i != index_len;i++) index->pointers[i] = s3dat_internal_read32LE(mem);
	}

	return S3DAT_READ_SUCCESSFUL;
}

uint32_t s3dat_internal_read_seq(s3dat_t* mem, uint32_t from, s3dat_index_t* to) {
	mem->seek_func(mem->io_arg, from, SEEK_SET);
	uint32_t p = from;
	uint32_t size = mem->size_func(mem->io_arg);
	uint8_t seq_start[7];
	if(p+8 > size) {
		return S3DAT_ERROR_FILE_TOO_SHORT;
	}

	mem->read_func(mem->io_arg, seq_start, 7);
	if(memcmp(seq_start, s3dat_seq_start, 7) != 0) {
		return S3DAT_ERROR_CORRUPT_SEQUENCE;
	}
	p += 7;

	uint8_t frames = s3dat_internal_read8(mem);
	p++;
	if(p+frames*4 > size) {
		return S3DAT_ERROR_FILE_TOO_SHORT;
	}

	to->len = frames;
	to->pointers = mem->alloc_func(mem->mem_arg, 4*frames);

	for(uint8_t i = 0;i != frames;i++) {
		to->pointers[i] = s3dat_internal_read32LE(mem)+from;
	}

	return S3DAT_READ_SUCCESSFUL;
}

void s3dat_default_read_func(uint32_t arg, void* bfr, size_t len) {
	read(arg, bfr, len);
}

size_t s3dat_default_size_func(uint32_t arg) {
	struct stat file_stat;
	fstat(arg, &file_stat);
	return file_stat.st_size;
}

size_t s3dat_default_pos_func(uint32_t arg) {
	return lseek(arg, 0, SEEK_CUR);
}

void s3dat_default_seek_func(uint32_t arg, uint32_t pos, int whence) {
	lseek(arg, pos,  whence);
}

uint32_t s3dat_internal_read32LE(s3dat_t* mem) {
	uint32_t dat;
	mem->read_func(mem->io_arg, &dat, 4);
	return le32toh(dat);
}

uint16_t s3dat_internal_read16LE(s3dat_t* mem) {
	uint16_t dat;
	mem->read_func(mem->io_arg, &dat, 2);
	return le16toh(dat);
}

uint16_t s3dat_internal_read8(s3dat_t* mem) {
	uint8_t dat;
	mem->read_func(mem->io_arg, &dat, 1);
	return dat;
}

