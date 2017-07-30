#include "s3dat.h"

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>

int main() {
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

		s3dat_t* s3dat_mem = s3dat_new_malloc();

		int fd = open(name, O_RDONLY);

		s3dat_exception_t* ex = NULL;

		s3dat_readfile_fd(s3dat_mem, fd, &ex);
		printf("[%i] new file %s\n", i, name);
		if(ex != NULL) {
			s3dat_print_exception(ex);
			s3dat_delete_exception(s3dat_mem, ex);
			ex = NULL;
		} else if(s3dat_mem->sound_index.len == 0) {
			printf("[%i] %i settler sequences\n", i, s3dat_mem->settler_index.len);
			printf("[%i] %i shadow sequences\n", i, s3dat_mem->shadow_index.len);
			printf("[%i] %i torso sequences\n", i, s3dat_mem->torso_index.len);
			printf("[%i] %i gui entries\n", i, s3dat_mem->gui_index.len);
			printf("[%i] %i animation entries\n", i, s3dat_mem->animation_index.len);
			printf("[%i] %i palette entries\n", i, s3dat_mem->palette_index.len);
			printf("[%i] %i landscape entries\n", i, s3dat_mem->landscape_index.len);
			printf("[%i] %i string entries\n", i, s3dat_mem->string_index.len);
		} else {
			printf("[%i] %i sound entries\n", i, s3dat_mem->sound_index.len);
		}

		if(s3dat_mem->string_index.len > 0 && false) {
			for(uint32_t s = 0;s != s3dat_mem->string_index.len;s++) {
				s3dat_string_t* strings = s3dat_new_strings(s3dat_mem, 8);
				for(uint16_t l = 0;l != 8;l++) {
					s3dat_extract_string(s3dat_mem, s, l, strings+l, true, &ex);
					if(ex != NULL) {
						s3dat_print_exception(ex);
						s3dat_delete_exception(s3dat_mem, ex);
						ex = NULL;
					} else {
						printf("%s", strings[l].string_data);
					}
					if(l != 7) printf("|");
				}
				s3dat_delete_strings(strings, 8);
				printf("\n");
			}
		}
		if(s3dat_mem->animation_index.len > 0 && false) {
			s3dat_animation_t* ani = s3dat_new_animation(s3dat_mem);
			for(uint32_t e = 0;e != s3dat_mem->animation_index.len;e++) {
				printf("[%i] animation %i at %i\n", i, e, s3dat_mem->animation_index.pointers[e]);
				s3dat_extract_animation(s3dat_mem, e, ani, &ex);

				if(ex != NULL) {
					s3dat_print_exception(ex);
					s3dat_delete_exception(s3dat_mem, ex);
					ex = NULL;
				} else for(int d = 0;d != ani->len;d++) {
					printf("[%i] x=%hi y=%hi sfile=%hu sid=%hu sframe=%hu tfile=%hu tid=%hu  tframe=%hu hfile=%hu hid=%hu flags={0x%x,0x%x}\n", i, ani->frames[d].posx, ani->frames[d].posy, ani->frames[d].settler_file, ani->frames[d].settler_id, ani->frames[d].settler_frame, ani->frames[d].torso_file, ani->frames[d].torso_id, ani->frames[d].torso_frame, ani->frames[d].shadow_file, ani->frames[d].shadow_id, ani->frames[d].flag1, ani->frames[d].flag2);
				}
			}
			s3dat_delete_animation(ani);
		}

		close(fd);

		s3dat_delete(s3dat_mem);

		i++;
	}
	if(gfx_dir) closedir(gfx_dir);
	if(snd_dir) closedir(snd_dir);
}

