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


void main() {
    float x = float(vertData & 0x1Fu);
    float y = float((vertData >> 5u) & 0x1FFu);
    float z = float((vertData >> 14u) & 0x1Fu);
    if (int((vertData >> 31u) & 0x1u) == 1) {
        y -= 256.0; //water. possible todo: separate transparent meshes, leave them out of shadowmap
    }
    gl_Position = sunMatrix * transform * vec4(x, y, z, 1.0f);
}