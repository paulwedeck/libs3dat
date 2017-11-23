#include "s3dat.h"

void invert_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	s3dat_bitmap_t* bmp = res->resdata;
	for(uint16_t i = 0;i != bmp->width*bmp->height;i++) {
		s3dat_color_t* color = bmp->data+i;
		color->red = 256-color->red;
		color->green = 256-color->green;
		color->blue = 256-color->blue;
	}
}

s3dat_exception_t* invert_ex;
s3dat_exception_t** throws;

int main() {
	throws = &invert_ex;

	s3dat_t* read = s3dat_new_malloc();
	s3dat_t* write = NULL;
 
	s3dat_readfile_name(read, "GFX/Siedler3_10.f8007e01f.dat", throws);
	if(!s3dat_catch_exception(throws, read)) {
		goto end;
	}

	s3dat_extracthandler_t* handlers[2] = {s3dat_new_exhandler(read), s3dat_new_exhandler(read)};

	handlers[0]->call = invert_handler;
	handlers[1]->call = s3dat_pack_handler;

	s3dat_add_extracthandler(read, handlers[0]);
	s3dat_add_extracthandler(read, handlers[1]);

	write = s3dat_writeable_fork(read, "GFX/Siedler3_10.f8007e01f.dat.invert");
	s3dat_writefile(write, throws);
	if(!s3dat_catch_exception(throws, read)) {
		goto end;
	}

	end:
	s3dat_delete_fork(write);
	s3dat_delete(read);
	return 0;
}
