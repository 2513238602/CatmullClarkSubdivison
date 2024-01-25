#version 330 core

layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 VertexColor; // Added output variable for vertex color

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Set a unique color for each vertex (you can customize these colors)
    if (gl_VertexID == 0) {
        VertexColor = vec4(1.0, 0.0, 0.0, 1.0); // Red
    } else if (gl_VertexID == 1) {
        VertexColor = vec4(0.0, 1.0, 0.0, 1.0); // Green
    } else if (gl_VertexID == 2) {
        VertexColor = vec4(0.0, 0.0, 1.0, 1.0); // Blue
    } else {
        VertexColor = vec4(1.0, 1.0, 0.0, 1.0); // Yellow
    }
}
