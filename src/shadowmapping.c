#include <stdlib.h>
#include <glad/glad.h>
#include <glfw3.h>
#include "main.h"
#include "shadowmapping.h"

#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

ShadingInfo shadows;

ShadingInfo new_ShadowMapping();
void ShadowMapping_update() {
	if (!shadows) shadows = new_ShadowMapping();

	// render to depth map
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadows->depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	ConfigureShaderAndMatrices();
	//RenderScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// render scene as normal with shadow mapping (using depth map)
	glViewport(0, 0, windowX, windowY);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, shadows->depthMap);
	//RenderScene();
}

ShadingInfo new_ShadowMapping() {
	ShadingInfo this = (ShadingInfo)malloc(sizeof(struct ShadingInfo));
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return this;
}

void ShadowMapping_free() {
	if (!shadows) return;
	free(shadows);
}