/**
 * @brief Vertex shader for rendering Mandelbrot
 *
 * @param pos,  vertex position attribute. 
 * @return planePos, 2d plane position forwarded to the fragment shader.
 */

#version 450 core

out vec2 planePos;                  // Output 2d plane position
layout (location = 0) in vec2 aPos; // incoming vertex position attribute

void main()
{
    // Set current vertex position
    // Note: gl_Position is a homogeneous coordinate
    gl_Position = vec4(aPos.xy, 0.,1.);

    // foward position to the fragment shader
    planePos = aPos;

    return;
}