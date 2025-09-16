
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // The vertex normal from the VBO

// frag
out vec3 FragPos;
out vec3 Normal;

// mat
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    FragPos = vec3(u_Model * vec4(aPos, 1.0));
    
    Normal = mat3(transpose(inverse(u_Model))) * aNormal;

    gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
}