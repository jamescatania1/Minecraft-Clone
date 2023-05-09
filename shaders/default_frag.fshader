#version 330 core

#define BLOCKFACE_TOP 0
#define BLOCKFACE_TOP_BIAS 0.0003
#define BLOCKFACE_BOTTOM 1
#define BLOCKFACE_BOTTOM_BIAS 0.000
#define BLOCKFACE_FRONT 2
#define BLOCKFACE_FRONT_BIAS -0.03
#define BLOCKFACE_BACK 3
#define BLOCKFACE_BACK_BIAS 0.003
#define BLOCKFACE_LEFT 4
#define BLOCKFACE_LEFT_BIAS -0.03
#define BLOCKFACE_RIGHT 5
#define BLOCKFACE_RIGHT_BIAS 0.003

#define DISPLAY_FOG true
#define PCF_PIXELRADIUS 1
#define PCF_PIXELSPREADRADIUS 0.7
#define PCF_PIXELCT 9

out vec4 FragColor;
  
in float color;
in vec2 texCoord;
in float distance;
in vec3 fragPos;
in vec4 fragPosLightSpace;
flat in uint face;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix;
    float near;
    float far;
};

uniform sampler2D atlas;
uniform sampler2D shadowMap;

float fog_start = 0.75;
float fog_end = 0.84;

void main()
{
    //depth fog for far chunks
    float depth = (distance - near) / far;
    if(depth > fog_start && DISPLAY_FOG) {
        FragColor = texture(atlas, texCoord) * vec4(vec3(1.0), 
        -depth / (fog_end - fog_start) + fog_start / (fog_end - fog_start) + 1.0);
    }
    else{
        FragColor = texture(atlas, texCoord);
    }
    

    //shadows
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.0;
    if(face == uint(0)) bias = BLOCKFACE_TOP_BIAS;
    else if(face == uint(1)) bias = BLOCKFACE_BOTTOM_BIAS;
    else if(face == uint(2)) bias = BLOCKFACE_FRONT_BIAS;
    else if(face == uint(3)) bias = BLOCKFACE_BACK_BIAS;
    else if(face == uint(4)) bias = BLOCKFACE_LEFT_BIAS;
    else if(face == uint(5)) bias = BLOCKFACE_RIGHT_BIAS;

    float shadow = 0.0;
    float finLight = color;
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.y < 0.0 || projCoords.x > 1.0 || projCoords.y > 1.0) {
        finLight *= color;
    }
    else{ //in shadowmapped region
        vec2 texelSize = PCF_PIXELSPREADRADIUS / textureSize(shadowMap, 0);
        for(int x = -PCF_PIXELRADIUS; x <= PCF_PIXELRADIUS; x++) //pcf filtering
        {
            for(int y = -PCF_PIXELRADIUS; y <= PCF_PIXELRADIUS; y++)
            {
                  for (int i = 0; i < 4; i++){ //poisson sampling
                        vec2 sample;
                        if(i == 0) sample = vec2( -0.94201624, -0.39906216 );
                        else if(i == 1) sample = vec2( 0.94558609, -0.76890725 );
                        else if(i == 2) sample = vec2( -0.094184101, -0.92938870 ); 
                        else if(i == 3) sample = vec2( 0.34495938, 0.29387760 );
                        
                        float sampleDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize + sample / 1500.0).r; 
                        shadow += currentDepth - bias > sampleDepth ? 1.0 : 0.0;      
                  }
            }    
        }
        shadow /= PCF_PIXELCT * 8.0;
    }

    //final fragment color
    FragColor = vec4(FragColor.xyz * finLight * (1.0 - shadow * 0.5), FragColor.w);
}