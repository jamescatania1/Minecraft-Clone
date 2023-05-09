#version 330 core

#define BLOCKFACE_TOP 0
#define BLOCKFACE_TOP_BIAS 0.00
#define BLOCKFACE_BOTTOM 1
#define BLOCKFACE_BOTTOM_BIAS -0.1
#define BLOCKFACE_FRONT 2
#define BLOCKFACE_FRONT_BIAS -0.1
#define BLOCKFACE_BACK 3
#define BLOCKFACE_BACK_BIAS 0.00
#define BLOCKFACE_LEFT 4
#define BLOCKFACE_LEFT_BIAS -0.05
#define BLOCKFACE_RIGHT 5
#define BLOCKFACE_RIGHT_BIAS 0.00

#define DISPLAY_FOG true
#define PCF_PIXELRADIUS 2
#define PCF_PIXELSPREADRADIUS 0.5
#define PCF_PIXELCT 25.0

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
        FragColor = texture(atlas, texCoord) * vec4(color, color, color, 
        -depth / (fog_end - fog_start) + fog_start / (fog_end - fog_start) + 1.0);
    }
    else{
        FragColor = texture(atlas, texCoord) * vec4(color, color, color, 1.0);
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
    vec2 texelSize = PCF_PIXELSPREADRADIUS / textureSize(shadowMap, 0);
    for(int x = -PCF_PIXELRADIUS; x <= PCF_PIXELRADIUS; x++) //pcf filtering
    {
        for(int y = -PCF_PIXELRADIUS; y <= PCF_PIXELRADIUS; y++)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= PCF_PIXELCT;
    if(projCoords.z > 1.0) shadow = 0.0;


    //final fragment color
    FragColor = vec4(FragColor.xyz * (1.0 - shadow * 0.55), 1.0);
}