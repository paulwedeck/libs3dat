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


s3dat_string_t* s3dat_new_string(s3dat_t* parent) {
	return s3dat_new_strings(parent, 1);
}

s3dat_sound_t* s3dat_new_sound(s3dat_t* parent) {
	return s3dat_new_sounds(parent, 1);
}

s3dat_animation_t* s3dat_new_animation(s3dat_t* parent) {
	return s3dat_new_animations(parent, 1);
}

s3dat_bitmap_t* s3dat_new_bitmap(s3dat_t* parent) {
	return s3dat_new_bitmaps(parent, 1);
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


void s3dat_delete_string(s3dat_string_t* string) {
	s3dat_delete_strings(string, 1);
}

void s3dat_delete_sound(s3dat_sound_t* sound) {
	s3dat_delete_sounds(sound, 1);
}

void s3dat_delete_animation(s3dat_animation_t* ani) {
	s3dat_delete_animations(ani, 1);
}

void s3dat_delete_bitmap(s3dat_bitmap_t* bmp) {
	s3dat_delete_bitmaps(bmp, 1);
}


void s3dat_delete_strings(s3dat_string_t* strings, uint32_t count) {
	s3dat_delete_stringdatas(strings, count);
	strings->src->free_func(strings->src->mem_arg, strings);
}

void s3dat_delete_sounds(s3dat_sound_t* sounds, uint32_t count) {
	s3dat_delete_snddatas(sounds, count);
	sounds->src->free_func(sounds->src->mem_arg, sounds);
}

void s3dat_delete_animations(s3dat_animation_t* anis, uint32_t count) {
	s3dat_delete_frames(anis, count);
	anis->src->free_func(anis->src->mem_arg, anis);
}

void s3dat_delete_bitmaps(s3dat_bitmap_t* bmps, uint32_t count) {
	s3dat_delete_pixdatas(bmps, count);
	bmps->src->free_func(bmps->src->mem_arg, bmps);
}


void s3dat_delete_string_array(s3dat_string_t** strings, uint32_t count) {
	s3dat_delete_stringdata_array(strings, count);
	for(uint32_t i = 0;i != count;i++) strings[i]->src->free_func(strings[i]->src->mem_arg, strings[i]);
}

void s3dat_delete_sound_array(s3dat_sound_t** sounds, uint32_t count) {
	s3dat_delete_snddata_array(sounds, count);
	for(uint32_t i = 0;i != count;i++) sounds[i]->src->free_func(sounds[i]->src->mem_arg, sounds[i]);
}

void s3dat_delete_animation_array(s3dat_animation_t** anis, uint32_t count) {
	s3dat_delete_frame_array(anis, count);
	for(uint32_t i = 0;i != count;i++) anis[i]->src->free_func(anis[i]->src->mem_arg, anis[i]);
}

void s3dat_delete_bitmap_array(s3dat_bitmap_t** bmps, uint32_t count) {
	s3dat_delete_pixdata_array(bmps, count);
	for(uint32_t i = 0;i != count;i++) bmps[i]->src->free_func(bmps[i]->src->mem_arg, bmps[i]);
}


void s3dat_delete_stringdata(s3dat_string_t* string) {
	s3dat_delete_stringdatas(string, 1);
}

void s3dat_delete_snddata(s3dat_sound_t* sound) {
	s3dat_delete_snddatas(sound, 1);
}

void s3dat_delete_frame(s3dat_animation_t* ani) {
	s3dat_delete_frames(ani, 1);
}

void s3dat_delete_pixdata(s3dat_bitmap_t* ani) {
	s3dat_delete_pixdatas(ani, 1);
}


void s3dat_delete_stringdatas(s3dat_string_t* strings, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		strings->src->free_func(strings->src->mem_arg, strings[i].string_data);
		strings[i].string_data = NULL;
	}
}

void s3dat_delete_snddatas(s3dat_sound_t* sounds, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		sounds->src->free_func(sounds->src->mem_arg, sounds[i].data);
		sounds[i].data = NULL;
	}
}

void s3dat_delete_frames(s3dat_animation_t* anis, uint32_t count) {
	for(int i = 0;i != count;i++) {
		anis[i].src->free_func(anis[i].src->mem_arg, anis[i].frames);
		anis[i].frames = NULL;
	}
}

void s3dat_delete_pixdatas(s3dat_bitmap_t* bmps, uint32_t count) {
	for(int i = 0;i != count;i++) {
		bmps[i].src->free_func(bmps[i].src->mem_arg, bmps[i].data);
		bmps[i].data = NULL;
	}
}


void s3dat_delete_stringdata_array(s3dat_string_t** strings, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		strings[i]->src->free_func(strings[i]->src->mem_arg, strings[i]->string_data);
		strings[i]->string_data = NULL;
	}
}

void s3dat_delete_snddata_array(s3dat_sound_t** sounds, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		sounds[i]->src->free_func(sounds[i]->src->mem_arg, sounds[i]->data);
		sounds[i]->data = NULL;
	}
}

void s3dat_delete_frame_array(s3dat_animation_t** anis, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		anis[i]->src->free_func(anis[i]->src->mem_arg, anis[i]->frames);
		anis[i]->frames = NULL;
	}
}

void s3dat_delete_pixdata_array(s3dat_bitmap_t** bmps, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		bmps[i]->src->free_func(bmps[i]->src->mem_arg, bmps[i]->data);
		bmps[i]->data = NULL;
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

