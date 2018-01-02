#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "structure.c"
#endif

//exhandler
s3dat_extracthandler_t* s3dat_new_exhandler(s3dat_t* parent) {
	return s3dat_new_exhandlers(parent, 1);
}
s3dat_extracthandler_t* s3dat_new_exhandlers(s3dat_t* parent, uint32_t count) {
	s3dat_extracthandler_t* handlers = parent->alloc_func(parent->mem_arg, sizeof(s3dat_extracthandler_t)*count);
	for(uint32_t i = 0;i != count;i++) handlers[i].parent = parent;
	return handlers;
}

void s3dat_delete_exhandler(s3dat_extracthandler_t* exhandler) {
	s3dat_delete_exhandlers(exhandler, 1);
}

void s3dat_delete_exhandlers(s3dat_extracthandler_t* exhandlers, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) if(exhandlers[i].arg_deref != NULL) exhandlers[i].arg_deref(exhandlers[i].arg);
	exhandlers->parent->free_func(exhandlers->parent->mem_arg, exhandlers);
}

void s3dat_delete_packed(s3dat_packed_t* package) {
	if(package->data) package->parent->free_func(package->parent->mem_arg, package->data);
	package->parent->free_func(package->parent->mem_arg, package);
}

s3dat_ref_t* s3dat_new_ref(s3dat_t* parent, s3dat_restype_t* type) {
	s3dat_ref_t* ref = parent->alloc_func(parent->mem_arg, sizeof(s3dat_ref_t));
	ref->src = parent;
	ref->refs = 1;
	ref->data.raw = type->alloc(parent);
	ref->type = type;
	return ref;
}

s3dat_ref_t* s3dat_new_string(s3dat_t* parent, uint32_t strlen) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_str_ref));
	ref->data.str->src = parent;
	ref->data.str->original_encoding = true;
	ref->data.str->language = s3dat_unknown_language;
	ref->data.str->string_data = parent->alloc_func(parent->mem_arg, strlen);
	return ref;
}

s3dat_string_t* s3dat_new_raw_string(s3dat_t* parent) {
	s3dat_string_t* string = parent->alloc_func(parent->mem_arg, sizeof(s3dat_string_t));
	string->src = parent;
	return string;
}

s3dat_ref_t* s3dat_new_sound(s3dat_t* parent, uint32_t freq, uint16_t len) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_snd_ref));
	ref->data.snd->src = parent;
	ref->data.snd->freq = freq;
	ref->data.snd->len = len;
	ref->data.snd->data = parent->alloc_func(parent->mem_arg, 2*len);
	return ref;
}

s3dat_sound_t* s3dat_new_raw_sound(s3dat_t* parent) {
	s3dat_sound_t* sound = parent->alloc_func(parent->mem_arg, sizeof(s3dat_sound_t));
	sound->src = parent;
	return sound;
}

s3dat_ref_t* s3dat_new_animation(s3dat_t* parent, uint32_t frames) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_ani_ref));
	ref->data.ani->src = parent;
	ref->data.ani->len = frames;
	ref->data.ani->frames = parent->alloc_func(parent->mem_arg, frames*sizeof(s3dat_frame_t));
	return ref;
}

s3dat_animation_t* s3dat_new_raw_animation(s3dat_t* parent) {
	s3dat_animation_t* animation = parent->alloc_func(parent->mem_arg, sizeof(s3dat_animation_t));
	animation->src = parent;
	return animation;
}

s3dat_ref_t* s3dat_new_bitmap(s3dat_t* parent, uint16_t width, uint16_t height) {
	s3dat_ref_t* ref = s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_bmp_ref));
	ref->data.bmp->src = parent;
	ref->data.bmp->width = width;
	ref->data.bmp->height = height;
	ref->data.bmp->type = s3dat_unknown_color;
	ref->data.bmp->data = parent->alloc_func(parent->mem_arg, width*height*sizeof(s3dat_color_t));
	return ref;
}

s3dat_bitmap_t* s3dat_new_raw_bitmap(s3dat_t* parent) {
	s3dat_bitmap_t* bitmap = parent->alloc_func(parent->mem_arg, sizeof(s3dat_bitmap_t));
	bitmap->src = parent;
	return bitmap;
}

s3dat_ref_t* s3dat_new_packed(s3dat_t* parent) {
	return s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_pkd_ref));
}

s3dat_packed_t* s3dat_new_raw_packed(s3dat_t* parent) {
	s3dat_packed_t* packed = parent->alloc_func(parent->mem_arg, sizeof(s3dat_packed_t));
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
	string->src->free_func(string->src->mem_arg, string->string_data);
	string->src->free_func(string->src->mem_arg, string);
}

void s3dat_delete_sound(s3dat_sound_t* sound) {
	sound->src->free_func(sound->src->mem_arg, sound->data);
	sound->src->free_func(sound->src->mem_arg, sound);
}

void s3dat_delete_animation(s3dat_animation_t* ani) {
	ani->src->free_func(ani->src->mem_arg, ani->frames);
	ani->src->free_func(ani->src->mem_arg, ani);
}

void s3dat_delete_bitmap(s3dat_bitmap_t* bmp) {
	bmp->src->free_func(bmp->src->mem_arg, bmp->data);
	bmp->src->free_func(bmp->src->mem_arg, bmp);
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
		refs[i]->src->free_func(refs[i]->src->mem_arg, refs[i]);
	}
}


// s3dat_t
s3dat_t* s3dat_new_malloc() {
	return s3dat_new_func(0, s3dat_default_alloc_func, s3dat_default_free_func);
}

s3dat_t* s3dat_new_malloc_monitor(void* arg, s3dat_ioset_t* ioset, bool open) {
	void* ioarg = open ? ioset->open_func(arg, arg) : arg;
	if(!ioarg) return NULL;

	s3dat_monitor_t* monitor = s3dat_default_alloc_func(0, sizeof(s3dat_monitor_t));
	monitor->io_arg = ioarg;
	monitor->ioset = ioset;
	monitor->close = open;

	monitor->last_state = 0;
	monitor->mem_arg = 0;
	monitor->alloc_func = s3dat_default_alloc_func;
	monitor->free_func = s3dat_default_free_func;

	s3dat_monitor_print(monitor);

	return s3dat_new_func(monitor, s3dat_monitor_alloc_func, s3dat_monitor_free_func);
}


s3dat_t* s3dat_new_func(void* arg, void* (*alloc_func) (void*, size_t), void (*free_func) (void*, void*)) {
	s3dat_t* handle = alloc_func(arg, sizeof(s3dat_t));
	handle->mem_arg = arg;
	handle->alloc_func = alloc_func;
	handle->free_func = free_func;

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

	handle->free_func(handle->mem_arg, handle->settler_index);
	handle->free_func(handle->mem_arg, handle->shadow_index);
	handle->free_func(handle->mem_arg, handle->torso_index);
	handle->free_func(handle->mem_arg, handle->string_index);
	handle->free_func(handle->mem_arg, handle->sound_index);

	handle->free_func(handle->mem_arg, handle->landscape_index);
	handle->free_func(handle->mem_arg, handle->animation_index);
	handle->free_func(handle->mem_arg, handle->palette_index);
	handle->free_func(handle->mem_arg, handle->gui_index);

	s3dat_delete_fork(handle);
}


void s3dat_internal_delete_index(s3dat_t* handle, s3dat_index_t* index) {
	s3dat_internal_delete_indices(handle, index, 1);
}

void s3dat_internal_delete_indices(s3dat_t* handle, s3dat_index_t* indices, uint32_t count) {
	if(count == 0) return;
	for(uint32_t i = 0;i != count;i++) {
		if(indices[i].type != 0) handle->free_func(handle->mem_arg, indices[i].pointers);
	}
}

void s3dat_internal_delete_index32(s3dat_t* handle, s3dat_index32_t* index) {
	if(index->type == 0) return;

	handle->free_func(handle->mem_arg, index->pointers);
}


void s3dat_internal_delete_seq(s3dat_t* handle, s3dat_seq_index_t* seq) {
	if(seq->type == 0) return;

	for(uint16_t i = 0;i != seq->len;i++) {
		s3dat_internal_delete_index(handle, seq->sequences+i);
	}
	handle->free_func(handle->mem_arg, seq->sequences);
}

void s3dat_internal_delete_seq32(s3dat_t* handle, s3dat_seq_index32_t* seq) {
	if(seq->type == 0) return;

	for(uint16_t i = 0;i != seq->len;i++) {
		s3dat_internal_delete_index32(handle, seq->sequences+i);
	}
	handle->free_func(handle->mem_arg, seq->sequences);
}

void* s3dat_default_alloc_func(void* arg, size_t size) {
	return calloc(size, 1);
}

void* s3dat_monitor_alloc_func(void* arg, size_t size) {
	s3dat_monitor_t* mon = arg;
	mon->last_state += size;

	s3dat_monitor_print(mon);

	uint8_t* mem = mon->alloc_func(mon->mem_arg, size+4);
	*((uint32_t*)mem) = size;

	return mem+4;
}


void s3dat_default_free_func(void* arg, void* mem) {
	if(mem != NULL) free(mem);
}

void s3dat_monitor_free_func(void* arg, void* mem) {
	s3dat_monitor_t* mon = arg;
	mon->last_state -= *(((uint32_t*)mem)-1);

	s3dat_monitor_print(mon);

	mon->free_func(mon->mem_arg, ((uint32_t*)mem)-1);

	if(mon->last_state == 0) {
		if(mon->close) mon->ioset->close_func(mon->io_arg);
		mon->free_func(mon->mem_arg, mon);
	}
}


void* s3dat_alloc_func(s3dat_t* handle, size_t size, s3dat_exception_t** throws) {
	void* new_block = handle->alloc_func(handle->mem_arg, size);

	if(new_block == NULL) {
		S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws);
	}

	return new_block;
}

void s3dat_free_func(s3dat_t* handle, void* data) {
	handle->free_func(handle->mem_arg, data);
}

void s3dat_monitor_print(s3dat_monitor_t* monitor) {
	char bfr[1024];

	snprintf(bfr, 1023, "%li %u\n", clock(), monitor->last_state);

	monitor->ioset->write_func(monitor->io_arg, bfr, strlen(bfr));
}

s3dat_t* s3dat_fork(s3dat_t* handle) {
	if(handle->fork_func == NULL) return NULL;

	s3dat_t* fork = handle->alloc_func(handle->mem_arg, sizeof(s3dat_t));
	memcpy(fork, handle, sizeof(s3dat_t));

	fork->io_arg = handle->fork_func(handle->io_arg);

	return fork;
}

void s3dat_delete_fork(s3dat_t* handle) {
	if(handle->close_func != NULL) handle->close_func(handle->io_arg);
	handle->free_func(handle->mem_arg, handle);
}

s3dat_t* s3dat_writeable_fork(s3dat_t* handle, void* io_arg) {
	s3dat_t* fork = handle->alloc_func(handle->mem_arg, sizeof(s3dat_t));
	memcpy(fork, handle, sizeof(s3dat_t));

	fork->io_arg = io_arg;

	return fork;
}

void s3dat_cache_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
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
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	tmp_cache = s3dat_new_cache(handle);

	memcpy(&tmp_cache->res, res, sizeof(s3dat_res_t));
	s3dat_ref(res->res);

	tmp_cache->next = me->arg;
	me->arg = tmp_cache;
}

void s3dat_add_cache(s3dat_t* parent) {
	s3dat_extracthandler_t* exhandler = s3dat_new_exhandler(parent);
	exhandler->call = s3dat_cache_handler;
	exhandler->arg_deref = (void (*) (void*)) s3dat_delete_cache_r;

	s3dat_add_extracthandler(parent, exhandler);
}

s3dat_cache_t* s3dat_new_cache(s3dat_t* handle) {
	s3dat_cache_t* cache = handle->alloc_func(handle->mem_arg, sizeof(s3dat_cache_t));
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
		tmp2->parent->free_func(tmp2->parent->mem_arg, tmp2);
	}
}

uint16_t s3dat_indexlen(s3dat_t* handle, s3dat_content_type type) {
	switch(type) {
		case s3dat_snd:
		return handle->sound_index->len;

		case s3dat_settler:
		return handle->settler_index->len;

		case s3dat_torso:
		return handle->torso_index->len;

		case s3dat_shadow:
		return handle->shadow_index->len;

		case s3dat_landscape:
		return handle->landscape_index->len;

		case s3dat_gui:
		return handle->gui_index->len;

		case s3dat_animation:
		return handle->animation_index->len;

		case s3dat_palette:
		return handle->palette_index->len;

		case s3dat_string:
		return handle->string_index->len;

		default:
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
