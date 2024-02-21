#include <stdio.h>
#include <stdlib.h>
#include "complex.h"
#include <pthread.h>
#include <SDL.h>

#define CONVERGE_VAL_MAX 100000
#define CONVERGE_ITER_MAX 100

#define MAX_FRACTAL_DATA_SIZE 1000000000	

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 700


// f_parameter(argument) = argument^2 + parameter
complex mandelbrot_func(complex argument, complex parameter)
{
	// old version
    //complex values[] = { argument, parameter };
	//return complex_arithmetic("(a * a) + b", values);
	
	complex result = {argument.re * argument.re - argument.im * argument.im + parameter.re,
	2 * argument.re * argument.im + parameter.im};
	
	return result;
}

// returns 0 when the function converges and 1 otherwise
char does_mandelbrot_converge(complex func_parameter)
{
    complex z = { 0, 0 };
    for (int i = 0; i < CONVERGE_ITER_MAX; i++)
    {
        z = mandelbrot_func(z, func_parameter);
        if (z.re > CONVERGE_VAL_MAX || z.im > CONVERGE_VAL_MAX)
        {
            return i;
        }
    }

    return 0;
}

typedef struct FractalData
{
	double xa, xb, ya, yb, spacing;
	int height, width;
	int fractal_data_offset;
	char (*does_function_converge)(complex);
	
} FractalData;

char fractal_image[MAX_FRACTAL_DATA_SIZE];

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void *compute_mandelbrot_part(void *ptr)
{
	FractalData* fractal_data = (FractalData*) ptr;
	
	// printf("%d", fractal_data.fractal_data_offset);
	
	double y = fractal_data->yb;
    for (int h = fractal_data->height - 1; h >= 0; h--)
    {
		double x = fractal_data->xa;
        for (int w = 0; w < fractal_data->width; w++)
        {
			//Sleep(1);
            complex parameter = { x, y };
            char symbol = fractal_data->does_function_converge(parameter);
			fractal_image[fractal_data->fractal_data_offset] = symbol;
			fractal_data->fractal_data_offset++;
			
			x += fractal_data->spacing;
		}
		
		y -= fractal_data->spacing;
    }
	
	printf("%d\n", fractal_data->fractal_data_offset);
	
	// printf(" %d\n", fractal_data.fractal_data_offset);
	
	return;
}


void render_iter(SDL_Renderer* renderer, int image_width, int image_height)
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);
	
	for(int i = 0; i < image_width; i++)
	{
		for(int j = 0; j < image_height; j++)
		{
			char symbol = fractal_image[i + image_width * j];
			if(symbol == 0)
			{
				SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, symbol * 2, symbol * 3, symbol * 4, 0xFF);
			}
			
			SDL_RenderDrawPoint(renderer, i, j);
		}
	}
	
	SDL_RenderPresent(renderer);
}

void render_fractal(int image_width, int image_height)
{
	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window *window = SDL_CreateWindow(	"Fractal window",
											SDL_WINDOWPOS_UNDEFINED,
											SDL_WINDOWPOS_UNDEFINED,
											SCREEN_WIDTH, 
											SCREEN_HEIGHT,
											SDL_WINDOW_SHOWN);
	
	SDL_Renderer *renderer = SDL_CreateRenderer(window, 
												-1,
												SDL_RENDERER_ACCELERATED);
												
	SDL_RenderSetScale(renderer, (double)SCREEN_WIDTH/image_width, (double)SCREEN_WIDTH/image_width);
	int quit = 0;
	
	// rendering loop
	while(!quit)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					int mouse_x, mouse_y;
					SDL_GetMouseState(&mouse_x, &mouse_y);
					break;
			}
		}
		
		render_iter(renderer, image_width, image_height);
	}
	
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit(); 	
}

int main(int argc, char* argv[])
{
	// + 1 for a new line in each ine, + 1 for first symbol (from 0.0 to 10.0 a spacing of 2.6 can fit only 3 times, so there are symbols: symbol spacing symbol spacing symbol spacing symbol -> one more symbol than there are spacings)
	// int image_width = ((int)((xb - xa) / spacing + 1)) * 2 + 1, image_height = (yb - ya) / spacing + 1;
	
	double xa = -3, xb = 1.5, ya = -1.5, yb = 1.5, spacing = 0.005;
	
	// double xa = 0, xb = 0.05, ya = 0.7, yb = 0.75, spacing = 0.0001;
	
	// image dimensions for rendering (without extra spaces and new lines
	int image_width = (xb - xa) / spacing + 1, image_height = (yb - ya) / spacing + 1;
	
	printf("%dX%d\n", image_width, image_height);
	
	int threads_count = 8;
	pthread_t *threads = malloc(sizeof(pthread_t) * threads_count);
	
	FractalData *fractal_data = malloc(sizeof(FractalData) * threads_count);
	
	for(int i = 0; i < threads_count; i++)
	{	
		int regular_height = ceil(image_height / (double)threads_count);
		fractal_data[i] = (FractalData){.xa = xa,
										.xb = xb,
										.ya = ya + ((yb - ya) * i) / threads_count,
										.yb = ya + ((yb - ya) * i + (yb - ya)) / threads_count,
										.width = image_width,
										.height = regular_height,
										.spacing = spacing, 
										.fractal_data_offset = regular_height * image_width * (threads_count - (i+1)),
										.does_function_converge = does_mandelbrot_converge};
		
		// the shortest part (for example: when dividing 599 into 3 parts: 200 200 199)				
		if(i == 0)
		{
			fractal_data[i].height = image_height - (threads_count - 1) * regular_height;
		}
										
		printf("%d %d\n", fractal_data[i].fractal_data_offset, fractal_data[i].height);

		pthread_create(&(threads[i]), NULL, compute_mandelbrot_part, (void*) &(fractal_data[i]));
	}
	
	
	
	for(int i = 0; i < threads_count; i++)
	{
		pthread_join(threads[i], NULL);
	}
	
	render_fractal(image_width, image_height);
	
    //int fractal_data_size = print_fractal(fractal_data, does_mandelbrot_converge, xa, xb, ya, yb, spacing);
	
	//printf("fractal_data_size = (true) %d ?= %d (calculated)\n", fractal_data_size, image_width * image_height);
	
	int fractal_image_size = image_width * image_height;
	// FILE* file = fopen("mandelbrot_out.txt", "wb");
	
	// fwrite(fractal_image, 1, fractal_image_size, file);
	// fclose(file);
		
	//render_fractal(image_width + 1, image_height);	
		
	free(fractal_data);
	
	free(threads);
	
	return 0;
}

//TODO: seperate SDL version of the project into a different folder (so that there is a folder for ASCII rendering and another folder for SDL rendering)