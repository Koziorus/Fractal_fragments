#include <stdio.h>
#include "complex.h"

#define CONVERGE_VAL_MAX 100000
#define CONVERGE_ITER_MAX 100


// f_parameter(argument) = argument^2 + parameter
complex mandelbrot_func(complex argument, complex parameter)
{
    complex values[] = { argument, parameter };
    return complex_arithmetic("(a * a) + b", values);
}

// returns 0 when the function converges and 1 otherwise
int does_mandelbrot_converge(complex func_parameter)
{
    complex z = { 0, 0 };
    for (int i = 0; i < CONVERGE_ITER_MAX; i++)
    {
        z = mandelbrot_func(z, func_parameter);
        if (z.re > CONVERGE_VAL_MAX || z.im > CONVERGE_VAL_MAX)
        {
            return 1;
        }
    }

    return 0;
}

void print_fractal(int (*does_function_converge)(complex), double xa, double xb, double ya, double yb, double spacing)
{
    FILE* file = fopen("mandelbrot_out.txt", "w");
    
    for (double y = yb; y >= ya; y -= spacing)
    {
        for (double x = xa; x <= xb; x += spacing)
        {
            complex parameter = { x, y };
            char symbol = (does_function_converge(parameter) == 0 ? '#' : ' ');
            fprintf(file, "%c ", symbol);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

int main()
{
    print_fractal(does_mandelbrot_converge, -3, 1.5, -1.5, 1.5, 0.01);

    return 0;
}