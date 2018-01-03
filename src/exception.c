#include "s3dat_internal.h"
#ifdef PRIVATE_FILENAME
#line __LINE__ "exception.c"
#endif

s3dat_internal_stack_t s3dat_internal_out_of_memory_stack = {NULL, NULL, 0, NULL};
s3dat_exception_t s3dat_internal_out_of_memory = {S3DAT_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, NULL};

void s3dat_add_to_stack(s3dat_t* handle, s3dat_exception_t** throws, uint8_t* file, const uint8_t* func, uint32_t line) {
	if(*throws == NULL) return;
	s3dat_internal_stack_t* now;

	if((*throws)->type == S3DAT_EXCEPTION_OUT_OF_MEMORY) {
		if((*throws)->stack != &s3dat_internal_out_of_memory_stack) now = &s3dat_internal_out_of_memory_stack;
			else return; // only one stack member is supported
	} else {
		now = s3dat_alloc_func(handle, sizeof(s3dat_internal_stack_t), NULL);
		if(now == NULL) return;
	}

	now->file = file;
	now->function = func;
	now->line = line;
	now->down = (*throws)->stack;
	(*throws)->stack = now;
}

void s3dat_add_attr(s3dat_t* handle, s3dat_exception_t** throws, uint32_t name, uint32_t value) {
	if((*throws)->type == S3DAT_EXCEPTION_OUT_OF_MEMORY) return;

	s3dat_internal_attribute_t* attr = s3dat_alloc_func(handle, sizeof(s3dat_internal_attribute_t), NULL);
	if(attr == NULL) return;

	attr->name = name;
	attr->value = value;
	attr->next = (*throws)->attrs;
	(*throws)->attrs = attr;
}

void s3dat_throw(s3dat_t* handle, s3dat_exception_t** throws, uint32_t type, uint8_t* file, const uint8_t* func, uint32_t line) {
	if(type == S3DAT_EXCEPTION_OUT_OF_MEMORY) {
		*throws = &s3dat_internal_out_of_memory;
	} else {
		*throws = s3dat_alloc_func(handle, sizeof(s3dat_exception_t), NULL);
		if(*throws == NULL) {
			s3dat_throw(handle, throws, S3DAT_EXCEPTION_OUT_OF_MEMORY, NULL, NULL, 0); // out_of_memory has priority
		}
		s3dat_add_to_stack(handle, throws, file, func, line);
	}
	(*throws)->parent = handle;
	(*throws)->type = type;
}

void s3dat_delete_exception(s3dat_t* handle, s3dat_exception_t* ex) {
	if(ex->type == S3DAT_EXCEPTION_OUT_OF_MEMORY) return;

	s3dat_internal_stack_t* stack1;

	stack1 = ex->stack;

	while(stack1 != NULL) {
		s3dat_internal_stack_t* stack2 = stack1;
		stack1 = stack1->down;
		s3dat_free_func(handle, stack2);
	}

	s3dat_internal_attribute_t* attr1;

	attr1 = ex->attrs;

	while(attr1 != NULL) {
		s3dat_internal_attribute_t* attr2 = attr1;
		attr1 = attr1->next;
		s3dat_free_func(handle, attr2);
	}

	s3dat_free_func(handle, ex);
}

typedef struct {
	uint32_t type;
	uint8_t* name;
} s3dat_internal_map_entry_t;

s3dat_internal_map_entry_t exception_map[] = {
	{S3DAT_EXCEPTION_IOERROR, "IOError"},
	{S3DAT_EXCEPTION_HEADER, "WrongHeaderError"},
	{S3DAT_EXCEPTION_CONFLICTING_DATA, "ConflictingDataError"},
	{S3DAT_EXCEPTION_INDEXTYPE, "IndexTypeError"},
	{S3DAT_EXCEPTION_OUT_OF_RANGE, "OutOfRangeError"},
	{S3DAT_EXCEPTION_ICONV_ERROR, "IconvError"},
	{S3DAT_EXCEPTION_OPEN, "OpenError"},
	{S3DAT_EXCEPTION_IOSET, "NullIOSetError"},
	{S3DAT_EXCEPTION_OUT_OF_MEMORY, "OutOfMemoryError"},
	{0, NULL}
};

s3dat_internal_map_entry_t attr_map[] = {
	{S3DAT_ATTRIBUTE_INDEX, "index"},
	{S3DAT_ATTRIBUTE_SEQ, "sequence"},
	{S3DAT_ATTRIBUTE_SONG, "song"},
	{0, NULL}
};

uint8_t* s3dat_internal_find_entry(s3dat_internal_map_entry_t* map, uint32_t type) {
	uint8_t* re_value = NULL;
	uint32_t index = 0;
	while(map[index].name != NULL) {
		if(map[index].type == type) re_value = map[index].name;
		index++;
	}

	return re_value;
}

void s3dat_print_exception(s3dat_exception_t* ex) {
	uint8_t* name = s3dat_internal_find_entry(exception_map, ex->type);

	if(name != NULL) printf("%s caught\n", name);
				else printf("exception caught 0x%x\n", ex->type);

	s3dat_internal_attribute_t* attr = ex->attrs;
	while(attr != NULL) {
		printf("at ");
		uint8_t* attr_name = s3dat_internal_find_entry(attr_map, attr->name);
		if(attr_name != NULL) printf("%s", attr_name);
			else printf("%u", attr->name);
		printf(": %u\n", attr->value);
		attr = attr->next;
	}

	s3dat_internal_stack_t* stack = ex->stack;
	while(stack != NULL) {
		printf(" at %s(%s:%u)\n", stack->function, stack->file, stack->line);
		stack = stack->down;
	}
}

bool s3dat_catch_exception(s3dat_exception_t** throws) {
	if(*throws != NULL) {
		s3dat_print_exception(*throws);
		s3dat_delete_exception((*throws)->parent, *throws);
		*throws = NULL;
		return false;
	} else return true;
}

