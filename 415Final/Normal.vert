#version 330

// Input vertex data
in vec4 vertexPosition;
in vec2 vertexUV;
in vec3 vertexNormal;
in vec3 vertTangent;
in vec3 vertBiTangent;

//layout (location = 1) in vec3 vertexColor;


// Output texture coordinates data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 fragmentNormal;
out vec4 fragmentPosition;
out vec3 tangentLight;
out vec4 view;
out vec3 upVector;
//out vec3 fragmentColor;

uniform vec3 lightPosition;
uniform mat4 Matrix;
uniform mat4 NormalMatrix;
uniform mat4 modelview;
uniform mat4 PMMatrix;
uniform vec3 eyeUpVector;

void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position = PMMatrix * vertexPosition;

	vec4 tanLight, eyePos;

	mat3 matrixM = transpose(inverse(mat3(modelview)));
	mat4 inverseModelView = inverse(modelview);

	// UV of the vertex. No special space for this one.
	UV = vertexUV;

	fragmentNormal = (matrixM * vertexNormal).xyz;

	fragmentPosition = (modelview * vertexPosition);
	
	tanLight = inverseModelView * vec4(lightPosition,1);
	tanLight = tanLight - vertexPosition;
	tangentLight = vec3( dot(tanLight.xyz, vertTangent), dot(tanLight.xyz, vertBiTangent),dot(tanLight.xyz, vertexNormal));

	eyePos = inverseModelView * vec4(0,0,0,1);
	view = eyePos - vertexPosition;
	view = vec4( dot(view.xyz, vertTangent), dot(view.xyz, vertBiTangent),dot(view.xyz, vertexNormal), 1);

	upVector = vec3( dot(eyeUpVector, vertTangent), dot(eyeUpVector, vertBiTangent),dot(eyeUpVector, vertexNormal));

	//fragmentColor = vertexColor;
}

