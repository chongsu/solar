#version 440 

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in vec4 ShadowCoord;
in float fog_visibility;

// Ouput data
out vec4 finalcolor;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform sampler2D myTextureSampler_2;
uniform sampler2D myTextureSampler_3;

uniform vec3 ambientLight;
uniform vec3 lightPositionWorld;
uniform vec3 eyePositionWorld;
uniform float diffusefactor;
uniform float specularfactor;
uniform sampler2D shadowMap;

uniform bool normalMapping_flag;

void main(){


     vec3 normal;
     normal=vertexPositionWorld;
     if(normalMapping_flag)
     {
     normal=texture(myTextureSampler_3,UV).rgb;
     normal=normalize(normal * 2.0 -1.0);
     }


     float bias = 0.005;
     float visibility = 1.0;
     vec2 poissonDisk[4] = vec2[](
     vec2( -0.94201624, -0.39906216 ),
     vec2( 0.94558609, -0.76890725 ),
     vec2( -0.094184101, -0.92938870 ),
     vec2( 0.34495938, 0.29387760 )
    );

    for (int i=0;i<4;i++){
        if ( texture( shadowMap, ShadowCoord.xy + poissonDisk[i]/700.0 ).z  <  ShadowCoord.z-bias ){
            visibility-=0.2;
         }
    }
    vec4 newambientLight = vec4(ambientLight,1.0);
    vec3 MaterialColor = (0.1*texture(myTextureSampler,UV)+0.9*texture(myTextureSampler_2,UV)).rgb;

	vec4 newMaterialColor = vec4(MaterialColor,1.0);
	vec4 MaterialSpecularColor = vec4(0.5,0.5,0.5,0);

	vec3 lightVectorWorld = normalize(lightPositionWorld - normal);
	float brightness = diffusefactor * dot(lightVectorWorld, normalize(normalWorld));
	vec4 diffuseLight = vec4(brightness,brightness,brightness,1.0);


	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld,normalWorld);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - normal);
	float s= clamp(dot(reflectedLightVectorWorld,eyeVectorWorld), 0, 1);
	s = specularfactor * pow(s,300);
	vec4 specularLight =  vec4 (s, s, s, 1);

	finalcolor = mix(vec4(0.7, 0.7, 0.7, 0.4), (newMaterialColor * newambientLight +visibility* newMaterialColor*clamp(diffuseLight,0,1)
	+visibility*MaterialSpecularColor * specularLight), fog_visibility);
	

}