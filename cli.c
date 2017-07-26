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

		int error = s3dat_readfile_fd(s3dat_mem, fd);
		printf("[%i] new file %s\n", i, name);
		if(error != 0) {
			printf("%i\n", error);
		} else if(s3dat_mem->sound_index.len == 0) {
			printf("[%i] %i settler sequences\n", i, s3dat_mem->settler_index.len);
			printf("[%i] %i shadow sequences\n", i, s3dat_mem->shadow_index.len);
			printf("[%i] %i torso sequences\n", i, s3dat_mem->torso_index.len);
			printf("[%i] %i gui entries\n", i, s3dat_mem->gui_index.len);
			printf("[%i] %i animation entries\n", i, s3dat_mem->animation_index.len);
			s3dat_mem->seek_func(s3dat_mem->io_arg, 80, SEEK_SET);
			uint32_t palette_pos;
			s3dat_mem->read_func(s3dat_mem->io_arg, &palette_pos, 4);
			printf("[%i] %i palette entries at %u\n", i, s3dat_mem->palette_index.len, palette_pos);
			printf("[%i] %i landscape entries\n", i, s3dat_mem->landscape_index.len);
		} else {
			printf("[%i] %i sound entries\n", i, s3dat_mem->sound_index.len);
		}
		if(i+1 != 49) printf("\n");

		if(i == 48) {
			s3dat_sound_t snd_data;
			s3dat_extract_sound(s3dat_mem, 0, 0, &snd_data);

			FILE* fw = fopen("snd.raw", "wb");
			fwrite(snd_data.data, 1, snd_data.len*2, fw);
			fclose(fw);			
		}

		/*if(s3dat_mem->palette_index.len > 0) {
			for(uint32_t p = 0;p != s3dat_mem->palette_index.len;p++) {
				printf("[%i]"
			}
		}*/

		if(s3dat_mem->animation_index.len > 0 && false) {
			for(uint32_t e = 0;e != s3dat_mem->animation_index.len;e++) {
				printf("[%i] nyi %i at %i\n", i, e, s3dat_mem->animation_index.pointers[e]);

				char dump_name[100];
				snprintf(dump_name, 100, "nyi-dump-%i-%i.data", i, e);
				FILE* dump_file = fopen(dump_name, "w");
				s3dat_mem->seek_func(s3dat_mem->io_arg, s3dat_mem->animation_index.pointers[e], SEEK_SET);
				uint32_t count;
				s3dat_mem->read_func(s3dat_mem->io_arg, &count, 4);
				uint32_t len = (count*24)+4;
				//fwrite(&count, 4, 1, dump_file);
				for(int d = 0;d != count;d++) {
					short posx, posy, flag3, flag4;
					int obid = 0, obfile = 0, torsoid = 0, torsofile = 0, shadowid = 0, shadowfile = 0, obframe = 0, torsoframe = 0;
					s3dat_mem->read_func(s3dat_mem->io_arg, &posx, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &posy, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &obid, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &obfile, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &torsoid, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &torsofile, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &shadowid, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &shadowfile, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &obframe, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &torsoframe, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &flag3, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &flag4, 2);
					printf("[%i] %hu %hu %i %i %i %i %i %i %i %i %hu %hu\n", i, posx, posy, obid, obfile, shadowid, shadowfile, obframe, torsoframe, flag3, flag4);
				}
				fclose(dump_file);
				/*printf("[%i] nyi entry:%i at %i\n", i, e, s3dat_mem->animation_index.pointers[e]);*/

				if(e+1 != s3dat_mem->animation_index.len) {
					if(s3dat_mem->animation_index.pointers[e+1]-s3dat_mem->animation_index.pointers[e] != len) printf("WrongWrongWrong %i != %i\n", s3dat_mem->animation_index.pointers[e+1]-s3dat_mem->animation_index.pointers[e], len);
				}
			}
		}

		close(fd);

		s3dat_delete(s3dat_mem);

		i++;
	}
	if(gfx_dir) closedir(gfx_dir);
	if(snd_dir) closedir(snd_dir);
}

