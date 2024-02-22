#include <stdio.h>
#include <stdlib.h>
#include "complex.h"
#include <pthread.h>
#include <SDL.h>

#define CONVERGE_VAL_MAX 100000
#define CONVERGE_ITER_MAX 100

#define MAX_FRACTAL_DATA_SIZE 10000000	

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 720

// #define SCREEN_WIDTH 2560
// #define SCREEN_HEIGHT 1440

// f_parameter(argument) = argument^2 + parameter
complex mandelbrot_func(complex argument, complex parameter)
{
	complex result = {argument.re * argument.re - argument.im * argument.im + parameter.re,
	2 * argument.re * argument.im + parameter.im};
	
	return result;
}

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

typedef struct ThreadFractalData
{
	long double x, y, spacing;
	int height, width;
	int data_offset;
	char (*does_function_converge)(complex);
	
} ThreadFractalData;

typedef struct FractalData
{
	long double x;
	long double y;
	long double spacing;
	int image_width;
	int image_height;
	char (*does_function_converge)(complex);

} FractalData;

char fractal_image[MAX_FRACTAL_DATA_SIZE];

pthread_mutex_t data_update_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  data_update_cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t stdout_synch_mutex = PTHREAD_MUTEX_INITIALIZER;

void *compute_mandelbrot_part(void *ptr)
{
	ThreadFractalData* thread_data = (ThreadFractalData*) ptr;

	while(1)
	{
		long double y = thread_data->y;
		for (int h = thread_data->height - 1; h >= 0; h--)
		{
			long double x = thread_data->x;
			for (int w = 0; w < thread_data->width; w++)
			{
				// Sleep(5);
				complex parameter = { x, y };
				char symbol = thread_data->does_function_converge(parameter);
				fractal_image[thread_data->data_offset] = symbol;
				thread_data->data_offset++;
				
				x += thread_data->spacing;
			}

			y -= thread_data->spacing;
		}

		pthread_mutex_lock(&data_update_mutex);

		pthread_cond_wait(&data_update_cond, &data_update_mutex);

		pthread_mutex_unlock(&data_update_mutex);
	}
	
	return NULL;
}

void set_threads_data(ThreadFractalData *thread_data_arr, int threads_count, FractalData fractal)
{
	for(int i = 0; i < threads_count; i++)
	{	
		int regular_height = ceil(fractal.image_height / (double)threads_count);
		thread_data_arr[i] = (ThreadFractalData){	.x = fractal.x,
													.y = fractal.y - ((fractal.image_height * (threads_count - (i+1)) * fractal.spacing) / threads_count),
													.width = fractal.image_width,
													.height = regular_height,
													.spacing = fractal.spacing, 
													.data_offset = regular_height * fractal.image_width * (threads_count - (i+1)),
													.does_function_converge = fractal.does_function_converge};
		
		// the shortest part (for example: when dividing 599 into 3 parts: 200 200 199)				
		if(i == 0)
		{
			thread_data_arr[i].height = fractal.image_height - (threads_count - 1) * regular_height;
		}
	}
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
				// SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_SetRenderDrawColor(renderer, symbol * 5, symbol * 8, symbol * 14, 0xFF);
			}
			
			SDL_RenderDrawPoint(renderer, i, j);
		}
	}
	
	SDL_RenderPresent(renderer);
}


void render_fractal(int image_width, int image_height, FractalData fractal, ThreadFractalData *thread_data_arr, int threads_count)
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
	
	double render_scale_factor =  (double)SCREEN_WIDTH/image_width;
	SDL_RenderSetScale(renderer, render_scale_factor, render_scale_factor);
	
	double zoom_scale_factor = 2; // says how much it zooms 

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

					pthread_mutex_lock(&data_update_mutex);

					long double zoom_factor = (event.button.button == SDL_BUTTON_LEFT ? zoom_scale_factor : 1 / zoom_scale_factor);
					
					fractal.x = fractal.x + ((mouse_x - (SCREEN_WIDTH / (long double) zoom_factor) / 2.0) * fractal.spacing) / render_scale_factor;
					fractal.y = fractal.y - ((mouse_y - (SCREEN_HEIGHT / (long double) zoom_factor) / 2.0) * fractal.spacing) / render_scale_factor;
					fractal.spacing = fractal.spacing / zoom_factor;

					set_threads_data(thread_data_arr, threads_count, fractal);
					
					pthread_cond_broadcast(&data_update_cond);
					pthread_mutex_unlock(&data_update_mutex);

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
	double resolution = 1; // 1 means that one screen pixel has one sample from the fractal, 0.25 means that 4 screen pixels has the same one sample from the fractal

	FractalData mandelbrot = {	.x = -2.5,
								.y = 1.5,
								.spacing = 0.004 / resolution,
								.image_width = SCREEN_WIDTH * resolution,
								.image_height = SCREEN_HEIGHT * resolution,
								.does_function_converge = does_mandelbrot_converge};

	printf("%dX%d\n", mandelbrot.image_width, mandelbrot.image_height);
	
	int threads_count = 6;
	pthread_t *threads = malloc(sizeof(pthread_t) * threads_count);
	
	ThreadFractalData *thread_data_arr = malloc(sizeof(ThreadFractalData) * threads_count);
	set_threads_data(thread_data_arr, threads_count, mandelbrot);

	for(int i = 0; i < threads_count; i++)
	{	
		pthread_create(&(threads[i]), NULL, compute_mandelbrot_part, (void*) &(thread_data_arr[i]));
	}

	render_fractal(mandelbrot.image_width, mandelbrot.image_height, mandelbrot, thread_data_arr, threads_count);
	
	for(int i = 0; i < threads_count; i++)
	{
		pthread_join(threads[i], NULL);
	}

	free(thread_data_arr);
	free(threads);
	
	return 0;
}