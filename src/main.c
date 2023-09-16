#include <time.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../Headers/window.h"
#include "./bird.c"
#include "../Headers/backgroundchunk.h"
#include "../Headers/pipe.h"

#define GRAVITY 0.6
#define WINNING_SCORE 10

int addscore(int *score) {
  (*score)++;
  printf("%d\n", *score);
  return *score;
}

bool game(SDL_Renderer *renderer, TTF_Font *Arial, SDL_Color fontBlack,
          SDL_Texture *chunkTexture, const char *filename);

int main(int argc, char *argv[]) {
  const char *filename = argv[1];
  SDL_Init(SDL_INIT_AUDIO);
  SDL_Window *window =
      SDL_CreateWindow(WINDOW_TITLE, WINDOW_X, WINDOW_Y, WINDOW_WIDTH,
                       WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  SDL_SetWindowTitle(window, "Flappy Bird");
  SDL_SetRenderDrawColor(renderer, 100, 208, 232, 255);

  // SDL_ttf font setup
  TTF_Init();
  TTF_Font *Arial = TTF_OpenFont("./Fonts/Ceriph0765.ttf", 16);
  SDL_Color fontBlack = {0, 0, 0};

  init(renderer);
  SDL_Texture *chunkTexture =
      IMG_LoadTexture(renderer, "./assets/background.png");
  SDL_SetTextureBlendMode(chunkTexture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(chunkTexture, 180);

  while (true) {
    bird_init_impl();
    bool result = game(renderer, Arial, fontBlack, chunkTexture, filename);
    if (!result)
      break;
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

bool game(SDL_Renderer *renderer, TTF_Font *Arial, SDL_Color fontBlack,
          SDL_Texture *chunkTexture, const char *filename) {
  time_t t;
  srand((unsigned)time(&t));

  int iter = 0;
  int score = 0;
  char scoreText[10];
  bool alreadyCheckedScores[1000] = {};

  BackgroundChunk bgChunks[500] = {};
  Pipe pipes[1000] = {};
  size_t pipesSize = 0;

  for (int i = 0; i < 500; ++i) {
    BackgroundChunk *chunk = &bgChunks[i];
    chunk->x = i * WINDOW_WIDTH;
    chunk->y = WINDOW_HEIGHT - 300;
    chunk->speedX = 1;
    chunk->width = WINDOW_WIDTH;
    chunk->height = 300;
    chunk->textureSrc = "./assets/background.png";
    chunk->texture = chunkTexture;
    chunk->rect.x = chunk->x;
    chunk->rect.y = chunk->y;
    chunk->rect.w = chunk->width;
    chunk->rect.h = chunk->height;
    chunk->alpha = 180;
  }

  // SDL_Rect demoRect = {WINDOW_WIDTH / 2 - 225, WINDOW_HEIGHT / 2 - 135, 450,
  //                      270};
  // SDL_Texture *demoTexture = IMG_LoadTexture(renderer, "./assets/demo.png");

  bool running = true;
  bool startFalling = false;
  bool dead = false;
  int score_start_iter = 1;
  SDL_Event e;
  while (running) {

    iter++;

    sprintf(scoreText, "%d", score);

    SDL_Surface *scoreSurface =
        TTF_RenderText_Solid(Arial, scoreText, fontBlack);
    SDL_Texture *scoreTexture =
        SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {WINDOW_WIDTH / 2 - 25, 10, 25, 50};

    if (iter % 50 == 0) {
      pipesSize++;
      Pipe *pipe = &pipes[pipesSize - 1];

      int heightTop =
          rand() % (int)((WINDOW_HEIGHT / 2) * 1.2 - WINDOW_HEIGHT / 10 + 1) +
          WINDOW_HEIGHT / 10;

      // asset handling
      pipe->surfaceTop = IMG_Load("./assets/pipe-down.png");
      pipe->surfaceBottom = IMG_Load("./assets/pipe-up.png");
      pipe->textureTop =
          SDL_CreateTextureFromSurface(renderer, pipe->surfaceTop);
      pipe->textureBottom =
          SDL_CreateTextureFromSurface(renderer, pipe->surfaceBottom);
      SDL_FreeSurface(pipe->surfaceTop);
      SDL_FreeSurface(pipe->surfaceBottom);

      pipe->gap = 200;
      pipe->width = 60;
      pipe->speedX = !startFalling && !dead ? 0.25 : 2;
      // pipe->speedX = 2;

      pipe->top.x = pipesSize * 100 + 500;
      pipe->top.y = 0;
      pipe->top.w = pipe->width;
      pipe->top.h = heightTop;

      pipe->bottom.x = pipesSize * 100 + 500;
      pipe->bottom.y = heightTop + pipe->gap;
      pipe->bottom.w = pipe->width;
      pipe->bottom.h = WINDOW_HEIGHT - (pipe->bottom.y);
    }

    if (player.y + 30 > WINDOW_HEIGHT) {
      dead = score >= WINNING_SCORE ? false : true;
    } else if (player.y < 0) {
      dead = score >= WINNING_SCORE ? false : true;
    } else if (startFalling) {
      player.velocityY += GRAVITY * 0.5;
    }

    // collision detection
    for (int i = 0; i < pipesSize; ++i) {
      if (collides(pipes[i], player)) {
        for (int j = 0; j < pipesSize; ++j) {
          pipes[j].speedX = 0;
        }
        player.velocityY = 0;
        player.y = WINDOW_HEIGHT - player.height;
        for (int j = 0; j < 500; ++j) {
          bgChunks[j].speedX = 0;
        }
        dead = score >= WINNING_SCORE ? false : true;
      }

      if (!alreadyCheckedScores[i] == true && player.x > pipes[i].top.x) {
        alreadyCheckedScores[i] = true;
        addscore(&score);
      }
    }

    for (int i = 0; i < 500; ++i) {
      bgChunks[i].rect.x = bgChunks[i].x;
      bgChunks[i].x -= bgChunks[i].speedX;
    }

    for (int i = 0; i < pipesSize; ++i) {
      pipes[i].top.x -= pipes[i].speedX;
      pipes[i].bottom.x -= pipes[i].speedX;
    }

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_SPACE:
          player.velocityY = -6;
          startFalling = true;
          for (int i = 0; i < pipesSize; i++) {
            pipes[i].speedX = 2;
          }
          break;
        default:
          break;
        }
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        startFalling = true;
        player.velocityY = -6;
      } else if (e.type == SDL_QUIT) {
        running = false;
        break;
      }
    }

    player.y += (int)player.velocityY;

    SDL_SetRenderDrawColor(renderer, 100, 208, 232, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < 500; ++i) {
      SDL_RenderFillRect(renderer, &bgChunks[i].rect);
      SDL_RenderCopy(renderer, bgChunks[i].texture, NULL, &bgChunks[i].rect);
    }
    if (!dead) {
      renderBird(renderer);
    }
    SDL_RenderCopy(renderer, player.texture, NULL, &player.Rect);
    for (int i = 0; i < pipesSize; ++i) {
      SDL_Rect pipeTop = pipes[i].top;
      SDL_Rect pipeBottom = pipes[i].bottom;

      SDL_RenderCopy(renderer, pipes[i].textureTop, NULL, &pipeTop);
      SDL_RenderCopy(renderer, pipes[i].textureBottom, NULL, &pipeBottom);
    }
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

    if (!startFalling && !dead) {
      // SDL_RenderCopy(renderer, demoTexture, NULL, &demoRect);
      char buf[128];
      sprintf((char *)&buf, "Get a score of 10 to finish bundling: %s",
              filename);
      SDL_Surface *titleSurface = TTF_RenderText_Solid(Arial, buf, fontBlack);
      SDL_Texture *titleTexture =
          SDL_CreateTextureFromSurface(renderer, titleSurface);
      int width = titleSurface->w;
      int height = titleSurface->h;
      SDL_Rect titleRect = {WINDOW_WIDTH / 2 - width / 2, 150, width, height};
      SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    }

    if (score >= WINNING_SCORE) {
      if (score_start_iter % 120 == 0)
        return false;
      score_start_iter++;
      SDL_Surface *titleSurface =
          TTF_RenderText_Solid(Arial, "You did an amazing job!", fontBlack);
      SDL_Texture *titleTexture =
          SDL_CreateTextureFromSurface(renderer, titleSurface);
      int width = titleSurface->w;
      int height = titleSurface->h;
      SDL_Rect titleRect = {WINDOW_WIDTH / 2 - width / 2, 150, width, height};
      SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    }

    if (dead) {
      SDL_Rect gameOverRect = {WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 80,
                               300, 160};
      SDL_Texture *gameOverTexture =
          IMG_LoadTexture(renderer, "./assets/game-over.png");
      SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
      return true;
    }

    SDL_SetRenderDrawColor(renderer, 18, 161, 27, 255);
    SDL_Rect ground = {0, WINDOW_HEIGHT - 25, WINDOW_WIDTH, 25};
    SDL_RenderFillRect(renderer, &ground);

    SDL_RenderPresent(renderer);
  }

  return false;
}