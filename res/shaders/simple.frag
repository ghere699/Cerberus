#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform vec3 u_viewPos;
// uniform sampler2D u_diffuseTexture; // Commentato per ora

// --- IL NOSTRO NUOVO FLAG ---
// 0 = Solid, 1 = Wireframe, 2 = Points
uniform int u_renderMode; 

void main()
{
    if (u_renderMode == 0) // Modalità Solida
    {
        // Usa un colore bianco di base invece della texture per ora
        vec3 texColor = vec3(1.0, 1.0, 1.0);

        // --- Calcolo dell'illuminazione (invariato) ---
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * u_lightColor;
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(u_lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * u_lightColor;
        float specularStrength = 0.5;
        vec3 viewDir = normalize(u_viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * u_lightColor;

        vec3 result = (ambient + diffuse + specular) * texColor;
        FragColor = vec4(result, 1.0);
    }
    else // Modalità Wireframe o Punti
    {
        // Ignora tutta l'illuminazione e disegna con un colore verde brillante
        FragColor = vec4(0.0, 1.0, 0.4, 1.0); 
    }
}