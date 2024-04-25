/**
 * @brief Vertex shader for line geometry drawing
 *
 * @param aPos, vertex position attribute.
 * @param aColor, color attribute for each vertex.
 * @return color, color of the vertex, forwarded to the fragment shader
 */

#version 450 core

out vec3 color; // Forward color to fragment shader

layout(location = 0) in vec3 aPos;   // Vertex position attribute
layout(location = 1) in vec3 aColor; // Color attribute

void main() {
    // Set current vertex position
    gl_Position = vec4(aPos, 1.0);

    // Pass color attribute to fragment shader
    color = aColor;  

    return;
}