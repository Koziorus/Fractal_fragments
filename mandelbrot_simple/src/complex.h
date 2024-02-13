#pragma once

#define MAX_COMPUTATION_STR_LEN 100
#define MAX_STACK_LEN 100 
#define MAX_STR_LEN 200

typedef struct
{
    double re, im;
} complex;

void complex_to_str(char* dest_str, complex number);

complex complex_mult(complex a, complex b);

complex complex_add(complex a, complex b);

complex complex_arithmetic(char comp_str[MAX_COMPUTATION_STR_LEN], complex* values);