#include "s3dat.h"

#include <string.h>
#include <stdio.h>
#include <endian.h>

uint32_t s3dat_internal_readsnd_index(s3dat_t* mem, uint32_t from, s3dat_index32_t* to);

uint8_t s3dat_internal_snd_header[16] = { 68, 21, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 0 };

uint32_t s3dat_internal_readsnd(s3dat_t* mem) {
	mem->seek_func(mem->io_arg, 0, SEEK_SET);

	uint32_t size = mem->size_func(mem->io_arg);
	uint32_t p = 36;
	if(36 >= size) return S3DAT_ERROR_FILE_TOO_SHORT;

	uint8_t header[16];
	mem->read_func(mem->io_arg, header, 16);

	if(memcmp(s3dat_internal_snd_header, header, 16) != 0) return S3DAT_ERROR_FILE_TOO_SHORT;

	uint32_t size_field;
	size_field = s3dat_internal_read32LE(mem);

	if(size != size_field) {
#ifndef S3DAT_COMPATIBILITY_MODE
		return S3DAT_ERROR_CORRUPT_HEADER;
#endif
	}

	mem->seek_func(mem->io_arg, 34, SEEK_SET);

	uint16_t len = s3dat_internal_read16LE(mem);

	if(len*4+p >= size) return S3DAT_ERROR_FILE_TOO_SHORT;

	s3dat_index32_t* indices_data = mem->alloc_func(mem->mem_arg, len*sizeof(s3dat_index32_t));
	uint32_t* pointers = mem->alloc_func(mem->mem_arg, len*4);
	for(uint16_t i = 0;i != len;i++) {
		pointers[i] = s3dat_internal_read32LE(mem);
	}

	uint16_t alive_len = 0;

	for(uint16_t i = 0;i != len;i++) {
		uint32_t return_value = s3dat_internal_readsnd_index(mem, pointers[i], indices_data+alive_len);
		if(return_value != S3DAT_READ_SUCCESSFUL) {
#ifndef S3DAT_COMPATIBILITY_MODE
			mem->free_func(mem->mem_arg, pointers);
			mem->free_func(mem->mem_arg, indices_data);
			return return_value;
#endif
		} else {
			alive_len++;
		}
	}

	mem->free_func(mem->io_arg, pointers);

	mem->sound_index.type = s3dat_snd;
	if(alive_len != len) {
		s3dat_index32_t* alive_data = mem->alloc_func(mem->mem_arg, alive_len*sizeof(s3dat_index32_t));
		memcpy(alive_data, indices_data, alive_len*sizeof(s3dat_index32_t));
		mem->free_func(mem->io_arg, indices_data);
		mem->sound_index.sequences = alive_data;
		mem->sound_index.len = alive_len;
	} else {
		mem->sound_index.sequences = indices_data;
		mem->sound_index.len = len;
	}
	return S3DAT_READ_SUCCESSFUL;
}

uint32_t s3dat_internal_readsnd_index(s3dat_t* mem, uint32_t from, s3dat_index32_t* to) {
	mem->seek_func(mem->io_arg, from, SEEK_SET);
	uint32_t p = from;
	uint32_t size = mem->size_func(mem->io_arg);

	if(p+4 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

	uint32_t len = s3dat_internal_read32LE(mem);
	p += 4;

	if(p+len*4 > size) return S3DAT_ERROR_FILE_TOO_SHORT;

	uint32_t* pointers = mem->alloc_func(mem->mem_arg, len*4);
	for(uint32_t i = 0;i != len;i++) {
		pointers[i] = s3dat_internal_read32LE(mem);
	}

	to->type = s3dat_snd;
	to->len = len;
	to->pointers = pointers;

	return S3DAT_READ_SUCCESSFUL;
}

uint32_t s3dat_extract_sound(s3dat_t* mem, uint16_t soundtype, uint32_t altindex, s3dat_sound_t* to) {
	if(mem->sound_index.len <= soundtype || soundtype < 0) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;
	if(mem->sound_index.sequences[soundtype].len <= altindex || altindex < 0) return S3DAT_ERROR_VALUE_HIGHER_THAN_MAX;

	uint32_t from = mem->sound_index.sequences[soundtype].pointers[altindex];

	mem->seek_func(mem->io_arg, from, SEEK_SET);
	uint32_t size = mem->size_func(mem->io_arg);
	uint32_t p = from;

	if(p+4>size) return S3DAT_ERROR_FILE_TOO_SHORT;

	uint32_t len = s3dat_internal_read32LE(mem)/2-16;
	p += 4;

	to->len = len;
	mem->seek_func(mem->io_arg, 8, SEEK_CUR);
	to->freq = s3dat_internal_read32LE(mem);
	mem->seek_func(mem->io_arg, 4, SEEK_CUR);
	p += 16;

	if(p+len*2>size) return S3DAT_ERROR_FILE_TOO_SHORT;

	to->data = mem->alloc_func(mem->mem_arg, len*2);
	for(uint32_t i = 0;i != len;i++) {
		to->data[i] = s3dat_internal_read16LE(mem);
	}

	return S3DAT_READ_SUCCESSFUL;
}
