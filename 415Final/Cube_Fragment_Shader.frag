#version 330 core

// Texture coordinate values from the vertex shaders
in vec2 UV;
in vec3 fragmentNormal;
in vec4 fragmentPosition;
//in vec3 fragmentColor;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture_Colors;
uniform vec3 lightPosition;
uniform vec3 upVector;
uniform vec3 ambientLight;
uniform vec3 diffuseLight;
uniform vec3 specularLight;

uniform float specCoefficient;
uniform float shine;

void main(){

	vec3 normNormal, V, R, lightDirection, normLight, colors;
	float lightDotNormal;

	colors = texture2D( texture_Colors, UV ).rgb;

	//normLight = normalize(lightPosition);

	lightDirection = normalize( lightPosition - fragmentPosition.xyz);

	V = normalize(-fragmentPosition.xyz);

	normNormal = normalize(fragmentNormal);

	lightDotNormal = dot(normNormal, lightDirection);

	R = normalize(2 * lightDotNormal * normNormal - lightDirection);

	color.rgb = vec3(0,0,0);
	
	//Ambient
	color = color + ambientLight * colors * (0.5f + 0.5f * dot(normNormal, normalize(upVector)));
	
	//Diffuse
	color = color + diffuseLight * colors * (max(0.0f, dot(normNormal,lightDirection)));

	//Spec
	color = color + specCoefficient * specularLight * pow(max(0.0f,dot(V,R)), shine);

	//color = upVector;

}
