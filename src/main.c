#pragma warning(disable: 6286)
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <glfw3.h>
#include <stdio.h>
#include <stb_image.h>
#include "main.h"
#include "input.h"
#include "world.h"

#include <crtdbg.h>
#include "windows.h"
#define DEBUG_MEMORY 1

#define FPS_CAPPED 0
#define FPS_LIMIT 120
#define FIXED_UPDATES_PER_SECOND 120
#define SCREEN_DEFAULT_WIDTH 800
#define SCREEN_DEFAULT_HEIGHT 800

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void update();
void fixedUpdate();
void draw();

World activeWorld;
long progStartTimeMicroseconds;
#include "structures/hashmap.h"
int main() {
	progStartTimeMicroseconds = (long)(glfwGetTime() * 1000000.0);

	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	if(DEBUG_MEMORY) _CrtMemCheckpoint(&sOld);

	//initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	//create GLFW window
	window = glfwCreateWindow(SCREEN_DEFAULT_WIDTH, SCREEN_DEFAULT_HEIGHT, "\"Minecraft\"", NULL, NULL);
	if (window == NULL) {
		printf("failed to create window\n");
		glfwTerminate();
		return -1;
	}
	// Center window on screen
	GLFWmonitor* primary = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(primary);
	glfwSetWindowPos(window, (mode->width - SCREEN_DEFAULT_WIDTH) / 2, (mode->height - SCREEN_DEFAULT_HEIGHT) / 2);
	glfwMakeContextCurrent(window);

	//initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("failed to initialize GLAD\n");
		glfwTerminate();
		return -1;
	}

	//set resize callback function
	framebufferSizeCallback(window, SCREEN_DEFAULT_WIDTH, SCREEN_DEFAULT_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	//enable depth testing, backface culling and UBOs
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_UNIFORM_BUFFER);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);

	char shadow_cascadesString[8];
	sprintf(shadow_cascadesString, "%d", SHADOW_CASCADES);
	Shader_addGlobalDefine("SHADOW_CASCADES", shadow_cascadesString);
	char shadow_widthString[8], shadow_heightString[8];
	sprintf(shadow_widthString, "%d", SHADOW_WIDTH);
	sprintf(shadow_heightString, "%d", SHADOW_HEIGHT);
	Shader_addGlobalDefine("SHADOW_WIDTH", shadow_widthString);
	Shader_addGlobalDefine("SHADOW_HEIGHT", shadow_heightString);

	//initialize input manager
	InputManager_init();

	//initialize world
	activeWorld = new_World();

	//main loop
	double lastDrawTime = 0.0;
	double lastFixedUpdateTime = 0.0;
	double lastUpdateTime = 0.0;

	updateInputManager();
	while (!glfwWindowShouldClose(window)) {

		//input
		updateInputManager();

		//update loop
		update();
		deltaTime = glfwGetTime() - lastUpdateTime;
		lastUpdateTime = glfwGetTime();
		
		//fixed update
		fixedDeltaTime = glfwGetTime() - lastFixedUpdateTime;
		if (glfwGetTime() - lastFixedUpdateTime > (1.0 / (double)FIXED_UPDATES_PER_SECOND)) {
			lastFixedUpdateTime = glfwGetTime();

			updateInputManagerFixedTime();
			fixedUpdate();
		}

		//rendering
		if (!FPS_CAPPED || glfwGetTime() - lastDrawTime > (1.0 / (double)FPS_LIMIT)) {
			lastDrawTime = glfwGetTime();
			//clear background
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//draw everything
			draw();

			//check events and swap buffers
			glfwPollEvents();
			glfwSwapBuffers(window);
		}
	}

	//free active world
	World_free(activeWorld);

	//free input manager
	InputManager_free();

	glfwTerminate();
	
	if (DEBUG_MEMORY) {
		_CrtMemCheckpoint(&sNew); //take a snapshot 
		if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
		{
			OutputDebugString(L"=== Memory Leak Statistics ===\n");
			_CrtMemDumpStatistics(&sDiff);
			OutputDebugString(L"=== Dumped Objects ===");
			_CrtMemDumpAllObjectsSince(&sOld);
			//OutputDebugString(L"=== Memory Leaks ===\n");
			//_CrtDumpMemoryLeaks();
		}
		else {
			OutputDebugStringA("No leaks--all allocated memory has been freed.\n");
		}
	}

	return 0;
}

void update() {
	World_update(activeWorld);
}

void fixedUpdate() {
	World_update(activeWorld);
}

void draw() {
	World_draw(activeWorld);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	setViewportSize(width, height);
	windowX = width; windowY = height;
	resolutionSensitivityOffset = 1.0;
}

void setViewportSize(int width, int height) {
	if (width >= height) glViewport(0, (height - width) / 2, width, width);
	else glViewport((width - height) / 2, 0, height, height);
}

long getCurrentTimeMicroseconds() {
	long curTime = (long)(glfwGetTime() * 1000000.0);
	return curTime - progStartTimeMicroseconds;
}