/**
 * @brief Upsample pixel data from a texture.
 * 
 *
 */

#version 450 core

out vec4 FragColor; // Output color
in vec2 texCoords; // Texture coordinates

uniform sampler2D mandelbrotTexture; // Mandelbrot texture sampler

void main() 
{
    // Sample the 'mandelbrotTextur' at the given texture coordinates
    FragColor = texture(mandelbrotTexture, texCoords);

    return;
}