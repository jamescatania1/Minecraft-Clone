#version 430 core

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
  
in float f_color;
in vec2 f_texCoord;
flat in int f_texImage;
flat in int f_cascade;
in float f_distance;
in vec3 f_fragPos;
in vec4 f_fragPosLightSpace[3];
flat in uint f_face;
flat in int f_cascadeValid;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[3];
    float near;
    float far;
};

uniform sampler2DArray atlas;
uniform sampler2DArray shadowMap;
uniform float cascadePlaneDistances[3];

float fog_start = 0.85;
float fog_end = 0.96;

float sampleShadowmap(vec3 projCoords[3], float bias, vec2 offset) {
    if(f_cascadeValid == 1) {
        float sampleDepth = texture(shadowMap, vec3(projCoords[f_cascade].xy + offset, f_cascade)).r;
        return projCoords[f_cascade].z - bias > sampleDepth ? 1.0 : 0.0;
    }
    for (int i = 0; i < 3; i++) { //in ambiguous cascade level region, sample all cascades
        float sampleDepth = texture(shadowMap, vec3(projCoords[i].xy + offset, i)).r;
        if (projCoords[i].z - bias > sampleDepth) {
            return 1.0;
        }
    }
    return 0.0;
}

void main()
{
    // shadows
    float shadow = 0.0;
    float finLight = f_color;

    if (f_cascade == -1) {
        //squaring the color offset per chunk when shadows are not present
        //to reduce the visual jump in contrast between the two regions
        finLight *= f_color;
    }
    else {
        //sample shadowmap
        vec3 projCoords[3];
        for(int i = 0; i < 3; i++) {
            projCoords[i] = f_fragPosLightSpace[i].xyz / f_fragPosLightSpace[i].w;
            projCoords[i] = projCoords[i] * 0.5 + 0.5;
        }

        float bias = 0.0;
        if(f_face == uint(0)) bias = BLOCKFACE_TOP_BIAS;
        else if(f_face == uint(1)) bias = BLOCKFACE_BOTTOM_BIAS;
        else if(f_face == uint(2)) bias = BLOCKFACE_FRONT_BIAS;
        else if(f_face == uint(3)) bias = BLOCKFACE_BACK_BIAS;
        else if(f_face == uint(4)) bias = BLOCKFACE_LEFT_BIAS;
        else if(f_face == uint(5)) bias = BLOCKFACE_RIGHT_BIAS;

        if (f_cascade == 0){ //in shadowmapped region that will be filtered (first cascade in this instance)

            //using PCF filtering
            vec2 texelSize = PCF_PIXELSPREADRADIUS / vec2(2048.0, 2048.0);
            for(int x = -PCF_PIXELRADIUS; x <= PCF_PIXELRADIUS; x++) //pcf filtering
            {
                for(int y = -PCF_PIXELRADIUS; y <= PCF_PIXELRADIUS; y++)
                {
                      for (int i = 0; i < 4; i++){ //poisson sampling
                            vec2 poisSample;
                            if(i == 0) poisSample = vec2( -0.94201624, -0.39906216 );
                            else if(i == 1) poisSample = vec2( 0.94558609, -0.76890725 );
                            else if(i == 2) poisSample = vec2( -0.094184101, -0.92938870 ); 
                            else if(i == 3) poisSample = vec2( 0.34495938, 0.29387760 );
                        
                            //float sampleDepth = sampleShadowmap(vec2(projCoords.xy + vec2(x, y) * texelSize + poisSample / 1500.0), f_cascade);         
                            shadow += sampleShadowmap(projCoords, bias, vec2(x, y) * texelSize + poisSample / 1500.0);
                      }
                }    
            }
            shadow /= PCF_PIXELCT * 4.0;        
        }
        else { //in shadowmapped region that will not be filtered (>=2nd cascades in this instance)
            shadow = sampleShadowmap(projCoords, bias, vec2(0.0));
        }

        //visualize cascades (debugging)
        //if(cascade == 0) FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        //if(cascade == 1) FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        //if(cascade == 2) FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }

    //depth fog for far chunks
    float depth = (f_distance - near) / far;
    float depthAlpha = 1.0f;
    if (depth > fog_start && DISPLAY_FOG) {
        depthAlpha = -depth / (fog_end - fog_start) + fog_start / (fog_end - fog_start) + 1.0;
    }

    //final fragment color
    FragColor = texture(atlas, vec3(f_texCoord, f_texImage)) * vec4(vec3(finLight) * (1.0 - shadow * 0.35), depthAlpha);
    
    //visualize ambiuous cascade level region (debugging)
    //if(f_cascadeValid == 0) FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}