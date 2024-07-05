#version 440 

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;



// Ouputdata
out layout(location = 0) float fragmentdepth;
void main(){
// Not really needed, OpenGL does it anyway
fragmentdepth= gl_FragCoord.z;
}
