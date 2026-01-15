#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 textureCoordinatesVertex;

//Transformation
uniform mat4 transformIn;

out vec2 textureCoordinatesFrag;

void main()
{
    //Transformation applied to vertices
    gl_Position = transformIn * vec4(position.x, position.y, position.z, 1.0);
    //Sending texture coordinates to next stage
    textureCoordinatesFrag = textureCoordinatesVertex;
}