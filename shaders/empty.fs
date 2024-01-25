#version 330 core

in vec4 VertexColor; // Input variable from the vertex shader

out vec4 FragColor;

void main()
{
    FragColor = VertexColor;
}
