#version 440 
// Input vertex data, different for all executions of this shader.

in layout(location = 0) vec3 position_modelSpace;

// Values that stay constant for the whole mesh.

uniform mat4 depthMVP;

void main(){
gl_Position= depthMVP * vec4(position_modelSpace,1);
}