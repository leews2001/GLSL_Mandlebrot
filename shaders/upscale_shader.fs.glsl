#version 450 core

out vec4 FragColor;

in vec2 texCoords;
uniform sampler2D mandelbrotTexture;

void main() {
    FragColor = texture(mandelbrotTexture, texCoords);

    return;
}