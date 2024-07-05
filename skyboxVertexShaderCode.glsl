#version 440
layout (location = 0) in vec3 position;
out vec3 TexCoords;
out float visibility;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 M;
const float FogDensity = 0.05;
const float FogGradient = 3.5;
void main()
{
   float distance = length(view * M * vec4(position, 1));
   visibility = exp(-pow((distance * FogDensity), FogGradient));
   visibility = clamp(visibility, 0, 1);
   vec4 pos = projection * view * M * vec4(position, 1.0);
   gl_Position = pos;
   TexCoords = position;
}
