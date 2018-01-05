/*
This is a example how to write new restypes:
* You may leave create_example_res as null (
if you dont want to use s3dat_new_ref)
* wrappers around struct member is only
 a proposal, its not really necessary.
* the rest is more or less without alternative

NOTE: you obviously need s3dat headers and
libs3dat.so to compile and link this code
*/

//We want to create extensions
#include <s3dat_ext.h>

typedef struct {
	s3dat_t* parent;
	int data1;
} example_res_t;

// make your life easy
#define EXAMPLES_RES(res)  ((examples_res_t*)res->data.raw)

void delete_example_res(example_res_t* example_res) {
	// deallocate everything again through s3dat_free_func
	s3dat_free_func(example_res->parent, example_res);
}

example_res_t* create_example_res(s3dat_t* handle, s3dat_exception_t** throws) {
	// allocate everything through s3dat_alloc_func
	example_res_t* eres = s3dat_alloc_func(handle, sizeof(example_res_t), throws);

	// it wont do anything, if *throws == null (there is no exception)
	s3dat_add_to_stack(handle, throws, __FILE__, __func__, __LINE__);
	return eres;
}

s3dat_restype_t example_restype = {"example_res", (void (*) (void*)) delete_example_res, (void* (*) (void*, s3dat_exception_t**)) create_example_res};

bool is_example_res(s3dat_ref_t* res) {
	return res->type == &example_restype;
}

//write a wrapper around each member and hide the decleration of your struct
int examples_res_data1(s3dat_ref_t* res) {
	// check the type !!!
	if(!is_example_res(res)) return -1;
	example_res_t* eres = res->data.raw;
	return eres->data1;
}

