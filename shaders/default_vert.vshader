#version 330 core
layout(location = 0) in uint vertData;

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
    
    gl_Position = viewMatrix * transform * vec4(x, y, z, 1.0f);
    if (int((vertData >> 31u) & 0x1u) == 1) {
        gl_Position.y -= 0.15f;
    }

    float tx = float((vertData >> 19u) & 0xFu);
    float ty = float((vertData >> 23u) & 0xFu);
    texCoord = vec2(0.0625 * tx, 0.0625 * ty);

    color = float((vertData >> 27u) & 0xFu) / 15.0f;

    distance = sqrt(gl_Position.x * gl_Position.x + gl_Position.y * gl_Position.y + gl_Position.z * gl_Position.z);
    gl_Position = projMatrix * gl_Position;
}