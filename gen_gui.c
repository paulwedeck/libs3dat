#include <s3dat_ext.h>
#include <math.h>

void create_color_gui_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws) {
	res->res = s3dat_new_bitmap(me->parent, 64, 64, throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(me->parent), throws, __FILE__, __func__, __LINE__);

	s3util_color_t color = {0, 0, 0, 0xFF};
	for(uint16_t x = 0;x != 64;x++) {
		for(uint16_t y = 0;y != 64;y++) {
			if(x != 0) color.red = abs(32-x)*8;
			color.green = (cos(x/8.0*M_PI)+1)*127;
			color.blue = (cos(y/4.0*M_PI)+1)*127;
			s3dat_bmpdata(res->res)[y*64+x] = color;
		}
	}
}

s3util_exception_t* ex;
int main() {
	s3dat_t* datfile = s3dat_new_malloc();

	s3dat_setindexlen(datfile, s3dat_gui, 1);

	s3dat_extracthandler_t* createhandler = s3dat_new_exhandler(datfile, &ex);
	s3util_catch_exception(&ex);

	s3dat_extracthandler_t* packhandler = s3dat_new_exhandler(datfile, &ex);
	s3util_catch_exception(&ex);

	createhandler->call = create_color_gui_handler;
	packhandler->call = s3dat_pack_handler;

	s3dat_add_extracthandler(datfile, createhandler);
	s3dat_add_extracthandler(datfile, packhandler);

	s3dat_writefile_name(datfile, "GFX/color_gui.dat", &ex);
	s3util_catch_exception(&ex);

	s3dat_delete(datfile);
}
