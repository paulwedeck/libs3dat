#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "write.c"
#endif

extern uint8_t s3dat_header_start_part1[33];

extern uint8_t s3dat_header_start_part2[10];

extern uint8_t s3dat_header_rgb5[5];
extern uint8_t s3dat_header_rgb565[5];

extern uint8_t s3dat_seq_start[7];

void s3dat_internal_write_seq_index(s3dat_t* handle, uint32_t to, uint8_t len, uint32_t* positions, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(handle, to, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(!handle->write_func(handle->io_arg, s3dat_seq_start, 7)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	s3dat_internal_write8(handle, len, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	for(uint8_t i = 0;i != len;i++) {
		s3dat_internal_write32LE(handle, positions != NULL ? positions[i]-to : 0, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_write_empty_seq_pos(s3dat_t* handle, uint32_t* positions, s3dat_seq_index_t* index, s3dat_exception_t** throws) {
	for(uint16_t i = 0;i != index->len;i++) {
		s3dat_internal_write_seq_index(handle, positions[i], index->sequences[i].len,  NULL, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_write_string_index(s3dat_t* handle, uint32_t to, uint32_t* positions, s3dat_exception_t** throws) {
	s3dat_internal_seek_func(handle, to, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write32LE(handle, s3dat_string, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write32LE(handle, handle->string_index->len != 0 ? handle->string_index->len*handle->string_index->sequences[0].len*4+12 : 12, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write16LE(handle, handle->string_index->len, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(handle->string_index->len != 0) {
		s3dat_internal_write16LE(handle, handle->string_index->sequences[0].len, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		for(uint16_t i = 0;i != handle->string_index->len*handle->string_index->sequences[0].len;i++) {
			s3dat_internal_write32LE(handle, positions != NULL ? positions[i] : 0, throws);
			S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
		}
	} else {
		s3dat_internal_write16LE(handle, 0, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_write_index(s3dat_t* handle, uint32_t to, s3dat_content_type type, uint16_t len, uint32_t* positions, s3dat_exception_t** throws) {
	if(type == s3dat_string) return;

	s3dat_internal_seek_func(handle, to, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write32LE(handle, type, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write16LE(handle, len*4+(type == s3dat_palette ? 12 : 8), throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write16LE(handle, len, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	if(type == s3dat_palette) {
		s3dat_internal_write32LE(handle, handle->palette_line_length, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}

	for(uint16_t i = 0;i != len;i++) {
		s3dat_internal_write32LE(handle, positions != NULL ? positions[i] : 0, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_gen_seq_pos(s3dat_seq_index_t* seq, uint32_t* start_pos, uint32_t* pos) {
	for(uint16_t i = 0;i != seq->len;i++) {
		pos[i] = *start_pos;
		*start_pos += seq->sequences[i].len*4+8;
	}
}

void s3dat_write_packed(s3dat_t* handle, s3dat_res_t* res, uint32_t* pos, uint32_t* written_to, s3dat_exception_t** throws) {
	s3dat_extract(handle, res, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	S3DAT_CHECK_TYPE(handle, res, "s3dat_packed_t", throws, __FILE__, __func__, __LINE__);

	if(*pos % 2 == 1) {
		s3dat_internal_write8(handle, 0, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		*pos += 1;
	}

	*written_to = *pos;

	if(!handle->write_func(handle->io_arg, res->res->data.pkd->data, res->res->data.pkd->len)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
	}

	*pos += res->res->data.pkd->len;

	s3dat_unref(res->res);
}

void s3dat_internal_export_seq_index(s3dat_t* handle, s3dat_seq_index_t* index, uint32_t* seq_pos, uint32_t pos, uint32_t* append_pos, s3dat_exception_t** throws) {
	s3dat_res_t res = {0, 0, index->type, NULL};

	for(uint16_t s = 0;s != index->len;s++) {
		res.first_index = s;

		uint32_t positions[index->sequences[s].len];

		for(uint8_t i = 0;i != index->sequences[s].len;i++) {
			res.second_index = i;

			s3dat_write_packed(handle, &res, append_pos, positions+i, throws);
			S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
		}

		s3dat_internal_write_seq_index(handle, seq_pos[s], index->sequences[s].len, positions, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

		s3dat_internal_seek_func(handle, *append_pos, S3DAT_SEEK_SET, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}
}

void s3dat_internal_export_index(s3dat_t* handle, s3dat_index_t* index, uint32_t pos, uint32_t* append_pos, s3dat_exception_t** throws) {
	uint32_t data_positions[index->len];

	s3dat_res_t res = {0, 0, index->type, NULL};

	for(uint16_t i = 0;i != index->len;i++) {
		res.first_index = i;

		s3dat_write_packed(handle, &res, append_pos, data_positions+i, throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}

	s3dat_internal_write_index(handle, pos, index->type, index->len, data_positions, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_seek_func(handle, *append_pos, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
}

void s3dat_writefile(s3dat_t* handle, s3dat_exception_t** throws) {
	if(handle->open_func != NULL) handle->io_arg = handle->open_func(handle->io_arg, true);

	if(handle->io_arg == NULL) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_OPEN, __FILE__, __func__, __LINE__);
		return;
	}

	if(!handle->write_func(handle->io_arg, s3dat_header_start_part1, 33)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(!handle->write_func(handle->io_arg, handle->green_6b ? s3dat_header_rgb565 : s3dat_header_rgb5, 5)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	if(!handle->write_func(handle->io_arg, s3dat_header_start_part2, 10)) {
		s3dat_throw(handle, throws, S3DAT_EXCEPTION_IOERROR, __FILE__, __func__, __LINE__);
		return;
	}

	uint32_t string_pos = 84;
	uint32_t landscape_pos;
	if(handle->string_index->len == 0) {
		landscape_pos = 96;
	} else {
		landscape_pos = string_pos + handle->string_index->len*handle->string_index->sequences[0].len*4+12;
	}
	uint32_t gui_pos = landscape_pos + handle->landscape_index->len*4+8;
	uint32_t settler_pos = gui_pos + handle->gui_index->len*4+8;
	uint32_t torso_pos = settler_pos + handle->settler_index->len*4+8;
	uint32_t shadow_pos = torso_pos + handle->torso_index->len*4+8;
	uint32_t animation_pos = shadow_pos + handle->shadow_index->len*4+8;
	uint32_t palette_pos = animation_pos + handle->animation_index->len*4+8;

	uint32_t meta_gap[9] = {0, string_pos, landscape_pos, gui_pos, settler_pos, torso_pos, shadow_pos, animation_pos, palette_pos};

	for(int i = 0;i != 9;i++) {
		s3dat_internal_write32LE(handle, meta_gap[i], throws);
		S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
	}

	uint32_t append_pos = palette_pos+handle->palette_index->len*4+12;

	uint32_t settler_seq_pos[handle->settler_index->len];
	uint32_t torso_seq_pos[handle->torso_index->len];
	uint32_t shadow_seq_pos[handle->shadow_index->len];

	s3dat_internal_gen_seq_pos(handle->settler_index, &append_pos, settler_seq_pos);
	s3dat_internal_gen_seq_pos(handle->torso_index, &append_pos, torso_seq_pos);
	s3dat_internal_gen_seq_pos(handle->shadow_index, &append_pos, shadow_seq_pos);

	s3dat_internal_write_string_index(handle, string_pos, NULL, throws);
	s3dat_internal_write_index(handle, landscape_pos, s3dat_landscape, handle->landscape_index->len, NULL, throws);
	s3dat_internal_write_index(handle, gui_pos, s3dat_gui, handle->gui_index->len, NULL, throws);

	s3dat_internal_write_index(handle, settler_pos, s3dat_settler, handle->settler_index->len, settler_seq_pos, throws);
	s3dat_internal_write_index(handle, torso_pos, s3dat_torso, handle->torso_index->len, torso_seq_pos, throws);
	s3dat_internal_write_index(handle, shadow_pos, s3dat_shadow, handle->shadow_index->len, shadow_seq_pos, throws);

	s3dat_internal_write_index(handle, animation_pos, s3dat_animation, handle->animation_index->len, NULL, throws);
	s3dat_internal_write_index(handle, palette_pos, s3dat_palette, handle->palette_index->len, NULL, throws);

	s3dat_internal_write_empty_seq_pos(handle, settler_seq_pos, handle->settler_index, throws);
	s3dat_internal_write_empty_seq_pos(handle, torso_seq_pos, handle->torso_index, throws);
	s3dat_internal_write_empty_seq_pos(handle, shadow_seq_pos, handle->shadow_index, throws);

	if(handle->string_index->len > 0) {
		uint32_t written_string_pos[handle->string_index->len*handle->string_index->sequences[0].len];
		for(uint16_t l = 0;l != handle->string_index->sequences[0].len;l++) {
			for(uint16_t t = 0;t != handle->string_index->len;t++) {
				s3dat_res_t res = {t, l, s3dat_string, NULL};

				s3dat_write_packed(handle, &res, &append_pos, written_string_pos+l*handle->string_index->len+t, throws);
				S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
			}
		}

		s3dat_internal_write_string_index(handle, string_pos, written_string_pos, throws);
	}

	s3dat_internal_seek_func(handle, append_pos, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_index(handle, handle->landscape_index, landscape_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_index(handle, handle->gui_index, gui_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_seq_index(handle, handle->settler_index, settler_seq_pos, settler_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_seq_index(handle, handle->torso_index, torso_seq_pos, torso_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_seq_index(handle, handle->shadow_index, shadow_seq_pos, shadow_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_index(handle, handle->animation_index, animation_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_export_index(handle, handle->palette_index, palette_pos, &append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_seek_func(handle, 48, S3DAT_SEEK_SET, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	s3dat_internal_write32LE(handle, append_pos, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);
}


void s3dat_pack_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);


	s3dat_packed_t* package = s3dat_alloc_func(handle, sizeof(s3dat_packed_t), throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	package->parent = handle;

	if(res->type == s3dat_snd) {
		s3dat_pack_sound(handle, res->res->data.raw, package, throws);
		s3dat_delete_sound(res->res->data.raw);

	} else if(res->type == s3dat_animation) {
		s3dat_pack_animation(handle, res->res->data.raw, package, throws);
		s3dat_delete_animation(res->res->data.raw);

	} else if(res->type == s3dat_palette) {
		s3dat_pack_palette(handle, res->res->data.raw, package, throws);
		s3dat_delete_bitmap(res->res->data.raw);

	} else if(res->type == s3dat_string) {
		s3dat_pack_string(handle, res->res->data.raw, package, throws);
		s3dat_delete_string(res->res->data.raw);

	} else {
		s3dat_pack_bitmap(handle, res->res->data.raw, res->type, package, throws);
		s3dat_delete_bitmap(res->res->data.raw);
	}

	if(*throws == NULL) {
		res->res->data.raw = package;
		res->res->type = s3dat_internal_get_restype(s3dat_pkd_ref);
	} else {
		s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
		s3dat_free_func(handle, package);
		res->res->data.raw = NULL;
		s3dat_free_func(handle, res->res);
		res->res = NULL;
	}
}

void s3dat_pack_palette(s3dat_t* handle, s3dat_bitmap_t* palette, s3dat_packed_t* packed, s3dat_exception_t** throws) {
	uint32_t pixel_count = palette->width*palette->height;

	packed->data = s3dat_alloc_func(handle, pixel_count*2, throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	packed->len = pixel_count*2;

	uint16_t* ptr16 = packed->data;
	for(uint32_t i = 0;i != pixel_count;i++) {
		s3dat_internal_8b_to_native(palette->data+i, ptr16+i, palette->type);
		ptr16[i] = s3dat_le16(ptr16[i]);
	}
}

void s3dat_pack_animation(s3dat_t* handle, s3dat_animation_t* animation, s3dat_packed_t* packed, s3dat_exception_t** throws) {
	packed->data = s3dat_alloc_func(handle, animation->len*sizeof(s3dat_frame_t), throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	packed->len = animation->len*sizeof(s3dat_frame_t);

	uint16_t* ptr16 = packed->data;
	for(uint32_t i = 0;i != animation->len;i++) {
		*(ptr16++) = s3dat_le16(animation->frames[i].posx);
		*(ptr16++) = s3dat_le16(animation->frames[i].posy);
		*(ptr16++) = s3dat_le16(animation->frames[i].settler_id);
		*(ptr16++) = s3dat_le16(animation->frames[i].settler_file);
		*(ptr16++) = s3dat_le16(animation->frames[i].torso_id);
		*(ptr16++) = s3dat_le16(animation->frames[i].torso_file);
		*(ptr16++) = s3dat_le16(animation->frames[i].shadow_id);
		*(ptr16++) = s3dat_le16(animation->frames[i].shadow_file);
		*(ptr16++) = s3dat_le16(animation->frames[i].settler_frame);
		*(ptr16++) = s3dat_le16(animation->frames[i].torso_frame);
		*(ptr16++) = s3dat_le16(animation->frames[i].flag1);
		*(ptr16++) = s3dat_le16(animation->frames[i].flag2);
	}
}

void s3dat_pack_string(s3dat_t* handle, s3dat_string_t* string, s3dat_packed_t* packed, s3dat_exception_t** throws) {
	packed->data = s3dat_alloc_func(handle, strlen(string->string_data), throws);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	packed->len = strlen(string->string_data);
	strcpy(packed->data, string->string_data);
}

