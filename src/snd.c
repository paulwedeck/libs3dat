#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "snd.c"
#endif

uint8_t s3dat_internal_snd_header[16] = { 68, 21, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 0 };

void s3dat_internal_readsnd(s3dat_t* handle, s3util_exception_t** throws) {
	s3dat_internal_seek_func(handle, 0, S3UTIL_SEEK_SET, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint8_t header[16];
	s3dat_ioset(handle)->read_func(s3dat_ioset(handle)->arg, header, 16);

	if(memcmp(s3dat_internal_snd_header, header, 16) != 0) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_HEADER, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t size_field;
	size_field = S3DAT_INTERNAL_READ(32LE, handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	if(s3dat_ioset(handle)->size_func(s3dat_ioset(handle)->arg) != size_field) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_CONFLICTING_DATA, __FILE__, __func__, __LINE__);
	}

	s3dat_internal_seek_func(handle, 34, S3UTIL_SEEK_SET, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint16_t len = S3DAT_INTERNAL_READ(16LE, handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint32_t* pointers = s3util_alloc_func(s3dat_memset(handle), (size_t) len*4, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	for(uint16_t i = 0;i != len;i++) {
		pointers[i] = S3DAT_INTERNAL_READ(32LE, handle, throws);

		if(*throws != NULL) {
			s3util_free_func(s3dat_memset(handle), pointers);
			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
			return;
		}
	}

	s3dat_index32_t* indices = s3util_alloc_func(s3dat_memset(handle), len*sizeof(s3dat_index32_t), throws);
	if(*throws != NULL) {
		s3util_free_func(s3dat_memset(handle), pointers);
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return;
	}

	for(uint16_t i = 0;i != len;i++) {
		s3dat_internal_readsnd_index(handle, pointers[i], &indices[i], throws);
		s3util_add_attr(s3dat_memset(handle), throws, S3UTIL_ATTRIBUTE_SONG, i);

		if(*throws != NULL) {
			s3util_free_func(s3dat_memset(handle), pointers);
			s3util_free_func(s3dat_memset(handle), indices);
			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
			return;
		}
	}

	s3util_free_func(s3dat_memset(handle), pointers);

	handle->sound_index->sequences = indices;
	handle->sound_index->len = len;
	handle->sound_index->type = s3dat_snd;
}

void s3dat_internal_readsnd_index(s3dat_t* handle, uint32_t from, s3dat_index32_t* to, s3util_exception_t** throws) {
	s3dat_internal_seek_func(handle, from, S3UTIL_SEEK_SET, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint32_t len = S3DAT_INTERNAL_READ(32LE, handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	uint32_t* pointers = s3util_alloc_func(s3dat_memset(handle), len*4, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	for(uint32_t i = 0;i != len;i++) {
		pointers[i] = S3DAT_INTERNAL_READ(32LE, handle, throws);

		if(*throws != NULL) {
			s3util_free_func(s3dat_memset(handle), pointers);
			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		}
	}

	to->type = s3dat_snd;
	to->len = len;
	to->pointers = pointers;
}

void s3dat_pack_sound(s3dat_t* handle, s3dat_sound_t* sound, s3dat_packed_t* packed, s3util_exception_t** throws) {
	packed->data = s3util_alloc_func(s3dat_memset(handle), (size_t) (sound->len*2)+16, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	packed->len = (sound->len*2)+16;

	uint32_t* ptr32 = packed->data;
	ptr32[0] = s3util_le32(packed->len);
	ptr32[1] = s3util_le32(0x1010);
	ptr32[2] = s3util_le32(sound->freq / 2);
	ptr32[3] = s3util_le32(sound->freq);
	uint16_t* ptr16 = packed->data+16;

	for(uint32_t i = 0;i != sound->len;i++) {
		ptr16[i] = s3util_le16(sound->data[i]);
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
	return (uint16_t*) snd->data.snd->data;
}

