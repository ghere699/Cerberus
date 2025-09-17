#version 330 core
out vec4 FragColor;

// Input from the vertex shader
in vec3 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}