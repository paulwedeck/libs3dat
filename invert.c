#include "s3dat_ext.h"

void invert_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	int pixel_count = s3dat_width(res->res)*s3dat_height(res->res);

	s3dat_color_t* bmp_data = s3dat_bmpdata(res->res);
	for(uint16_t i = 0;i != pixel_count;i++) {
		bmp_data[i].red = 256-bmp_data[i].red;
		bmp_data[i].green = 256-bmp_data[i].green;
		bmp_data[i].blue = 256-bmp_data[i].blue;
	}
}

s3util_exception_t* invert_ex;
s3util_exception_t** throws;

int main() {
	throws = &invert_ex;

	s3dat_t* read = s3dat_new_malloc();
	s3dat_t* write = NULL;

	s3dat_readfile_name(read, "GFX/Siedler3_10.f8007e01f.dat", throws);
	if(!s3util_catch_exception(throws)) {
		goto end;
	}

	s3dat_extracthandler_t* invert_ex = s3dat_new_exhandler(read, throws);
	s3dat_extracthandler_t* pack_ex = s3dat_new_exhandler(read, throws);

	invert_ex->call = invert_handler;
	pack_ex->call = s3dat_pack_handler;

	s3dat_add_extracthandler(read, invert_ex);
	s3dat_add_extracthandler(read, pack_ex);

	write = s3dat_writeable_fork(read, "GFX/Siedler3_10.f8007e01f.dat.invert", throws);
	if(!s3util_catch_exception(throws)) goto end;

	s3dat_writefile(write, throws);
	s3util_catch_exception(throws);

	end:
	s3dat_delete_fork(write);
	s3dat_delete(read);
	return 0;
}
