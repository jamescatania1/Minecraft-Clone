#version 330 core
out vec4 FragColor;
  
in vec3 color;
in vec2 texCoord;
in float distance;

layout (std140) uniform Camera {
    mat4 viewMatrix;
    mat4 projMatrix;
    float near;
    float far;
};

uniform sampler2D atlas;

float fog_start = 0.75;
float fog_end = 0.84;

void main()
{
    float depth = (distance - near) / far;

    if(depth > fog_start) {
        FragColor = texture(atlas, texCoord) * vec4(color, 
        -depth / (fog_end - fog_start) + fog_start / (fog_end - fog_start) + 1.0);
    }
    else{
        FragColor = texture(atlas, texCoord) * vec4(color, 1.0);
    }

    //show depth buffer only
    //FragColor = vec4(vec3(depth), 1.0);
}