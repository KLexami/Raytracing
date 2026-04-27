#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define WIDTH  1200
#define HEIGHT 900
#define WHITE  0xffffff
#define BLACK  0x000000
#define RAY_COLOR 0xdbb407

// ------------------------ STRUCTS -------------------------------

typedef struct Material {
    double red, green, blue;
    double reflection;
} Material;

typedef struct Sphere3D {
    double x, y, z;
    double radius;
    Material mat;
} Sphere3D;

typedef struct Light3D {
    double x, y, z;
    double red, green, blue;
} Light3D;

// ------------------------ METHODS -------------------------------

static int hitSphere3D(double ox, double oy, double oz,
                       double dx, double dy, double dz,
                       const Sphere3D* s, double* t)
{
    double ex = ox - s->x;
    double ey = oy - s->y;
    double ez = oz - s->z;

    double B = dx*ex + dy*ey + dz*ez;
    double C = ex*ex + ey*ey + ez*ez - s->radius*s->radius;
    double D = B*B - C;
    if (D < 0.0) return 0;

    double sqrtD = sqrt(D);
    double t0 = -B - sqrtD;
    double t1 = -B + sqrtD;

    int hit = 0;
    if (t0 > 0.1 && t0 < *t) { *t = t0; hit = 1; }
    if (t1 > 0.1 && t1 < *t) { *t = t1; hit = 1; }
    return hit;
}

static void raytrace3D(int px, int py,
                       Sphere3D* spheres, int nbSpheres,
                       Light3D*  lights,  int nbLights,
                       double* outR, double* outG, double* outB)
{
    double red = 0.0, green = 0.0, blue = 0.0;
    double coef  = 1.0;

    double rox = (double)px, roy = (double)py, roz = -10000.0;
    double rdx = 0.0,        rdy = 0.0,        rdz = 1.0;

    int level = 0;
    do {
        double t = 20000.0;
        int currentSphere = -1;

        for (int i = 0; i < nbSpheres; i++) {
            if (hitSphere3D(rox, roy, roz, rdx, rdy, rdz, &spheres[i], &t))
                currentSphere = i;
        }
        if (currentSphere == -1) break;

        double nx_pos = rox + t * rdx;
        double ny_pos = roy + t * rdy;
        double nz_pos = roz + t * rdz;

        double nx = nx_pos - spheres[currentSphere].x;
        double ny = ny_pos - spheres[currentSphere].y;
        double nz = nz_pos - spheres[currentSphere].z;
        double len = sqrt(nx*nx + ny*ny + nz*nz);
        if (len == 0.0) break;
        nx /= len; ny /= len; nz /= len;

        Material* mat = &spheres[currentSphere].mat;

        for (int j = 0; j < nbLights; j++) {
            double lx = lights[j].x - nx_pos;
            double ly = lights[j].y - ny_pos;
            double lz = lights[j].z - nz_pos;
            double dot = nx*lx + ny*ly + nz*lz;
            if (dot <= 0.0) continue;

            double lt = sqrt(lx*lx + ly*ly + lz*lz);
            if (lt <= 0.0) continue;
            lx /= lt; ly /= lt; lz /= lt;

            int inShadow = 0;
            double st = lt;
            for (int i = 0; i < nbSpheres; i++) {
                if (hitSphere3D(nx_pos, ny_pos, nz_pos, lx, ly, lz,
                                &spheres[i], &st)) {
                    inShadow = 1;
                    break;
                }
            }
            if (inShadow) continue;

            double lambert = (lx*nx + ly*ny + lz*nz) * coef;
            red   += lambert * lights[j].red   * mat->red;
            green += lambert * lights[j].green * mat->green;
            blue  += lambert * lights[j].blue  * mat->blue;
        }

        coef *= mat->reflection;
        double reflet = 2.0 * (rdx*nx + rdy*ny + rdz*nz);
        rox = nx_pos; roy = ny_pos; roz = nz_pos;
        rdx = rdx - reflet * nx;
        rdy = rdy - reflet * ny;
        rdz = rdz - reflet * nz;

        level++;
    } while (coef > 0.0 && level < 10);

    *outR = red   < 1.0 ? red   : 1.0;
    *outG = green < 1.0 ? green : 1.0;
    *outB = blue  < 1.0 ? blue  : 1.0;
}

// ------------------------ MAIN -------------------------------

int main(int argc, char* argv[])
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window*  window  = SDL_CreateWindow("Raytracing 3D",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WIDTH, HEIGHT, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    Sphere3D spheres[] = {
        { WIDTH/2.0,       HEIGHT/2.0,       600.0, 200.0,
          {0.9, 0.3, 0.1, 0.6} },
        { WIDTH/2.0 - 280, HEIGHT/2.0 + 50,  400.0, 100.0,
          {0.2, 0.4, 1.0, 0.3} },
        { WIDTH/2.0 + 320, HEIGHT/2.0 - 30,  500.0, 130.0,
          {0.1, 0.8, 0.2, 0.4} },
        { WIDTH/2.0,       HEIGHT/2.0 + 5000, 1000.0, 4800.0,
          {0.6, 0.6, 0.6, 0.5} },
    };
    int nbSpheres = (int)(sizeof(spheres)/sizeof(spheres[0]));

    Light3D lights[] = {
        { WIDTH/2.0 - 400, HEIGHT/2.0 - 600, -100.0, 1.0, 1.0, 1.0 },
        { WIDTH/2.0 + 600, HEIGHT/2.0 - 200,  200.0, 0.5, 0.5, 0.7 },
    };
    int nbLights = (int)(sizeof(lights)/sizeof(lights[0]));

    Uint32* rtBuffer = malloc(WIDTH * HEIGHT * sizeof(Uint32));
    int rtDirty = 1;

    int grabbedSphere = -1;
    int simulation_running = 1;
    SDL_Event event;

    while (simulation_running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                simulation_running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                double mx = event.button.x;
                double my = event.button.y;
                double t = 20000.0;
                grabbedSphere = -1;

                for (int i = 0; i < nbSpheres; i++) {
                    if (hitSphere3D(mx, my, -10000.0, 0.0, 0.0, 1.0, &spheres[i], &t)) {
                        grabbedSphere = i;
                    }
                }
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                grabbedSphere = -1;
            }

            if (event.type == SDL_MOUSEMOTION && grabbedSphere != -1 && grabbedSphere != 3) {
                spheres[grabbedSphere].x = event.motion.x;
                spheres[grabbedSphere].y = event.motion.y;
                rtDirty = 1;
            }
        }

// ------------- RAYTRACING CALCULATION ---------------

        if (rtDirty)
        {
            //printf("Debug : Calcul raytracing 3D... ");
            fflush(stdout);

            for (int py = 0; py < HEIGHT; py++)
            {
                for (int px = 0; px < WIDTH; px++)
                {
                    double r, g, b;
                    raytrace3D(px, py,
                               spheres, nbSpheres,
                               lights,  nbLights,
                               &r, &g, &b);

                    Uint8 R = (Uint8)(r * 255.0);
                    Uint8 G = (Uint8)(g * 255.0);
                    Uint8 B = (Uint8)(b * 255.0);
                    rtBuffer[py * WIDTH + px] =
                        SDL_MapRGB(surface->format, R, G, B);
                }
            }
            rtDirty = 0;
            //printf("OK\n");
        }

// ------------- DISPLAY ---------------

        SDL_LockSurface(surface);
        Uint32* pixels = (Uint32*)surface->pixels;
        for (int i = 0; i < WIDTH * HEIGHT; i++)
            pixels[i] = rtBuffer[i];
        SDL_UnlockSurface(surface);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    free(rtBuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
