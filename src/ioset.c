#include "s3dat_internal.h"

#ifdef __linux__
void* s3dat_linux_open_func(void* arg) {
	int fd = open(arg, O_RDONLY);

	if(fd == -1) {
		return NULL;
	}

	int* fd_p = calloc(1, sizeof(int));
	if(fd_p != NULL) *fd_p = fd;
		else close(fd);

	return fd_p;
}

void s3dat_linux_close_func(void* arg) {
	close(*(int*)(arg));
	free(arg);
}

bool s3dat_linux_read_func(void* arg, void* bfr, size_t len) {
	return read(*((int*)arg), bfr, len) == len;
}

size_t s3dat_linux_size_func(void* arg) {
	struct stat file_stat;
	fstat(*((int*)arg), &file_stat);
	return file_stat.st_size;
}

size_t s3dat_linux_pos_func(void* arg) {
	return lseek(*((int*)arg), 0, SEEK_CUR);
}

bool s3dat_linux_seek_func(void* arg, uint32_t pos, int whence) {
	int seek_whence = whence == S3DAT_SEEK_CUR ? SEEK_CUR : SEEK_SET;
	return lseek(*((int*)arg), pos,  seek_whence) != (off_t)-1;
}
#else
void* s3dat_linux_open_func(void* arg) {}
void s3dat_linux_close_func(void* arg) {}
bool s3dat_linux_read_func(void* arg, void* bfr, size_t len) {return 0;}
bool s3dat_linux_seek_func(void* arg, uint32_t pos, int whence) {return 0;}
size_t s3dat_linux_pos_func(void* arg) {return 0;}
size_t s3dat_linux_size_func(void* arg) {return 0;}
#endif

#ifdef _WIN32
void* s3dat_win32_open_func(void* arg) {
	HANDLE file_handle = CreateFile(arg,
		GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file_handle == INVALID_HANDLE_VALUE) return NULL;
	else return file_handle;
}

void s3dat_win32_close_func(void* arg) {
	CloseHandle(arg);
}

bool s3dat_win32_read_func(void* arg, void* bfr, size_t len) {
	//return fread(bfr, 1, len, arg) == len;
	return ReadFile(arg, bfr, len, NULL, NULL);
}

bool s3dat_win32_seek_func(void* arg, uint32_t pos, int whence) {
	DWORD seek_whence = whence == S3DAT_SEEK_CUR ? FILE_CURRENT : FILE_BEGIN;
	return SetFilePointer(arg, pos, NULL, seek_whence) != INVALID_SET_FILE_POINTER;
}

size_t s3dat_win32_pos_func(void* arg) {
	return SetFilePointer(arg, 0, NULL, FILE_CURRENT);
}

size_t s3dat_win32_size_func(void* arg) {
	DWORD pos = SetFilePointer(arg, 0, NULL, FILE_CURRENT);

	size_t size = SetFilePointer(arg, 0, NULL, FILE_END); // TODO make this portable
	SetFilePointer(arg, pos, NULL, FILE_BEGIN);

	return size;
}
#else
void* s3dat_win32_open_func(void* arg) {}
void s3dat_win32_close_func(void* arg) {}
bool s3dat_win32_read_func(void* arg, void* bfr, size_t len) {return 0;}
bool s3dat_win32_seek_func(void* arg, uint32_t pos, int whence) {return 0;}
size_t s3dat_win32_pos_func(void* arg) {return 0;}
size_t s3dat_win32_size_func(void* arg) {return 0;}
#endif

void* s3dat_libc_open_func(void* arg) {
	return fopen(arg, "rb");
}

void s3dat_libc_close_func(void* arg) {
	fclose(arg);
}

bool s3dat_libc_read_func(void* arg, void* bfr, size_t len) {
	return fread(bfr, 1, len, arg) == len;
}

bool s3dat_libc_seek_func(void* arg, uint32_t pos, int whence) {
	int seek_whence = whence == S3DAT_SEEK_CUR ? SEEK_CUR : SEEK_SET;
	return fseek(arg, pos, seek_whence) != -1;
}

size_t s3dat_libc_pos_func(void* arg) {
	return ftell(arg);
}

size_t s3dat_libc_size_func(void* arg) {
	size_t pos = ftell(arg);

	fseek(arg, 0, SEEK_END); // TODO make this portable
	size_t size = ftell(arg);
	fseek(arg, pos, SEEK_SET);

	return size;
}

#ifdef __linux__
void* s3dat_mmf_linux_fd_open_func(void* arg) {
	s3dat_mmf_t* mmf = calloc(1, sizeof(s3dat_mmf_t));
	mmf->len = s3dat_linux_size_func(arg);
	mmf->addr = mmap(NULL, mmf->len, PROT_READ, MAP_PRIVATE, *(int*)arg, 0);

	return mmf;
}

void* s3dat_mmf_linux_name_open_func(void* arg) {
	int fd = open(arg, O_RDONLY);
	if(fd == -1) return NULL;

	s3dat_mmf_t* mmf = s3dat_mmf_linux_fd_open_func(&fd);
	close(fd);

	return mmf;
}

void s3dat_mmf_linux_close_func(void* varg) {
	s3dat_mmf_t* arg = varg;

	if(!arg->fork) munmap(arg->addr, arg->len);
	free(arg);
}
#else
void* s3dat_mmf_linux_fd_open_func(void* arg) {}
void* s3dat_mmf_linux_name_open_func(void* arg) {}
void s3dat_mmf_linux_close_func(void* varg) {}
#endif

#ifdef _WIN32
typedef struct {
	HANDLE win32fm;
	HANDLE file;
} s3dat_win32_add_t;

void* s3dat_mmf_win32_handle_open_func(void* arg) {
	HANDLE win32fm = CreateFileMapping(arg, NULL, PAGE_READONLY, 0, 0, NULL);

	s3dat_mmf_t* mmf = calloc(1, sizeof(s3dat_mmf_t));

	mmf->addr = MapViewOfFile(win32fm, FILE_MAP_READ, 0, 0, 0);
	mmf->len = s3dat_win32_size_func(arg);
	mmf->additional_data = calloc(1, sizeof(s3dat_win32_add_t));
	((s3dat_win32_add_t*)mmf->additional_data)->win32fm = win32fm;
	((s3dat_win32_add_t*)mmf->additional_data)->file = NULL;

	return mmf;
}

void* s3dat_mmf_win32_name_open_func(void* arg) {
	HANDLE file = s3dat_win32_open_func(arg);
	if(file == NULL) return NULL;

	s3dat_mmf_t* mmf = s3dat_mmf_win32_handle_open_func(file);
	if(mmf == NULL) return NULL;

	((s3dat_win32_add_t*)mmf->additional_data)->file = file;
	return mmf;
}

void s3dat_mmf_win32_close_func(void* varg) {
	s3dat_mmf_t* arg = varg;

	if(!arg->fork) {
		UnmapViewOfFile(arg->addr);

		CloseHandle(((s3dat_win32_add_t*)arg->additional_data)->win32fm);
		if(((s3dat_win32_add_t*)arg->additional_data)->file) CloseHandle(((s3dat_win32_add_t*)arg->additional_data)->file);

		free(arg->additional_data);
	}
	free(arg);
}
#else
void* s3dat_mmf_win32_handle_open_func(void* arg) {}
void* s3dat_mmf_win32_name_open_func(void* arg) {}
void s3dat_mmf_win32_close_func(void* arg) {}
#endif

bool s3dat_mmf_read_func(void* varg, void* bfr, size_t len) {
	s3dat_mmf_t* arg = varg;

	if(arg->len < arg->pos+len) return false;

	memcpy(bfr, arg->addr+arg->pos, len);

	arg->pos += len;

	return true;
}

bool s3dat_mmf_seek_func(void* varg, uint32_t pos, int whence) {
	s3dat_mmf_t* arg = varg;

	uint32_t from = whence == S3DAT_SEEK_SET ? 0 : arg->pos;

	if(arg->len < (pos+from)) return false;
	arg->pos = pos+from;

	return true;
}

size_t s3dat_mmf_pos_func(void* varg) {
	s3dat_mmf_t* arg = varg;
	return arg->pos;
}

size_t s3dat_mmf_size_func(void* varg) {
	s3dat_mmf_t* arg = varg;
	return arg->len;
}

void* s3dat_mmf_fork_func(void* varg) {
	s3dat_mmf_t* arg = varg;

	s3dat_mmf_t* fork = calloc(1, sizeof(s3dat_mmf_t));

	fork->fork = true;

	return fork;
}

s3dat_ioset_t s3dat_internal_linux_ioset = {
	s3dat_linux_read_func,
	s3dat_linux_size_func,
	s3dat_linux_pos_func,
	s3dat_linux_seek_func,
	s3dat_linux_open_func,
	s3dat_linux_close_func,
	NULL,
	#ifdef __linux__
	true
	#else
	false
	#endif
};

s3dat_ioset_t s3dat_internal_mmf_linux_name_ioset = {
	s3dat_mmf_read_func,
	s3dat_mmf_size_func,
	s3dat_mmf_pos_func,
	s3dat_mmf_seek_func,
	s3dat_mmf_linux_name_open_func,
	s3dat_mmf_linux_close_func,
	s3dat_mmf_fork_func,
	#ifdef __linux__
	true
	#else
	false
	#endif
};

s3dat_ioset_t s3dat_internal_mmf_linux_fd_ioset = {
	s3dat_mmf_read_func,
	s3dat_mmf_size_func,
	s3dat_mmf_pos_func,
	s3dat_mmf_seek_func,
	s3dat_mmf_linux_fd_open_func,
	s3dat_mmf_linux_close_func,
	s3dat_mmf_fork_func,
	#ifdef __linux__
	true
	#else
	false
	#endif
};


s3dat_ioset_t s3dat_internal_win32_ioset = {
	s3dat_win32_read_func,
	s3dat_win32_size_func,
	s3dat_win32_pos_func,
	s3dat_win32_seek_func,
	s3dat_win32_open_func,
	s3dat_win32_close_func,
	NULL,
	#ifdef _WIN32
	true
	#else
	false
	#endif
};

s3dat_ioset_t s3dat_internal_mmf_win32_name_ioset = {
	s3dat_mmf_read_func,
	s3dat_mmf_size_func,
	s3dat_mmf_pos_func,
	s3dat_mmf_seek_func,
	s3dat_mmf_win32_name_open_func,
	s3dat_mmf_win32_close_func,
	s3dat_mmf_fork_func,
	#ifdef _WIN32
	true
	#else
	false
	#endif
};

s3dat_ioset_t s3dat_internal_mmf_win32_handle_ioset = {
	s3dat_mmf_read_func,
	s3dat_mmf_size_func,
	s3dat_mmf_pos_func,
	s3dat_mmf_seek_func,
	s3dat_mmf_win32_handle_open_func,
	s3dat_mmf_win32_close_func,
	s3dat_mmf_fork_func,
	#ifdef _WIN32
	true
	#else
	false
	#endif
};


s3dat_ioset_t s3dat_internal_libc_ioset = {
	s3dat_libc_read_func,
	s3dat_libc_size_func,
	s3dat_libc_pos_func,
	s3dat_libc_seek_func,
	s3dat_libc_open_func,
	s3dat_libc_close_func,
	NULL,
	true
};

s3dat_ioset_t s3dat_internal_null_ioset = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, false
};

s3dat_ioset_t* s3dat_get_default_ioset(uint32_t type) {
	if(type == S3DAT_IOSET_NATIVEOS) {
		#ifdef _WIN32
		return &s3dat_internal_win32_ioset;
		#elif (defined __linux__)
		return &s3dat_internal_linux_ioset;
		#endif
	} else if(type == S3DAT_IOSET_LINUX) {
		return &s3dat_internal_linux_ioset;
	} else if(type == S3DAT_IOSET_WIN32) {
		return &s3dat_internal_win32_ioset;
	} else if(type == S3DAT_IOSET_LIBC || type == S3DAT_IOSET_DEFAULT) {
		return &s3dat_internal_libc_ioset;
	} else if(type == S3DAT_IOSET_NATIVEOS_MMF) {
		#ifdef _WIN32
		return &s3dat_internal_mmf_win32_name_ioset;
		#elif (defined __linux__)
		return &s3dat_internal_mmf_linux_name_ioset;
		#endif
	} else if(type == S3DAT_IOSET_LINUX_MMF) {
		return &s3dat_internal_mmf_linux_name_ioset;
	} else if(type == S3DAT_IOSET_WIN32_MMF) {
		return &s3dat_internal_mmf_win32_name_ioset;
	} else if(type == S3DAT_IOSET_LINUX_MMF_FD) {
		return &s3dat_internal_mmf_linux_fd_ioset;
	} else if(type == S3DAT_IOSET_WIN32_MMF_HANDLE) {
		return &s3dat_internal_mmf_win32_handle_ioset;
	}

	return &s3dat_internal_null_ioset;
}
