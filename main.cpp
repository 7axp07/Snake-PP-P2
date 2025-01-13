// based on the template provided for the second project

#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCR_WIDTH 600
#define SCR_HEIGHT 400
#define SNAKE_LENGTH 100
#define TILE_SIZE 20


// Structs for storing information (positions, snake lenght etc)
struct Point {
	int x, y;
};

struct Snake {
	Point body[SNAKE_LENGTH];
	int length;
	int direction;
};

// ------------- SDL DRAWING FUNCTIONS ------------- taken from the template project

// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void drawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void drawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

// draw a single pixel
void drawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void drawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		drawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

// draw a rectangle of size l by k
void drawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	drawLine(screen, x, y, k, 0, 1, outlineColor);
	drawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	drawLine(screen, x, y, l, 1, 0, outlineColor);
	drawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		drawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};


// ------------- SNAKE FUNCTIONS -------------

void moveSnake(Snake &snake, SDL_Surface *screen) {
    for (int i = snake.length - 1; i > 0; --i) {
        snake.body[i] = snake.body[i - 1]; // movement
    }
    switch (snake.direction) { 
        case SDLK_UP: 
            if (snake.body[1].y != snake.body[0].y - TILE_SIZE) {
				snake.body[0].y -= TILE_SIZE;
				}
			else { snake.body[0].y += TILE_SIZE;}
            break;
        case SDLK_DOWN: 
            if (snake.body[1].y != snake.body[0].y + TILE_SIZE) { snake.body[0].y += TILE_SIZE;}
			else {snake.body[0].y -= TILE_SIZE;}
            break;
        case SDLK_LEFT:
            if (snake.body[1].x != snake.body[0].x - TILE_SIZE) { snake.body[0].x -= TILE_SIZE;}
			else { snake.body[0].x += TILE_SIZE; }
            break;
        case SDLK_RIGHT: 
            if (snake.body[1].x != snake.body[0].x + TILE_SIZE) { snake.body[0].x += TILE_SIZE;}
			else { snake.body[0].x -= TILE_SIZE;}
            break;
    }
}


// ------------- GAME FUNCTIONS -------------

bool checkCollision(const Point &point, const Snake &snake) {
    for (int i = 0; i < snake.length; ++i) {
        if (point.x == snake.body[i].x && point.y == snake.body[i].y) return true; // true = collided
    }
    return false;
}

void generateFood(Point &food, const Snake &snake) {
	do {
        food.x = (rand() % (SCR_WIDTH / TILE_SIZE)) * TILE_SIZE;
        do{ food.y = (rand() % (SCR_HEIGHT / TILE_SIZE)) * TILE_SIZE;} while (food.y <= 32);
    } while (checkCollision(food, snake));
}


void newGame(Snake &snake, Point &food, int &frames, double &worldTime, double &fpsTimer, double &fps, int &t1, Uint32 &lastMove) {
	snake = {{{SCR_WIDTH / 2, SCR_HEIGHT / 2}}, 1, SDLK_RIGHT};
	generateFood(food, snake);
	frames = 0;
	worldTime = 0;
	fpsTimer = 0;
	fps = 0;
	t1 = SDL_GetTicks();
	lastMove = SDL_GetTicks();
}

SDL_Surface* loadSurface(const char *path) {
	SDL_Surface* s = SDL_LoadBMP(path);
	if (s == NULL) {
		printf("Unable to load image %s! SDL Error: %s\n", path, SDL_GetError());
		return NULL;
	}
	return s;
}

// ------------- MAIN FUNCTION -------------

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, snakeslowness;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *snake, *food;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	printf("Printf commands go here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	rc = SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCR_WIDTH, SCR_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Snake Game");


	screen = SDL_CreateRGBSurface(0, SCR_WIDTH, SCR_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);

	// turning off cursor
	SDL_ShowCursor(SDL_DISABLE);

	// loading cs8x8.bmp
	charset = loadSurface("./cs8x8.bmp"); //SDL_LoadBMP("./cs8x8.bmp");
	snake = loadSurface("./snakebody.bmp");
	food = loadSurface("./food.bmp");
	if(charset == NULL || food == NULL || snake == NULL) {
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	Snake snakeInfo = {{{SCR_WIDTH / 2, SCR_HEIGHT / 2}}, 1, SDLK_RIGHT};
	Point foodInfo;
	generateFood(foodInfo, snakeInfo);
	
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	int slowness = 100; // Increase value to slow down
    Uint32 lastMove = SDL_GetTicks();


	while(!quit) {
		t2 = SDL_GetTicks();
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

		SDL_FillRect(screen, NULL, czarny);

		drawSurface(screen, snake, snakeInfo.body[0].x, snakeInfo.body[0].y);
		drawSurface(screen, food, foodInfo.x, foodInfo.y);
			    
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};

		//  info text
		drawRectangle(screen, 4, 4, SCR_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "Elapsed time = %.1lf s  %.0lf fps", worldTime, fps);
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		//
		sprintf(text, "Esc - exit, n - new game, arrows - moving");
		drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if(event.key.keysym.sym == SDLK_n) {
						newGame(snakeInfo, foodInfo, frames, fpsTimer, fps, worldTime, t1, lastMove);
						}
					else if(event.key.keysym.sym == SDLK_LEFT) snakeInfo.direction = SDLK_LEFT;
					else if(event.key.keysym.sym == SDLK_RIGHT) snakeInfo.direction = SDLK_RIGHT;
					else if(event.key.keysym.sym == SDLK_UP) snakeInfo.direction = SDLK_UP;
					else if(event.key.keysym.sym == SDLK_DOWN) snakeInfo.direction = SDLK_DOWN;

					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		 if (SDL_GetTicks() - lastMove > slowness) {
            moveSnake(snakeInfo, screen);
            lastMove = SDL_GetTicks();
        }
        };

	// freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
