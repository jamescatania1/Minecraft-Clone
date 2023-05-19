#ifndef _main_h
#define _main_h

static int RENDER_DISTANCE = 18;
static int SHADOW_CASCADES = 3;
static float SHADOW_CASCADE_DISTANCES[] = { 20.0f, 50.0f, 150.0f };
static int SHADOW_CASCADE_PCF_PIXELRADIUS[] = { 1, 1, 0 };
static float SHADOW_CASCADE_PCF_SPREADRADIUS[] = { 0.7f, 0.4f, 0.0f };
static int SHADOW_CASCADE_POISSON_SAMPLES[] = { 4, 0, 0 };

GLFWwindow* window;

double deltaTime;
double fixedDeltaTime;
int windowX;
int windowY;

//gets the current time, in miliseconds since program start
extern long getCurrentTimeMicroseconds();

//appropriately sets viewport size
extern void setViewportSize(int width, int height);

/* example time profile code:

long startTime = getCurrentTimeMicroseconds();
printf("%lf ms\n", ((double)(getCurrentTimeMicroseconds() - startTime)) / 1000.0);

*/

#endif