#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 u_CameraPos;

float compute_fade(float dist, float fade_start, float fade_end) {
    return 1.0 - clamp((dist - fade_start) / (fade_end - fade_start), 0.0, 1.0);
}

void main()
{
    float gridSpacing = 1.0; 
    float lineThickness = 0.01; 

    vec2 coord = WorldPos.xz / gridSpacing;
    
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    
    float line = min(grid.x, grid.y);
    
    float gridAlpha = 1.0 - min(line, 1.0);

    float fade_start = 50.0;
    float fade_end = 100.0;
    float fade = compute_fade(distance(u_CameraPos.xz, WorldPos.xz), fade_start, fade_end);

    vec3 gridColor = vec3(0.3);
    
    // Set the final color
    FragColor = vec4(gridColor, gridAlpha * fade);
}