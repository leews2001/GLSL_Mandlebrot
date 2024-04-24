#version 450 core

out vec3 color; // Forward color to fragment shader

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor; // Add color attribute

void main() {
    gl_Position = vec4(aPos, 1.0);
    color = aColor; // Pass color to fragment shader
    return;
}