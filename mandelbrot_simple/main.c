#include <stdio.h>
#include "complex.h"
#include <pthread.h>

#define CONVERGE_VAL_MAX 100000
#define CONVERGE_ITER_MAX 100

#define MAX_FRACTAL_DATA_SIZE 10000


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

int print_fractal(char* fractal_data, int (*does_function_converge)(complex), double xa, double xb, double ya, double yb, double spacing)
{
    int fractal_data_ptr = 0;
    
    for (double y = yb; y >= ya; y -= spacing)
    {
        for (double x = xa; x <= xb; x += spacing)
        {
            complex parameter = { x, y };
            char symbol = (does_function_converge(parameter) == 0 ? '#' : ' ');
			fractal_data[fractal_data_ptr++] = symbol;
			fractal_data[fractal_data_ptr++] = ' ';
        }
        fractal_data[fractal_data_ptr++] = '\n';
    }
	
	return fractal_data_ptr;
}

typedef struct ThreadData
{
	int ID;
} ThreadData;

char* fractal_data[MAX_FRACTAL_DATA_SIZE];

int main()
{
	double xa = -3, xb = 1.5, ya = -1.5, yb = 1.5, spacing = 0.07;
	
	
	
    int fractal_data_size = print_fractal(fractal_data, does_mandelbrot_converge, xa, xb, ya, yb, spacing);
	FILE* file = fopen("mandelbrot_out.txt", "wb");
	fwrite(fractal_data, fractal_data_size, 1, file);
	fclose(file);
}

