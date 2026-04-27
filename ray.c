#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>


#define WIDTH 1200
#define HEIGHT 900
#define WHITE 0xffffff
#define BLACK 0x000000
#define RAY_COLOR 0xdbb407


// ------------------------ STRUCTS -------------------------------


typedef struct Circle
{
  double x;
  double y;
  double radius;
} Circle;


typedef struct Ray 
{
  double ox, oy;
  double dx, dy;
} Ray;


// ------------------------ METHODS -------------------------------

Circle* initializeCircle(double x, double y, double radius)
{

  Circle* circle1 = malloc(sizeof(Circle));
  
  circle1->x=x;
  circle1->y=y;
  circle1->radius=radius;

  return circle1;
}

void drawCircle(SDL_Surface* surface, Circle* circle, int color)
{
    for (int y = -circle->radius; y <= circle->radius; y++)
    {
        int x = (int)sqrt(circle->radius * circle->radius - y * y);

        SDL_Rect ligne = {circle->x - x, circle->y + y, 2*x, 1};
        SDL_FillRect(surface, &ligne, color);
    }
}

double intersectCircle(Ray* ray, Circle* circle)
{
    double fx = ray->ox - circle->x;
    double fy = ray->oy - circle->y;

    double a = ray->dx * ray->dx + ray->dy * ray->dy;
    double b = 2 * (fx * ray->dx + fy * ray->dy);
    double c = fx*fx + fy*fy - circle->radius * circle->radius;

    double discriminant = b*b - 4*a*c;

    if (discriminant < 0)
        return -1;

    return (-b - sqrt(discriminant)) / (2*a);
}

// ------------------------ MAIN -------------------------------

int main(void)
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  SDL_Surface* surface = SDL_GetWindowSurface(window);

  Circle* circlePlanet = initializeCircle(800, 430, 100);
  Circle* circleSol = initializeCircle(300, 550, 50);
  SDL_Rect eraser = {0, 0, WIDTH, HEIGHT};

  int simulation_running = 1;
  SDL_Event event;
  while (simulation_running)
  {
    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT)
      {
        simulation_running = 0;
      }
      if(event.type == SDL_MOUSEMOTION && event.motion.state != 0)
      {
        circleSol->x = event.motion.x;
        circleSol->y = event.motion.y;
      }
    }
    SDL_FillRect(surface, &eraser, BLACK);


// ------------- RAY DRAWING ---------------


    for (int py = 0; py < HEIGHT; py++)
    {
        for (int px = 0; px < WIDTH; px++)
        {
          Ray ray = {px, py, circleSol->x - px, circleSol->y - py};
          double len = sqrt(ray.dx*ray.dx + ray.dy*ray.dy);
          ray.dx /= len;
          ray.dy /= len;

          double t = intersectCircle(&ray, circlePlanet);

          Uint32 color;
          if (t > 0 && t < len)
              color = BLACK;
          else
              color = RAY_COLOR;

          SDL_Rect pixel = {px, py, 1, 1};
          SDL_FillRect(surface, &pixel, color);
        }
    }
  

    drawCircle(surface, circleSol, WHITE);
    drawCircle(surface, circlePlanet, WHITE);


    SDL_UpdateWindowSurface(window);
    SDL_Delay(10);

  }

  free(circlePlanet);
  free(circleSol);

}
