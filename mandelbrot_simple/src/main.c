#include <stdio.h>
#include "complex.h"

#define CONVERGE_VAL_MAX 100000
#define CONVERGE_ITER_MAX 100


// f_parameter(argument) = argument^2 + parameter
complex mandelbrot_func(complex argument, complex parameter)
{
    //return 
}

int does_function_converge(complex func_parameter)
{

}

int main()
{
    complex values[] = { {0, 1}, {0, 1} };

    // for(int i = 0; i < 5; i++)
    // {
    //     char str[MAX_STR_LEN];
    //     complex_to_str(str, values[i]);
    //     printf("[%s]\n", str);
    // }

    complex number = complex_arithmetic("a * (b)", values);

    char str[MAX_STR_LEN];
    complex_to_str(str, number);
    printf("[%s]\n", str);
    

    return 0;
}