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
	s3dat_t* s3dat_mem = alloc_func(arg, sizeof(s3dat_t));
	s3dat_mem->mem_arg = arg;
	s3dat_mem->alloc_func = alloc_func;
	s3dat_mem->free_func = free_func;

	s3dat_mem->settler_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	s3dat_mem->shadow_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	s3dat_mem->torso_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	s3dat_mem->string_index = alloc_func(arg, sizeof(s3dat_seq_index_t));
	s3dat_mem->sound_index = alloc_func(arg, sizeof(s3dat_seq_index32_t));

	s3dat_mem->landscape_index = alloc_func(arg, sizeof(s3dat_index_t));
	s3dat_mem->gui_index = alloc_func(arg, sizeof(s3dat_index_t));
	s3dat_mem->animation_index = alloc_func(arg, sizeof(s3dat_index_t));
	s3dat_mem->palette_index = alloc_func(arg, sizeof(s3dat_index_t));
}

void s3dat_delete(s3dat_t* mem) {
	while(mem->last_handler != NULL) {
		s3dat_extracthandler_t* tmp = mem->last_handler;
		mem->last_handler = mem->last_handler->before;
		s3dat_delete_exhandler(tmp);
	}

	s3dat_internal_delete_seq(mem, mem->settler_index);
	s3dat_internal_delete_seq(mem, mem->shadow_index);
	s3dat_internal_delete_seq(mem, mem->torso_index);
	s3dat_internal_delete_seq(mem, mem->string_index);
	s3dat_internal_delete_seq32(mem, mem->sound_index);

	s3dat_internal_delete_index(mem, mem->landscape_index);
	s3dat_internal_delete_index(mem, mem->animation_index);
	s3dat_internal_delete_index(mem, mem->palette_index);
	s3dat_internal_delete_index(mem, mem->gui_index);

	mem->free_func(mem->mem_arg, mem->settler_index);
	mem->free_func(mem->mem_arg, mem->shadow_index);
	mem->free_func(mem->mem_arg, mem->torso_index);
	mem->free_func(mem->mem_arg, mem->string_index);
	mem->free_func(mem->mem_arg, mem->sound_index);

	mem->free_func(mem->mem_arg, mem->landscape_index);
	mem->free_func(mem->mem_arg, mem->animation_index);
	mem->free_func(mem->mem_arg, mem->palette_index);
	mem->free_func(mem->mem_arg, mem->gui_index);

	s3dat_delete_fork(mem);
}


void s3dat_internal_delete_index(s3dat_t* mem, s3dat_index_t* index) {
	s3dat_internal_delete_indices(mem, index, 1);
}

void s3dat_internal_delete_indices(s3dat_t* mem, s3dat_index_t* indices, uint32_t count) {
	if(count == 0) return;
	for(uint32_t i = 0;i != count;i++) {
		if(indices[i].type != 0) mem->free_func(mem->mem_arg, indices[i].pointers);
	}
}

void s3dat_internal_delete_index32(s3dat_t* mem, s3dat_index32_t* index) {
	if(index->type == 0) return;

	mem->free_func(mem->mem_arg, index->pointers);
}


void s3dat_internal_delete_seq(s3dat_t* mem, s3dat_seq_index_t* seq) {
	if(seq->type == 0) return;

	for(uint16_t i = 0;i != seq->len;i++) {
		s3dat_internal_delete_index(mem, seq->sequences+i);
	}
	mem->free_func(mem->mem_arg, seq->sequences);
}

void s3dat_internal_delete_seq32(s3dat_t* mem, s3dat_seq_index32_t* seq) {
	if(seq->type == 0) return;

	for(uint16_t i = 0;i != seq->len;i++) {
		s3dat_internal_delete_index32(mem, seq->sequences+i);
	}
	mem->free_func(mem->mem_arg, seq->sequences);
}

void* s3dat_default_alloc_func(void* arg, size_t size) {

	return calloc(size, 1);
}

void s3dat_default_free_func(void* arg, void* mem) {
	if(mem != NULL) free(mem);
}

void* s3dat_internal_alloc_func(s3dat_t* mem, size_t size, s3dat_exception_t** throws) {
	void* new_block = mem->alloc_func(mem->mem_arg, size);

	if(new_block == NULL) {
		S3DAT_INTERNAL_OUT_OF_MEMORY(mem, throws);
	}

	return new_block;
}

s3dat_t* s3dat_fork(s3dat_t* mem) {
	if(mem->fork_func == NULL) return NULL;

	s3dat_t* fork = mem->alloc_func(mem->mem_arg, sizeof(s3dat_t));
	memcpy(fork, mem, sizeof(s3dat_t));

	fork->io_arg = mem->fork_func(mem->io_arg);

	return fork;
}

void s3dat_delete_fork(s3dat_t* mem) {
	if(mem->close_func != NULL) mem->close_func(mem->io_arg);
	mem->free_func(mem->mem_arg, mem);
}

