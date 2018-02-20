#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "string.c"
#endif

char* s3dat_internal_read_cstr(s3dat_t* handle, s3util_exception_t** throws) {
	char* bfr = s3util_alloc_func(s3dat_memset(handle), STRING_BUFFER, throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	uint32_t bfr_size = STRING_BUFFER;
	uint32_t pos = 0;
	do {
		bfr[pos] = S3DAT_INTERNAL_READ(8, handle, throws);
		if(*throws != NULL) {
			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
			s3util_free_func(s3dat_memset(handle), bfr);
			return NULL;
		}

		if(pos+1 == bfr_size) {
			char* bfr2 = s3util_alloc_func(s3dat_memset(handle), bfr_size+STRING_BUFFER, throws);
			if(*throws != NULL) {
				s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
				s3util_free_func(s3dat_memset(handle), bfr);
				return NULL;
			}

			memcpy(bfr2, bfr, bfr_size);
			s3util_free_func(s3dat_memset(handle), bfr);
			bfr = bfr2;
			bfr_size += STRING_BUFFER;
		}
	} while(bfr[pos++] != '\0');

	return bfr;
}



void s3dat_internal_extract_string(s3dat_t* handle, uint16_t text, uint16_t language, s3dat_ref_t** to, s3util_exception_t** throws) {
	if(text > handle->string_index->len || language > handle->string_index->sequences[text].len) {
		s3util_throw(s3dat_memset(handle), throws, S3UTIL_EXCEPTION_OUT_OF_RANGE, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_seek_func(handle, handle->string_index->sequences[text].pointers[language], S3UTIL_SEEK_SET, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	char* cstr = s3dat_internal_read_cstr(handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	s3util_short(s3dat_memset(handle), &cstr);

	s3dat_ref_t* pack = s3dat_new_packed(handle, throws);
	if(*throws != NULL) {
		s3util_free_func(s3dat_memset(handle), cstr);
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return;
	}

	pack->data.pkd->len = strlen(cstr);
	pack->data.pkd->data = cstr;
	*to = pack;
}

void s3dat_utf8_encoding_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws) {
	s3dat_t* handle = me->parent;

	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	if(res->type == s3dat_string) {
		S3DAT_CHECK_TYPE(handle, res, "s3dat_string_t", throws, __FILE__, __func__, __LINE__);

		s3dat_string_t* string = res->res->data.str;

		char* utf8_str = NULL;

		char* charset;

		switch(string->language) {
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

		s3util_exception_t* tmpex = NULL;
		s3util_iconv_dat_to_utf8(s3dat_memset(handle), charset, string->string_data, &utf8_str, &tmpex);

		if(tmpex == NULL) {
			s3util_free_func(s3dat_memset(handle), string->string_data);
			string->original_encoding = false;
			s3util_short(s3dat_memset(handle), &utf8_str);
			string->string_data = utf8_str;
		} else if(tmpex->type == S3UTIL_EXCEPTION_NOICONV) {
			s3util_delete_exception(s3dat_memset(handle), tmpex);

			if(string->language != s3dat_japanese && string->language != s3dat_korean) {
				s3util_iso8859_to_utf8(s3dat_memset(handle), &string->string_data, strlen(string->string_data)+1, string->language == s3dat_polish, throws);

				string->original_encoding = false;
			}
		} else {
			*throws = tmpex;
			s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		}
	}
}

bool s3dat_utf8(s3dat_ref_t* str) {
	if(!s3dat_is_string(str))return false;
	return !str->data.str->original_encoding;
}

char* s3dat_strdata(s3dat_ref_t* str) {
	if(!s3dat_is_string(str)) return NULL;
	return str->data.str->string_data;
}
