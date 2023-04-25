#ifndef _skybox_h
#define _skybox_h

#include "renderComponent.h"

typedef struct Skybox {
	RenderComponent renderer;
} *Skybox;

extern Skybox new_Skybox();

extern void Skybox_free(Skybox skybox);

#endif