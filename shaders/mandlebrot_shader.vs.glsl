#version 450 core

layout (location = 0) in vec3 pos;
out vec2 pass_Position;

uniform mat4 projection;

void main()
{
    gl_Position =  vec4(pos.xyz, 1.0);
    pass_Position = pos.xy;
}