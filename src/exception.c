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

void s3dat_print_exception(s3dat_exception_t* ex) {
	printf("exception caught 0x%x\n", ex->type);
	s3dat_internal_stack_t* stack = ex->stack;

	while(stack != NULL) {
		printf(" at %s(%s:%i)\n", stack->function, stack->file, stack->line);
		stack = stack->down;
	}
}

