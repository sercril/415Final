#version 330 core


in vec4 vertexPosition;

out vec2 UV;

void main(){
	gl_Position = vertexPosition;
	UV = (vertexPosition.xy+vec2(1,1))/2.0;
}