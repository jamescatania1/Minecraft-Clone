#version 330 core
layout(location = 0) in uint vertData;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix;
    float near;
    float far;
};

uniform mat4 transform;

out float color;
out float distance;
out vec2 texCoord;
out vec3 fragPos;
out vec4 fragPosLightSpace;
flat out uint face;

//out vec3 FragPos;
//out vec4 FragPosLightSpace;

// Bits of "vertData" -> Vertex Attribute:
// 0-4      ->      x
// 5-13     ->      y
// 14-18    ->      z
// 19-22    ->      texture x
// 23-26    ->      texture y
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

    float tx = float((vertData >> 19u) & 0xFu);
    float ty = float((vertData >> 23u) & 0xFu);
    texCoord = vec2(0.0625 * tx, 0.0625 * ty);

    face = (vertData >> 27u) & 0xFu;
    if (face == uint(0)) color = 1.0; //top
    else if (face == uint(1)) color = 0.6; //bottom
    else if (face == uint(2)) color = 1.0; //front
    else if (face == uint(3)) color = 1.0; //back
    else if (face == uint(4)) color = 1.0; //left
    else if (face == uint(5)) color = 1.0; //right
    else color = 1.0;

    distance = sqrt(gl_Position.x * gl_Position.x + gl_Position.y * gl_Position.y + gl_Position.z * gl_Position.z);
    gl_Position = projMatrix * gl_Position;

    fragPos = vec3(transform * vec4(x, y, z, 1.0));
    fragPosLightSpace = sunMatrix * vec4(fragPos, 1.0);
}