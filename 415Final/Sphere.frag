#version 330 core

// Texture coordinate values from the vertex shaders
in vec2 UV;
in vec3 fragmentNormal;
in vec4 fragmentPosition;
//in vec3 fragmentColor;

// Ouput data
layout(location = 0) out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture_Colors;
uniform vec3 lightPosition;
uniform vec3 upVector;
uniform float lightIntensity;
uniform float specCoefficient;
uniform float shine;

void main(){

	vec3 normNormal, V, R, lightDirection, normLight, colors, lightColor;
	float lightDotNormal;

	lightColor = vec3(1.0,1.0,1.0);

	colors = texture2D( texture_Colors, UV).rgb;
	
	lightDirection = normalize( lightPosition - fragmentPosition.xyz);

	V = normalize(-fragmentPosition.xyz);

	normNormal = normalize(fragmentNormal);

	lightDotNormal = dot(normNormal, lightDirection);

	R = normalize(2 * lightDotNormal * normNormal - lightDirection);

	color.rgb = vec3(0,0,0);
	
	//Ambient
	color = color + (lightColor * lightIntensity) * colors * (0.5f + 0.5f * dot(normNormal, normalize(upVector)));
	
	//Diffuse
	color = color + (lightColor * lightIntensity) * colors * (max(0.0f, dot(normNormal,lightDirection)));

	//Spec
	color = color + specCoefficient * (lightColor * lightIntensity) * pow(max(0.0f,dot(V,R)), shine);

	//color = vec3(UV,0);

}
