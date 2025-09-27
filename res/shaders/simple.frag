#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform vec3 u_viewPos;

uniform vec3 u_objectColor;
uniform float u_shininess;
uniform float u_ambientStrength;
uniform float u_specularStrength;

// 0 = Solid, 1 = Wireframe, 2 = Points
uniform int u_renderMode; 

void main()
{
    if (u_renderMode == 0) // Lit, solid rendering
    {
        vec3 surfaceColor = u_objectColor;

        // --- Lighting calculations ---
        vec3 ambient = u_ambientStrength * u_lightColor;
        
        // --- Diffuse Calculation ---
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(u_lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        float diffuseStrength = 0.8;
        vec3 diffuse = diffuseStrength * diff * u_lightColor;

        // --- Specular Calculation ---
        float specularStrength = 1.0;
        vec3 viewDir = normalize(u_viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_shininess);
        vec3 specular = u_specularStrength * spec * u_lightColor;

        vec3 result = (ambient + diffuse + specular) * surfaceColor;
        FragColor = vec4(result, 1.0);
    }
    else // Wireframe or Point rendering
    {
        // Keep the hard-coded color for these modes
        FragColor = vec4(0.0, 1.0, 0.4, 1.0); 
    }
}