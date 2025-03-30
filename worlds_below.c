#include <SDL3/SDL_init.h>
#include <SDL3/SDL.h>
#include <stdio.h>

typedef struct color {
	int red;
	int green;
	int blue;
} color;

void spawn_house(SDL_FRect *p_house_rect, color *p_house_color, SDL_Rect *p_display_bounds) {
	const uint32_t HOUSE_WIDTH = 300;
	const uint32_t HOUSE_HEIGHT = 300;
	*p_house_rect = (SDL_FRect) {
		.w = HOUSE_WIDTH,
		.h = HOUSE_HEIGHT,
		.x = (p_display_bounds->w / 2) - (HOUSE_WIDTH / 2),
		.y = (p_display_bounds->h / 2) - (HOUSE_HEIGHT / 2),
	};
	*p_house_color = (color) {
		.red = 100,
		.green = 100,
		.blue = 100
	};
}

void printfFRect(SDL_FRect *p_sdl_f_rect) {
	printf("RECT: { x: %f, y: %f, w: %f, h: %f }\n", p_sdl_f_rect->x, p_sdl_f_rect->y, p_sdl_f_rect->w, p_sdl_f_rect->h);
}

void init(SDL_Rect *p_display_bounds, int entities[], int *p_entityCount, SDL_FRect rects[], int rectCount, color colors[], int colorCount) {
	for(int i = 0; i < *p_entityCount; i++) {
		rects[i] = (SDL_FRect) {
			.x = rand() / (RAND_MAX / p_display_bounds->w + 1),
			.y = rand() / (RAND_MAX / p_display_bounds->h + 1),
			.w = rand() / (RAND_MAX / p_display_bounds->w + 1),
			.h = rand() / (RAND_MAX / p_display_bounds->h + 1)
		};
		colors[i] = (color) {
			.red = rand() / (RAND_MAX / 255 + 1),
			.green = rand() / (RAND_MAX / 255 + 1),
			.blue = rand() / (RAND_MAX / 255 + 1),
		};
	}
	spawn_house(&rects[*p_entityCount], &colors[*p_entityCount], p_display_bounds);
	SDL_FRect *house = &rects[*p_entityCount];
	printfFRect(house);
	*p_entityCount += 1;
}

void cleanup(SDL_Window *p_sdl_window) {
	SDL_DestroyWindow(p_sdl_window);
	SDL_Quit();
}

int main() {
	SDL_Window *p_sdl_window;
	SDL_Renderer *p_sdl_renderer;
	int entityCount = 0;
	int entities[1000] = {0};
	int rectCount = 1000;
	SDL_FRect rects[1000] = {};
	int colorCount = 1000;
	color colors[1000] = {0};

	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_Rect displayBounds;
	SDL_DisplayID primaryDisplayId = SDL_GetPrimaryDisplay();
	SDL_GetDisplayBounds(primaryDisplayId, &displayBounds);
	if(primaryDisplayId == 0) {
		printf("failed to get primary display due to the following error:%s\n", SDL_GetError());
		exit(-1);
	}
	printf("Detected the following display bounds: { w: %d, h: %d } for displayId: %d\n", displayBounds.w, displayBounds.h, primaryDisplayId);

	SDL_CreateWindowAndRenderer("Worlds Below", displayBounds.w, displayBounds.h, SDL_WINDOW_FULLSCREEN, &p_sdl_window, &p_sdl_renderer);
	init(&displayBounds, entities, &entityCount, rects, rectCount, colors, colorCount);
	printf("ENTITY COUNT: %d\n", entityCount);

	bool running = true;
	while(running) {
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_KEY_DOWN: 
					// TODO: add proper logging with levels like trace/debug
					printf("detected a keyboard event.\n");
					if(event.key.key == SDLK_ESCAPE) {
						running = false;
					}
					break;
				case SDL_EVENT_QUIT:
					printf("detected a quit event.\n");
					running = false;
					break;
				default: 
					printf("detected an unhandled event.\n");
					break;
			}
			SDL_SetRenderDrawColor(p_sdl_renderer, 0, 0, 100, 0x00);
			SDL_RenderClear(p_sdl_renderer);
			
			for(int i = 0; i < entityCount; i++) {
				SDL_SetRenderDrawColor(p_sdl_renderer, colors[i].red, colors[i].green, colors[i].blue, SDL_ALPHA_OPAQUE);
				SDL_RenderFillRect(p_sdl_renderer, &rects[i]);
			}

			SDL_RenderPresent(p_sdl_renderer);
		}
	}
	cleanup(p_sdl_window);
	return 0;
}
