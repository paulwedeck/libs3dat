#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include "s3dat.h"

#define CRASH(msg) \
	do { \
	printf("%s\n", msg); \
	return_value = 1; \
	goto end; \
	} while(0)

int search_gfx() {
	int return_value = 0;
	DIR* dd = opendir("GFX");

	if(dd == NULL) CRASH("Failed to open GFX directory in your build directory\n");

	end:
	if(dd != NULL) closedir(dd);
	return return_value;
}

int open_landscape_file() {
	int return_value = 0;
	s3dat_exception_t* ex = NULL;
	s3dat_t* datfile = NULL;
	s3dat_bitmap_t* bmp = NULL;


	if(search_gfx() != 0) CRASH("test skipped, because search_gfx has failed\n");
	datfile = s3dat_new_malloc();

	s3dat_readfile_name(datfile, "GFX/Siedler3_00.f8007e01f.dat", &ex);
	if(!s3dat_catch_exception(&ex, datfile)) CRASH("couldn`t open GFX/Siedler3_00.f8007e01f.dat !\n");

	bmp = s3dat_extract_landscape(datfile, 0, &ex);
	if(!s3dat_catch_exception(&ex, datfile)) CRASH("couldn`t extract first bitmap\n");

	end:
	if(bmp != NULL) s3dat_delete_bitmap(bmp);
	if(datfile != NULL) s3dat_delete(datfile);

	return return_value;
}

int try_blending() {
	int return_value = 0;
	s3dat_exception_t* ex = NULL;
	s3dat_t* datfile = NULL;
	s3dat_bitmap_t* bmp = NULL;

	if(search_gfx() != 0) CRASH("test skipped, because search_gfx has failed\n");
	datfile = s3dat_new_malloc();

	s3dat_readfile_name(datfile, "GFX/Siedler3_00.f8007e01f.dat", &ex);
	if(!s3dat_catch_exception(&ex, datfile)) CRASH("couldn`t open GFX/Siedler3_00.f8007e01f.dat !\n");

	bmp = s3dat_extract_landscape(datfile, 0x1B, &ex);
	if(!s3dat_catch_exception(&ex, datfile)) CRASH("couldn`t extract bitmap 0x1B\n");

	if(bmp->data[0].red != 0 || bmp->data[0].green != 0xCE || bmp->data[0].blue != 0xEE || bmp->data[0].alpha != 0xFF) CRASH("the bitmap has a wrong color at it first pixel\n");

	s3dat_delete_bitmap(bmp);
	bmp = NULL;

	s3dat_add_landscape_blending(datfile);

	bmp = s3dat_extract_landscape(datfile, 0x1B, &ex);
	if(!s3dat_catch_exception(&ex, datfile)) CRASH("couldn`t extract bitmap 0x1B two times\n");

	if(bmp->data[0].alpha != 0) CRASH("landscape blending failed\n");

	end:
	if(bmp != NULL) s3dat_delete_bitmap(bmp);
	if(datfile != NULL) s3dat_delete(datfile);

	return return_value;
}

typedef struct {
	char* name;
	int (*func) ();
} testcase_t;

testcase_t tests[] = {
	{"search_gfx", search_gfx},
	{"open_landscape", open_landscape_file},
	{"try_blending", try_blending},
	{NULL, NULL},
};

int main(int argc, char** argv) {

	if(argc == 2) {
		int caseid = 0;
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
			printf("all tests:\n");
			while(tests[caseid].name != NULL) {
				printf("\t%s\n", tests[caseid].name);
				caseid++;
			}
			return 0;
		}

		while(tests[caseid].name != NULL) {
			if(strncmp(tests[caseid].name, argv[1], strlen(tests[caseid].name)) == 0) {
				return tests[caseid].func();
			}
			caseid++;
		}
	}

	printf("%s <testname>\n", argv[0]);
	printf("%s [-h] [--help]\n", argv[0]);

	return 0;
}

