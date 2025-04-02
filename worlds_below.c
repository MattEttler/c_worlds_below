#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <stdio.h>
#include <time.h>

const int MAX_ENTITY_COUNT = 1000;
const int MAX_HEALTH = 100;

void printfFRect(SDL_FRect *p_sdl_f_rect) {
	printf("{ x: %f, y: %f, w: %f, h: %f }\n", p_sdl_f_rect->x, p_sdl_f_rect->y, p_sdl_f_rect->w, p_sdl_f_rect->h);
}


// =======================================================================================
//  ┌─┐┌─┐┌┬┐┌─┐┌─┐┌┐┌┌─┐┌┐┌┌┬┐┌─┐
//  │  │ ││││├─┘│ ││││├┤ │││ │ └─┐
//  └─┘└─┘┴ ┴┴  └─┘┘└┘└─┘┘└┘ ┴ └─┘
// generally components should use the following conventions:
// - use typedef- this makes it easier to refactor the underlying data of the component
// - primitive data types when possible
// - prefix with `c_` to indicate it is a component

typedef bool c_character;
typedef struct c_color {
	int red;
	int green;
	int blue;
} c_color;
typedef bool c_oxygenator;
typedef float c_health;
// TODO: this should be refactored to not directly depend on the SDL_FRect... especially since we will eventually be drawing polygons.
typedef SDL_FRect c_boundingBox;


// =======================================================================================
//  ┌─┐┬ ┬┌─┐┌┬┐┌─┐┌┬┐┌─┐
//  └─┐└┬┘└─┐ │ ├┤ │││└─┐
//  └─┘ ┴ └─┘ ┴ └─┘┴ ┴└─┘
//  generally game systems use the following convention: 
//  `sys_<component_a>_<component_b>_...(entityCount, component_a[], component_b[])`
//  where the system is named based on the data components required to operate the system,
//  ideally in the same order as the parameters.
//

bool overlaps(c_boundingBox *pBoundingBoxA, c_boundingBox *pBoundingBoxB) {
	bool result = (
			(pBoundingBoxA->x+pBoundingBoxA->w < pBoundingBoxB->x) ||
			(pBoundingBoxB->x+pBoundingBoxB->w < pBoundingBoxA->x) ||
			(pBoundingBoxA->y+pBoundingBoxA->h < pBoundingBoxB->y) ||
			(pBoundingBoxB->y+pBoundingBoxB->h < pBoundingBoxA->y)
		);
	return !result;
}

void sys_health_oxygenator_boundingBox(long *p_time_since_last_tick, size_t *pEntityCount, c_health healths[], c_oxygenator oxygenators[], c_boundingBox boundingBoxes[]) {
	const float O2_RECOVERY_RATE_PER_SECOND = 5;
	const float O2_RECOVERY_RATE_PER_NANOSECOND = O2_RECOVERY_RATE_PER_SECOND / 1000000000;
	float delta = (*p_time_since_last_tick) * O2_RECOVERY_RATE_PER_NANOSECOND;
	for(size_t i = 0; i < *pEntityCount; i++) {
		c_health *pHealth = &healths[i];
		c_boundingBox *pBoundingBoxA = &boundingBoxes[i];
		bool boundingBoxWithHealth = (pHealth != NULL && pBoundingBoxA != NULL);
		if(boundingBoxWithHealth) {
			bool oxygenatorAndHealthOverlap = false;
			for(size_t j = 0; j < *pEntityCount; j++) {
				if(i == j) {
					continue;
				}
				c_oxygenator *pOxygenator = &oxygenators[j];
				c_boundingBox *pBoundingBoxB = &boundingBoxes[j];
				bool oxygenatorWithBoundingBox = (pOxygenator != NULL && *pOxygenator && pBoundingBoxB != NULL);
				if(oxygenatorWithBoundingBox) {
					oxygenatorAndHealthOverlap = overlaps(pBoundingBoxA, pBoundingBoxB);
					if(oxygenatorAndHealthOverlap) {
						break;
					}
				}
			}
			if(oxygenatorAndHealthOverlap) {
				*pHealth = min(MAX_HEALTH, *pHealth+delta);
			} else {
				*pHealth = max(0, *pHealth-delta);
			}
		}
	}
}

void spawn_characters(uint32_t spawnCount, size_t *p_entityCount, SDL_FRect rects[], c_color colors[], c_health healths[], SDL_FRect *p_rect_spawn_bounds) {
	uint32_t character_width = 50;
	uint32_t character_height = 50;
	for(int i = 0; i < spawnCount; i++) {
		size_t entity = (*p_entityCount) + i;
		rects[entity] = (SDL_FRect) {
			.x = rand() / (RAND_MAX / (p_rect_spawn_bounds->w - character_width + 1)) + p_rect_spawn_bounds->x,
			.y = rand() / (RAND_MAX / (p_rect_spawn_bounds->h - character_height + 1)) + p_rect_spawn_bounds->y,
			.w = character_width,
			.h = character_height,
		};
		colors[entity] = (c_color) {
			.red = 255,
			.green = 255,
			.blue = 0,
		};
		healths[entity] = MAX_HEALTH;
		printf("<CHARACTER_SPAWNED> %zu", *p_entityCount);
		(*p_entityCount)++;
		printfFRect(&rects[entity]);
	}
}

void spawn_player(size_t *p_entityCount, bool player_controlled[], SDL_FRect rects[], c_color colors[], c_health healths[], SDL_FRect *p_rect_spawn_bounds) {
	player_controlled[*p_entityCount] = true;
	spawn_characters(1, p_entityCount, rects, colors, healths, p_rect_spawn_bounds);
	printf("<PLAYER SPAWNED>%s\n", player_controlled[*p_entityCount] ? "true" : "false");
}

void spawn_house(size_t *p_entityCount, c_oxygenator oxygenators[], SDL_FRect rects[], c_color colors[], SDL_Rect *p_display_bounds) {
	const uint32_t HOUSE_WIDTH = 300;
	const uint32_t HOUSE_HEIGHT = 300;
	oxygenators[*p_entityCount] = true;
	SDL_FRect *p_house_rect = &rects[*p_entityCount];
	*p_house_rect = (SDL_FRect) {
		.w = HOUSE_WIDTH,
		.h = HOUSE_HEIGHT,
		.x = (p_display_bounds->w / 2) - (HOUSE_WIDTH / 2),
		.y = (p_display_bounds->h / 2) - (HOUSE_HEIGHT / 2),
	};
	c_color *p_house_color = &colors[*p_entityCount];
	*p_house_color = (c_color) {
		.red = 100,
		.green = 100,
		.blue = 100
	};
	printf("<HOUSE_SPAWNED> %zu", *p_entityCount);
	(*p_entityCount)++;
	printfFRect(p_house_rect);
}

void init(SDL_Rect *p_display_bounds, size_t *p_entityCount, c_oxygenator oxygenators[], SDL_FRect rects[], c_color colors[], c_health healths[], bool player_controlled[]) {
	spawn_house(p_entityCount, oxygenators, rects, colors, p_display_bounds);
	SDL_FRect character_spawn_bounds;
	SDL_RectToFRect(p_display_bounds, &character_spawn_bounds);
	spawn_characters(10, p_entityCount, rects, colors, healths, &character_spawn_bounds);
	spawn_player(p_entityCount, player_controlled, rects, colors, healths, &character_spawn_bounds);
}

void update_player(long *p_time_since_last_tick, size_t *p_entityCount, bool player_controlled[], SDL_FRect rects[], bool left, bool right, bool up, bool down) {
	float pixels_per_foot = 50.0f;
	float fps = 10.0f;
	float fpns = fps / 1000000000;
	float delta = ((*p_time_since_last_tick) * fpns) * pixels_per_foot;

	for(size_t i = 0; i < *p_entityCount; i++) {
		if(player_controlled[i] == true) {
			SDL_FRect *p_player_rect = &rects[i];
			if(left) {
				p_player_rect->x -= delta;
			}
			if(right) {
				p_player_rect->x += delta;
			}
			if(up) {
				p_player_rect->y -= delta;
			}
			if(down) {
				p_player_rect->y += delta;
			}
		}
	}
}

// TODO: This renders a health bar above houses and it should not
void render_characters(size_t *p_entityCount, c_health healths[], SDL_FRect rects[], SDL_Renderer *p_sdl_renderer) {
	SDL_FRect health_background = {
		.x = 0,
		.y = 0,
		.w = 80,
		.h = 10
	};
	SDL_FRect health_foreground = {
		.x = 0,
		.y = 0,
		.w = 80,
		.h = 20
	};

	for(size_t i = 0; i < *p_entityCount; i++) {
		if(&healths[i] != NULL && &rects[i] != NULL) {
			float health_x = rects[i].x - (health_background.w / 2) + (rects[i].w / 2);
			float health_y = rects[i].y - 30;

			health_background.x = health_x;
			health_background.y = health_y;
			health_foreground.x = health_x;
			health_foreground.y = health_y - (health_background.h / 2);
			health_foreground.w = ((float)healths[i] / MAX_HEALTH ) * health_background.w;
			// Render Red Bar Background
			SDL_SetRenderDrawColor(p_sdl_renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(p_sdl_renderer, &health_background);
			// Render Green Bar Background
			SDL_SetRenderDrawColor(p_sdl_renderer, 0, 255, 0, 100);
			SDL_RenderFillRect(p_sdl_renderer, &health_foreground);
		}
	}
}

void cleanup(SDL_Window *p_sdl_window) {
	SDL_DestroyWindow(p_sdl_window);
	SDL_Quit();
}

int main() {
	SDL_Window *p_sdl_window;
	SDL_Renderer *p_sdl_renderer;

	size_t entityCount = 0;
	
	// components
	SDL_FRect rects[MAX_ENTITY_COUNT] = {};
	c_color colors[MAX_ENTITY_COUNT] = {};
	bool player_controlled[MAX_ENTITY_COUNT] = {false};
	c_health healths[MAX_ENTITY_COUNT] = {};
	c_oxygenator oxygenators[MAX_ENTITY_COUNT] = {false};

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
	init(&displayBounds, &entityCount, oxygenators, rects, colors, healths, player_controlled);
	printf("ENTITY COUNT: %zu\n", entityCount);

	bool player_left = false;
	bool player_right = false;
	bool player_up = false;
	bool player_down = false;

	struct timespec start, end;
	long time_since_last_tick = 0;
	long time_since_last_fps_calc = 0;
	uint32_t fps = 0;
	uint32_t frame_count = 0;
	timespec_get(&start, TIME_UTC);

	bool running = true;
	while(running) {
		timespec_get(&end, TIME_UTC);
		time_since_last_tick = ((end.tv_sec - start.tv_sec) * 1000000000 ) + (end.tv_nsec - start.tv_nsec);
		timespec_get(&start, TIME_UTC);
		//printf("NS SINCE LAST TICK: %ld\n", time_since_last_tick);

		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_KEY_DOWN: 
					// TODO: add proper logging with levels like trace/debug
					// printf("detected a keyboard event. %d\n", event.key.key);
					switch(event.key.key) {
						case SDLK_ESCAPE:
							if(event.key.key == SDLK_ESCAPE) {
								running = false;
							}
							break;
						case SDLK_LEFT:
							printf("LEFT\n");
							player_left = true;
							break;
						case SDLK_RIGHT:
							printf("RIGHT\n");
							player_right = true;
							break;
						case SDLK_UP:
							printf("UP\n");
							player_up = true;
							break;
						case SDLK_DOWN:
							printf("DOWN\n");
							player_down = true;
							break;
					}
					break;
				case SDL_EVENT_KEY_UP: 
					// TODO: add proper logging with levels like trace/debug
					switch(event.key.key) {
						case SDLK_LEFT:
							player_left = false;
							break;
						case SDLK_RIGHT:
							player_right = false;
							break;
						case SDLK_UP:
							player_up = false;
							break;
						case SDLK_DOWN:
							player_down = false;
							break;
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
		}
		
		SDL_SetRenderDrawColor(p_sdl_renderer, 0, 0, 0, 0x00);
		SDL_RenderClear(p_sdl_renderer);
		for(int i = 0; i < entityCount; i++) {
			SDL_SetRenderDrawColor(p_sdl_renderer, colors[i].red, colors[i].green, colors[i].blue, SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(p_sdl_renderer, &rects[i]);
		}


		time_since_last_fps_calc += time_since_last_tick;
		frame_count += 1;
		if(time_since_last_fps_calc > 1000000000) {
			fps = frame_count;
			frame_count = 0;
			time_since_last_fps_calc = 0;
		}

		int length = snprintf(NULL, 0, "FPS: %u", fps);
		char* str = malloc(length + 1);
		snprintf(str, length + 1, "FPS: %u", fps);

		render_characters(&entityCount, healths, rects, p_sdl_renderer);
		SDL_RenderDebugText(p_sdl_renderer, 10, 10, str);
		SDL_RenderPresent(p_sdl_renderer);
		update_player(&time_since_last_tick, &entityCount, player_controlled, rects, player_left, player_right, player_up, player_down);
		sys_health_oxygenator_boundingBox(&time_since_last_tick, &entityCount, healths, oxygenators, rects);

	}
	cleanup(p_sdl_window);
	return 0;
}
