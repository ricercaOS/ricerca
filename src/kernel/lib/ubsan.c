//! @file ubsan.c
//! @brief Implementation of runtime needed for UB sanitizer

#include <lib/panic.h>
#include <lib/target.h>
#include <misc/attributes.h>
#include <misc/types.h>

MODULE("lib/ubsan")

struct ubsan_type {
	uint16_t kind;
	uint16_t info;
	char name[];
};

struct ubsan_source_location {
	const char *file;
	uint32_t line;
	uint32_t column;
};

struct ubsan_mismatch_data {
	struct ubsan_source_location location;
	struct ubsan_type *type;
	uint8_t align;
	uint8_t kind;
};

struct ubsan_overflow_data {
	struct ubsan_source_location location;
	struct ubsan_type *type;
};

struct ubsan_shift_out_of_bounds_data {
	struct ubsan_source_location location;
	struct ubsan_type *lhs_type;
	struct ubsan_type *rhs_type;
};

struct ubsan_out_of_bounds_data {
	struct ubsan_source_location location;
	struct ubsan_type *array_type;
	struct ubsan_type *index_type;
};

struct ubsan_invalid_value_data {
	struct ubsan_source_location location;
	struct ubsan_type *type;
};

struct ubsan_vla_bound_data {
	struct ubsan_source_location location;
	struct ubsan_type *type;
};

struct ubsan_invalid_builtin_data {
	struct ubsan_source_location location;
	uint8_t kind;
};

#define UBSAN_LOC_FMT "%s:%d:%d"
#define UBSAN_FROM_LOC(location) location.file, location.line, location.column

attribute_noinline void __ubsan_handle_type_mismatch_v1(struct ubsan_mismatch_data *data,
                                                        uintptr_t ptr) {
	if (ptr == 0) {
		panic_handler("ubsan", UBSAN_LOC_FMT " null dereference", UBSAN_FROM_LOC(data->location));
	} else if (data->align != 0 && align_up(ptr, 1 << data->align) != ptr) {
		panic_handler("ubsan", UBSAN_LOC_FMT " pointer %p is not aligned to %d",
		              UBSAN_FROM_LOC(data->location), ptr, 1 << data->align);
	} else {
		panic_handler("ubsan",
		              UBSAN_LOC_FMT
		              " pointer %p points to a cell that is not big enough to store \"%s\"",
		              UBSAN_FROM_LOC(data->location), ptr, data->type->name);
	}
}

attribute_noinline void __ubsan_handle_add_overflow() {
	panic_handler("ubsan", "addition overflow");
}

attribute_noinline void __ubsan_handle_sub_overflow() {
	panic_handler("ubsan", "substraction overflow");
}

attribute_noinline void __ubsan_handle_mul_overflow() {
	panic_handler("ubsan", "multiplication overflow");
}

attribute_noinline void __ubsan_handle_divrem_overflow() {
	panic_handler("ubsan", "division overflow");
}

attribute_noinline void __ubsan_handle_negate_overflow() {
	panic_handler("ubsan", "negate overflow");
}

attribute_noinline void __ubsan_handle_load_invalid_value(void *data_raw) {
	struct ubsan_invalid_value_data *data = (struct ubsan_invalid_value_data *)data_raw;
	panic_handler("ubsan", UBSAN_LOC_FMT " load of invalid value (type \"%s\")",
	              UBSAN_FROM_LOC(data->location), data->type->name);
}

attribute_noinline void __ubsan_handle_pointer_overflow(void *data_raw, void *lhs, void *rhs) {
	struct ubsan_overflow_data *data = (struct ubsan_overflow_data *)data_raw;
	panic_handler("ubsan", UBSAN_LOC_FMT " pointer overflow with operands %p, %p",
	              UBSAN_FROM_LOC(data->location), lhs, rhs);
}

attribute_noinline void __ubsan_handle_shift_out_of_bounds(void *data_raw, void *lhs, void *rhs) {
	struct ubsan_out_of_bounds_data *data = (struct ubsan_out_of_bounds_data *)data_raw;
	panic_handler("ubsan", UBSAN_LOC_FMT " shift out of bounds %U << %U",
	              UBSAN_FROM_LOC(data->location), lhs, rhs);
}

attribute_noinline void __ubsan_handle_out_of_bounds(void *data_raw, void *index) {
	struct ubsan_out_of_bounds_data *data = (struct ubsan_out_of_bounds_data *)data_raw;
	panic_handler("ubsan", UBSAN_LOC_FMT " out of bounds at index %d",
	              UBSAN_FROM_LOC(data->location), (intptr_t)index);
}

attribute_noinline void __ubsan_handle_vla_bound_not_positive(void *data_raw, void *bound) {
	struct ubsan_vla_bound_data *data = (struct ubsan_vla_bound_data *)data_raw;
	panic_handler("ubsan", UBSAN_LOC_FMT " vla bound %p not positive",
	              UBSAN_FROM_LOC(data->location), (intptr_t)bound);
}

attribute_noinline void __ubsan_handle_invalid_builtin(void *data_raw) {
	struct ubsan_invalid_builtin_data *data = (struct ubsan_invalid_builtin_data *)data_raw;
	panic_handler("ubsan", UBSAN_LOC_FMT " invalid builtin", UBSAN_FROM_LOC(data->location));
}
