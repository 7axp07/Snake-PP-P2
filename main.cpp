// based on the template provided for the second project

#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SNAKE_LENGTH 100
#define TILE_SIZE 30

// Changable parameters
#define SCR_WIDTH 600
#define SCR_HEIGHT 400
#define POINT_PER_FOOD 1
#define SPEED_UP 5
#define SPEED_UP_FREQUENCY 5
#define INITIAL_POS_X SCR_WIDTH/2
#define INITIAL_POS_Y SCR_HEIGHT/2



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

void moveSnake(Snake &snake) {
    for (int i = snake.length - 1; i > 0; --i) {
        snake.body[i] = snake.body[i - 1]; // movement
    }
    switch (snake.direction) { 
        case SDLK_UP: 
            snake.body[0].y -= TILE_SIZE;
            break;
        case SDLK_DOWN: 
            snake.body[0].y += TILE_SIZE;
            break;
        case SDLK_LEFT:
            snake.body[0].x -= TILE_SIZE;
            break;
        case SDLK_RIGHT: 
            snake.body[0].x += TILE_SIZE;
            break;
    }
}

void changeDirection(Snake &snake, int newDirection) {
    if ((snake.direction == SDLK_UP && newDirection == SDLK_DOWN) ||
        (snake.direction == SDLK_DOWN && newDirection == SDLK_UP) ||
        (snake.direction == SDLK_LEFT && newDirection == SDLK_RIGHT) ||
        (snake.direction == SDLK_RIGHT && newDirection == SDLK_LEFT)) {
        return; //prevents reversing
    }
    snake.direction = newDirection;
}

void addSnakePart(Snake &snakeInfo) {
	snakeInfo.body[snakeInfo.length] = snakeInfo.body[snakeInfo.length - 1];
	snakeInfo.length++;
}

int forceTurn(Snake &snake) {
    int dir = snake.direction;
    int x = snake.body[0].x;
    int y = snake.body[0].y;

	if (x <= TILE_SIZE && dir != SDLK_UP && dir != SDLK_DOWN) { 
		if (y<TILE_SIZE+31) { return dir = SDLK_DOWN; }
		else { return dir = SDLK_UP; }
	}
	else if (x >= SCR_WIDTH - TILE_SIZE && dir != SDLK_DOWN && dir != SDLK_UP) {
		if (y >= SCR_HEIGHT - TILE_SIZE) { return dir = SDLK_UP; }
		else { return dir = SDLK_DOWN; }
	}
	if (y <= TILE_SIZE+50&& dir != SDLK_LEFT && dir != SDLK_RIGHT) {
		if (x >= SCR_WIDTH - TILE_SIZE) { return dir = SDLK_LEFT; }
		else { return dir = SDLK_RIGHT; }
	}
	else if (y >= SCR_HEIGHT - TILE_SIZE && dir != SDLK_RIGHT && dir != SDLK_LEFT) {
		if (x <= TILE_SIZE) { return dir = SDLK_RIGHT; }
		else { return dir = SDLK_LEFT; }
	}

    return dir;
}


// ------------- GAME FUNCTIONS -------------

// Collision

bool checkCollision(const Point &point, const Snake &snake, bool isSnake) {
	int l = isSnake ? snake.length : 1; double d = isSnake ? 2.0 : 1.5; int i = isSnake ? 1 : 0;
    for (i; i < l; ++i) {
    if (((point.x >= snake.body[i].x-TILE_SIZE/d)
	&& (point.x <= snake.body[i].x + TILE_SIZE/d))
	&& ((point.y >= snake.body[i].y - TILE_SIZE/d)
	&& (point.y <= snake.body[i].y + TILE_SIZE/d))) {return true;}} // true = collided
    return false;
}

// Food 
void generateFood(Point &food, const Snake &snake) {
	do {
        food.x = (rand() % (SCR_WIDTH / TILE_SIZE)) * TILE_SIZE;
        do{ food.y = (rand() % (SCR_HEIGHT / TILE_SIZE)) * TILE_SIZE;} while (food.y <= TILE_SIZE+50);
    } while (checkCollision(food, snake, true));
}

void checkFoodEaten(Snake &snake, Point &food, int &points) {
	if (checkCollision(food, snake, false)) {
		addSnakePart(snake);
		generateFood(food, snake);
		points+= POINT_PER_FOOD;
	}
}

// New Game (n)
void newGame(Snake &snake, Point &food, int &frames, double &worldTime, double &fpsTimer, double &fps, int &t1, Uint32 &lastMove) {
	snake = {{{INITIAL_POS_X, INITIAL_POS_Y}}, 1, SDLK_RIGHT};
	generateFood(food, snake);
	frames = 0;
	worldTime = 0;
	fpsTimer = 0;
	fps = 0;
	t1 = SDL_GetTicks();
	lastMove = SDL_GetTicks();
}

// Loading BMPs
SDL_Surface* loadSurface(const char *path) {
	SDL_Surface* s = SDL_LoadBMP(path);
	if (s == NULL) {
		printf("Unable to load image %s! SDL Error: %s\n", path, SDL_GetError());
		return NULL;
	}
	return s;
}

void updatePoints(int &points, int &slowness, int &speed, int &lastMilestone) {
	if (points > 0 && points % SPEED_UP_FREQUENCY == 0 && points != lastMilestone) {
		if (slowness >= SPEED_UP*2){
			slowness -= SPEED_UP;
			speed++;
		}
		lastMilestone = points;
	}
}

// ------------- MAIN FUNCTION -------------

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc, points, lastMilestone, slowness, speed, gameover;
	double delta, worldTime, fpsTimer, fps;
	SDL_Event event;
	SDL_Surface *screen, *charset,  *snake, *food;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	// initialization
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
	//
	// turning off cursor
	SDL_ShowCursor(SDL_DISABLE);

	// loading bmp
	charset = loadSurface("./cs8x8.bmp"); //SDL_LoadBMP("./cs8x8.bmp");
	food = loadSurface("./food.bmp");
	snake = loadSurface("./snakebody.bmp");
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

	Snake snakeInfo = {{{INITIAL_POS_X, INITIAL_POS_Y}}, 1, SDLK_RIGHT};
	Point foodInfo;
	//Point redPoint;
	generateFood(foodInfo, snakeInfo);
	
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();
	frames = 0; fpsTimer = 0; fps = 0; worldTime = 0;
	points = 0; lastMilestone = 0;
	slowness = 100; // Increase value to slow down
	speed = 1; // To indicate speed level
    Uint32 lastMove = SDL_GetTicks();
	gameover = 0; quit = 0;

	while(!quit) {
		t2 = SDL_GetTicks();
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

		SDL_FillRect(screen, NULL, czarny);

		drawSurface(screen, food, foodInfo.x, foodInfo.y);
		for (int i = 0; i < snakeInfo.length; i++) {
			drawSurface(screen, snake, snakeInfo.body[i].x, snakeInfo.body[i].y);
		}
		    
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

		//  info text
		drawRectangle(screen, 4, 4, SCR_WIDTH - 8, 50, czerwony, niebieski);
		if (!gameover){
		sprintf(text, "Elapsed time = %.1lf s  %.0lf fps. Points = %.2d. Speed = %.0d", worldTime, fps, points, speed);
		drawString(screen, 4, 10, text, charset);
		sprintf(text, "Esc - exit, n - new game, arrow keys - moving");
		drawString(screen, 4, 20, text, charset);

		sprintf(text, "Project by: ,Copyright 2025. ");
		drawString(screen, screen->w - strlen(text)*8 -4 , 30, text, charset);
		sprintf(text, "Implemented requirements: 1-4, A, B");	
		drawString(screen, screen->w - strlen(text)*8 -4 , 40, text, charset);	
		}
		else {
			sprintf(text, "Game Over! Points = %.2d", points);
			drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);
			sprintf(text,"Press n to start a new game or Esc to exit");
			drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 30, text, charset);
		}
		updatePoints(points, slowness, speed, lastMilestone);
			
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// handling of events 
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if(event.key.keysym.sym == SDLK_n) {
						newGame(snakeInfo, foodInfo, frames, fpsTimer, fps, worldTime, t1, lastMove);
						gameover = 0;
						}
					else if(event.key.keysym.sym == SDLK_LEFT) changeDirection(snakeInfo, SDLK_LEFT);
            		else if(event.key.keysym.sym == SDLK_RIGHT) changeDirection(snakeInfo, SDLK_RIGHT);
            		else if(event.key.keysym.sym == SDLK_UP) changeDirection(snakeInfo, SDLK_UP);
            		else if(event.key.keysym.sym == SDLK_DOWN) changeDirection(snakeInfo, SDLK_DOWN);
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		if (!gameover && SDL_GetTicks() - lastMove > slowness) {
			moveSnake(snakeInfo);
			if (snakeInfo.body[0].x < TILE_SIZE || snakeInfo.body[0].x >= SCR_WIDTH-TILE_SIZE || snakeInfo.body[0].y < TILE_SIZE+50|| snakeInfo.body[0].y >= SCR_HEIGHT-TILE_SIZE){
				snakeInfo.direction= forceTurn(snakeInfo);
			}
			checkFoodEaten(snakeInfo, foodInfo, points);
			if (snakeInfo.length > 2 && checkCollision(snakeInfo.body[0], snakeInfo, true)){ gameover = 1; }
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
