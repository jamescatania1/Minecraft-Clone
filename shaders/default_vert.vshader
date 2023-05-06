#version 330 core
layout(location = 0) in float vertData;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    float near;
    float far;
};

uniform mat4 transform;

out float color;
out vec2 texCoord;
out float distance;

// Bits of "vertData" -> Vertex Attribute:
// 0-4      ->      x
// 5-14     ->      y
// 15-19    ->      z
// 20-23    ->      texture x
// 24-27    ->      texture y
// 28-31    ->      color

void main() {
    uint data = uint(vertData);
    float x = float(data & 0x1Fu);
    float y = float((data >> 5u) & 0x1FFu);
    float z = float((data >> 14u) & 0x1Fu);
    gl_Position = viewMatrix * transform * vec4(x, y, z, 1.0f);

    float tx = float((data >> 19u) & 0xFu);
    float ty = float((data >> 23u) & 0xFu);
    texCoord = vec2(0.0625 * tx, 0.0625 * ty);

    color = 1.0f;

    distance = sqrt(gl_Position.x * gl_Position.x + gl_Position.y * gl_Position.y + gl_Position.z * gl_Position.z);
    gl_Position = projMatrix * gl_Position;
}