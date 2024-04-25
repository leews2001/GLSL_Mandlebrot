/**
 * @brief Fragment shader for rendering 2D lines.
 *
 * @param color, color attribute from the vertex shader.
 * @return FragColor, color of the fragment with alpha 0.5.
 */

#version 450 core

out vec4 FragColor; // Output color
in vec3 color;      // Input color from vertex shader

void main() {
    // Assign the color received from the vertex shader
    FragColor = vec4(color, 0.5);

    return;
}