#include <s3dat.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <GLFW/glfw3.h>
#include <math.h>

int width = 640, height = 360;


void onresize(GLFWwindow* wnd, int w, int h) {
	width = w;
	height = h;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, -10, 1);
	glMatrixMode(GL_MODELVIEW);
}

void bitmaps_to_textures(int c, s3dat_bitmap_t* bitmaps, int* ida) {
	glGenTextures(c, ida);

	for(int i = 0;i != c;i++) {
		glBindTexture(GL_TEXTURE_2D, ida[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmaps[i].width, bitmaps[i].height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, bitmaps[i].data);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

int main() {
	s3dat_exception_t* ex = NULL;
	int dat00_fd = open("GFX/Siedler3_00.f8007e01f.dat", O_RDONLY);
	int dat20_fd = open("GFX/Siedler3_10.f8007e01f.dat", O_RDONLY);

	s3dat_t* dat00 = s3dat_new_malloc();
	s3dat_t* dat20 = s3dat_new_malloc();

	s3dat_readfile_fd(dat00, dat00_fd, &ex);
	if(ex != NULL) {
		s3dat_print_exception(ex);
		s3dat_delete_exception(dat00, ex);
		ex = NULL;
	}
	s3dat_readfile_fd(dat20, dat20_fd, &ex);
	if(ex != NULL) {
		s3dat_print_exception(ex);
		s3dat_delete_exception(dat00, ex);
		ex = NULL;
	}

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

	s3dat_bitmap_t* grass_bitmap = s3dat_new_bitmap(dat00);
	s3dat_bitmap_t* settler_bitmaps = s3dat_new_bitmaps(dat20, 72);
	s3dat_bitmap_t* torso_bitmaps = s3dat_new_bitmaps(dat20, 72);
	s3dat_extract_landscape(dat00, 0, grass_bitmap, &ex);

	short settler_xoff[72];
	short settler_yoff[72];

	int grass_tex;
	int settler_texs[72];
	int torso_texs[72];

	int ex_s = 0;

	for(int i = 0;i != 72;i++) {
		s3dat_extract_settler(dat20, ex_s, i, settler_bitmaps+i, settler_xoff+i, settler_yoff+i, &ex);
		if(ex != NULL) {
			s3dat_print_exception(ex);
			s3dat_delete_exception(dat00, ex);
			ex = NULL;
		}
		s3dat_extract_torso(dat20, ex_s, i, torso_bitmaps+i, NULL, NULL, &ex);
		if(ex != NULL) {
			s3dat_print_exception(ex);
			s3dat_delete_exception(dat00, ex);
			ex = NULL;
		}
	}

	bitmaps_to_textures(72, settler_bitmaps, settler_texs);
	bitmaps_to_textures(72, torso_bitmaps, torso_texs);
	bitmaps_to_textures(1, grass_bitmap, &grass_tex);

	s3dat_delete_pixdatas(settler_bitmaps, 72);
	s3dat_delete_pixdatas(torso_bitmaps, 72);
	s3dat_delete_pixdata(grass_bitmap);

	int settler_frame = 0xc;

	double time = glfwGetTime();
	double move_value = 0;
	int move_state = 0;

	double move_factor = 0.36;

	while(!glfwWindowShouldClose(wnd)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		int rows = floor(height / grass_bitmap->height)+1;
		int columns = floor(width / grass_bitmap->width)+1;
		for(int row = 0;row != rows;row++) {
			for(int column = 0;column != columns;column++) {
				glBindTexture(GL_TEXTURE_2D, grass_tex);
				glPushMatrix();
				glTranslatef(column*grass_bitmap->width, row*grass_bitmap->height, 0);
				glBegin(GL_QUADS);
				glTexCoord2d(0, 0); glVertex2i(0, 0);
				glTexCoord2d(0, 1); glVertex2i(0, grass_bitmap->height);
				glTexCoord2d(1, 1); glVertex2i(grass_bitmap->width, grass_bitmap->height);
				glTexCoord2d(1, 0); glVertex2i(grass_bitmap->width, 0);
				glEnd();
				glPopMatrix();
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		glBindTexture(GL_TEXTURE_2D, settler_texs[settler_frame]);
		glPushMatrix();
		glTranslatef(590-move_value+settler_xoff[settler_frame]-settler_xoff[0xc], 100+settler_yoff[settler_frame]-settler_yoff[0xc], 1);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2i(0, 0);
		glTexCoord2d(0, 1); glVertex2i(0, settler_bitmaps[settler_frame].height);
		glTexCoord2d(1, 1); glVertex2i(settler_bitmaps[settler_frame].width, settler_bitmaps[settler_frame].height);
		glTexCoord2d(1, 0); glVertex2i(settler_bitmaps[settler_frame].width, 0);
		glEnd();
		glPopMatrix();
		glPushMatrix();
		glColor3f(1, 0, 0);
		glTranslatef(590-move_value+settler_xoff[settler_frame]-settler_xoff[0xc], 100+settler_yoff[settler_frame]-settler_yoff[0xc], 2);
		glBindTexture(GL_TEXTURE_2D, torso_texs[settler_frame]);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2i(0, 0);
		glTexCoord2d(0, 1); glVertex2i(0, settler_bitmaps[settler_frame].height);
		glTexCoord2d(1, 1); glVertex2i(settler_bitmaps[settler_frame].width, settler_bitmaps[settler_frame].height);
		glTexCoord2d(1, 0); glVertex2i(settler_bitmaps[settler_frame].width, 0);
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

	glDeleteTextures(1, &grass_tex);
	glDeleteTextures(72, settler_texs);
	glDeleteTextures(72, torso_texs);
	glfwHideWindow(wnd);
	glfwDestroyWindow(wnd);
	glfwTerminate();
	s3dat_delete_bitmaps(settler_bitmaps, 72);
	s3dat_delete_bitmaps(torso_bitmaps, 72);
	s3dat_delete_bitmap(grass_bitmap);
	s3dat_delete(dat00);
	s3dat_delete(dat20);
	close(dat00_fd);
	close(dat20_fd);
}