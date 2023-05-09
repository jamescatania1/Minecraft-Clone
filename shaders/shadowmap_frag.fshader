#version 330 core

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix;
    float near;
    float far;
};

void main()
{             
	gl_FragDepth = gl_FragCoord.z;
}