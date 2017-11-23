#include "s3dat.h"

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

extern void* s3dat_test_ex(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws);

int main() {
	s3dat_exception_t* ex = NULL;

	DIR* gfx_dir = opendir("GFX");
	DIR* snd_dir = opendir("SND");
	if(!gfx_dir && !snd_dir) {
		printf("No \"GFX\" , nor \"SND\" dir found!\n");
		return 0;
	}

	int i = 0;
	struct dirent* ent;
	while((gfx_dir && (ent = readdir(gfx_dir)) != NULL) || (snd_dir && (ent = readdir(snd_dir)) != NULL)) {
		if(ent->d_name[0] == '.') continue;
		if(i != 0) printf("\n");

		int len = strlen(ent->d_name);
		char name[len+5];
		if(len > 15) {
			name[0] = 'G';
			name[1] = 'F';
			name[2] = 'X';
			name[3] = '/';
		} else {
			name[0] = 'S';
			name[1] = 'N';
			name[2] = 'D';
			name[3] = '/';
		}
		strcpy(name+4, ent->d_name);

		s3dat_t* handle = s3dat_new_malloc();

		s3dat_readfile_name(handle, name, &ex);
		s3dat_add_utf8_encoding(handle);

		printf("[%i] new file %s\n", i, name);
		if(s3dat_catch_exception(&ex, handle)) {
			if(handle->sound_index->len == 0) {
				printf("[%i] %i settler sequences\n", i, handle->settler_index->len);
				printf("[%i] %i shadow sequences\n", i, handle->shadow_index->len);
				printf("[%i] %i torso sequences\n", i, handle->torso_index->len);
				printf("[%i] %i gui entries\n", i, handle->gui_index->len);
				printf("[%i] %i animation entries\n", i, handle->animation_index->len);
				printf("[%i] %i palette entries with %i bytes per line\n", i, handle->palette_index->len, handle->palette_line_length);
				printf("[%i] %i landscape entries\n", i, handle->landscape_index->len);
				printf("[%i] %i string entries\n", i, handle->string_index->len);
			} else {
				printf("[%i] %i sound entries\n", i, handle->sound_index->len);
			}
		}

		/*if(handle->string_index->len > 0 && false) {
			for(uint32_t s = 0;s != handle->string_index->len;s++) {
				s3dat_string_t* strings[8];
				for(uint16_t l = 0;l != 8;l++) {
					strings[l] = s3dat_extract_string(handle, s, l, &ex);
					if(s3dat_catch_exception(&ex, handle)) {
						printf("%s", strings[l]->string_data);
					}
					if(l != 7) printf("|");
				}
				s3dat_delete_string_array(strings, 8);
				printf("\n");
			}
		}
		if(handle->palette_index->len > 0 && false) {
			for(uint32_t p = 0;p != handle->palette_index->len;p++) {
				handle->seek_func(handle->io_arg, handle->palette_index->pointers[0], S3DAT_SEEK_SET);
				char name[100];
				snprintf(name, 100, "palette_dump-%i.data", p);
				FILE* file = fopen(name, "wb");

				s3dat_bitmap_t* bmp = s3dat_extract_palette(handle, p, &ex);
				if(ex != NULL) {
					s3dat_print_exception(ex);
					s3dat_delete_exception(handle, ex);
					ex = NULL;
				} else {
					fwrite(bmp->data, bmp->width*bmp->height, 4, file);
				}

				s3dat_delete_bitmap(bmp);
				fclose(file);
			}
		}

		if(handle->animation_index->len > 0 && false) {
			for(uint32_t e = 0;e != handle->animation_index->len;e++) {
				printf("[%i] animation %i at %i\n", i, e, handle->animation_index->pointers[e]);
				s3dat_animation_t* ani = s3dat_extract_animation(handle, e, &ex);

				if(ex != NULL) {
					s3dat_print_exception(ex);
					s3dat_delete_exception(handle, ex);
					ex = NULL;
				} else for(int d = 0;d != ani->len;d++) {
					printf("[%i] x=%hi y=%hi sfile=%hu sid=%hu sframe=%hu tfile=%hu tid=%hu  tframe=%hu hfile=%hu hid=%hu flags={0x%x,0x%x}\n", i, ani->frames[d].posx, ani->frames[d].posy, ani->frames[d].settler_file, ani->frames[d].settler_id, ani->frames[d].settler_frame, ani->frames[d].torso_file, ani->frames[d].torso_id, ani->frames[d].torso_frame, ani->frames[d].shadow_file, ani->frames[d].shadow_id, ani->frames[d].flag1, ani->frames[d].flag2);
				}
				s3dat_delete_animation(ani);
			}
		}
		if(handle->sound_index->len > 0 && false) {
			for(uint32_t p = 0;p != handle->sound_index->len;p++) {
				for(uint32_t sp = 0;sp != handle->sound_index->sequences[p].len;sp++) {
					handle->seek_func(handle->io_arg, handle->sound_index->sequences[p].pointers[sp], S3DAT_SEEK_SET);
					printf("sound %i|%i at %u: %u, %u, %u, %u\n", p, sp, handle->sound_index->sequences[p].pointers[sp], s3dat_internal_read32LE(handle, NULL), s3dat_internal_read32LE(handle, NULL), s3dat_internal_read32LE(handle, NULL), s3dat_internal_read32LE(handle, NULL));
				}
			}
		}
		if(handle->gui_index->len > 0 && false) {
			for(uint16_t gui = 0;gui != handle->gui_index->len;gui++) {
				s3dat_bitmap_t* bmp = s3dat_extract_gui(handle, gui, &ex);
				if(s3dat_catch_exception(&ex, handle)) {
					printf("%02hx: ", gui);
					for(int32_t pi = 31;pi >= 0;pi--) {
							printf("%i", (bmp->gui_type>>pi)&1);
					}
					printf(" (%i)\n", bmp->gui_type);
					s3dat_delete_bitmap(bmp);
				}
			}
		}*/

		s3dat_delete(handle);

		i++;
	}
	if(gfx_dir) closedir(gfx_dir);
	if(snd_dir) closedir(snd_dir);
}

