#include "core.h"
#include "vec.h"

#define REGISTER_COUNT 8

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
    Vec arrays;
    Vec available_arrays;
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

#define registers_on_stack(i)                   \
    uint8_t ra = register_a(i);                 \
    uint8_t rb = register_b(i);                 \
    uint8_t rc = register_c(i)

void cond_move_h(uint32_t i, State* s) {
    registers_on_stack(i);
    if (s->registers[rc] != 0) {
        s->registers[ra] = s->registers[rb];
    }
}

void array_get_h(uint32_t i, State* s) {
    registers_on_stack(i);
    Array* array = Vec_get_mut(&s->arrays, s->registers[rb]);
    uint32_t index = s->registers[rc];
    if (index >= array->size) {
        fprintf(stderr, "index %d out of bounds (array #%d is %d plates big)\n", index, s->registers[rb], array->size);
        exit(EXIT_FAILURE);
    }
    s->registers[ra] = array->mem[index];
}

void array_set_h(uint32_t i, State* s) {
    registers_on_stack(i);
    Array* array = Vec_get_mut(&s->arrays, s->registers[ra]);
    uint32_t index = s->registers[rb];
    array->mem[index] = s->registers[rc];
}

#define operator_h(name, pre, mid)                                      \
    void name##_h(uint32_t i, State* s) {                               \
        registers_on_stack(i);                                          \
        s->registers[ra] = pre (s->registers[rb] mid s->registers[rc]); \
    }

operator_h(add, , +)
operator_h(mul, , *)
operator_h(div, , /)
operator_h(nand, ~, &)

void stop_h(uint32_t i, State* s) {
    (void)(i);
    (void)(s);
    exit(EXIT_SUCCESS);
}

void alloc_h(uint32_t i, State* s) {
    uint32_t size = s->registers[register_c(i)];
    uint32_t* mem = calloc(size, sizeof(uint32_t));
    assert_alloc(mem);
    Array array = (Array) {
        .mem = mem,
        .size = size
    };

    uint32_t index;
    if (Vec_pop(&s->available_arrays, &index)) {
        Array* recycled = Vec_get_mut(&s->arrays, index);
        *recycled = array;
    } else {
        index = Vec_len(&s->arrays);
        Vec_push(&s->arrays, &array);
    }

    s->registers[register_b(i)] = index;
}

void free_h(uint32_t i, State* s) {
    uint32_t index = s->registers[register_c(i)];
    Array* array = Vec_get_mut(&s->arrays, index);
    free(array->mem);
    *array = (Array) {
        .mem = NULL,
        .size = 0
    };
    Vec_push(&s->available_arrays, &index);
}

void output_h(uint32_t i, State* s) {
    fprintf(stdout, "%c", s->registers[register_c(i)]);
}

void input_h(uint32_t i, State* s) {
    uint32_t* out = &s->registers[register_c(i)];
    int c = fgetc(stdin);
    if (c == EOF) {
        *out = ~0;
    } else {
        *out = c;
    }
}

void load_code_h(uint32_t i, State* s) {
    uint32_t index = s->registers[register_b(i)];
    Array* array = Vec_get_mut(&s->arrays, index);
    Array* program = Vec_get_mut(&s->arrays, 0);
    assert(array->size > 0);
    size_t bytes = array->size * sizeof(uint32_t);
    uint32_t* mem = malloc(bytes);
    assert_alloc(mem);
    memcpy(mem, array->mem, bytes);
    if (program->mem) {
        free(program->mem);
    }
    *program = (Array) {
        .mem = mem,
        .size = array->size
    };
}

void load_h(uint32_t i, State* s) {
    uint8_t r = (i >> 25) & THREE_BITS;
    uint32_t seven_bits = (THREE_BITS << 6) | (THREE_BITS << 3) | 1;
    uint32_t value = i & ~(seven_bits << 25);
    s->registers[r] = value;
}

IHandler ihandlers[OP_COUNT] = {
    cond_move_h,
    array_get_h,
    array_set_h,
    add_h,
    mul_h,
    div_h,
    nand_h,
    stop_h,
    alloc_h,
    free_h,
    output_h,
    input_h,
    load_code_h,
    load_h
};

void State_init(State* s) {
    *s = (State) {
        .instruction_pointer = 0,
        .registers = { 0 },
        .arrays = Vec_with_capacity(32, sizeof(Array)),
        .available_arrays = Vec_with_capacity(8, sizeof(uint32_t))
    };
}

void State_drop(State* s) {
    Array array;
    while (Vec_pop(&s->arrays, &array)) {
        if (array.mem) {
            free(array.mem);
        }
    }

    Vec_drop(&s->arrays);
    Vec_drop(&s->available_arrays);
}

uint32_t endianness_switch(uint32_t value)
{
    uint32_t result = 0;
    result |= (value & 0x000000FF) << 24;
    result |= (value & 0x0000FF00) << 8;
    result |= (value & 0x00FF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;
    return result;
}

const char* string_of_opcode(int opCode) {
    static const char* ops[] = {
        "cond_move_h",
        "array_get_h",
        "array_set_h",
        "add_h",
        "mul_h",
        "div_h",
        "nand_h",
        "stop_h",
        "alloc_h",
        "free_h",
        "output_h",
        "input_h",
        "load_code_h",
        "load_h",
        "BAD_OPCODE"
    };
    return ops[opCode];
}

int main(int argc, char** argv) {
    State s;
    State_init(&s);

    if (argc != 2) {
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        return 1;
    }

    FILE* fhandler = fopen(argv[1], "rb");
    if (fhandler == NULL) {
        perror(argv[1]);
        return 1;
    }

    uint32_t instr;
    int nbRead = 0;
    while ((nbRead = fread(&instr, sizeof(uint32_t), 1, fhandler)) > 0) {
        int opCode = extract_operator(endianness_switch(instr));

        if (opCode >= OP_COUNT) {
            fprintf(stderr, "Bad instruction !!\n");
            return 1;
        }

        printf("evaluating %s\n", string_of_opcode(opCode));

        ihandlers[opCode](instr, &s);
    }    

    fclose(fhandler);

    State_drop(&s);
    return EXIT_SUCCESS;
}
