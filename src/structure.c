#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "structure.c"
#endif

//exhandler
s3dat_extracthandler_t* s3dat_new_exhandler(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_extracthandler_t* exhandler = s3dat_new_exhandlers(parent, 1, throws);
	s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
	return exhandler;
}

s3dat_extracthandler_t* s3dat_new_exhandlers(s3dat_t* parent, uint32_t count, s3util_exception_t** throws) {
	s3dat_extracthandler_t* handlers = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_extracthandler_t)*count, throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	for(uint32_t i = 0;i != count;i++) handlers[i].parent = parent;
	return handlers;
}

void s3dat_delete_exhandler(s3dat_extracthandler_t* exhandler) {
	s3dat_delete_exhandlers(exhandler, 1);
}

void s3dat_delete_exhandlers(s3dat_extracthandler_t* exhandlers, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) if(exhandlers[i].arg_deref != NULL) exhandlers[i].arg_deref(exhandlers[i].arg);
	s3util_free_func(s3dat_memset(exhandlers->parent), exhandlers);
}

void s3dat_delete_packed(s3dat_packed_t* package) {
	if(package->data) s3util_free_func(s3dat_memset(package->parent), package->data);
	s3util_free_func(s3dat_memset(package->parent), package);
}

s3dat_ref_t* s3dat_new_ref(s3dat_t* parent, s3dat_restype_t* type, s3util_exception_t** throws) {
	s3dat_ref_t* ref = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_ref_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	ref->src = parent;
	ref->refs = 1;
	ref->data.raw = type->alloc(parent, throws);
	if(*throws != NULL) {
		s3util_free_func(s3dat_memset(parent), ref);
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	ref->type = type;
	return ref;
}

s3dat_ref_t* s3dat_new_string(s3dat_t* parent, uint32_t strlen, s3util_exception_t** throws) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_str_ref), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	ref->data.str->original_encoding = true;
	ref->data.str->language = s3dat_unknown_language;
	ref->data.str->string_data = s3util_alloc_func(s3dat_memset(parent), strlen, throws);
	if(*throws != NULL) {
		s3dat_unref(ref);
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}
	return ref;
}

s3dat_string_t* s3dat_new_raw_string(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_string_t* string = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_string_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}
	string->src = parent;
	return string;
}

s3dat_ref_t* s3dat_new_sound(s3dat_t* parent, uint32_t freq, uint16_t len, s3util_exception_t** throws) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_snd_ref), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	ref->data.snd->freq = freq;
	ref->data.snd->len = len;
	ref->data.snd->data = s3util_alloc_func(s3dat_memset(parent), 2*len, throws);
	if(*throws != NULL) {
		s3dat_unref(ref);
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}
	return ref;
}

s3dat_sound_t* s3dat_new_raw_sound(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_sound_t* sound = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_sound_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	sound->src = parent;
	return sound;
}

s3dat_ref_t* s3dat_new_animation(s3dat_t* parent, uint32_t frames, s3util_exception_t** throws) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_ani_ref), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	ref->data.ani->len = frames;
	ref->data.ani->frames = s3util_alloc_func(s3dat_memset(parent), frames*sizeof(s3dat_frame_t), throws);
	if(*throws != NULL) {
		s3dat_unref(ref);
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	return ref;
}

s3dat_animation_t* s3dat_new_raw_animation(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_animation_t* animation = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_animation_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	animation->src = parent;
	return animation;
}

s3dat_ref_t* s3dat_new_bitmap(s3dat_t* parent, uint16_t width, uint16_t height, s3util_exception_t** throws) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_bmp_ref), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	ref->data.bmp->width = width;
	ref->data.bmp->height = height;
	ref->data.bmp->type = s3util_unknown_color;
	ref->data.bmp->data = s3util_alloc_func(s3dat_memset(parent), width*height*sizeof(s3util_color_t), throws);
	if(*throws != NULL) {
		s3dat_unref(ref);
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	return ref;
}

s3dat_bitmap_t* s3dat_new_raw_bitmap(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_bitmap_t* bitmap = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_bitmap_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	bitmap->src = parent;
	return bitmap;
}

s3dat_ref_t* s3dat_new_packed(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_pkd_ref), throws);
	s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
	return ref;
}

s3dat_packed_t* s3dat_new_raw_packed(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_packed_t* packed = s3util_alloc_func(s3dat_memset(parent), sizeof(s3dat_packed_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	packed->parent = parent;
	return packed;
}

void s3dat_ref(s3dat_ref_t* ref) {
	s3dat_ref_array(&ref, 1);
}

void s3dat_unref(s3dat_ref_t* ref) {
	s3dat_unref_array(&ref, 1);
}

void s3dat_delete_ref(s3dat_ref_t* ref) {
	s3dat_delete_ref_array(&ref, 1);
}

void s3dat_delete_string(s3dat_string_t* string) {
	s3util_free_func(s3dat_memset(string->src), string->string_data);
	s3util_free_func(s3dat_memset(string->src), string);
}

void s3dat_delete_sound(s3dat_sound_t* sound) {
	s3util_free_func(s3dat_memset(sound->src), sound->data);
	s3util_free_func(s3dat_memset(sound->src), sound);
}

void s3dat_delete_animation(s3dat_animation_t* ani) {
	s3util_free_func(s3dat_memset(ani->src), ani->frames);
	s3util_free_func(s3dat_memset(ani->src), ani);
}

void s3dat_delete_bitmap(s3dat_bitmap_t* bmp) {
	s3util_free_func(s3dat_memset(bmp->src), bmp->data);
	s3util_free_func(s3dat_memset(bmp->src), bmp);
}

void s3dat_ref_array(s3dat_ref_t** refs, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) refs[i]->refs++;
}

void s3dat_unref_array(s3dat_ref_t** refs, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		refs[i]->refs--;
		if(refs[i]->refs == 0) s3dat_delete_ref(refs[i]);
	}
}

void s3dat_delete_ref_array(s3dat_ref_t** refs, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		refs[i]->type->deref(refs[i]->data.raw);
		s3util_free_func(s3dat_memset(refs[i]->src), refs[i]);
	}
}


// s3dat_t
s3dat_t* s3dat_new_malloc() {
	return s3dat_new_func(0, s3util_default_alloc_func, s3util_default_free_func);
}

s3dat_t* s3dat_new_malloc_monitor(void* arg, s3util_ioset_t* ioset, bool open) {
	void* ioarg = open ? ioset->open_func(arg, arg) : arg;
	if(!ioarg) return NULL;

	s3util_monitor_t* monitor = s3util_default_alloc_func(0, sizeof(s3util_monitor_t));
	monitor->io_arg = ioarg;
	monitor->ioset = ioset;
	monitor->close = open;

	monitor->last_state = 0;
	monitor->mem_arg = 0;
	monitor->alloc_func = s3util_default_alloc_func;
	monitor->free_func = s3util_default_free_func;

	s3util_monitor_print(monitor);

	return s3dat_new_func(monitor, s3util_monitor_alloc_func, s3util_monitor_free_func);
}


s3dat_t* s3dat_new_func(void* arg, void* (*alloc_func) (void*, size_t), void (*free_func) (void*, void*)) {
	s3dat_t* handle = alloc_func(arg, sizeof(s3dat_t));
	handle->memset.arg = arg;
	handle->memset.alloc_func = alloc_func;
	handle->memset.free_func = free_func;

	handle->settler_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	handle->shadow_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	handle->torso_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	handle->string_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	handle->sound_index = alloc_func(arg, sizeof(s3dat_seq_index32_t));

	handle->landscape_index = alloc_func(arg, sizeof(s3dat_index_t));
	handle->gui_index = alloc_func(arg, sizeof(s3dat_index_t));
	handle->animation_index = alloc_func(arg, sizeof(s3dat_index_t));
	handle->palette_index = alloc_func(arg, sizeof(s3dat_index_t));

	return handle;
}

void s3dat_delete(s3dat_t* handle) {
	while(handle->last_handler != NULL) {
		s3dat_extracthandler_t* tmp = handle->last_handler;
		handle->last_handler = handle->last_handler->before;
		s3dat_delete_exhandler(tmp);
	}

	s3dat_internal_delete_seq(handle, handle->settler_index);
	s3dat_internal_delete_seq(handle, handle->shadow_index);
	s3dat_internal_delete_seq(handle, handle->torso_index);
	s3dat_internal_delete_seq(handle, handle->string_index);
	s3dat_internal_delete_seq32(handle, handle->sound_index);

	s3dat_internal_delete_index(handle, handle->landscape_index);
	s3dat_internal_delete_index(handle, handle->animation_index);
	s3dat_internal_delete_index(handle, handle->palette_index);
	s3dat_internal_delete_index(handle, handle->gui_index);

	s3util_free_func(s3dat_memset(handle), handle->settler_index);
	s3util_free_func(s3dat_memset(handle), handle->shadow_index);
	s3util_free_func(s3dat_memset(handle), handle->torso_index);
	s3util_free_func(s3dat_memset(handle), handle->string_index);
	s3util_free_func(s3dat_memset(handle), handle->sound_index);

	s3util_free_func(s3dat_memset(handle), handle->landscape_index);
	s3util_free_func(s3dat_memset(handle), handle->animation_index);
	s3util_free_func(s3dat_memset(handle), handle->palette_index);
	s3util_free_func(s3dat_memset(handle), handle->gui_index);

	s3dat_delete_fork(handle);
}


void s3dat_internal_delete_index(s3dat_t* handle, s3dat_index_t* index) {
	s3dat_internal_delete_indices(handle, index, 1);
}

void s3dat_internal_delete_indices(s3dat_t* handle, s3dat_index_t* indices, uint32_t count) {
	if(count == 0) return;
	for(uint32_t i = 0;i != count;i++) {
		if(indices[i].type != 0) s3util_free_func(s3dat_memset(handle), indices[i].pointers);
	}
}

void s3dat_internal_delete_index32(s3dat_t* handle, s3dat_index32_t* index) {
	if(index->type == 0) return;

	s3util_free_func(s3dat_memset(handle), index->pointers);
}


void s3dat_internal_delete_seq(s3dat_t* handle, s3dat_seq_index_t* seq) {
	if(seq->type == 0) return;

	for(uint16_t i = 0;i != seq->len;i++) {
		s3dat_internal_delete_index(handle, seq->sequences+i);
	}
	s3util_free_func(s3dat_memset(handle), seq->sequences);
}

void s3dat_internal_delete_seq32(s3dat_t* handle, s3dat_seq_index32_t* seq) {
	if(seq->type == 0) return;

	for(uint16_t i = 0;i != seq->len;i++) {
		s3dat_internal_delete_index32(handle, seq->sequences+i);
	}
	s3util_free_func(s3dat_memset(handle), seq->sequences);
}

s3dat_t* s3dat_fork(s3dat_t* handle, s3util_exception_t** throws) {
	if(s3dat_ioset(handle)->fork_func == NULL) return NULL;

	s3dat_t* fork = s3util_alloc_func(s3dat_memset(handle), sizeof(s3dat_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	memcpy(fork, handle, sizeof(s3dat_t));

	fork->ioset.arg = s3dat_ioset(handle)->fork_func(s3dat_ioset(handle)->arg);

	return fork;
}

void s3dat_delete_fork(s3dat_t* handle) {
	if(s3dat_ioset(handle)->close_func != NULL) s3dat_ioset(handle)->close_func(s3dat_ioset(handle)->arg);
	s3util_free_func(s3dat_memset(handle), handle);
}

s3dat_t* s3dat_writeable_fork(s3dat_t* handle, void* io_arg, s3util_exception_t** throws) {
	s3dat_t* fork = s3util_alloc_func(s3dat_memset(handle), sizeof(s3dat_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	memcpy(fork, handle, sizeof(s3dat_t));

	fork->ioset.arg = io_arg;

	return fork;
}

void s3dat_cache_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws) {
	s3dat_t* handle = me->parent;

	s3dat_cache_t* tmp_cache = me->arg;

	while(tmp_cache) {
		if(tmp_cache->res.first_index == res->first_index &&
			tmp_cache->res.second_index == res->second_index &&
			tmp_cache->res.type == res->type) {
			res->res = tmp_cache->res.res;
			s3dat_ref(res->res);
			return;
		}

		tmp_cache = tmp_cache->next;
	}

	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	tmp_cache = s3dat_new_cache(handle, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	memcpy(&tmp_cache->res, res, sizeof(s3dat_res_t));
	s3dat_ref(res->res);

	tmp_cache->next = me->arg;
	me->arg = tmp_cache;
}

void s3dat_add_cache(s3dat_t* parent, s3util_exception_t** throws) {
	s3dat_extracthandler_t* exhandler = s3dat_new_exhandler(parent, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(parent), throws, __FILE__, __func__, __LINE__);

	exhandler->call = s3dat_cache_handler;
	exhandler->arg_deref = (void (*) (void*)) s3dat_delete_cache_r;

	s3dat_add_extracthandler(parent, exhandler);
}

s3dat_cache_t* s3dat_new_cache(s3dat_t* handle, s3util_exception_t** throws) {
	s3dat_cache_t* cache = s3util_alloc_func(s3dat_memset(handle), sizeof(s3dat_cache_t), throws);
	if(*throws != NULL) {
		s3util_add_to_stack(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);
		return NULL;
	}

	cache->parent = handle;

	return cache;
}

void s3dat_delete_cache_r(s3dat_cache_t* cache) {
	s3dat_cache_t* tmp1 = cache;
	s3dat_cache_t* tmp2;

	while(tmp1) {
		tmp2 = tmp1;
		tmp1 = tmp1->next;
		s3dat_unref(tmp2->res.res);
		s3util_free_func(s3dat_memset(tmp2->parent), tmp2);
	}
}

uint16_t s3dat_indexlen(s3dat_t* handle, s3dat_content_type type) {
	s3dat_index_t* index = NULL;
	s3dat_seq_index_t* seq_index = NULL;

	switch(type) {
		case s3dat_snd:
		if(handle->sound_index->type != 0) {
			return handle->sound_index->len;
		} else {
			return 0;
		}

		case s3dat_settler:
		seq_index = handle->settler_index;
		break;

		case s3dat_torso:
		seq_index = handle->torso_index;
		break;

		case s3dat_shadow:
		seq_index = handle->shadow_index;
		break;

		case s3dat_landscape:
		index = handle->landscape_index;
		break;

		case s3dat_gui:
		index = handle->gui_index;
		break;

		case s3dat_animation:
		index = handle->animation_index;
		break;

		case s3dat_palette:
		index = handle->palette_index;
		break;

		case s3dat_string:
		seq_index = handle->string_index;
		break;

		default:
		return 0;
	}

	if(seq_index && seq_index->type != 0) {
		return seq_index->len;
	} else if(index && index->type != 0) {
		return index->len;
	} else {
		return 0;
	}
}

uint32_t s3dat_seqlen(s3dat_t* handle, uint16_t seq, s3dat_content_type type) {
	if(s3dat_indexlen(handle, type) <= seq) return 0;

	switch(type) {
		case s3dat_snd:
		return handle->sound_index->sequences[seq].len;

		case s3dat_settler:
		return handle->settler_index->sequences[seq].len;

		case s3dat_torso:
		return handle->torso_index->sequences[seq].len;

		case s3dat_shadow:
		return handle->shadow_index->sequences[seq].len;

		case s3dat_string:
		return handle->string_index->sequences[seq].len;

		default:
		return 0;
	}
}

uint32_t s3dat_indexaddr(s3dat_t* handle, uint16_t index, s3dat_content_type type) {
	if(s3dat_indexlen(handle, type) <= index) return 0;

	switch(type) {
		case s3dat_landscape:
		return handle->landscape_index->pointers[index];

		case s3dat_gui:
		return handle->gui_index->pointers[index];

		case s3dat_animation:
		return handle->animation_index->pointers[index];

		case s3dat_palette:
		return handle->palette_index->pointers[index];

		default:
		return 0;
	}
}

uint32_t s3dat_seqaddr(s3dat_t* handle, uint16_t seq, uint32_t index, s3dat_content_type type) {
	if(s3dat_indexlen(handle, type) <= seq || s3dat_seqlen(handle, seq, type) <= index) return 0;

	switch(type) {
		case s3dat_snd:
		return handle->sound_index->sequences[seq].pointers[index];

		case s3dat_settler:
		return handle->settler_index->sequences[seq].pointers[index];

		case s3dat_torso:
		return handle->torso_index->sequences[seq].pointers[index];

		case s3dat_shadow:
		return handle->shadow_index->sequences[seq].pointers[index];

		case s3dat_string:
		return handle->string_index->sequences[seq].pointers[index];

		default:
		return 0;
	}
}
bool s3dat_setindexlen(s3dat_t* handle, s3dat_content_type type, uint16_t indexlen) {
	s3dat_index_t* index = NULL;
	s3dat_seq_index_t* seqindex = NULL;

	switch(type) {
		case s3dat_settler:
			seqindex = handle->settler_index;
		break;
		case s3dat_torso:
			seqindex = handle->torso_index;
		break;
		case s3dat_shadow:
			seqindex = handle->shadow_index;
		break;
		case s3dat_string:
			seqindex = handle->string_index;
		break;
		case s3dat_landscape:
			index = handle->landscape_index;
		break;
		case s3dat_gui:
			index = handle->gui_index;
		break;
		case s3dat_animation:
			index = handle->animation_index;
		break;
		case s3dat_palette:
			index = handle->palette_index;
		break;
		default:
		break;
	}

	if(index) {
		index->len = indexlen;
	} else if(seqindex) {
		seqindex->sequences = s3util_alloc_func(s3dat_memset(handle), sizeof(s3dat_index_t)*indexlen, NULL);
		if(seqindex->sequences == NULL) return false;
		seqindex->len = indexlen;
	} else {
		return false;
	}

	return true;
}

bool s3dat_setseqindexlen(s3dat_t* handle, s3dat_content_type type, uint16_t index, uint16_t seqindexlen) {
	s3dat_seq_index_t* seqindex = NULL;

	switch(type) {
		case s3dat_settler:
			seqindex = handle->settler_index;
		break;
		case s3dat_torso:
			seqindex = handle->torso_index;
		break;
		case s3dat_shadow:
			seqindex = handle->shadow_index;
		break;
		case s3dat_string:
			seqindex = handle->string_index;
		break;
		default:
			return false;
		break;
	}
	if(seqindex->len <= index) return false;

	seqindex->sequences[index].len = type == s3dat_string ? index : (index&0xFF);
	return true;
}

uint32_t s3dat_palette_width(s3dat_t* handle) {
	return handle->palette_line_length;
}

uint32_t s3dat_anilen(s3dat_ref_t* ani) {
	if(!s3dat_is_animation(ani)) return 0;
	return ani->data.ani->len;
}

s3dat_frame_t* s3dat_frame(s3dat_ref_t* ani, uint32_t frame) {
	if(s3dat_anilen(ani) <= frame) return NULL;
	return &ani->data.ani->frames[frame];
}

s3util_ioset_t* s3dat_ioset(s3dat_t* handle) {
    return &handle->ioset;
}

s3util_memset_t* s3dat_memset(s3dat_t* handle) {
    return &handle->memset;
}
