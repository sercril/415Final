#include "SceneObject.h"

using namespace std;

SceneObject::SceneObject()
{

}
SceneObject::SceneObject(string objectFile, gmtl::Vec3f dimensions, GLuint program) : SceneObject(objectFile, dimensions[0], dimensions[1], dimensions[2], program)
{}
SceneObject::SceneObject(string objectFile, float length, float width, float depth, GLuint program)
{
	this->length = length;
	this->width = width;
	this->depth = depth;
	this->radius = 0;


	this->scale = gmtl::makeScale<gmtl::Matrix44f>(gmtl::Vec3f(this->length, this->width, this->depth));
	this->scale.setState(gmtl::Matrix44f::AFFINE);

	this->VAO = VertexArrayObject(objectFile, program);

	this->Init();

}

SceneObject::SceneObject(string objectFile, float radius, GLuint program)
{

	this->radius = radius;

	this->scale = gmtl::makeScale<gmtl::Matrix44f>(gmtl::Vec3f(this->radius, this->radius, this->radius));
	this->scale.setState(gmtl::Matrix44f::AFFINE);

	this->VAO = VertexArrayObject(objectFile, program);

	this->Init();

}
SceneObject::SceneObject(string objectFile, float radius, float height, GLuint program)
{
	this->radius = radius;
	this->width = height;
	this->scale = gmtl::makeScale<gmtl::Matrix44f>(gmtl::Vec3f(this->radius, this->width, this->radius));
	this->scale.setState(gmtl::Matrix44f::AFFINE | gmtl::Matrix44f::NON_UNISCALE);

	this->VAO = VertexArrayObject(objectFile, program);

	this->Init();
}

SceneObject::~SceneObject()
{

}

void SceneObject::Init()
{
	gmtl::identity(this->translation);
	gmtl::identity(this->transform);
	this->rotation = gmtl::Quatf(0.0f, 0.0f, 0.0f, 1.0f);


	this->upVector_loc = glGetUniformLocation(this->VAO.program, "upVector");
	this->specCoefficient_loc = glGetUniformLocation(this->VAO.program, "specCoefficient");
	this->shine_loc = glGetUniformLocation(this->VAO.program, "shine");	
	this->modelview_loc = glGetUniformLocation(this->VAO.program, "modelview");

	this->specCoefficient = 0.1f;
	this->shine = 0.5f;

	this->mass = 5.0f;

	this->velocity = gmtl::Vec3f(0, 0, 0);

}

void SceneObject::Draw(gmtl::Matrix44f viewMatrix, gmtl::Matrix44f projection)
{
	gmtl::Matrix44f rotation = gmtl::make<gmtl::Matrix44f>(this->rotation);
	gmtl::Matrix44f newMV = viewMatrix * this->translation * rotation;
	gmtl::Matrix44f render = projection * newMV * this->scale;


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->texture.textureHeight, this->texture.textureWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, this->texture.imageData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	
	glUniformMatrix4fv(this->VAO.matrix_loc, 1, GL_FALSE, &render[0][0]);


	glUniform3f(this->upVector_loc, viewMatrix[1][0], viewMatrix[1][1], viewMatrix[1][2]);
	glUniformMatrix4fv(this->modelview_loc, 1, GL_FALSE, &newMV[0][0]);
	glUniform1f(this->specCoefficient_loc, this->specCoefficient);
	glUniform1f(this->shine_loc, this->shine);

	// Draw the transformed cuboid
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
	glDrawElements(GL_TRIANGLES, this->VAO.index_data.size(), GL_UNSIGNED_SHORT, NULL);

	if (!this->children.empty())
	{
		for (std::vector<SceneObject *>::iterator it = this->children.begin();
			it < this->children.end();
			++it)
		{
			(*it)->Draw(newMV, projection);
		}		
	}
}

void SceneObject::SetTexture(Texture t)
{
	this->texture = t;
}

void SceneObject::AddTranslation(gmtl::Matrix44f t)
{
	this->translation *=  t;
}

void SceneObject::AddTranslation(gmtl::Vec3f t)
{
	this->translation *= gmtl::makeTrans<gmtl::Matrix44f>(t);
}

void SceneObject::AddRotation(gmtl::Quatf r)
{
	this->rotation = r * this->rotation;
}

void SceneObject::Move()
{
	gmtl::Vec3f currentPos, d;
	float angle;

	currentPos = this->GetPosition();
	this->AddTranslation(this->velocity);
	this->velocity += this->acceleration;

	d =  this->GetPosition() - currentPos;

	if (this->radius > 0)
	{
		angle = (gmtl::length(d) / this->radius);
		d = gmtl::makeNormal(gmtl::Vec3f(d[2], 0, -d[0]));
		this->AddRotation(gmtl::Quatf(d[0] * sin(angle / 2), d[1] * sin(angle / 2), d[2] * sin(angle / 2), cos(angle / 2)));
	}
}

gmtl::Vec3f SceneObject::GetPosition()
{
	return gmtl::Vec3f(float(this->translation[0][3]), float(this->translation[1][3]), float(this->translation[2][3]));
}