#ifndef _atlas_h
#define _atlas_h

#include "chunk.h"

#define ATLAS_BLOCKFACE int
#define BLOCKFACE_TOP 0
#define BLOCKFACE_BOTTOM 1
#define BLOCKFACE_FRONT 2
#define BLOCKFACE_BACK 3
#define BLOCKFACE_LEFT 4
#define BLOCKFACE_RIGHT 5

typedef struct BlockFaceInfo {
	int x, y;
	int randRotation;
	int randFlipX;
	int randFlipY;
} *BlockFaceInfo;

typedef struct BlockInfo {
	BlockFaceInfo faces[6];
} *BlockInfo;

extern void BlockInfo_free(BlockInfo block);

extern void TextureAtlas_init();

extern void TextureAtlas_free();

//retrieves coordinates of given block/face in format of bytes 0-5: texture index, bytes 5-7: corner index
extern GLubyte* TextureAtlas_getFaceCoords(BLOCK_TYPE block, ATLAS_BLOCKFACE face);

extern unsigned int TextureAtlas_currentTexture();

#endif