#include "s3dat_internal.h"
#line __LINE__ "exception.c"

void s3dat_add_to_stack(s3dat_t* mem, s3dat_exception_t** throws, char* file, const char* func, int line) {
	s3dat_internal_stack_t* now = mem->alloc_func(mem->mem_arg, sizeof(s3dat_internal_stack_t));
	now->file = file;
	now->function = func;
	now->line = line;
	now->down = (*throws)->stack;
	(*throws)->stack = now;
}

void s3dat_add_attr(s3dat_t* mem, s3dat_exception_t** throws, uint32_t name, uint32_t value) {
	s3dat_internal_attribute_t* attr = mem->alloc_func(mem->mem_arg, sizeof(s3dat_internal_attribute_t));
	attr->name = name;
	attr->value = value;
	attr->next = (*throws)->attrs;
	(*throws)->attrs = attr;
}

void s3dat_internal_throw(s3dat_t* mem, s3dat_exception_t** throws, uint32_t type, char* file, const char* func, int line) {
	*throws = mem->alloc_func(mem->mem_arg, sizeof(s3dat_exception_t));
	(*throws)->type = type;
	(*throws)->stack = NULL;
	(*throws)->attrs = NULL;
	s3dat_add_to_stack(mem, throws, file, func, line);
}

void s3dat_delete_exception(s3dat_t* mem, s3dat_exception_t* ex) {
	s3dat_internal_stack_t* stack1, *stack2;

	stack1 = ex->stack;

	while(stack1 != NULL) {
		stack2 = stack1;
		stack1 = stack1->down;
		mem->free_func(mem->mem_arg, stack2);
	}

	s3dat_internal_attribute_t* attr1, *attr2;

	attr1 = ex->attrs;

	while(attr1 != NULL) {
		attr2 = attr1;
		attr1 = attr1->next;
		mem->free_func(mem->mem_arg, attr2);
	}

	mem->free_func(mem->mem_arg, ex);
}

typedef struct {
	uint32_t type;
	char* name;
} s3dat_internal_map_entry_t;

s3dat_internal_map_entry_t exception_map[] = {
	{S3DAT_EXCEPTION_IOERROR, "IOError"},
	{S3DAT_EXCEPTION_HEADER, "WrongHeaderError"},
	{S3DAT_EXCEPTION_CONFLICTING_DATA, "ConflictingDataError"},
	{S3DAT_EXCEPTION_INDEXTYPE, "IndexTypeError"},
	{S3DAT_EXCEPTION_OUT_OF_RANGE, "OutOfRangeError"},
	{S3DAT_EXCEPTION_ICONV_ERROR, "IconvError"},
	{0, NULL}
};

s3dat_internal_map_entry_t attr_map[] = {
	{S3DAT_ATTRIBUTE_INDEX, "index"},
	{S3DAT_ATTRIBUTE_SEQ, "sequence"},
	{S3DAT_ATTRIBUTE_SONG, "song"},
	{0, NULL}
};

char* s3dat_internal_find_entry(s3dat_internal_map_entry_t* map, uint32_t type) {
	char* re_value = NULL;
	int index = 0;
	while(map[index].name != NULL) {
		if(map[index].type == type) re_value = map[index].name;
		index++;
	}

	return re_value;
}

void s3dat_print_exception(s3dat_exception_t* ex) {
	char* name = s3dat_internal_find_entry(exception_map, ex->type);

	if(name != NULL) printf("%s caught\n", name);
				else printf("exception caught 0x%x\n", ex->type);

	s3dat_internal_attribute_t* attr = ex->attrs;
	while(attr != NULL) {
		printf("at ");
		char* attr_name = s3dat_internal_find_entry(attr_map, attr->name);
		if(attr_name != NULL) printf("%s", attr_name);
			else printf("%i", attr->name);
		printf(": %i\n", attr->value);
		attr = attr->next;
	}

	s3dat_internal_stack_t* stack = ex->stack;
	while(stack != NULL) {
		printf(" at %s(%s:%i)\n", stack->function, stack->file, stack->line);
		stack = stack->down;
	}
}

