#pragma warning( disable: 6011 )

#include <stdlib.h>
#include <glfw3.h>
#include "input.h"
#include "main.h"
#include <stdio.h>
KeyState prevKeyState[349];
KeyState keyState[349];
MouseState prevMouseState;
MouseState mouseState;
int prevCursorLocked;

void scrollCallback(double xoffset, double yoffset);

void InputManager_init() {
	mouseSensitivity = 2.0f;
	prevCursorLocked = 1;

	for (int i = 0; i < 349; i++) {
		if (glfwGetKeyScancode(i) == -1) continue;
		keyState[i] = (KeyState)malloc(sizeof(struct KeyState));
		prevKeyState[i] = (KeyState)malloc(sizeof(struct KeyState));
	}

	prevMouseState = (MouseState)malloc(sizeof(struct MouseState));
	prevMouseState->left = (KeyState)malloc(sizeof(struct KeyState));
	prevMouseState->right = (KeyState)malloc(sizeof(struct KeyState));
	prevMouseState->wheel = (KeyState)malloc(sizeof(struct KeyState));

	mouseState = (MouseState)malloc(sizeof(struct MouseState));
	mouseState->left = (KeyState)malloc(sizeof(struct KeyState));
	mouseState->right = (KeyState)malloc(sizeof(struct KeyState));
	mouseState->wheel = (KeyState)malloc(sizeof(struct KeyState));

	mouseState->dx = 0.0f;
	mouseState->dy = 0.0f;
	mouseState->scrollVal = 0.0f;
	glfwSetScrollCallback(window, scrollCallback);
}

KeyState getKey(int keyCode) {
	if (!keyState[keyCode]) return NULL;
	return keyState[keyCode];
}

void updateInputManager() {
	for (int i = 0; i < 349; i++) {
		if (!keyState[i]) continue;
		keyState[i]->down = glfwGetKey(window, i) == GLFW_PRESS;
		keyState[i]->pressed = (keyState[i]->down && !prevKeyState[i]->down);
		keyState[i]->released = (!keyState[i]->down && prevKeyState[i]->down);
		prevKeyState[i]->down = keyState[i]->down;
	}

	//update mouse input
	mouseState->left->down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	mouseState->right->down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	mouseState->wheel->down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;

	mouseState->left->pressed = (mouseState->left->down && !prevMouseState->left->down);
	mouseState->right->pressed = (mouseState->right->down && !prevMouseState->right->down);
	mouseState->wheel->pressed = (mouseState->wheel->down && !prevMouseState->wheel->down);

	mouseState->left->released = (!mouseState->left->down && prevMouseState->left->down);
	mouseState->right->released = (!mouseState->right->down && prevMouseState->right->down);
	mouseState->wheel->released = (!mouseState->wheel->down && prevMouseState->wheel->down);

	prevMouseState->left->down = mouseState->left->down;
	prevMouseState->right->down = mouseState->right->down;
	prevMouseState->wheel->down = mouseState->wheel->down;

	//update mouse position
	glfwGetCursorPos(window, &mouseState->posX, &mouseState->posY);
	int pixX = (int)mouseState->posX; int pixY = (int)mouseState->posY;
	mouseState->posX /= 0.5 * (double)windowX;
	mouseState->posY /= -0.5 * (double)windowY;
	mouseState->posX -= 1.0;
	mouseState->posY += 1.0;
	if (mouseState->posX < -1.0) mouseState->posX = -1.0;
	if (mouseState->posX > 1.0) mouseState->posX = 1.0;
	if (mouseState->posY < -1.0) mouseState->posY = -1.0;
	if (mouseState->posY > 1.0) mouseState->posY = 1.0;
	if (windowX > windowY) mouseState->posY *= (double)windowY / (double)windowX;
	if (windowY > windowX) mouseState->posX *= (double)windowX / (double)windowY;
	if (cursorLocked) {
		mouseState->dx = (double)(pixX - windowX / 2);
		mouseState->dy = (double)(pixY - windowY / 2);
		mouseState->dx /= 0.5 * (double)windowX;
		mouseState->dy /= 0.5 * (double)windowY;
		if (windowX > windowY) mouseState->dx /= (double)windowY / (double)windowX;
		if (windowY > windowX) mouseState->dy /= (double)windowX / (double)windowY;
		glfwSetCursorPos(window, windowX / 2, windowY / 2);
	}
	else {
		prevMouseState->dx = 0.0f;
		mouseState->dx = 0.0f;
		mouseState->dy = 0.0f;
	}
}

void updateInputManagerFixedTime() {
	mouseState->scrollVal = mouseState->scrollVal * .3f;
}

void scrollCallback(double xoffset, double yoffset) {
	mouseState->scrollVal += (float)yoffset;
	if (mouseState->scrollVal < -2.0f) mouseState->scrollVal = -2.0f;
	if (mouseState->scrollVal > 2.0f) mouseState->scrollVal = 2.0f;
}

void InputManager_free() {
	for (int i = 0; i < 349; i++) {
		if (!keyState[i]) continue;
		free(keyState[i]);
		free(prevKeyState[i]);
	}

	free(prevMouseState->left);
	free(prevMouseState->right);
	free(prevMouseState->wheel);
	free(prevMouseState);

	free(mouseState->left);
	free(mouseState->right);
	free(mouseState->wheel);
	free(mouseState);
}