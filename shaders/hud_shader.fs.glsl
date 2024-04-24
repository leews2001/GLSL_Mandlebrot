#version 450 core

out vec4 FragColor;

in vec3 color; // Received color from vertex shader

void main() {
    FragColor = vec4(color, 0.5); // Output color received from vertex shader
    return;
}