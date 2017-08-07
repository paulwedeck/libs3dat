#include "s3dat_internal.h"
#line __LINE__ "snd.c"


uint8_t s3dat_internal_snd_header[16] = { 68, 21, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 0 };

void s3dat_internal_readsnd(s3dat_t* mem, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, 0, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint8_t header[16];
	mem->read_func(mem->io_arg, header, 16);

	if(memcmp(s3dat_internal_snd_header, header, 16) != 0)  {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t size_field;
	size_field = s3dat_internal_read32LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	if(mem->size_func(mem->io_arg) != size_field) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
	}

	s3dat_internal_seek_func(mem, 34, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint16_t len = s3dat_internal_read16LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t* pointers = mem->alloc_func(mem->mem_arg, len*4);
	for(uint16_t i = 0;i != len;i++) {
		pointers[i] = s3dat_internal_read32LE(mem, throws);

		if(*throws != NULL) {
			mem->free_func(mem->mem_arg, pointers);
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			return;
		}
	}

	s3dat_index32_t* indices_data = mem->alloc_func(mem->mem_arg, len*sizeof(s3dat_index32_t));
	uint16_t alive_len = 0;

	for(uint16_t i = 0;i != len;i++) {
		s3dat_internal_readsnd_index(mem, pointers[i], indices_data+alive_len, throws);

		if(*throws != NULL) {
			mem->free_func(mem->mem_arg, pointers);
			mem->free_func(mem->mem_arg, indices_data);
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
			return;
		} else {
			alive_len++;
		}
	}

	mem->free_func(mem->mem_arg, pointers);

	mem->sound_index.type = s3dat_snd;
	if(alive_len != len) {
		s3dat_index32_t* alive_data = mem->alloc_func(mem->mem_arg, alive_len*sizeof(s3dat_index32_t));
		memcpy(alive_data, indices_data, alive_len*sizeof(s3dat_index32_t));
		mem->free_func(mem->mem_arg, indices_data);
		mem->sound_index.sequences = alive_data;
		mem->sound_index.len = alive_len;
	} else {
		mem->sound_index.sequences = indices_data;
		mem->sound_index.len = len;
	}
}

void s3dat_internal_readsnd_index(s3dat_t* mem, uint32_t from, s3dat_index32_t* to, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(mem, from, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t len = s3dat_internal_read32LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t* pointers = mem->alloc_func(mem->mem_arg, len*4);
	for(uint32_t i = 0;i != len;i++) {
		pointers[i] = s3dat_internal_read32LE(mem, throws);

		if(*throws != NULL) {
			mem->free_func(mem->mem_arg, pointers);
			s3dat_add_to_stack(mem, throws, __FILE__, __func__, __LINE__);
		}
	}

	to->type = s3dat_snd;
	to->len = len;
	to->pointers = pointers;
}

void s3dat_extract_sound(s3dat_t* mem, uint16_t soundtype, uint32_t altindex, s3dat_sound_t* to, s3dat_exception_t** throws) {
	if(mem->sound_index.len <= soundtype || mem->sound_index.sequences[soundtype].len <= altindex) {
		s3dat_internal_throw(mem, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t from = mem->sound_index.sequences[soundtype].pointers[altindex];

	s3dat_internal_seek_func(mem, from, S3DAT_SEEK_SET, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	uint32_t len = s3dat_internal_read32LE(mem, throws)/2-16;
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->len = len;
	s3dat_internal_seek_func(mem, 8, S3DAT_SEEK_CUR, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->freq = s3dat_internal_read32LE(mem, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_seek_func(mem, 4, S3DAT_SEEK_CUR, throws);
	S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);

	to->data = mem->alloc_func(mem->mem_arg, len*2);
	for(uint32_t i = 0;i != len;i++) {
		to->data[i] = s3dat_internal_read16LE(mem, throws);
		S3DAT_INTERNAL_HANDLE_EXCEPTION(mem, throws, __FILE__, __func__, __LINE__);
	}
}
