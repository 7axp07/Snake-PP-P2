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
#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#define POINT_PER_FOOD 1
#define SPEED_UP 5
#define SPEED_UP_FREQUENCY 5 // in seconds
#define INITIAL_POS_X SCR_WIDTH/2
#define INITIAL_POS_Y SCR_HEIGHT/2
#define RED_DOT_SPAWN_CHANCE 10
#define INITIAL_LENGTH 2
#define SHORTEN_SNAKE_LENGHT 1
#define RED_DOT_TIME 5



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
		if (y<TILE_SIZE+50) { return dir = SDLK_DOWN; }
		else { return dir = SDLK_UP; }
	}
	else if (x >= SCR_WIDTH - TILE_SIZE && dir != SDLK_DOWN && dir != SDLK_UP) {
		if (y >= SCR_HEIGHT - TILE_SIZE) { return dir = SDLK_UP; }
		else { return dir = SDLK_DOWN; }
	}
	if (y <= TILE_SIZE+50 && dir != SDLK_LEFT && dir != SDLK_RIGHT) {
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

// Food related functions (Red & Blue Dots)
void generateFood(Point &food, const Snake &snake) {
	do {
        do {food.x = (rand() % (SCR_WIDTH / TILE_SIZE)) * TILE_SIZE;} while (food.x <= TILE_SIZE);
        do{ food.y = (rand() % (SCR_HEIGHT / TILE_SIZE)) * TILE_SIZE - TILE_SIZE;} while (food.y <= TILE_SIZE+50);
    } while (checkCollision(food, snake, true));
}

void deSpawn(Point &redDot, SDL_Surface *screen, int czarny) {
	drawRectangle(screen, redDot.x, redDot.y, TILE_SIZE, TILE_SIZE, czarny, czarny);
	redDot.x = -10;
	redDot.y = -10;
}

void checkFoodEaten(Snake &snake, Point &food, int &points) {
	if (checkCollision(food, snake, false)) {
		addSnakePart(snake);
		generateFood(food, snake);
		points+= POINT_PER_FOOD;
	}
}

void redDotFunction(Point &redDot, int &redDotSpawn, double &redDotTimer, Snake &snakeInfo, int &slowness, unsigned int &speed, SDL_Surface *screen, int czarny, int czerwony, int &points) {
	if (redDotTimer <= 0) {
			redDotSpawn = 0;
			redDotTimer = RED_DOT_TIME;
			deSpawn(redDot, screen, czarny);
		}
	if (redDotSpawn==0 && (rand() % RED_DOT_SPAWN_CHANCE) == 1){
		redDotSpawn = 1;
		generateFood(redDot, snakeInfo);
	}
	if (checkCollision(redDot, snakeInfo, false)&& redDotTimer > 0){
		redDotTimer = RED_DOT_TIME;
		points++;
		int r = rand() % 2; 
		if (snakeInfo.length>1 && r == 0) { // shortening the snake
			snakeInfo.length -= SHORTEN_SNAKE_LENGHT;
		}
		else { // slows down
			slowness += SPEED_UP*SHORTEN_SNAKE_LENGHT;
			speed == 1 ? speed = 1 : speed -= SHORTEN_SNAKE_LENGHT;
		}
		redDotSpawn = 0;
	}
}

void checkSpeedup(double &worldTime, unsigned int &speed, int &slowness, double &lastSpeedupTime) {
    if (worldTime - lastSpeedupTime >= SPEED_UP_FREQUENCY) {
        slowness -= SPEED_UP;
        speed++;
        lastSpeedupTime = worldTime; // Update the last speedup time
    }
}

// info text

void drawInfoText(SDL_Surface *screen, SDL_Surface *charset, char *text, double &worldTime, double &fps, int &points, unsigned int &speed, int &gameover){
		if (!gameover){
		sprintf(text, "Elapsed time = %.1lf s  %.0lf fps. Points = %.2d. Speed = %.0d", worldTime, fps, points, speed);
		drawString(screen, 4, 10, text, charset);
		sprintf(text, "Esc - exit, n - new game, arrow keys - moving");
		drawString(screen, 4, 20, text, charset);

		sprintf(text, "Project by: ,Copyright 2025. ");
		drawString(screen, screen->w - strlen(text)*8 -4 , 30, text, charset);
		sprintf(text, "Implemented requirements: 1-4, A, B, C, D");	
		drawString(screen, screen->w - strlen(text)*8 -4 , 40, text, charset);	
		}
		else {
			sprintf(text, "Game Over! Points = %.2d", points);
			drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);
			sprintf(text,"Press n to start a new game or Esc to exit");
			drawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 30, text, charset);
		}
}

// New Game (n)
void newGame(Snake &snake, Point &food, int &frames, double &worldTime, double &fpsTimer, double &fps, int &t1, Uint32 &lastMove) {
	snake = {{{INITIAL_POS_X, INITIAL_POS_Y}}, INITIAL_LENGTH, SDLK_RIGHT};
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

//

// ------------- MAIN FUNCTION -------------

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc, points, slowness, gameover;
	unsigned int speed;
	double delta, worldTime, lastSpeedup, fpsTimer, fps;
	SDL_Event event;
	SDL_Surface *screen, *charset,  *snake, *food, *reddot;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) { printf("SDL_Init error: %s\n", SDL_GetError()); return 1;}
	rc = SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) { SDL_Quit(); printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError()); return 1; };
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCR_WIDTH, SCR_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Snake Game");

	screen = SDL_CreateRGBSurface(0, SCR_WIDTH, SCR_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);
	// turning off cursor
	SDL_ShowCursor(SDL_DISABLE);

	// loading bmp
	charset = loadSurface("./cs8x8.bmp"); //SDL_LoadBMP("./cs8x8.bmp");
	food = loadSurface("./food.bmp");
	snake = loadSurface("./snakebody.bmp");
	reddot = loadSurface("./reddot.bmp");
	if(charset == NULL || food == NULL || snake == NULL || reddot == NULL) {
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	//creating initial snake and food
	Snake snakeInfo = {{{INITIAL_POS_X, INITIAL_POS_Y}}, INITIAL_LENGTH, SDLK_RIGHT};
	Point foodInfo;
	Point redDot; double redDotTimer = RED_DOT_TIME; int redDotSpawn = 0;
	generateFood(foodInfo, snakeInfo);
	
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();
	frames = 0; fpsTimer = 0; fps = 0; worldTime = 0; lastSpeedup = 0;
	points = 0; 
	slowness = 100; // Increase value to slow down
	speed = 1; // To indicate speed level
    Uint32 lastMove = SDL_GetTicks(); // For appropriate movement 
	gameover = 0; quit = 0;

	while(!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001; t1 = t2;

		worldTime += delta;
		if (redDotSpawn){redDotTimer -= delta;}

		SDL_FillRect(screen, NULL, czarny);

		drawSurface(screen, food, foodInfo.x, foodInfo.y);
		for (int i = 0; i < snakeInfo.length; i++) {
			drawSurface(screen, snake, snakeInfo.body[i].x, snakeInfo.body[i].y);
		}
		if (redDotSpawn){
			drawSurface(screen, reddot, redDot.x, redDot.y);
			drawRectangle(screen, 0, 52, (SCR_WIDTH/RED_DOT_TIME)*redDotTimer , 4, czerwony, czerwony);	
		}
		    
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

		//  info text
		drawRectangle(screen, 4, 4, SCR_WIDTH - 8, 50, czerwony, niebieski);
		drawInfoText(screen, charset, text, worldTime, fps, points, speed, gameover);
			
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		checkSpeedup(worldTime, speed, slowness, lastSpeedup);


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
			if (snakeInfo.body[0].x < TILE_SIZE || snakeInfo.body[0].x >= SCR_WIDTH-TILE_SIZE || snakeInfo.body[0].y < TILE_SIZE+45|| snakeInfo.body[0].y >= SCR_HEIGHT-TILE_SIZE){
				snakeInfo.direction= forceTurn(snakeInfo);
			}
			checkFoodEaten(snakeInfo, foodInfo, points);
			redDotFunction(redDot, redDotSpawn, redDotTimer, snakeInfo, slowness, speed, screen, czarny, czerwony, points);
		
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
