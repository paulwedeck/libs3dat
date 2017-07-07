#include "s3dat.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>

int main() {
	char* files[49] = {
		"GFX/Siedler3_00.f8007e01f.dat", "GFX/Siedler3_12.f8007e01f.dat", "GFX/Siedler3_25.f8007e01f.dat", "GFX/Siedler3_41.f8007e01f.dat",
		"GFX/Siedler3_01.f8007e01f.dat", "GFX/Siedler3_13.f8007e01f.dat", "GFX/Siedler3_26.f8007e01f.dat", "GFX/siedler3_42.f8007e01f.dat",
		"GFX/Siedler3_02.f8007e01f.dat", "GFX/Siedler3_14.f8007e01f.dat", "GFX/Siedler3_27.f8007e01f.dat", "GFX/Siedler3_43.f8007e01f.dat",
		"GFX/Siedler3_03.f8007e01f.dat", "GFX/Siedler3_15.f8007e01f.dat", "GFX/Siedler3_30.f8007e01f.dat", "GFX/Siedler3_44.f8007e01f.dat",
		"GFX/Siedler3_04.f8007e01f.dat", "GFX/Siedler3_16.f8007e01f.dat", "GFX/Siedler3_31.f8007e01f.dat", "GFX/Siedler3_45.f8007e01f.dat",
		"GFX/Siedler3_05.f8007e01f.dat", "GFX/Siedler3_17.f8007e01f.dat", "GFX/Siedler3_32.f8007e01f.dat", "GFX/Siedler3_46.f8007e01f.dat",
		"GFX/Siedler3_06.f8007e01f.dat", "GFX/Siedler3_18.f8007e01f.dat", "GFX/Siedler3_33.f8007e01f.dat", "GFX/Siedler3_47.f8007e01f.dat",
		"GFX/Siedler3_07.f8007e01f.dat", "GFX/Siedler3_20.f8007e01f.dat", "GFX/Siedler3_34.f8007e01f.dat", "GFX/Siedler3_48.f8007e01f.dat",
		"GFX/siedler3_08.f8007e01f.dat", "GFX/Siedler3_21.f8007e01f.dat", "GFX/Siedler3_35.f8007e01f.dat", "GFX/Siedler3_60.f8007e01f.dat",
		"GFX/siedler3_09.f8007e01f.dat", "GFX/Siedler3_22.f8007e01f.dat", "GFX/Siedler3_36.f8007e01f.dat", "GFX/siedler3_61.f8007e01f.dat",
		"GFX/Siedler3_10.f8007e01f.dat", "GFX/Siedler3_23.f8007e01f.dat", "GFX/Siedler3_37.f8007e01f.dat", "GFX/Siedler3_99.f8007e01f.dat",
		"GFX/Siedler3_11.f8007e01f.dat", "GFX/Siedler3_24.f8007e01f.dat", "GFX/Siedler3_40.f8007e01f.dat", 
		"SND/Siedler3_00.dat", "SND/Siedler3_01.dat",
	};

	for(int i = 0;i != 49;i++) {

		s3dat_t* s3dat_mem = s3dat_new_malloc();

		int fd = open(files[i], O_RDONLY);

		int error = s3dat_readfile_fd(s3dat_mem, fd);
		printf("[%i] new file %s\n", i, files[i]);
		if(error != 0) {
			printf("%i\n", error);
		} else if(s3dat_mem->sound_index.len == 0) {
			printf("[%i] %i settler sequences\n", i, s3dat_mem->settler_index.len);
			printf("[%i] %i shadow sequences\n", i, s3dat_mem->shadow_index.len);
			printf("[%i] %i torso sequences\n", i, s3dat_mem->torso_index.len);
			printf("[%i] %i gui entries\n", i, s3dat_mem->gui_index.len);
			printf("[%i] %i nyi entries\n", i, s3dat_mem->nyi_index.len);
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

		close(fd);

		s3dat_delete(s3dat_mem);
	}
}

