#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "snd.c"
#endif

uint8_t s3dat_internal_snd_header[16] = { 68, 21, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 0 };

void s3dat_internal_readsnd(s3dat_t* handle, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(handle, 0, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint8_t header[16];
	handle->read_func(handle->io_arg, header, 16);

	if(memcmp(s3dat_internal_snd_header, header, 16) != 0) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t size_field;
	size_field = s3dat_internal_read32LE(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(handle->size_func(handle->io_arg) != size_field) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
	}

	s3dat_internal_seek_func(handle, 34, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint16_t len = s3dat_internal_read16LE(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint32_t* pointers = s3dat_alloc_func(handle, len*4, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	for(uint16_t i = 0;i != len;i++) {
		pointers[i] = s3dat_internal_read32LE(handle, throws);

		if(*throws != NULL) {
			handle->free_func(handle->mem_arg, pointers);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		}
	}

	s3dat_index32_t* indices_data = s3dat_alloc_func(handle, len*sizeof(s3dat_index32_t), throws);
	if(*throws != NULL) {
		handle->free_func(handle->mem_arg, pointers);
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	uint16_t alive_len = 0;

	for(uint16_t i = 0;i != len;i++) {
		s3dat_internal_readsnd_index(handle, pointers[i], indices_data+alive_len, throws);
		S3DAT_INTERNAL_ADD_ATTR(handle, throws, S3DAT_ATTRIBUTE_SONG, i);

		if(*throws != NULL) {
			handle->free_func(handle->mem_arg, pointers);
			handle->free_func(handle->mem_arg, indices_data);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			return;
		} else {
			alive_len++;
		}
	}

	handle->free_func(handle->mem_arg, pointers);

	handle->sound_index->type = s3dat_snd;
	if(alive_len != len) {
		s3dat_index32_t* alive_data = s3dat_alloc_func(handle, alive_len*sizeof(s3dat_index32_t), throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		} else {
			memcpy(alive_data, indices_data, alive_len*sizeof(s3dat_index32_t));
			handle->sound_index->sequences = alive_data;
			handle->sound_index->len = alive_len;
		}
		handle->free_func(handle->mem_arg, indices_data);
	} else {
		handle->sound_index->sequences = indices_data;
		handle->sound_index->len = len;
	}
}

void s3dat_internal_readsnd_index(s3dat_t* handle, uint32_t from, s3dat_index32_t* to, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(handle, from, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint32_t len = s3dat_internal_read32LE(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint32_t* pointers = s3dat_alloc_func(handle, len*4, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	for(uint32_t i = 0;i != len;i++) {
		pointers[i] = s3dat_internal_read32LE(handle, throws);

		if(*throws != NULL) {
			handle->free_func(handle->mem_arg, pointers);
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		}
	}

	to->type = s3dat_snd;
	to->len = len;
	to->pointers = pointers;
}

void s3dat_pack_sound(s3dat_t* handle, s3dat_sound_t* sound, s3dat_packed_t* packed, s3dat_exception_t** throws) {
	packed->data = handle->alloc_func(handle->mem_arg, (sound->len*2)+16);
	packed->len = (sound->len*2)+16;

	uint32_t* ptr32 = packed->data;
	ptr32[0] = le32(packed->len);
	ptr32[1] = le32(0x1010);
	ptr32[2] = le32(sound->freq/2);
	ptr32[3] = le32(sound->freq);
	uint16_t* ptr16 = packed->data+16;

	for(uint32_t i = 0;i != sound->len;i++) {
		ptr16[i] = le16(sound->data[i]);
	}
}

uint32_t s3dat_freq(s3dat_ref_t* snd) {
	if(!s3dat_is_sound(snd)) return 0;
	return snd->data.snd->freq;
}

uint16_t s3dat_samples(s3dat_ref_t* snd) {
	if(!s3dat_is_sound(snd)) return 0;
	return snd->data.snd->len;
}
uint16_t* s3dat_snddata(s3dat_ref_t* snd) {
	if(!s3dat_is_sound(snd)) return NULL;
	return snd->data.snd->data;
}

