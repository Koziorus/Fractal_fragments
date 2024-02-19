#include <stdio.h>
#include <stdlib.h>
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

int print_fractal(char* fractal_image, int (*does_function_converge)(complex), double xa, double xb, double ya, double yb, double spacing)
{
    int fractal_data_ptr = 0;
    
    for (double y = yb; y >= ya; y -= spacing)
    {
        for (double x = xa; x <= xb; x += spacing)
        {
            complex parameter = { x, y };
            char symbol = (does_function_converge(parameter) == 0 ? '#' : ' ');
			fractal_image[fractal_data_ptr++] = symbol;
			fractal_image[fractal_data_ptr++] = ' ';
        }
		
		fractal_image[fractal_data_ptr++] = '\n';
    }
	
	return fractal_data_ptr;
}

typedef struct FractalData
{
	double xa, xb, ya, yb, spacing;
	int fractal_data_offset;
	int (*does_function_converge)(complex);
	
} FractalData;

char fractal_image[MAX_FRACTAL_DATA_SIZE];

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *compute_mandelbrot_part(void *ptr)
{
	FractalData fractal_data = *((FractalData*) ptr);
	
	printf("%d", fractal_data.fractal_data_offset);
	
    for (double y = fractal_data.yb; y >= fractal_data.ya; y -= fractal_data.spacing)
    {
        for (double x = fractal_data.xa; x <= fractal_data.xb; x += fractal_data.spacing)
        {
            complex parameter = { x, y };
            char symbol = (fractal_data.does_function_converge(parameter) == 0 ? '#' : ' ');
			fractal_image[fractal_data.fractal_data_offset] = symbol;
			fractal_data.fractal_data_offset++;
			fractal_image[fractal_data.fractal_data_offset] = ' ';
			fractal_data.fractal_data_offset++;
		}
		
		fractal_image[fractal_data.fractal_data_offset] = '\n';
		fractal_data.fractal_data_offset++;
    }
	
	printf(" %d\n", fractal_data.fractal_data_offset);
	
	return;
}

int main()
{
	double xa = -3, xb = 1.5, ya = -1.5, yb = 1.5, spacing = 0.05;
	
	// + 1 for a new line in each ine, + 1 for first symbol (from 0.0 to 10.0 a spacing of 2.6 can fit only 3 times, so there are symbols: symbol spacing symbol spacing symbol spacing symbol -> one more symbol than there are spacings)
	int image_width = ((int)((xb - xa) / spacing + 1)) * 2 + 1, image_height = (yb - ya) / spacing + 1;
	
	printf("%dX%d\n", image_width, image_height);
	
	int threads_count = 6;
	pthread_t *threads = malloc(sizeof(pthread_t) * threads_count);
	
	FractalData *fractal_data = malloc(sizeof(FractalData) * threads_count);
	
	for(int i = 0; i < threads_count; i++)
	{	
		fractal_data[i] = (FractalData){.xa = xa,
										.xb = xb,
										.ya = ya + ((yb - ya) * i) / threads_count,
										.yb = ya + ((yb - ya) * i + (yb - ya)) / threads_count,
										.spacing = spacing, 
										.fractal_data_offset = ((image_height * image_width) * (threads_count - (i+1))) / threads_count,
										.does_function_converge = does_mandelbrot_converge};
										
		printf("%f %f %f %f %f %d\n", fractal_data[i].xa, fractal_data[i].xb, fractal_data[i].ya, fractal_data[i].yb, fractal_data[i].spacing, fractal_data[i].fractal_data_offset);

		pthread_create(&(threads[i]), NULL, compute_mandelbrot_part, (void*) &(fractal_data[i]));
	}
	
	for(int i = 0; i < threads_count; i++)
	{
		pthread_join(threads[i], NULL);
	}
	
    //int fractal_data_size = print_fractal(fractal_data, does_mandelbrot_converge, xa, xb, ya, yb, spacing);
	
	//printf("fractal_data_size = (true) %d ?= %d (calculated)\n", fractal_data_size, image_width * image_height);
	
	int fractal_image_size = image_width * image_height;
	FILE* file = fopen("mandelbrot_out.txt", "wb");
	
	fwrite(fractal_image, 1, fractal_image_size, file);
	fclose(file);
	
	free(fractal_data);
	
	free(threads);
}

