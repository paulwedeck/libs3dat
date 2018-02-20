#include <s3dat_ext.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

int width = 640, height = 360;

s3util_exception_t* ex;


typedef struct {
	s3dat_t* parent;
	GLuint tex_id;
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

	GLuint tex_id;
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

void draw_texture(s3dat_ref_t* ref, double x, double y, double z, double cr, double cg, double cb, double ca, bool flip) {
		glColor4d(cr, cg, cb, ca);
		glBindTexture(GL_TEXTURE_2D, GLTEX(ref)->tex_id);

		glPushMatrix();
		glTranslated(x+GLTEX(ref)->xoff, y+GLTEX(ref)->yoff, z);
		glBegin(GL_QUADS);
		glTexCoord2d(0, flip); glVertex2i(0, 0);
		glTexCoord2d(0, !flip); glVertex2i(0, GLTEX(ref)->height);
		glTexCoord2d(1, !flip); glVertex2i(GLTEX(ref)->width, GLTEX(ref)->height);
		glTexCoord2d(1, flip); glVertex2i(GLTEX(ref)->width, 0);
		glEnd();
		glPopMatrix();
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

int main(int argc, char** argv) {
	s3dat_t* ani_file = NULL;
	s3dat_ref_t* ani = NULL;

	s3dat_t* img_gfx = NULL;

	uint32_t ani_index = 0;
	char* ani_file_name = "GFX/Siedler3_25.f8007e01f.dat";
	if(argc == 3) {
		ani_index = atoi(argv[2]);
		ani_file_name = argv[1];
	} else if(argc > 1) {
		printf("%s [file] [ani_index]\n", argv[0]);
		return 1;
	}

	ani_file = s3dat_new_malloc();
	s3dat_readfile_name(ani_file, ani_file_name, &ex);
	s3util_catch_exception(&ex);

	ani = s3dat_extract_animation(ani_file, ani_index, &ex);
	s3util_catch_exception(&ex);

	uint32_t ani_len = s3dat_anilen(ani);

	uint32_t file = ani_len > 0 ? s3dat_frame(ani, 0)->settler_file : 0;


	char img_name[31];
	snprintf(img_name, 30, "GFX/Siedler3_%.2i.f8007e01f.dat", file);
	img_gfx = s3dat_new_malloc();
	s3dat_readfile_name(img_gfx, img_name, &ex);
	s3util_catch_exception(&ex);

	s3dat_extracthandler_t* exhandler = s3dat_new_exhandler(img_gfx, &ex);
	s3util_catch_exception(&ex);
	exhandler->call = bitmap_to_gl_handler;
	s3dat_add_extracthandler(img_gfx, exhandler);

	s3dat_ref_t* settler_frames[ani_len];
	s3dat_ref_t* torso_frames[ani_len];
	s3dat_ref_t* shadow_frames[ani_len];

	glfwInit();

	GLFWwindow* wnd = glfwCreateWindow(width, height, "animation", NULL, NULL);
	glfwSetFramebufferSizeCallback(wnd, onresize);
	glfwMakeContextCurrent(wnd);
	glfwSwapInterval(1);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	onresize(wnd, width, height);

	for(uint32_t i = 0;i != ani_len;i++) {
		s3dat_frame_t* frame = s3dat_frame(ani, i);
		printf("%i:%i:%i %i:%i:%i %i:%i:%i\n", frame->settler_file, frame->settler_id, frame->settler_frame,
					frame->torso_file, frame->torso_id, frame->torso_frame,
					frame->shadow_file, frame->shadow_id, 0);

		settler_frames[i] = s3dat_extract_settler(img_gfx, frame->settler_id, frame->settler_frame, &ex);
		s3util_catch_exception(&ex);

		if(frame->torso_id != 65535) {
			torso_frames[i] = s3dat_extract_torso(img_gfx, frame->torso_id, frame->torso_frame, &ex);
			s3util_catch_exception(&ex);
		} else {
			torso_frames[i] = NULL;
		}

		shadow_frames[i] = s3dat_extract_shadow(img_gfx, frame->shadow_id, frame->torso_frame, &ex);	
		s3util_catch_exception(&ex);
	}

	double time = glfwGetTime();
	uint32_t frame = 0;

	glClearColor(0.8, 0.8, 0.8, 1);
	while(!glfwWindowShouldClose(wnd)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		draw_texture(shadow_frames[frame], 100, 100, 0, 1, 1, 0, 1, false);
		draw_texture(settler_frames[frame], 100, 100, 1, 1, 1, 1, 1, false);
		if(torso_frames[frame]) draw_texture(torso_frames[frame], 100, 100, 2, 1, 1, 0, 1, false);

		if(glfwGetTime()>time+0.08) {
			time = glfwGetTime();
			frame++;
			frame %= ani_len;
		}

		glfwSwapBuffers(wnd);
		glfwPollEvents();
	}

	if(ani != NULL) s3dat_unref(ani);
	if(ani_file != NULL) s3dat_delete(ani_file);

	for(uint32_t i = 0;i != ani_len;i++) {
		s3dat_unref(settler_frames[i]);
		if(torso_frames[i]) s3dat_unref(torso_frames[i]);
		s3dat_unref(shadow_frames[i]);
	}

	if(img_gfx != NULL) s3dat_delete(img_gfx);

	glfwTerminate();

}
