#ifndef __VERTEX_H__
#define __VERTEX_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <array>


#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>


class Vertex
{


public:

	Vertex();
	Vertex(gmtl::Vec3f position, gmtl::Vec3f normal, GLfloat u, GLfloat v);
	~Vertex();

	std::vector<float> Pos2Vec();
	std::vector<float> Norm2Vec();
	

	gmtl::Vec3f position, normal;
	GLfloat u,v;


private:
	std::vector<float> Vec2Vec(gmtl::Vec3f v);
};

#endif __VERTEX_H__