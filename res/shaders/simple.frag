
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 u_objectColor;
uniform vec3 u_lightColor;
uniform vec3 u_lightPos;
uniform vec3 u_viewPos;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // 32 is the shininess
    vec3 specular = specularStrength * spec * u_lightColor;

    vec3 result = (ambient + diffuse + specular) * u_objectColor;
    FragColor = vec4(result, 1.0);
}