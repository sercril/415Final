
#include "Vertex.h"

Vertex::Vertex() {}
Vertex::Vertex(gmtl::Vec3f position, gmtl::Vec3f normal, GLfloat u, GLfloat v)
{
	this->position = position;
	this->normal = normal;
	this->u = u;
	this->v = v;
}
Vertex::~Vertex() {}

std::vector<float> Vertex::Pos2Vec()
{
	return this->Vec2Vec(this->position);
}
std::vector<float> Vertex::Norm2Vec()
{
	return this->Vec2Vec(this->normal);
}

std::vector<float> Vertex::Vec2Vec(gmtl::Vec3f v)
{
	std::vector<float> r = {v[0], v[1], v[2]};
	return r;
}