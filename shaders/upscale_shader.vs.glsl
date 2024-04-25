/**
 * @brief Vertex shader for upscaling a texture.
 *
 * @param aPos, vertex position attribute.
 * @return texCoords, texture coordinates for sampling texture.
 */

#version 450 core

out vec2 texCoords; // Output texture coordinates

layout(location = 0) in vec2 aPos; // Vertex position attribute

void main() {
    // Convert vertex position from [-1, 1] to [0, 1] to generate texture coordinates
    texCoords = (aPos + 1.0) * 0.5;

    // Set the position of the current vertex
    gl_Position = vec4(aPos, 0.0, 1.0);
    return;
}