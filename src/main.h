#ifndef _main_h
#define _main_h

static int RENDER_DISTANCE = 18;

GLFWwindow* window;

double deltaTime;
double fixedDeltaTime;
int windowX;
int windowY;

//gets the current time, in miliseconds since program start
extern long getCurrentTimeMicroseconds();

/* example time profile code:

long startTime = getCurrentTimeMicroseconds();
printf("%lf ms\n", ((double)(getCurrentTimeMicroseconds() - startTime)) / 1000.0);

*/

#endif