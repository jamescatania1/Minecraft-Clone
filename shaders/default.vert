#version 430 core
layout(location = 0) in uint vertData;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[3];
    float near;
    float far;
};

uniform mat4 transform;
uniform float cascadePlaneDistances[3];
uniform sampler2DArray shadowMap;

out float f_color;
out float f_distance;
out vec2 f_texCoord;
flat out int f_texImage;
flat out int f_cascade;
out vec3 f_fragPos;
out vec4 f_fragPosLightSpace[3];
flat out uint f_face;
flat out int f_cascadeValid;

// Bits of "vertData" -> Vertex Attribute:
// 0-4      ->      x
// 5-13     ->      y
// 14-18    ->      z
// 19-24    ->      texture number
// 25-26    ->      texture corner
// 27-30    ->      color
// 31       ->      offset y

void main() {
    float x = float(vertData & 0x1Fu);
    float y = float((vertData >> 5u) & 0x1FFu);
    float z = float((vertData >> 14u) & 0x1Fu);
    if (int((vertData >> 31u) & 0x1u) == 1) {
        y -= 0.15f;
    }
    gl_Position = viewMatrix * transform * vec4(x, y, z, 1.0f);

    //texture image index in array texture
    f_texImage = int((vertData >> 19u) & 0x3Fu);

    //texture coordinates of vertex's corner in texture
    uint corner = (vertData >> 25u) & 0x3u;
    if (corner == uint(0)) f_texCoord = vec2(0.0, 0.0);
    else if (corner == uint(1)) f_texCoord = vec2(1.0, 0.0);
    else if (corner == uint(2)) f_texCoord = vec2(1.0, 1.0);
    else if (corner == uint(3)) f_texCoord = vec2(0.0, 1.0);

    f_face = (vertData >> 27u) & 0xFu;
    if (f_face == uint(0)) f_color = 1.0; //top
    else if (f_face == uint(1)) f_color = 0.6; //bottom
    else if (f_face == uint(2)) f_color = 0.7; //front
    else if (f_face == uint(3)) f_color = 0.95; //back
    else if (f_face == uint(4)) f_color = 0.8; //left
    else if (f_face == uint(5)) f_color = 0.9; //right
    else f_color = 1.0;

    f_distance = sqrt(gl_Position.x * gl_Position.x + gl_Position.y * gl_Position.y + gl_Position.z * gl_Position.z);
    gl_Position = projMatrix * gl_Position;

    f_fragPos = vec3(transform * vec4(x, y, z, 1.0));

    for (int i = 0; i < 3; i++) {
        f_fragPosLightSpace[i] = sunMatrix[i] * vec4(f_fragPos, 1.0);
    }

    f_cascade = -1;
    f_cascadeValid = 1;
    for(int i = 0; i < 3; i++) {
        if(f_distance < cascadePlaneDistances[i]) {
            f_cascade = i;
            if((i == 0 || f_distance > cascadePlaneDistances[i - 1] + 2.0)
            && f_distance < cascadePlaneDistances[i] - 2.0) {
                f_cascadeValid = 1;
            }
            else f_cascadeValid = 0;
            break;
        }
    }

}