#version 430 core

layout(std140) uniform Camera{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 sunMatrix[3];
    float near;
    float far;
};

layout(location = 0) in uint vertData;

flat out int cascade;

uniform mat4 transform;
uniform float cascadePlaneDistances[3];

void main() {
    float x = float(vertData & 0x1Fu);
    float y = float((vertData >> 5u) & 0x1FFu);
    float z = float((vertData >> 14u) & 0x1Fu);
    if (int((vertData >> 31u) & 0x1u) == 1) {
        y -= 256.0; //water. possible todo: separate transparent meshes, leave them out of shadowmap
    }
    gl_Position = transform * vec4(x, y, z, 1.0f);

    vec4 vpos = viewMatrix * gl_Position;
    cascade = -1;
    float distance = sqrt(vpos.x * vpos.x + vpos.y * vpos.y + vpos.z * vpos.z);
    for(int i = 0; i < 3; i++) {
        if(distance < cascadePlaneDistances[i]) {
            cascade = i;
            break;
        }
    }
}