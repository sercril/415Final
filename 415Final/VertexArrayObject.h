#ifndef __VERTEX_ARRAY_OBJECT_H__
#define __VERTEX_ARRAY_OBJECT_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <map>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

#include "Vertex.h"
#include "Texture.h"
#include "BlendObj.h"

using namespace std;

class VertexArrayObject
{
	public:
		VertexArrayObject();
		VertexArrayObject(string objectFile, GLuint program);
		~VertexArrayObject();

		void GetData();
		void BorstImport(string filename);

		void GenerateSmoothNormals();
		void GenerateSplitNormals();

		GLuint vertexArray, matrix_loc, program;
		std::vector<Vertex> verticies;
		std::vector<GLushort> index_data;

	private:
		void LoadVerticies(std::vector<GLfloat> vertexData, std::vector<GLfloat> normalData, std::vector<GLfloat> uvData);

		GLuint vertexBuffer, colorBuffer, indexBuffer, uvBuffer, normalBuffer, 
			vertposition_loc, vertcolor_loc, vertex_UV, textureLocation, normal_loc;

		std::vector<GLfloat> vertex_data, uv_data, normal_data, tangent_data, bitangent_data;
		

};



#endif __VERTEX_ARRAY_OBJECT_H__