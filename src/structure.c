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
	ref->data.raw = type->alloc(parent);
	ref->type = type;
	return ref;
}

s3dat_ref_t* s3dat_new_string(s3dat_t* parent) {
	return s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_str_ref));
}

s3dat_string_t* s3dat_new_raw_string(s3dat_t* parent) {
	s3dat_string_t* string = parent->alloc_func(parent->mem_arg, sizeof(s3dat_string_t));
	string->src = parent;
	return string;
}

s3dat_ref_t* s3dat_new_sound(s3dat_t* parent) {
	return s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_snd_ref));
}

s3dat_sound_t* s3dat_new_raw_sound(s3dat_t* parent) {
	s3dat_sound_t* sound = parent->alloc_func(parent->mem_arg, sizeof(s3dat_sound_t));
	sound->src = parent;
	return sound;
}

s3dat_ref_t* s3dat_new_animation(s3dat_t* parent) {
	return s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_ani_ref));
}

s3dat_animation_t* s3dat_new_raw_animation(s3dat_t* parent) {
	s3dat_animation_t* animation = parent->alloc_func(parent->mem_arg, sizeof(s3dat_animation_t));
	animation->src = parent;
	return animation;
}

s3dat_ref_t* s3dat_new_bitmap(s3dat_t* parent) {
	return s3dat_new_ref(parent, s3dat_internal_get_restype(s3dat_bmp_ref));
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



s3dat_string_t* s3dat_new_strings(s3dat_t* parent, uint32_t count) {
	s3dat_string_t* strings = parent->alloc_func(parent->mem_arg, sizeof(s3dat_string_t)*count);
	for(uint32_t i = 0;i != count;i++) strings[i].src = parent;
	return strings;
}

s3dat_sound_t* s3dat_new_sounds(s3dat_t* parent, uint32_t count) {
	s3dat_sound_t* sounds = parent->alloc_func(parent->mem_arg, sizeof(s3dat_sound_t)*count);
	for(uint32_t i = 0;i != count;i++) sounds[i].src = parent;
	return sounds;
}

s3dat_animation_t* s3dat_new_animations(s3dat_t* parent, uint32_t count) {
	s3dat_animation_t* anis = parent->alloc_func(parent->mem_arg, sizeof(s3dat_animation_t)*count);
	for(uint32_t i = 0;i != count;i++) anis[i].src = parent;
	return anis;
}

s3dat_bitmap_t* s3dat_new_bitmaps(s3dat_t* parent, uint32_t count) {
	s3dat_bitmap_t* bmps = parent->alloc_func(parent->mem_arg, sizeof(s3dat_bitmap_t)*count);
	for(uint32_t i = 0;i != count;i++) bmps[i].src = parent;
	return bmps;
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

void s3dat_default_free_func(void* arg, void* mem) {
	if(mem != NULL) free(mem);
}

void* s3dat_internal_alloc_func(s3dat_t* handle, size_t size, s3dat_exception_t** throws) {
	void* new_block = handle->alloc_func(handle->mem_arg, size);

	if(new_block == NULL) {
		S3DAT_INTERNAL_OUT_OF_MEMORY(handle, throws);
	}

	return new_block;
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
			return;
		}

		tmp_cache = tmp_cache->next;
	}

	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);
	S3DAT_HANDLE_EXCEPTION(handle, throws, __FILE__, __func__, __LINE__);

	tmp_cache = s3dat_new_cache(handle);
	memcpy(&tmp_cache->res, res, sizeof(s3dat_res_t));
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
		s3dat_delete_ref(tmp2->res.res);
		tmp2->parent->free_func(tmp2->parent->mem_arg, tmp2);
	}
}

