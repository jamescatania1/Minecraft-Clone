#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    float near;
    float far;
};

uniform mat4 transform;

out vec3 color;
out vec2 texCoord;
out float distance;

void main() {
    gl_Position = viewMatrix * transform  * vec4(aPos, 1.0f);
    distance = sqrt(gl_Position.x * gl_Position.x + gl_Position.y * gl_Position.y + gl_Position.z * gl_Position.z);
    gl_Position = projMatrix * gl_Position;
    color = aColor;
    texCoord = aTexCoord;
}