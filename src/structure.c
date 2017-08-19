#include "s3dat_internal.h"
#line __LINE__ "structure.c"

s3dat_animation_t* s3dat_new_animation(s3dat_t* parent) {
	return s3dat_new_animations(parent, 1);
}

s3dat_animation_t* s3dat_new_animations(s3dat_t* parent, uint32_t count) {
	s3dat_animation_t* bm = parent->alloc_func(parent->mem_arg, sizeof(s3dat_animation_t)*count);
	for(uint32_t i = 0;i != count;i++) bm[i].src = parent;
	return bm;
}

s3dat_string_t* s3dat_new_strings(s3dat_t* parent, uint32_t count) {
	s3dat_string_t* strings = parent->alloc_func(parent->mem_arg, sizeof(s3dat_string_t)*count);
	for(uint32_t i = 0;i != count;i++) strings[i].src = parent;
	return strings;
}

s3dat_string_t* sdat_new_string(s3dat_t* parent) {
	return s3dat_new_strings(parent, 1);
}

s3dat_bitmap_t* s3dat_new_bitmap(s3dat_t* parent) {
	return s3dat_new_bitmaps(parent, 1);
}

s3dat_bitmap_t* s3dat_new_bitmaps(s3dat_t* parent, uint32_t count) {
	s3dat_bitmap_t* bm = parent->alloc_func(parent->mem_arg, sizeof(s3dat_bitmap_t)*count);
	for(uint32_t i = 0;i != count;i++) bm[i].src = parent;
	return bm;
}

s3dat_t* s3dat_new_malloc() {
	return s3dat_new_func(0, s3dat_default_alloc_func, s3dat_default_free_func);
}

s3dat_t* s3dat_new_func(void* arg, void* (*alloc_func) (void*, size_t), void (*free_func) (void*, void*)) {
	s3dat_t* s3dat_mem = alloc_func(arg, sizeof(s3dat_t));
	s3dat_mem->mem_arg = arg;
	s3dat_mem->alloc_func = alloc_func;
	s3dat_mem->free_func = free_func;
}

void s3dat_delete(s3dat_t* mem) {
	if(mem->close_func != NULL) mem->close_func(mem->io_arg);

	s3dat_internal_delete_seq(mem, &mem->settler_index);
	s3dat_internal_delete_seq(mem, &mem->shadow_index);
	s3dat_internal_delete_seq(mem, &mem->torso_index);
	s3dat_internal_delete_seq(mem, &mem->string_index);
	s3dat_internal_delete_seq32(mem, &mem->sound_index);

	s3dat_internal_delete_index(mem, &mem->landscape_index);
	s3dat_internal_delete_index(mem, &mem->animation_index);
	s3dat_internal_delete_index(mem, &mem->palette_index);
	s3dat_internal_delete_index(mem, &mem->gui_index);

	mem->free_func(mem->mem_arg, mem);
}

void s3dat_delete_animation(s3dat_animation_t* mem) {
	s3dat_delete_animations(mem, 1);
}

void s3dat_delete_animations(s3dat_animation_t* mem, uint32_t count) {
	s3dat_delete_frames(mem, count);
	mem->src->free_func(mem->src->mem_arg, mem);
}

void s3dat_delete_frame(s3dat_animation_t* mem) {
	s3dat_delete_frames(mem, 1);
}

void s3dat_delete_frames(s3dat_animation_t* mem, uint32_t count) {
	for(int i = 0;i != count;i++) {
		if(mem[i].frames != NULL) {
			mem[i].src->free_func(mem[i].src->mem_arg, mem[i].frames);
			mem[i].frames = NULL;
		}
	}
}

void s3dat_delete_bitmap(s3dat_bitmap_t* mem) {
	s3dat_delete_bitmaps(mem, 1);
}

void s3dat_delete_bitmaps(s3dat_bitmap_t* mem, uint32_t count) {
	s3dat_delete_pixdatas(mem, count);
	mem->src->free_func(mem->src->mem_arg, mem);
}

void s3dat_delete_pixdata(s3dat_bitmap_t* mem) {
	s3dat_delete_pixdatas(mem, 1);
}

void s3dat_delete_pixdatas(s3dat_bitmap_t* mem, uint32_t count) {
	for(int i = 0;i != count;i++) {
		if(mem[i].data != NULL) {
			mem[i].src->free_func(mem[i].src->mem_arg, mem[i].data);
			mem[i].data = NULL;
		}
	}
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

void s3dat_delete_string(s3dat_string_t* string) {
	s3dat_delete_strings(string, 1);
}

void s3dat_delete_strings(s3dat_string_t* strings, uint32_t count) {
	s3dat_delete_stringdatas(strings, count);

	strings->src->free_func(strings->src->mem_arg, strings);
}

void s3dat_delete_stringdata(s3dat_string_t* string) {
	s3dat_delete_stringdatas(string, 1);
}

void s3dat_delete_stringdatas(s3dat_string_t* strings, uint32_t count) {
	for(uint32_t i = 0;i != count;i++) {
		if(strings[i].string_data) strings->src->free_func(strings->src->mem_arg, strings[i].string_data);
	}
}

void* s3dat_default_alloc_func(void* arg, size_t size) {
	return calloc(size, 1);
}

void s3dat_default_free_func(void* arg, void* mem) {
	free(mem);
}

