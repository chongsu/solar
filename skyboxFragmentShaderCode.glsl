#version 440


in vec3 TexCoords;
in float visibility;
out vec4 color;

uniform samplerCube skybox;

void main()
{    
    vec4 dacolor = texture(skybox, TexCoords);
	color = mix(vec4(0.5,0.5,0.5,1), dacolor, visibility);
}