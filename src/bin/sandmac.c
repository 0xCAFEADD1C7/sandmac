#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

const size_t REGISTER_COUNT = 8;

typedef enum {
	COND_MOVE,
	ARRAY_GET,
	ARRAY_SET,
	ADD,
	MUL,
	DIV,
	NAND,
	STOP,
	ALLOC,
	FREE,
	OUTPUT,
	INPUT,
	LOAD_CODE,
	LOAD,
	OP_COUNT,
} Operator;

typedef struct {
	uint32_t* mem;
	uint32_t size;
} Array;

typedef struct {
	uint32_t instruction_pointer;
	uint32_t registers[REGISTER_COUNT];
	Array* arrays;
	uint32_t array_count;
	uint32_t* available_arrays;
} State;

Operator extract_operator(uint32_t instruction) {
	return instruction >> 28;
}

const uint32_t THREE_BITS = 7;

uint8_t register_a(uint32_t i) {
	return i & THREE_BITS;
}

uint8_t register_b(uint32_t i) {
	return (i >> 3) & THREE_BITS;
}

uint8_t register_c(uint32_t i) {
	return (i >> 6) & THREE_BITS;
}

typedef void (*IHandler)(uint32_t, State*);

#define registers_on_stack(i) \
	uint8_t ra = register_a(i); \
	uint8_t rb = register_b(i); \
	uint8_t rc = register_c(i);

void cond_move_h(uint32_t i, State* s) {
	registers_on_stack(i);
	if (s->registers[rc] != 0) {
		s->registers[ra] = s->registers[rb];
	}
}

void array_get_h(uint32_t i, State* s) {
	registers_on_stack(i);
	s->registers[ra] = s->arrays[s->registers[rb]][s->registers[rc]];
}

void array_set_h(uint32_t i, State* s) {
	registers_on_stack(i);
	s->arrays[s->registers[ra]][s->registers[rb]] = s->registers[rc];
}

#define operator_h(name, pre, mid) \
	void name##_h(uint32_t i, State* s) { \
		registers_on_stack(i); \
		s->registers[ra] = pre (s->registers[rb] mid s->registers[rc]); \
	}

operator_h(add, , +)
operator_h(mul, , *)
operator_h(div, , /)
operator_h(nand, ~, &)

void stop_h(uint32_t i, State* s) {
	exit(EXIT_SUCCESS);
}

void alloc_h(uint32_t i, State* s) {
	registers_on_stack(i);
	uint32_t size = s->registers[rc]
	uint32_t* mem = calloc(size * sizeof(uint32_t));
	if (!mem) {
		fprintf(stderr, "allocation failure\n");
		exit(EXIT_FAILURE);
	}
	// TODO: recycle available arrays
	// FIXME: not allocated
	s->arrays[s->array_count] = (Array) {
		.mem = mem,
		.size = size
	};
	s->registers[rb] = s->array_count;
	s->array_count++;
}

void free_h(uint32_t i, State* s) {
	registers_on_stack(i);
	Array* array = &s->arrays[s->registers[rc]];
	free(array->mem);
	*array = (Array) {
		.mem = NULL,
		.size = 0
	};
	// TODO: recycle array
}

void output_h(uint32_t i, State* s) {
	registers_on_stack(i);
	fprintf(stdout, "%c", s->registers[rc]);
}

void input_h(uint32_t i, State* s) {
	registers_on_stack(i);
	int c = fgetc(stdin);
	if (c == EOF) {
		s->registers[rc] = ~0;
	} else {
		s->registers[rc] = c;
	}
}

void load_code_h(uint32_t i, State* s) {
	registers_on_stack(i);
	Array* array = s->arrays[s->registers[rb]];
	// TODO
}

IHandler ihandlers[OP_COUNT] = {
	cond_move_h,
	array_get_h,
	array_set_h,
	add_h,
	mul_h,
	div_h,
	nand_h,
	nand_h,
	stop_h,
	alloc_h,
	free_h,
	output_h,
	input_h,
	load_code_h,
	load_h
};

int main(int argc, char** argv) {

	return EXIT_SUCCESS;
}