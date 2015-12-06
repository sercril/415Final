#version 330

// Input vertex data
in vec4 vertexPosition;
in vec2 vertexUV;
in vec3 vertexNormal;
//layout (location = 1) in vec3 vertexColor;


// Output texture coordinates data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 fragmentNormal;
out vec4 fragmentPosition;
//out vec3 fragmentColor;

uniform mat4 Matrix;
uniform mat4 NormalMatrix;
uniform mat4 modelview;



void main(){
	// Output position of the vertex, in clip space : MVP * position
	gl_Position = Matrix * vertexPosition;

	mat3 matrixM = transpose(inverse(mat3(modelview)));

	// UV of the vertex. No special space for this one.
	UV = vertexUV;

	fragmentNormal = (matrixM * vertexNormal).xyz;

	fragmentPosition = (modelview * vertexPosition);
	

	//fragmentColor = vertexColor;
}

