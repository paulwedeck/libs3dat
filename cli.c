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
			printf("[%i] %i palette entries\n", i, s3dat_mem->palette_index.len);
			printf("[%i] %i landscape entries\n", i, s3dat_mem->landscape_index.len);
		} else {
			printf("[%i] %i sound entries\n", i, s3dat_mem->sound_index.len);
		}

		if(s3dat_mem->animation_index.len > 0 && false) {
			for(uint32_t e = 0;e != s3dat_mem->animation_index.len;e++) {
				printf("[%i] animation %i at %i\n", i, e, s3dat_mem->animation_index.pointers[e]);

				s3dat_mem->seek_func(s3dat_mem->io_arg, s3dat_mem->animation_index.pointers[e], SEEK_SET);
				uint32_t count;
				s3dat_mem->read_func(s3dat_mem->io_arg, &count, 4);

				for(int d = 0;d != count;d++) {
					short posx, posy;
					unsigned short settlerid = 0, settlerfile = 0, torsoid = 0, torsofile = 0, shadowid = 0, shadowfile = 0, settlerframe = 0, torsoframe = 0, flag3 = 0, flag4 = 0;
					s3dat_mem->read_func(s3dat_mem->io_arg, &posx, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &posy, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &settlerid, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &settlerfile, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &torsoid, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &torsofile, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &shadowid, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &shadowfile, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &settlerframe, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &torsoframe, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &flag3, 2);
					s3dat_mem->read_func(s3dat_mem->io_arg, &flag4, 2);
					printf("[%i] x=%hi y=%hi sfile=%hu sid=%hu sframe=%hu tfile=%hu tid=%hu  tframe=%hu hfile=%hu hid=%hu flags={0x%x,0x%x}\n", i, posx, posy, settlerfile, settlerid, settlerframe, torsofile, torsoid, torsoframe, shadowfile, shadowid, flag3, flag4);
				}
			}
		}
		if(i+1 != 49) printf("\n");

		close(fd);

		s3dat_delete(s3dat_mem);

		i++;
	}
	if(gfx_dir) closedir(gfx_dir);
	if(snd_dir) closedir(snd_dir);
}

