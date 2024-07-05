#version 440  

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 normal;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;
out vec4 ShadowCoord;
out float fog_visibility;

uniform mat4 depthMVP;
uniform mat4 modelTransformMatrix;
uniform mat4 projection;
uniform mat4 view;

const float FogDensity = 0.02;
const float FogGradient = 1.5;

void main()
{
    vec4 v = vec4(position,1.0);
    vec4 newPosition =  modelTransformMatrix * v;
	vec4 projectedPosition = projection * view  * newPosition;
	vertexPositionWorld = newPosition.xyz;

	vec4 normal_temp = modelTransformMatrix * vec4(normal,0);
	normalWorld = normal_temp.xyz;

	gl_Position = projectedPosition;
	UV = vertexUV;

	float distance = length((view * modelTransformMatrix * vec4(position, 1)));
	fog_visibility = exp(-pow((distance * FogDensity), FogGradient));
    fog_visibility = clamp(fog_visibility, 0, 1);

	ShadowCoord= depthMVP* vec4(position,1);
}