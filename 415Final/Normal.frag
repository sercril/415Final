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

uniform vec3 ambientLight;
uniform vec3 diffuseLight;
uniform vec3 specularLight;
uniform float specCoefficient;
uniform float shine;

void main()
{

	vec2 uv = vec2(1,-1);
	vec3 normNormal, V, R, lightDirection, normLightDirection, normLight, colors, normTangentLight, normal_colors, normUp, decodedNormal;
	float lightDotNormal;

	colors = texture2D( texture_Colors, UV ).rgb;	
	normal_colors = texture2D( normal_Colors, UV * 2.0f).rgb;

	colors = vec3(0.5,0.5,0.5);
	//normal_colors = vec3(0.5, 0.5, 1.0);

	normTangentLight = normalize(tangentLight);
	normUp = normalize(upVector);


	//decodedNormal = vec3(normal_colors.x * 2.0f, normal_colors.y * 2.0f,normal_colors.z * 2.0f);
	//decodedNormal = vec3(decodedNormal.x - 1.0f, decodedNormal.y - 1.0f,decodedNormal.z - 1.0f);
	decodedNormal = normalize((normal_colors*2.0f)-1.0f);

	//normLight = normalize(lightPosition);

	//lightDirection = normalize( lightPosition - fragmentPosition.xyz);
	normLightDirection = normalize(tangentLight - fragmentPosition.xyz);

	//V = normalize(-fragmentPosition.xyz);
	V = normalize(view);

	normNormal = normalize(fragmentNormal);

	//lightDotNormal = dot(normNormal, lightDirection);
	lightDotNormal = dot(decodedNormal, normTangentLight);

	//R = normalize(2 * lightDotNormal * normNormal - lightDirection);
	R = normalize(2 * lightDotNormal * decodedNormal - normLightDirection);
	R = reflect(-normLightDirection, decodedNormal);
	color.rgb = vec3(0,0,0);
	
	//Ambient
	//color = color + ambientLight * colors * (0.5f + 0.5f * dot(normNormal, normalize(upVector)));
	color = color + ambientLight * colors * (0.5f + 0.5f * dot(decodedNormal, normalize(normUp)));
	
	//Diffuse
	//color = color + diffuseLight * colors * (max(0.0f, dot(normNormal,lightDirection)));
	color = color + diffuseLight * colors * (max(0.0f, dot(decodedNormal,normLightDirection)));

	//Spec
	//color = color + specCoefficient * specularLight * pow(max(0.0f,dot(V,R)), shine);
	color = color + specCoefficient * specularLight * pow(max(0.0f,dot(V,R)), shine);

	//color = normUp;

}
