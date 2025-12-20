//Transformation
uniform mat4 transformIn;

void main()
{
    //Transformation applied to vertices
    gl_Position = transformIn * vec4(position.x, position.y, position.z, 1.0);
    //Sending texture coordinates to next stage
    textureCoordinatesFrag = textureCoordinatesVertex;
}