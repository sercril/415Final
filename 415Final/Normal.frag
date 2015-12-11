#version 330 core

// Texture coordinate values from the vertex shaders
in vec2 UV;
in vec3 fragmentNormal;
in vec4 fragmentPosition;
in vec3 tangentLight;
in vec3 view;
in vec3 upVector;
//in vec3 fragmentColor;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture_Colors;
uniform sampler2D normal_Colors;
uniform vec3 lightPosition;
uniform float lightIntensity;
uniform float specCoefficient;
uniform float shine;

void main()
{

	vec2 uv = vec2(1,-1);
	vec3 normNormal, V, R, lightDirection, normLightDirection, normLight, colors, normTangentLight, normal_colors, normUp, decodedNormal, lightColor;
	float lightDotNormal;
	
	colors = texture2D( texture_Colors, UV ).rgb;	
	normal_colors = texture2D( normal_Colors, UV ).rgb;

	lightColor = vec3(1.0,1.0,1.0);

	//colors = vec3(0.5,0.5,0.5);

	normTangentLight = normalize(tangentLight);
	normUp = normalize(upVector);

	decodedNormal = normalize((normal_colors*2.0f)-1.0f);
	
	normLightDirection = normalize(tangentLight - fragmentPosition.xyz);

	V = normalize(view);

	normNormal = normalize(fragmentNormal);

	lightDotNormal = dot(decodedNormal, normTangentLight);
	
	R = normalize(2 * lightDotNormal * decodedNormal - normLightDirection);
	
	color.rgb = vec3(0,0,0);
	
	//Ambient
	color = color + (lightColor * lightIntensity) * colors * (0.5f + 0.5f * dot(decodedNormal, normalize(normUp)));
	
	//Diffuse
	color = color + (lightColor * lightIntensity) * colors * (max(0.0f, dot(decodedNormal,normLightDirection)));

	//Spec
	color = color + specCoefficient * (lightColor * lightIntensity) * pow(max(0.0f,dot(V,R)), shine);

	//color = normUp;

}
