#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <SDL_ttf.h>
#include <string>
#include <SDL_mixer.h>
#define WIDTH 1280
#define HEIGHT 720
#define SPEED 9
#define BALL_SPEED 10
#define SIZE 16
#define COL 13
#define ROW 8
#define PI 3.1415
#define SPACING 16
#define FONT_SIZE 40

Mix_Music* backgroundmusic;
Mix_Chunk* collision_sound;
TTF_Font* font;
SDL_Window* window;
SDL_Renderer* renderer;
bool running;
int frameCount, timerFPS, lastFrame, fps;
SDL_Color color;
SDL_Rect paddle, ball, lives, brick;
float velY, velX;
int liveCount;
bool bricks[ROW * COL];
int livecount = 3;		
int points;
bool gameover = true;

void init_sound(std::string path) {
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}
	collision_sound = Mix_LoadWAV(path.c_str());
	Mix_PlayChannel(-1, collision_sound, 0);

}
SDL_Texture* loadTexture(std::string path)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}

void resetBricks()
{
	for (int i = 0; i < COL * ROW; i++) bricks[i] = 1;
	liveCount = 3;
	paddle.x = (WIDTH / 2) - (paddle.w / 2);
	ball.y = paddle.y - (paddle.h * 4);
	velY = BALL_SPEED / 2;
	velX = 0;
	ball.x = WIDTH / 2 - (SIZE / 2);
}

void setBricks(int i) 
{
	brick.x = (((i % COL) + 1) * SPACING) + ((i % COL) * brick.w) - (SPACING / 2);
	brick.y = brick.h * 3 + (((i % ROW) + 1) * SPACING) + ((i % ROW) * brick.h) - (SPACING / 2);
}

void write(std::string text, int x, int y) {
	SDL_Surface* surface;
	SDL_Texture* texture;
	const char* t = text.c_str();
	surface = TTF_RenderText_Solid(font, t, color);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	lives.w = surface->w;
	lives.h = surface->h;
	lives.x = x - lives.w;
	lives.y = y - lives.h;
	SDL_FreeSurface(surface);
	SDL_RenderCopy(renderer, texture, NULL, &lives);
	SDL_DestroyTexture(texture);
}

void update()
{
	if (liveCount <= 0)
	{
		init_sound("lose.mp3");
		gameover = false;
		for (int i = 1; i < COL * ROW; i++) bricks[i] = 0;
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, loadTexture("gameover.png"), NULL, NULL);
		SDL_RenderPresent(renderer);
		velX = 0;
		velY = 0;
		SDL_Event e;
		const Uint8* keystates = SDL_GetKeyboardState(NULL);
		while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
		if (keystates[SDL_SCANCODE_ESCAPE]) running = false;
		if (keystates[SDL_SCANCODE_SPACE])
		{
			SDL_RenderClear(renderer);
			resetBricks();
			gameover = true;
		}


		points = 0;
	}
	if (SDL_HasIntersection(&ball, &paddle))
	{
		double rel = (paddle.x + (paddle.w / 2)) - (ball.x + (SIZE / 2));
		double norm = rel / (paddle.w / 2);
		double bounce = norm * (5 * PI / 12);
		velY = -BALL_SPEED * cos(bounce);
		velX = BALL_SPEED * -sin(bounce);
		init_sound("collision.mp3");
	}
	if (ball.y <= 0)
	{
		velY = -velY;
		init_sound("collision.mp3");
	}
	if (ball.y + SIZE >= HEIGHT)
	{
		ball.x = paddle.x + 80;
		ball.y = paddle.y - (paddle.h * 4);
		velY = BALL_SPEED / 2;
		velX = 0;
		liveCount--;
	}
	if (ball.x <= 0 || ball.x + SIZE >= WIDTH)
	{
		velX = -velX;
		init_sound("collision.mp3");
	}
	ball.x += velX;
	ball.y += velY;
	if (paddle.x < 0)
	{
		paddle.x = 0;
	}
	if (paddle.x + paddle.w > WIDTH)
	{
		paddle.x = WIDTH - paddle.w;
	}
	bool reset = 1;
	for (int i = 0; i < COL * ROW; i++) {
		setBricks(i);
		if (SDL_HasIntersection(&ball, &brick) && bricks[i]) {
			bricks[i] = 0;
			if (ball.x >= brick.x) { velX = -velX; ball.x -= 20; }
			if (ball.x <= brick.x) { velX = -velX; ball.x += 20; }
			if (ball.y <= brick.y) { velY = -velY; ball.y -= 20; }
			if (ball.y >= brick.y) { velY = -velY; ball.y += 20; }
			points += 100;
			init_sound("collision.mp3");
		}
		if (bricks[i]) reset = 0;
	}
	if (reset) resetBricks();
}

void input() {
	SDL_Event e;
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
	if (keystates[SDL_SCANCODE_ESCAPE]) running = false;
	if (keystates[SDL_SCANCODE_LEFT]) paddle.x -= SPEED;
	if (keystates[SDL_SCANCODE_RIGHT]) paddle.x += SPEED;

}
void render() 
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 255);
	SDL_RenderClear(renderer);

	frameCount++;
	timerFPS = SDL_GetTicks() - lastFrame;
	if (timerFPS < (1000 / 60)) 
	{
		SDL_Delay((1000 / 60) - timerFPS);
	}
		
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &paddle);
	SDL_RenderFillRect(renderer, &ball);
	write(std::to_string(liveCount), WIDTH / 2 + FONT_SIZE / 2, FONT_SIZE * 1.5);
	write(std::to_string(fps),  1240+ FONT_SIZE /2, FONT_SIZE * 1.5);
	write("FPS:", 1150 + FONT_SIZE / 2, FONT_SIZE * 1.5);
	write(std::to_string(points), 300 + FONT_SIZE / 2, FONT_SIZE * 1.5);
	write("POINTS:", 150 + FONT_SIZE / 2, FONT_SIZE * 1.5);
	write("LIVES:", 600 + FONT_SIZE / 2, FONT_SIZE * 1.5);
	for (int i = 0; i < COL * ROW; i++) 
	{
		SDL_SetRenderDrawColor(renderer, 200, 50, 0, 255);
		if (i % 2 == 0)SDL_SetRenderDrawColor(renderer, 0, 200, 50, 255);
		if (bricks[i]) {
			setBricks(i);
			SDL_RenderFillRect(renderer, &brick);
		}
	}

	SDL_RenderPresent(renderer);
}


int main(int argc, char* args[]) 
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Failed at SDL_Init()" << std::endl;
	}
	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer) < 0)
	{
		std::cout << "Failed at SDL_CreateWindowAndRenderer()" << std::endl;
	}
	SDL_SetWindowTitle(window, "Breakout");
	TTF_Init();
	Mix_Init(MIX_INIT_MP3);
	font = TTF_OpenFont("Minecraft.ttf", FONT_SIZE);
	color.r = color.g = color.b = 255;
	running = 1;
	static int lastTime = 0;
	paddle.h = 12; 
	paddle.w = WIDTH / 8;
	paddle.y = HEIGHT - paddle.h - 32;
	ball.w = ball.h = SIZE;
	brick.w = (WIDTH - (SPACING * COL)) / COL;
	brick.h = 22;
	points = 0;

	resetBricks();
	gameover = true;

	while (running) {
		lastFrame = SDL_GetTicks();
		if (lastFrame >= lastTime + 1000) {
			lastTime = lastFrame;
			fps = frameCount;
			frameCount = 0;
		}


		update();
		
		if (gameover == true)
		{
			backgroundmusic = Mix_LoadMUS("Mixi.mp3");
			Mix_PlayMusic(backgroundmusic, 10);
			render();
			input();
		}
	}



	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
