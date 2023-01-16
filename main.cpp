#define _USE_MATH_DEFINES
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#include<stdio.h>
#include <time.h>
#include <windows.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

SDL_Event event;
SDL_Surface* screen, * charset;
SDL_Surface *player_car, *car1,*car2, *car3, *bull;
SDL_Texture* scrtex;
SDL_Window* window;
SDL_Renderer* renderer;

int black;
int green;
int red;
int blue;


struct bullet {
	int x = 0;
	int y = 380;
	bool alive = false;
};

struct car {
	SDL_Surface* model;
	int x = 320;
	int y = 55;
	int final_y = 55;
	bool alive = false;
	/*
	1 - chaser
	2 - attacs
	*/
	int type = 1; 
	bool enemy = true;
};

struct result {
	double time;
	int score;
};

void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
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

void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void Quit() {
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(player_car);
	SDL_FreeSurface(car1);
	SDL_FreeSurface(car2);
	SDL_FreeSurface(car3);
	SDL_FreeSurface(bull);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int Init() {
	srand(time(NULL));
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetWindowTitle(window, "Pavel Harelik 196766");
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE);
	black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	green = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	red = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	blue = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	player_car = SDL_LoadBMP("./car.bmp");
	if (player_car == NULL) {
		printf("SDL_LoadBMP(car.bmp) error: %s\n", SDL_GetError());
		return 1;
	};

	car1 = SDL_LoadBMP("./enemy_car.bmp");
	if (car1 == NULL) {
		printf("SDL_LoadBMP(enemy_car.bmp) error: %s\n", SDL_GetError());
		return 1;
	};

	car2 = SDL_LoadBMP("./enemy_car1.bmp");
	if (car1 == NULL) {
		printf("SDL_LoadBMP(enemy_car1.bmp) error: %s\n", SDL_GetError());
		return 1;
	};

	car3 = SDL_LoadBMP("./car1.bmp");
	if (car1 == NULL) {
		printf("SDL_LoadBMP(car1.bmp) error: %s\n", SDL_GetError());
		return 1;
	};

	bull = SDL_LoadBMP("./bullet.bmp");
	if (bull == NULL) {
		printf("SDL_LoadBMP(bullet.bmp) error: %s\n", SDL_GetError());
		return 1;
	};
	return 0;
}


int CompareByPoints(const void* x1, const void* x2)
{
	return (((result*)x2)->score > ((result*)x1)->score);
}

int CompareByTime(const void* x1, const void* x2)
{
	return (((result*)x2)->time > ((result*)x1)->time);
}

void Sort(int comparator, result* results, int size) {
	/*
	* 1 - sort by points
	* 2 - sort by time
	*/
	if (comparator == 1)
		qsort(results, size, sizeof(result), CompareByPoints);      // сортируем массив чисел
	else
		qsort(results, size, sizeof(result), CompareByTime);
}

result* ReadResults(int& size)
{
	FILE* file;
	file = fopen("scores.txt", "r");
	if (file) {
		fscanf(file, "%d\n", &size);
		result* results = (result*)malloc((size + 1) * sizeof(result));
		for (int i = 0; i < size; i++) {
			fscanf(file, "%d %lf\n", &results[i].score, &results[i].time);
		}
		fclose(file);
		return results;
	}
	else {
		result* results = (result*)malloc(sizeof(result));
		size = 0;
		return results;
	}
}

bool IsFree(int* road, int x, int distance,int dy = 0) {
	int width = (16 - road[(4 + dy + distance) % 72])*20;
	if (x > width && x < SCREEN_WIDTH - width)
		return true;
	return false;
}

void Pause(int& t1, int sort_by) {
	char text[50] = "Game is paused";
	bool nandle = true;
	int size;
	time_t now = time(0);
	tm* ltm = localtime(&now);

	DrawRectangle(screen, 160, 120, 320, 120, blue, red);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 130, text, charset);
	sprintf(text, "Press 'p' to continue");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 140, text, charset);
	if (sort_by == 1)
		sprintf(text, "Best scores by points:");
	else
		sprintf(text, "Best scores by time:");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 160, text, charset);

	result* results = ReadResults(size);
	Sort(sort_by, results, size);
	if (size > 5) size = 5;
	if (size == 0) {
		sprintf(text, "There is no saved scores");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 180, text, charset);
	}
	else {
		for (int i = 0; i < size; i++) {
			sprintf(text, "Score:%d, Time:%.2lf", results[i].score, results[i].time);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 180 + 10 * i, text, charset);
		}
	}
	free(results);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);

	while (true && nandle) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_p:
					t1 = SDL_GetTicks();
					nandle = false;
					break;
				case SDLK_ESCAPE:
					Quit();
				}
			}
			else if (event.type == SDL_QUIT)
				Quit();
		}
		SDL_Delay(20);
	}
}

void ShowAlert(char msg[]) {
	DrawRectangle(screen, 160, 120, 320, 60, blue, red);
	DrawString(screen, screen->w / 2 - strlen(msg) * 8 / 2, 140, msg, charset);
	DrawString(screen, screen->w / 2 - strlen("Press enter to continue") * 8 / 2, 150, "Press enter to continue", charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);

	bool nandle = true;
	while (nandle) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RETURN:
					nandle = false;
					break;
				case SDLK_ESCAPE:
					Quit();
				}
			}
			else if (event.type == SDL_QUIT)
				Quit();
		}
		SDL_Delay(20);
	}
}

bool SaveGame(double distanceance, double elapsed_time, int x, int lives, car* cars, bullet* bullets) {
	FILE* file;
	char file_name[] = "%.2d.%.2d_%.2d.%.2d.%.2d.game";
	time_t now = time(0);
	tm* ltm = localtime(&now);
	sprintf(file_name, file_name, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	file = fopen(file_name, "w");
	if (file) {
		fprintf(file, "%lf %lf %d %d\n", distanceance, elapsed_time, x, lives);
		for (int i = 0; i < 20; i++) {
			fprintf(file, "%d %d %d %d %d %d\n", cars[i].x, cars[i].y, cars[i].final_y, cars[i].alive, cars[i].type, cars[i].enemy);
			fprintf(file, "%d %d %d\n", bullets[i].x, bullets[i].y, bullets[i].alive);
		}
		fclose(file);
		ShowAlert("Succesfully saved game");
		return true;
	}
	ShowAlert("Couldn't save game");
	return false;
}

int ChooseFile(char files[50][30],int len) {
	int pos = 0;
	len--;
	while (true) {
		SDL_FillRect(screen, NULL, black);
		for (int i = 0; i <= len; i++) {
			DrawString(screen, 30, 20+10*i, files[i], charset);
		}
		DrawString(screen, 10, 20 + 10 * pos, "=>", charset);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Delay(20);
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RETURN:
					return pos;
					break;
				case SDLK_UP:
					if (pos > 0)
						pos--;
					break;
				case SDLK_DOWN:
					if (pos < len)
						pos++;
					break;
				case SDLK_ESCAPE:
					Quit();
				}
			}
			else if (event.type == SDL_QUIT)
				Quit();
		}
	}
	return 1;
}

bool LoadGame(double &distance, double &programm_time, int& x, int &lives, car* cars, bullet* bullets) {
	HANDLE hFind;
	WIN32_FIND_DATA data;
	char files[50][30];
	int n = 0;
	char filename[30];
	hFind = FindFirstFile(L"*.game*", &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do
		{
			sprintf_s(filename, "%ls", data.cFileName);
			strcpy_s(files[n++], filename);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	if (n == 0) {
		ShowAlert("There is no saved games");
		return false;
	}
	n = ChooseFile(files, n);
	FILE* file;
	file = fopen(files[n], "r");

	if (file) {
		fscanf(file, "%lf %lf %d %d", &distance, &programm_time, &x, &lives);
		for (int i = 0; i < 20; i++) {
			fscanf(file, "%d %d %d %d %d %d\n", &cars[i].x, &cars[i].y, &cars[i].final_y, &cars[i].alive, &cars[i].type, &cars[i].enemy);
			fscanf(file, "%d %d %d\n", &bullets[i].x, &bullets[i].y, &bullets[i].alive);
			if (cars[i].enemy) {
				if (cars[i].type == 1)
					cars[i].model = car1;
				else 
					cars[i].model = car2;
			}
			else 
				cars[i].model = car3;
		}
		fclose(file);
		ShowAlert("Succesfully loaded game");
		return true;
	}
	ShowAlert("Can't open file");
	return false;
}

void DrawRoad(SDL_Surface* screen, int* road, int distance) {
	int rec_size = 20, width;
	for (int i = 23; i > 1; i--) {
		width = 16 - road[(23 - i + distance) % 72];
		DrawRectangle(screen, 0, i * rec_size, rec_size * width, rec_size, green, green);
		DrawRectangle(screen, SCREEN_WIDTH - width * rec_size, i * rec_size, rec_size * width, rec_size, green, green);
	}
} 

void DrawBullets(bullet* bullets) {
	for (int i = 0; i < 20; i++) {
		if (bullets[i].alive) {
			DrawSurface(screen, bull, bullets[i].x, bullets[i].y);
		}
	}
}

void DrawCars(car *cars) {
	for (int i = 0; i < 20; i++) {
		if (cars[i].alive) {
			DrawSurface(screen, cars[i].model, cars[i].x, cars[i].y);
		}
	}
}

void DrawHood(int* road,bullet *bullets,car *cars,int x,double distance, double worldtime, int fps, int score, int lives) {
	static char text[128];
	SDL_FillRect(screen, NULL, black);
	DrawSurface(screen, player_car, x, 380);
	DrawRoad(screen, road, distance);
	DrawBullets(bullets);
	DrawCars(cars);

	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, red, blue);
	sprintf(text, "czas trwania = %.1lf s  %d klatek/s", worldtime, fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	sprintf(text, "Score = %d", score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	for (int i = 1; i <= lives; i++) {
		DrawSurface(screen, player_car, 30*i, 20);
	}

	DrawRectangle(screen, 470, 410, SCREEN_WIDTH - 470, SCREEN_HEIGHT-410, blue, blue);
	sprintf(text, "f: finish the game");
	DrawString(screen, 470, 410, text, charset);
	sprintf(text, "space: shooting");
	DrawString(screen, 470, 420, text, charset);
	sprintf(text, "arrows: moving");
	DrawString(screen, 470, 430, text, charset);
	sprintf(text, "n:start a new game");
	DrawString(screen, 470, 440, text, charset);
	sprintf(text, "s:save the game state");
	DrawString(screen, 470, 450, text, charset);
	sprintf(text, "l:load the game state");
	DrawString(screen, 470, 460, text, charset);
	sprintf(text, "p: pause/continue");
	DrawString(screen, 470, 470, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

bool LoadRoad(int* road) {
	FILE* file;
	file = fopen("road.txt",  "r");
	if (file) {
		for (int i = 0; i < 72; i++) {
			fscanf(file, "%d", &road[i]);
		}
		fclose(file);
		return true;
	}
	return false;
}

void UpdateBullets(bullet* bullets,car *cars, int &score, int &freeze_score) {
	for (int i = 0; i < 20; i++) {
		if (bullets[i].y < 44)
			bullets[i].alive = false;
		if (bullets[i].alive) {
			for (int j = 0; j < 20; j++) {
				if (cars[j].alive) {
					if (abs(cars[j].x- bullets[i].x)<15 && abs(cars[j].y - bullets[i].y)<22) {
						bullets[i].alive = false;
						cars[j].alive = false;
						if (cars[j].enemy && freeze_score <=0)
							score += 100;
						else
							freeze_score = 250;
					}
				}
			}
			bullets[i].y-=10;
		}
	}
}

int GetDeadCar(car* cars) {
	int n;
	for (int i = 0; i < 10; i++) {
		n = rand() % 20;
		if (!cars[i].alive)
			return i;
	}
	for (int i = 0; i < 20; i++) {
		if (!cars[i].alive)
			return i;
	}
	return -1;
}

void UpdateCars(car* cars, int* road,int distance, int &player_x, int speed, int &score, int &freeze_score) {
	int dead_car_num = GetDeadCar(cars);
	if (rand() % 100 == 4 && dead_car_num != -1) {
		cars[dead_car_num].alive = true;
		cars[dead_car_num].y = 55;
		cars[dead_car_num].final_y = rand() % 230 + 60;
		int width = (road[(cars[dead_car_num].final_y / 20 + distance) % 72] - 2) * 42;
		cars[dead_car_num].x = rand() % width + (SCREEN_WIDTH - width) / 2;
	}
	for (int i = 0; i < 20; i++) {
		if (cars[i].alive) {
			if (speed > 8)
				cars[i].y+=3;
			else if (speed != 8)
				cars[i].y-=5;
			if (cars[i].y < -20 || cars[i].y > SCREEN_HEIGHT + 20 || !IsFree(road, cars[i].x, distance, (380 - cars[i].y) / 20))
				cars[i].alive = false;

			if (!IsFree(road, cars[i].x - 20, distance, (380 - cars[i].y) / 20 + 2) || !IsFree(road, cars[i].x + 20, distance, (380 - cars[i].y) / 20 + 2))
				cars[i].x += 20;
			else if (!IsFree(road, cars[i].x - 20, distance, (380 - cars[i].y) / 20 - 2) || !IsFree(road, cars[i].x + 20, distance, (380 - cars[i].y) / 20 - 2))
				cars[i].x -= 20;
			
			
			switch (cars[i].type) {
			case 1:
				if (cars[i].final_y > 100) {
					cars[i].final_y -= 7;
					cars[i].y += 7;
				}
				else if (cars[i].final_y > 70) {
					cars[i].final_y -= 5;
					cars[i].y += 5;
				}
				else if (cars[i].final_y > 30) {
					cars[i].final_y -= 3;
					cars[i].y += 3;
				}
				break;
			case 2:
				if (cars[i].x < player_x - 26) {
					cars[i].x++;
				}
				else if (cars[i].x > player_x + 26) {
					cars[i].x--;
				}
				if (cars[i].y > 300) {
					if (player_x - cars[i].x > 0 && player_x - cars[i].x < 26) {
						cars[i].x--;
					}
					else if (player_x - cars[i].x < 0 && player_x - cars[i].x > -26) {
						cars[i].x++;
					}
				}
				if (cars[i].y < 380) {
					cars[i].y += 2;
				}
				else {
					cars[i].y--;
				}
				if (abs(cars[i].y - 380) < 26) {
					if (player_x - cars[i].x == 26)
						player_x++;
					else if (player_x - cars[i].x == -26)
						player_x--;
				}
				break;
			}

			if (abs(cars[i].y - 380) <= 28) {
				if (player_x - cars[i].x > 0 && player_x - cars[i].x < 26)
					cars[i].x = player_x - 32;
				else if (player_x - cars[i].x < 0 && player_x - cars[i].x > -26)
					cars[i].x = player_x + 32;
				if (!IsFree(road, cars[i].x, distance) && abs(player_x - cars[i].x) < 40) {
					cars[i].alive = false;
					if (cars[i].enemy && freeze_score <= 0)
						score += 100;
					else
						freeze_score = 250;
				}
			}					
		}
	}
}

void NewGame(double &time, int &x,double &distance, int &score, bullet* bullets, car* cars) {
	for (int i = 0; i < 20; i++) {
		cars[i].alive = false;
		bullets[i].alive = false;
	}
	time = 0;
	x = SCREEN_WIDTH / 2;
	distance = 0;
	score = 0;
}

void InitCars(car* cars) {
	int dice;
	for (int i = 0; i < 20; i++) {
		dice = rand() % 5;
		if (dice <2) {
			cars[i].model = car1;
			cars[i].type = 1;
		}
		else if (dice <4){
			cars[i].model = car2;
			cars[i].type = 2;
		}
		else {
			cars[i].model = car3;
			cars[i].enemy = false;
		}
	}
}

bool Crash(car* cars, int x, int* road, double distance) {
	if (!IsFree(road, x, distance, 1))
		return true;
	for (int i = 0; i < 20; i++) {
		if (cars[i].alive && abs(cars[i].y - 380) < 32 && abs(cars[i].x - x) < 26) {
			cars[i].alive = false;
			return true;
		}
	}
	return false;
}

void SaveResults(int size, result* results) {
	FILE* file;
	file = fopen("scores.txt", "w");
	if (file) {
		fprintf(file, "%d\n", size);
		for (int i = 0; i < size; i++) {
			fprintf(file, "%d %lf\n", results[i].score, results[i].time);
		}
		fclose(file);
	}
}

void AddResult(double time, int score) {
	int size;
	result* results = ReadResults(size);
	result res;
	res.score = score;
	res.time = time;
	results[size++] = res;
	SaveResults(size,results);
	free(results);
}

void GameOver(double time, int score) {
	bool nandle = true;
	char msg[50] = "Game is Over";

	DrawRectangle(screen, 160, 120, 320, 60, blue, red);
	DrawString(screen, screen->w / 2 - strlen(msg) * 8 / 2, 130, msg, charset);
	sprintf(msg, "You score: %d", score);
	DrawString(screen, screen->w / 2 - strlen(msg) * 8 / 2, 140, msg, charset);
	sprintf(msg, "Press s to save your result");
	DrawString(screen, screen->w / 2 - strlen(msg) * 8 / 2, 150, msg, charset);
	sprintf(msg, "Press enter to start new game");
	DrawString(screen, screen->w / 2 - strlen(msg) * 8 / 2, 160, msg, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);

	while (nandle) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RETURN:
					nandle = false;
					break;
				case SDLK_s:
					AddResult(time, score);
					ShowAlert("Your result sucessfuly saved");
					nandle = false;
					break;
				case SDLK_ESCAPE:
					Quit();
					break;
				}
			}
			else if (event.type == SDL_QUIT)
				Quit();
		}
		SDL_Delay(20);
	}
}

#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char **argv) {
	int t1, t2, quit = 0, frames = 0, x = SCREEN_WIDTH / 2, score = 0, speed = 8, bullet_num = 0, road[72], lives = 3, freeze_score = 0;
	double distance = 0, delta, worldTime = 0, fpsTimer = 0, fps = 0;
	char alert[25];
	LoadRoad(road);
	bullet bullets[20], bull;
	car cars[20];
	if (Init())
		Quit();
	InitCars(cars);
	
	t1 = SDL_GetTicks();

	while (!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;
		distance += (speed * delta);
		fpsTimer += delta;

		if (fpsTimer > 0.02) {
			UpdateBullets(bullets, cars, score, freeze_score);
			UpdateCars(cars, road, distance, x, speed, score, freeze_score);
			if (freeze_score-- <=0)
				score += 2;
			if (Crash(cars, x, road, distance)) {
				if (worldTime > 60) {
					if (lives--)
						x = SCREEN_WIDTH / 2;
					else {
						GameOver(worldTime, score);
						NewGame(worldTime, x, distance, score, bullets, cars);
						t1 = SDL_GetTicks();
					}
				}
				else {
					x = SCREEN_WIDTH / 2;
				}
			}
			fps = frames * 50;
			frames = 0;
			fpsTimer -= 0.02;
		};
		DrawHood(road, bullets,cars, x, distance, worldTime, fps, score, lives);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = 1;
					break;
				case SDLK_UP:
					speed = 12;
					break;
				case SDLK_DOWN:
					speed = 5;
					break;
				case SDLK_LEFT:
					if (IsFree(road, x - 20, distance))
						x -= 10;
					break;
				case SDLK_RIGHT:
					if (IsFree(road, x + 20, distance))
						x += 10;
					break;
				case SDLK_n:
					NewGame(worldTime, x, distance, score, bullets, cars);
					t1 = SDL_GetTicks();
					break;
				case SDLK_p:
					Pause(t1,1);
					break;
				case SDLK_t:
					Pause(t1, 2);
					break;
				case SDLK_f:
					GameOver(worldTime, score);
					NewGame(worldTime, x, distance, score, bullets, cars);
					t1 = SDL_GetTicks();
					break;
				case SDLK_s:
					SaveGame(distance, worldTime, x, lives, cars, bullets);
					t1 = SDL_GetTicks();
					break;
				case SDLK_l:
					LoadGame(distance, worldTime, x, lives, cars, bullets);
					t1 = SDL_GetTicks();
					break;
				case SDLK_SPACE:
					bull.x = x;
					bull.alive = true;
					bullets[bullet_num++ % 20] = bull;
					break;
				}
				break;
			case SDL_KEYUP:
				speed = 8;
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};
		frames++;
	};
	Quit();
	return 0;
	};
