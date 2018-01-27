#include "s3dat.h"

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

//#define MONITOR_MEMORY "alloc.log"

int main() {

	#ifdef MONITOR_MEMORY
	void* monitor_handle = s3dat_get_default_ioset(S3DAT_IOSET_LIBC)->open_func(MONITOR_MEMORY, true);
	#endif

	s3util_exception_t* ex = NULL;

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

		#ifdef MONITOR_MEMORY
		s3dat_t* handle = s3dat_new_malloc_monitor(monitor_handle, s3dat_get_default_ioset(S3DAT_IOSET_LIBC), false);
		#else
		s3dat_t* handle = s3dat_new_malloc();
		#endif

		s3dat_readfile_name(handle, name, &ex);

		printf("[%i] new file %s\n", i, name);
		if(s3util_catch_exception(&ex)) {
			if(s3dat_indexlen(handle, s3dat_snd) == 0) {
				printf("[%i] %hu settler sequences\n", i, s3dat_indexlen(handle, s3dat_settler));
				printf("[%i] %hu shadow sequences\n", i, s3dat_indexlen(handle, s3dat_shadow));
				printf("[%i] %hu torso sequences\n", i, s3dat_indexlen(handle, s3dat_torso));
				printf("[%i] %hu gui entries\n", i, s3dat_indexlen(handle, s3dat_gui));
				printf("[%i] %hu animation entries\n", i, s3dat_indexlen(handle, s3dat_animation));
				printf("[%i] %hu palette entries with %i bytes per line\n", i, s3dat_indexlen(handle, s3dat_palette), s3dat_palette_width(handle));
				printf("[%i] %hu landscape entries\n", i, s3dat_indexlen(handle, s3dat_landscape));
				printf("[%i] %hu string entries\n", i, s3dat_indexlen(handle, s3dat_string));
			} else {
				printf("[%i] %hu sound entries\n", i, s3dat_indexlen(handle, s3dat_snd));
			}
		}

		s3dat_add_utf8_encoding(handle, &ex);
		s3util_catch_exception(&ex);

		/*uint16_t stringindex_len = s3dat_indexlen(handle, s3dat_string);
		if(stringindex_len > 0 && false) {
			for(uint32_t s = 0;s != stringindex_len;s++) {
				s3dat_ref_t* strings[8];
				for(uint16_t l = 0;l != 8;l++) {
					strings[l] = s3dat_extract_string(handle, s, l, &ex);
					if(s3dat_catch_exception(&ex)) {
						printf("%s", s3dat_strdata(strings[l]));
					}
					if(l != 7) printf("|");
				}
				s3dat_unref_array(strings, 8);
				printf("\n");
			}
		}

		uint16_t paletteindex_len = s3dat_indexlen(handle, s3dat_palette);
		if(paletteindex_len > 0 && false) {
			for(uint32_t p = 0;p != paletteindex_len;p++) {
				char name[100];
				snprintf(name, 100, "palette_dump-%i.data", p);
				FILE* file = fopen(name, "wb");

				s3dat_ref_t* bmp = s3dat_extract_palette(handle, p, &ex);
				if(!s3dat_catch_exception(&ex)) {
					fwrite(s3dat_bmpdata(bmp), s3dat_width(bmp)*s3dat_height(bmp), 4, file);
				}

				s3dat_unref(bmp);
				fclose(file);
			}
		}

		uint16_t animationindex_len = s3dat_indexlen(handle, s3dat_animation);
		if(animationindex_len > 0 && false) {
			for(uint32_t e = 0;e != animationindex_len;e++) {
				printf("[%i] animation %i at %i\n", i, e, s3dat_indexaddr(handle, e, s3dat_animation));
				s3dat_ref_t* ani_ref = s3dat_extract_animation(handle, e, &ex);
				if(!s3dat_catch_exception(&ex)) {
					uint16_t ani_len = 0;
					for(uint16_t d = 0;d != ani_len;d++) {
						s3dat_frame_t* frame = s3dat_frame(ani_ref, d);
						printf("[%i] x=%hi y=%hi sfile=%hu sid=%hu sframe=%hu tfile=%hu tid=%hu  tframe=%hu hfile=%hu hid=%hu flags={0x%x,0x%x}\n", i, frame->posx, frame->posy, frame->settler_file, frame->settler_id, frame->settler_frame, frame->torso_file, frame->torso_id, frame->torso_frame, frame->shadow_file, frame->shadow_id, frame->flag1, frame->flag2);
					}
				}
				s3dat_unref(ani_ref);
			}
		}

		uint16_t guiindex_len = s3dat_indexlen(handle, s3dat_gui);
		if(guiindex_len > 0 && false) {
			for(uint16_t gui = 0;gui != guiindex_len;gui++) {
				s3dat_ref_t* bmp = s3dat_extract_gui(handle, gui, &ex);
				if(s3util_catch_exception(&ex)) {
					printf("%02hx: ", gui);
					uint32_t guitype = *s3dat_gui_meta(bmp);
					for(int32_t pi = 31;pi >= 0;pi--) {
							printf("%i", (guitype>>pi)&1);
					}
					printf(" (%i)\n", guitype);
					s3dat_unref(bmp);
				}
			}
		}*/

		s3dat_delete(handle);

		i++;
	}
	if(gfx_dir) closedir(gfx_dir);
	if(snd_dir) closedir(snd_dir);

	#ifdef MONITOR_MEMORY
	s3dat_get_default_ioset(S3DAT_IOSET_LIBC)->close_func(monitor_handle);
	#endif
}

