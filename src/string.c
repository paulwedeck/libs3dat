#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "string.c"
#endif

uint8_t* s3dat_internal_read_cstr(s3dat_t* handle, s3dat_exception_t** throws) {
	#define STRING_BUFFER 1024

	uint8_t* bfr = s3dat_internal_alloc_func(handle, STRING_BUFFER, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	uint32_t bfr_size = STRING_BUFFER;
	uint32_t pos = 0;
	do {
		bfr[pos] = s3dat_internal_read8(handle, throws);
		if(*throws != NULL) {
			s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
			handle->free_func(handle->mem_arg, bfr);
			return NULL;
		}

		if(pos+1 == bfr_size) {
			uint8_t* bfr2 = s3dat_internal_alloc_func(handle, bfr_size+STRING_BUFFER, throws);
			if(*throws != NULL) {
				s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
				handle->free_func(handle->mem_arg, bfr);
				return NULL;
			}

			memcpy(bfr2, bfr, bfr_size);
			handle->free_func(handle->mem_arg, bfr);
			bfr = bfr2;
			bfr_size += STRING_BUFFER;
		}
	} while(bfr[pos++] != '\0');

	return bfr;
}

void s3dat_internal_short(s3dat_t* handle, uint8_t** str) {
	uint8_t* bfr = *str;

	uint8_t* bfr2 = handle->alloc_func(handle, strlen(bfr)+1);
	if(bfr2 == NULL) return;

	strcpy(bfr2, bfr);
	handle->free_func(handle->mem_arg, bfr);

	*str = bfr2;
}

uint16_t s3dat_internal_iso8859_2_to_utf8_map[96] = {0xA0, 0x104, 0x2D8, 0x141, 0xA4, 0x13D, 0x15A, 0xA7, 0xA8, 0x160, 0x15E, 0x164, 0x179, 0xAD, 0x17D, 0x17B, 0xB0, 0x105, 0x2DB, 0x142, 0xB4, 0x13E, 0x15B, 0x2C7, 0xB8, 0x161, 0x15F, 0x165, 0x17A, 0x2DD, 0x17E, 0x17C, 0x154, 0xC1, 0xC2, 0x102, 0xC4, 0x139, 0x106, 0xC7, 0x10C, 0xC9, 0x118, 0xCB, 0x11A, 0xCD, 0xCE, 0x10E, 0x110, 0x143, 0x147, 0xD3, 0xD4, 0x150, 0xD6, 0xD7, 0x158, 0x16E, 0xDA, 0x170, 0xDC, 0xDD, 0x162, 0xDF, 0x155, 0xE1, 0xE2, 0x103, 0xE4, 0x13A, 0x107, 0xE7, 0x10D, 0xE9, 0x119, 0xEB, 0x11B, 0xED, 0xEE, 0x10F, 0x111, 0x144, 0x148, 0xF3, 0xF4, 0x151, 0xF6, 0xF7, 0x159, 0x16F, 0xFA, 0x171, 0xFC, 0xFD, 0x163, 0x2D9};

void s3dat_internal_iso8859_to_utf8(s3dat_t* handle, uint8_t** str, uint32_t len, bool iso8859_2, s3dat_exception_t** throws) {

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

	uint8_t* bfr2 = s3dat_internal_alloc_func(handle, real_len, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

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

	handle->free_func(handle->mem_arg, bfr);
	bfr2[real_len-1] = '\0';

	*str = bfr2;
}

#ifdef USE_ICONV
void s3dat_internal_iconv_dat_to_utf8(s3dat_t* handle, s3dat_language language, uint8_t* cstr, uint8_t** utf8_str, s3dat_exception_t** throws) {
	char* charset;

	switch(language) {
		case s3dat_polish:
			charset = "iso8859-2";
		break;
		case s3dat_korean:
			charset = "EUC-KR";
		break;
		case s3dat_japanese:
			charset = "SHIFT_JIS";
		break;
		default:
			charset = "iso8859-1";
		break;
	}
	iconv_t iconv_s = iconv_open("UTF8", charset);

	if(iconv_s == (iconv_t)-1) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_ICONV_ERROR, __FILE__, __func__, __LINE__);
		return;
	}

	size_t inlen = strlen(cstr);
	size_t outlen = inlen*4+4;
	uint8_t* utf8s = s3dat_internal_alloc_func(handle, outlen, throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	*utf8_str = utf8s;
	uint8_t* instr = cstr;

	if(iconv(iconv_s, (char**)&instr, &inlen, (char**)&utf8s, &outlen) == (size_t)-1) {
		handle->free_func(handle->mem_arg, *utf8_str);
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_ICONV_ERROR, __FILE__, __func__, __LINE__);
	}
	iconv_close(iconv_s);
}
#endif

void s3dat_internal_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, void** to, s3dat_exception_t** throws) {
	if(text > handle->string_index->len || language > handle->string_index->sequences[text].len) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(handle, handle->string_index->sequences[text].pointers[language], S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	uint8_t* cstr = s3dat_internal_read_cstr(handle, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_short(handle, &cstr);

	s3dat_packed_t* pack = s3dat_internal_alloc_func(handle, sizeof(s3dat_packed_t), throws);
	if(*throws != NULL) {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		return;
	}

	pack->len = strlen(cstr);
	pack->data = cstr;
	*to = pack;
}

void s3dat_utf8_encoding_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;

	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	if(res->type == s3dat_string) {
		s3dat_string_t* string = res->resdata;

		#ifdef USE_ICONV
		uint8_t* utf8_str = NULL;
		s3dat_internal_iconv_dat_to_utf8(handle, string->language, string->string_data, &utf8_str, throws);

		if(*throws == NULL) {
			handle->free_func(handle->mem_arg, string->string_data);
			string->original_encoding = false;
			s3dat_internal_short(handle, &utf8_str);
			string->string_data = utf8_str;
		}
		#else
		if(string->language != s3dat_japanese && string->language != s3dat_korean) {
			s3dat_internal_iso8859_to_utf8(handle, &(string->string_data), strlen(string->string_data)+1, string->language == s3dat_polish, throws);

			string->original_encoding = false;
		}
		#endif
	}
}

