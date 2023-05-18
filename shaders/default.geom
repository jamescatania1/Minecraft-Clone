#version 430 core

in float color[];
in vec2 texCoord[];
flat in int texImage[];
flat in int cascade[];
in float distance[];
in vec3 fragPos[];
in vec4 fragPosLightSpace[];
flat in uint face[];

out float f_color;
out vec2 f_texCoord;
flat out int f_texImage;
flat out int f_cascade;
out float f_distance;
out vec3 f_fragPos;
out vec4 f_fragPosLightSpace;
flat out uint f_face;


layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[3];
    float near;
    float far;
};

void main() {
    int cindex = cascade[0] < cascade[1] ? 0 : 1;
    cindex = cascade[cindex] < cascade[2] ? cindex : 2;
    for (int i = 0; i < 3; i++) {
        f_color = color[i];
        f_texCoord = texCoord[i];
        f_texImage = texImage[i];
        f_cascade = cascade[i]; //so it aligns with the shadowmap's cascade at each fragment
        f_distance = distance[i];
        f_fragPos = fragPos[i];
        f_fragPosLightSpace = fragPosLightSpace[i];
        f_face = face[i];

        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}