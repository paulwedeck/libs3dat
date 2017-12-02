#include <s3dat.h>

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

void delete_texture(gltex_t* tex) {
	glDeleteTextures(1, &tex->tex_id);
	tex->parent->free_func(tex->parent->mem_arg, tex);
}

s3dat_restype_t gl_bitmap_type = {"gltex", (void (*) (void*)) delete_texture};

void bitmap_to_gl_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3dat_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	S3DAT_CHECK_TYPE(handle, res, "s3dat_bitmap_t", throws, __FILE__, __func__, __LINE__);

	s3dat_bitmap_t* bitmap = res->resdata;

	int tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width, bitmap->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->data);
	glBindTexture(GL_TEXTURE_2D, 0);

	gltex_t* texhandle = handle->alloc_func(handle->mem_arg, sizeof(gltex_t));
	texhandle->parent = handle;
	texhandle->tex_id = tex_id;
	texhandle->width = bitmap->width;
	texhandle->height = bitmap->height;
	texhandle->xoff = bitmap->xoff;
	texhandle->yoff = bitmap->yoff;

	s3dat_delete_bitmap(bitmap);

	res->resdata = texhandle;
	res->restype = &gl_bitmap_type;
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

int main() {
	s3dat_exception_t* ex = NULL;

	s3dat_t* dat00 = s3dat_new_malloc();
	s3dat_t* dat10 = s3dat_new_malloc();

	s3dat_readfile_name(dat00, "GFX/Siedler3_00.f8007e01f.dat", &ex);
	s3dat_catch_exception(&ex, dat00);

	s3dat_readfile_name(dat10, "GFX/Siedler3_10.f8007e01f.dat", &ex);
	s3dat_catch_exception(&ex, dat10);

	glfwInit();

	GLFWwindow* wnd = glfwCreateWindow(width, height, "walk", NULL, NULL);
	glfwSetFramebufferSizeCallback(wnd, onresize);
	glfwMakeContextCurrent(wnd);
	glfwSwapInterval(1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.9);

	onresize(wnd, width, height);

	s3dat_extracthandler_t* exhandler1 = s3dat_new_exhandler(dat00), *exhandler2 = s3dat_new_exhandler(dat10);
	exhandler1->call = bitmap_to_gl_handler;
	exhandler2->call = bitmap_to_gl_handler;

	s3dat_add_extracthandler(dat00, exhandler1);
	s3dat_add_extracthandler(dat10, exhandler2);

	s3dat_add_cache(dat00);
	s3dat_add_cache(dat10);

	gltex_t* grass_tex = (gltex_t*) s3dat_extract_landscape(dat00, 0, &ex);
	s3dat_catch_exception(&ex, dat00);

	gltex_t* settler_texs[72];
	gltex_t* torso_texs[72];

	int ex_s = 0;

	for(int i = 0;i != 72;i++) {
		settler_texs[i] = (gltex_t*) s3dat_extract_settler(dat10, ex_s, i, &ex);
		s3dat_catch_exception(&ex, dat10);

		torso_texs[i] = (gltex_t*) s3dat_extract_torso(dat10, ex_s, i, &ex);
		s3dat_catch_exception(&ex, dat10);
	}

	int settler_frame = 0xc;

	double time = glfwGetTime();
	double move_value = 0;
	int move_state = 0;

	double move_factor = 0.36;

	while(!glfwWindowShouldClose(wnd)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		int rows = floor(height / grass_tex->height)+1;
		int columns = floor(width / grass_tex->width)+1;
		for(int row = 0;row != rows;row++) {
			for(int column = 0;column != columns;column++) {
				glBindTexture(GL_TEXTURE_2D, grass_tex->tex_id);
				glPushMatrix();
				glTranslatef(column*grass_tex->width, row*grass_tex->height, 0);
				glBegin(GL_QUADS);
				glTexCoord2d(0, 0); glVertex2i(0, 0);
				glTexCoord2d(0, 1); glVertex2i(0, grass_tex->height);
				glTexCoord2d(1, 1); glVertex2i(grass_tex->width, grass_tex->height);
				glTexCoord2d(1, 0); glVertex2i(grass_tex->width, 0);
				glEnd();
				glPopMatrix();
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		glBindTexture(GL_TEXTURE_2D, settler_texs[settler_frame]->tex_id);
		glPushMatrix();
		glTranslatef(590-move_value+settler_texs[settler_frame]->xoff-settler_texs[0xc]->xoff, 100+settler_texs[settler_frame]->yoff-settler_texs[0xc]->yoff, 1);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2i(0, 0);
		glTexCoord2d(0, 1); glVertex2i(0, settler_texs[settler_frame]->height);
		glTexCoord2d(1, 1); glVertex2i(settler_texs[settler_frame]->width, settler_texs[settler_frame]->height);
		glTexCoord2d(1, 0); glVertex2i(settler_texs[settler_frame]->width, 0);
		glEnd();
		glPopMatrix();
		glPushMatrix();
		glColor3f(1, 0, 0);
		glTranslatef(590-move_value+settler_texs[settler_frame]->xoff-settler_texs[0xc]->xoff, 100+settler_texs[settler_frame]->yoff-settler_texs[0xc]->yoff, 2);
		glBindTexture(GL_TEXTURE_2D, torso_texs[settler_frame]->tex_id);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2i(0, 0);
		glTexCoord2d(0, 1); glVertex2i(0, settler_texs[settler_frame]->height);
		glTexCoord2d(1, 1); glVertex2i(settler_texs[settler_frame]->width, settler_texs[settler_frame]->height);
		glTexCoord2d(1, 0); glVertex2i(settler_texs[settler_frame]->width, 0);
		glEnd();
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(1, 1, 1);

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

	glfwHideWindow(wnd);
	glfwDestroyWindow(wnd);
	glfwTerminate();
	s3dat_delete(dat00);
	s3dat_delete(dat10);
}
