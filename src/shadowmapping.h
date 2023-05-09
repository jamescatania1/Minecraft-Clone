#ifndef _shadowmapping_h
#define _shadowmapping_h

typedef struct ShadingInfo {
	unsigned int depthMapFBO;
	unsigned int depthMap;
} *ShadingInfo;

extern void ShadowMapping_update();

extern void ShadowMapping_free();

#endif