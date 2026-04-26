#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>


#define WIDTH 1200
#define HEIGHT 900
#define WHITE 0xffffff

typedef struct Circle
{
  double x;
  double y;
  double radius;
} Circle;

Circle* initializeCircle(double x, double y, double radius)
{

  Circle* circle1 = malloc(sizeof(Circle));
  
  circle1->x=x;
  circle1->y=y;
  circle1->radius=radius;

  return circle1;
}

void drawCircle(SDL_Surface* surface, Circle* circle)
{
    for (int y = -circle->radius; y <= circle->radius; y++)
    {
        int x = (int)sqrt(circle->radius * circle->radius - y * y);

        SDL_Rect ligne = {circle->x - x, circle->y + y, 2*x, 1};
        SDL_FillRect(surface, &ligne, WHITE);
    }
}

int main(void)
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  
  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_Rect Rect = (SDL_Rect) {200, 200, 500, 10};
  SDL_FillRect(surface, &Rect, WHITE);
  
  Circle* circleSol = initializeCircle(600, 450, 100);
  drawCircle(surface, circleSol);

  SDL_UpdateWindowSurface(window);


  SDL_Delay(3000);
}
