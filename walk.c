#include <s3dat_ext.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include <GL/gl.h>

int width = 640, height = 360;

typedef struct {
	s3dat_t* parent;
	int tex_id;
	uint16_t width;
	uint16_t height;
	int16_t xoff;
	int16_t yoff;
} gltex_t;

#define GLTEX(res) ((gltex_t*)res->data.raw)

void delete_texture(gltex_t* tex) {
	glDeleteTextures(1, &tex->tex_id);
	s3util_free_func(s3dat_memset(tex->parent), tex);
}

s3dat_restype_t gl_bitmap_type = {"gltex", (void (*) (void*)) delete_texture, NULL};

void bitmap_to_gl_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	if(res->type != s3dat_landscape && res->type != s3dat_settler && res->type != s3dat_torso && res->type != s3dat_shadow) return;

	S3DAT_CHECK_TYPE(handle, res, "s3dat_bitmap_t", throws, __FILE__, __func__, __LINE__);

	gltex_t* texhandle = s3util_alloc_func(s3dat_memset(handle), sizeof(gltex_t), throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	int tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s3dat_width(res->res), s3dat_height(res->res), 0, GL_RGBA, GL_UNSIGNED_BYTE, s3dat_bmpdata(res->res));
	glBindTexture(GL_TEXTURE_2D, 0);

	texhandle->parent = handle;
	texhandle->tex_id = tex_id;
	texhandle->width = s3dat_width(res->res);
	texhandle->height = s3dat_height(res->res);
	texhandle->xoff = *s3dat_xoff(res->res);
	texhandle->yoff = *s3dat_yoff(res->res);

	res->res->type->deref(res->res->data.raw);

	res->res->data.raw = texhandle;
	res->res->type = &gl_bitmap_type;
}

void onresize(GLFWwindow* wnd, int w, int h) {
	width = w;
	height = h;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -10, 1);
	glMatrixMode(GL_MODELVIEW);
}

void draw_texture(s3dat_ref_t* ref, double x, double y, double z, double cr, double cg, double cb, double ca, bool flip) {
		glColor4d(cr, cg, cb, ca);
		glBindTexture(GL_TEXTURE_2D, GLTEX(ref)->tex_id);

		glPushMatrix();
		glTranslated(x, y, z);
		glBegin(GL_QUADS);
		glTexCoord2d(0, flip); glVertex2i(0, 0);
		glTexCoord2d(0, !flip); glVertex2i(0, GLTEX(ref)->height);
		glTexCoord2d(1, !flip); glVertex2i(GLTEX(ref)->width, GLTEX(ref)->height);
		glTexCoord2d(1, flip); glVertex2i(GLTEX(ref)->width, 0);
		glEnd();
		glPopMatrix();
}

int main() {
	s3util_exception_t* ex = NULL;

	s3dat_t* dat00 = s3dat_new_malloc();
	s3dat_t* dat10 = s3dat_new_malloc();

	s3dat_readfile_name(dat00, "GFX/Siedler3_00.f8007e01f.dat", &ex);
	s3util_catch_exception(&ex);

	s3dat_readfile_name(dat10, "GFX/Siedler3_10.f8007e01f.dat", &ex);
	s3util_catch_exception(&ex);

	glfwInit();

	GLFWwindow* wnd = glfwCreateWindow(width, height, "walk", NULL, NULL);
	glfwSetFramebufferSizeCallback(wnd, onresize);
	glfwMakeContextCurrent(wnd);
	glfwSwapInterval(1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	onresize(wnd, width, height);

	s3dat_extracthandler_t* exhandler1 = s3dat_new_exhandler(dat00, &ex);
	s3util_catch_exception(&ex);
	exhandler1->call = bitmap_to_gl_handler;

	s3dat_extracthandler_t* exhandler2 = s3dat_new_exhandler(dat10, &ex);
	s3util_catch_exception(&ex);
	exhandler2->call = bitmap_to_gl_handler;

	s3dat_add_extracthandler(dat00, exhandler1);
	s3dat_add_extracthandler(dat10, exhandler2);

	s3dat_add_cache(dat00, &ex);
	s3util_catch_exception(&ex);

	s3dat_add_cache(dat10, &ex);
	s3util_catch_exception(&ex);

	s3dat_ref_t* grass_tex = s3dat_extract_landscape(dat00, 0, &ex);
	s3util_catch_exception(&ex);

	s3dat_ref_t* settler_texs[72];
	s3dat_ref_t* torso_texs[72];
	s3dat_ref_t* shadow_texs[72];

	memset(settler_texs, 0, sizeof(s3dat_ref_t*)*72);
	memset(torso_texs, 0, sizeof(s3dat_ref_t*)*72);
	memset(shadow_texs, 0, sizeof(s3dat_ref_t*)*72);

	int ex_s = 4;
	int last_s = -1;
	double last_time = glfwGetTime();

	int settler_frame = 0xc;

	double time = glfwGetTime();
	double move_value = 0;
	//int move_state = 0;

	double move_factor = 0.36;

	while(!glfwWindowShouldClose(wnd)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();


		if(glfwGetTime() > last_time+2) {
			last_time = glfwGetTime();
			//ex_s++;
		}

		if(ex_s != last_s) {
			for(int i = 0;i != 72;i++) {
				if(settler_texs[i]) s3dat_unref(settler_texs[i]);
				settler_texs[i] = s3dat_extract_settler(dat10, ex_s, i, &ex);
				s3util_catch_exception(&ex);

				if(torso_texs[i]) s3dat_unref(torso_texs[i]);
				torso_texs[i] = s3dat_extract_torso(dat10, ex_s, i, &ex);
				s3util_catch_exception(&ex);

				if(shadow_texs[i]) s3dat_unref(shadow_texs[i]);
				shadow_texs[i] = s3dat_extract_shadow(dat10, ex_s, 71-i, &ex);
				s3util_catch_exception(&ex);
			}

			last_s = ex_s;
		}

		int rows = floor(height / GLTEX(grass_tex)->height)+1;
		int columns = floor(width / GLTEX(grass_tex)->width)+1;
		for(int row = 0;row != rows;row++) {
			for(int column = 0;column != columns;column++) {
				draw_texture(grass_tex, column*GLTEX(grass_tex)->width, row*GLTEX(grass_tex)->height, 0, 1, 1, 1, 1, false);
			}
		}

		draw_texture(shadow_texs[settler_frame], 590-move_value+GLTEX(shadow_texs[settler_frame])->xoff, 100+GLTEX(shadow_texs[settler_frame])->yoff, 1, 1, 0, 0, 0.5, true);
		draw_texture(settler_texs[settler_frame], 590-move_value+GLTEX(settler_texs[settler_frame])->xoff, 100+GLTEX(settler_texs[settler_frame])->yoff, 2, 1, 1, 1, 1, false);
		draw_texture(torso_texs[settler_frame], 590-move_value+GLTEX(settler_texs[settler_frame])->xoff, 100+GLTEX(settler_texs[settler_frame])->yoff, 3, 1, 0, 0, 1, false);

		glfwSwapBuffers(wnd);
		glfwPollEvents();

		if(glfwGetTime() >= time+0.08) {
			time = glfwGetTime();
			settler_frame++;
			if(settler_frame == 0x39) settler_frame = 0x30;
			if(settler_frame == 0x17) settler_frame = 0xc;

			if(floor(move_value) > 540 && move_factor > 0) {
				move_factor = -move_factor;

				settler_frame = 0x30;
			}
			if(floor(move_value) < 0 && move_factor < 0) {
				move_factor = -move_factor;

				settler_frame = 0xc;
			}
		}
		// right: 0x30-0x39
		// left: 0xc-0x18
		move_value += move_factor;
	}

	s3dat_unref(grass_tex);
	s3dat_unref_array(settler_texs, 72);
	s3dat_unref_array(torso_texs, 72);
	s3dat_unref_array(shadow_texs, 72);

	glfwHideWindow(wnd);
	glfwDestroyWindow(wnd);
	glfwTerminate();
	s3dat_delete(dat00);
	s3dat_delete(dat10);
}
