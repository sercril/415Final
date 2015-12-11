#define USE_PRIMITIVE_RESTART
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <fstream>
#include <stack>
#include <functional>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

#include "LoadShaders.h"
#include "SceneObject.h"

#include "Texture.h"

#pragma comment (lib, "glew32.lib")
#pragma warning (disable : 4996) // Windows ; consider instead replacing fopen with fopen_s

using namespace std;

#pragma region Structs and Enums

enum Eye
{
	LEFT = 0,
	RIGHT
};

struct Collision
{
	SceneObject* a;
	SceneObject* b;

	Collision(SceneObject* a, SceneObject* b)
	{
		this->a = a;
		this->b = b;
	}
};

#pragma endregion

#pragma region "Global Variables"

#define SCREEN_WIDTH 1920.0f
#define SCREEN_HEIGHT 1080.0f
#define ZERO_VECTOR gmtl::Vec3f(0,0,0);

int mouseX, mouseY,
mouseDeltaX, mouseDeltaY,
simStep;

bool hit, c_tableCenter, c_cueFollow, c_cue, legballdist, bounce, attract;

float azimuth, elevation, ballRadius, ballDiameter, cameraZFactor,
drag, restitutionBall, restitutionWall, hitScale, delta,
nearValue, farValue, topValue, bottomValue, leftValue, rightValue,
screenWidth, screenHeight, ipd, frustumScale, focalDepth;


GLuint cubeProgram, normalProgram, sphereProgram, renderTextureProgram,
framebuffer, depthbuffer, renderTexture, renderTexture_loc,
framebuffer_vertex_buffer, framebuffer_vertposition_loc, time_loc,
DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };

GLenum errCode;


GLfloat lightIntensity;

const GLubyte *errString;


gmtl::Matrix44f view, leftProjection, rightProjection, originalView,
				elevationRotation, azimuthRotation, cameraZ, viewRotation, cameraTrans;

SceneObject *attractLeg, *hitBall;

std::vector<SceneObject*> sceneGraph;
std::vector<Vertex> ballData;
std::vector<Collision> collsionList;
gmtl::Point3f lightPosition;

gmtl::Vec3f ballDelta, hitDirection;

static const GLfloat framebuffer_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
};

#pragma endregion

#pragma region Camera

float arcToDegrees(float arcLength)
{
	return ((arcLength * 360.0f) / (2.0f * M_PI));
}

float degreesToRadians(float deg)
{
	return (2.0f * M_PI *(deg / 360.0f));
}

void cameraRotate()
{
	elevationRotation.set(
		1, 0, 0, 0,
		0, cos(elevation * -1), (sin(elevation * -1) * -1), 0,
		0, sin(elevation * -1), cos(elevation * -1), 0,
		0, 0, 0, 1);

	azimuthRotation.set(
		cos(azimuth * -1), 0, sin(azimuth * -1), 0,
		0, 1, 0, 0,
		(sin(azimuth * -1) * -1), 0, cos(azimuth * -1), 0,
		0, 0, 0, 1);

	elevationRotation.setState(gmtl::Matrix44f::ORTHOGONAL);

	azimuthRotation.setState(gmtl::Matrix44f::ORTHOGONAL);
	
	if (c_tableCenter)
	{
		cameraTrans = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0, 0, 0));
	}
	else if (c_cueFollow)
	{
		cameraTrans = gmtl::makeTrans<gmtl::Matrix44f>(sceneGraph[0]->GetPosition());
	}
	
	viewRotation = azimuthRotation * elevationRotation;
	gmtl::invert(viewRotation);

	originalView = cameraTrans * azimuthRotation * elevationRotation * cameraZ;

	view = gmtl::invert(cameraTrans * azimuthRotation * elevationRotation * cameraZ);

	glutPostRedisplay();
}

#pragma endregion

#pragma region Helper Functions

int AlreadyCollided(SceneObject* a, SceneObject* b)
{
	int i = 0;
	for (std::vector<Collision>::iterator it = collsionList.begin(); it < collsionList.end(); ++it)
	{
		if ((a == it->a && b == it->b) || (a == it->b && b == it->a))
		{
			return i;
		}
		++i;
	}
	return -1;
}
bool IsWall(SceneObject* obj)
{
	switch (obj->type)
	{
	case FRONT_WALL:
	case BACK_WALL:
	case LEFT_WALL:
	case RIGHT_WALL:
		return true;
	}

	return false;
}

// TODO Move this to SceneObject.
Texture LoadTexture(char* filename)
{
	unsigned int textureWidth, textureHeight;
	unsigned char *imageData;

	LoadPPM(filename, &textureWidth, &textureHeight, &imageData, 1);
	return Texture(textureWidth, textureHeight, imageData);
}

// TODO Generalize this.
SceneObject* AddWall(int i, gmtl::Vec3f floorDimensions)
{
	SceneObject* wall = new SceneObject();
	float wallHeight = 50.0f;
	switch (i)
	{
		case 0:
			wall = new SceneObject("OBJs/cube.obj", floorDimensions[0] + (ballDiameter*2), wallHeight, ballDiameter, cubeProgram);
			wall->AddTranslation(gmtl::Vec3f(0.0f, wallHeight, floorDimensions[2] + ballDiameter));
			wall->type = BACK_WALL;
			break;

		case 1:
			wall = new SceneObject("OBJs/cube.obj", floorDimensions[0] + (ballDiameter * 2), wallHeight, ballDiameter, cubeProgram);
			wall->AddTranslation(gmtl::Vec3f(0.0f, wallHeight, -floorDimensions[2]  - ballDiameter));
			wall->type = FRONT_WALL;
			break;

		case 2:
			wall = new SceneObject("OBJs/cube.obj", ballDiameter, wallHeight, floorDimensions[2], cubeProgram);
			wall->AddTranslation(gmtl::Vec3f(floorDimensions[0] + ballDiameter, wallHeight, 0.0f));
			wall->type = RIGHT_WALL;
			break;

		case 3:
			wall = new SceneObject("OBJs/cube.obj", ballDiameter, wallHeight, floorDimensions[2], cubeProgram);
			wall->AddTranslation(gmtl::Vec3f(-floorDimensions[0] - ballDiameter, wallHeight, 0.0f));
			wall->type = LEFT_WALL;
			break;
	}

	wall->parent = NULL;
	wall->children.clear();
	wall->SetTexture(LoadTexture("textures/dirt.ppm"));
	wall->mass = FLT_MAX;
	wall->SetLight(lightPosition, lightIntensity);
	return wall;
}

void buildTable(gmtl::Vec3f floorDimensions)
{
	float legHeight = 20.0f,
		legY = (floorDimensions[1] * 2.0f) + legHeight, //Y
		tableWidth = 25.0f,  //X
		tableLength = 25.0f; //Z

	SceneObject* tableTop = new SceneObject("OBJs/cube.obj", tableWidth, floorDimensions[1], tableLength, cubeProgram);
	tableTop->AddTranslation(gmtl::Vec3f(0.0f, legY+floorDimensions[1]+legHeight, 0.0f));
	tableTop->type = TABLETOP;
	tableTop->parent = NULL;
	tableTop->SetTexture(LoadTexture("textures/dirt.ppm"));
	tableTop->children.clear();
	tableTop->SetLight(lightPosition, lightIntensity);

	sceneGraph.push_back(tableTop);

	SceneObject* brleg = new SceneObject("OBJs/cylinder.obj", ballRadius, legHeight, sphereProgram);
	brleg->AddTranslation(gmtl::Vec3f(tableWidth - ballRadius, legY, -(tableWidth - ballRadius)));
	brleg->type = LEG;
	brleg->parent = NULL;
	brleg->SetTexture(LoadTexture("textures/dirt.ppm"));
	brleg->children.clear();
	brleg->mass = FLT_MAX;
	brleg->velocity = ZERO_VECTOR;
	brleg->SetLight(lightPosition, lightIntensity);

	sceneGraph.push_back(brleg);

	SceneObject* blleg = new SceneObject("OBJs/cylinder.obj", ballRadius, legHeight, sphereProgram);
	blleg->AddTranslation(gmtl::Vec3f(-(tableWidth - ballRadius), legY, -(tableWidth - ballRadius)));
	blleg->type = LEG;
	blleg->parent = NULL;
	blleg->SetTexture(LoadTexture("textures/dirt.ppm"));
	blleg->children.clear();
	blleg->mass = FLT_MAX;
	blleg->velocity = ZERO_VECTOR;
	blleg->SetLight(lightPosition, lightIntensity);

	sceneGraph.push_back(blleg);

	SceneObject* frleg = new SceneObject("OBJs/cylinder.obj", ballRadius, legHeight, sphereProgram);
	frleg->AddTranslation(gmtl::Vec3f(tableWidth - ballRadius, legY, tableWidth - ballRadius));
	frleg->type = LEG;
	frleg->parent = NULL;
	frleg->SetTexture(LoadTexture("textures/dirt.ppm"));
	frleg->children.clear();
	frleg->mass = FLT_MAX;
	frleg->velocity = ZERO_VECTOR;
	frleg->SetLight(lightPosition, lightIntensity);

	attractLeg = frleg;
	sceneGraph.push_back(frleg);	

	SceneObject* flleg = new SceneObject("OBJs/cylinder.obj", ballRadius, legHeight, sphereProgram);
	flleg->AddTranslation(gmtl::Vec3f(-(tableWidth - ballRadius), legY, tableWidth - ballRadius));
	flleg->type = LEG;
	flleg->parent = NULL;
	flleg->SetTexture(LoadTexture("textures/dirt.ppm"));
	flleg->children.clear();
	flleg->mass = FLT_MAX;
	flleg->velocity = ZERO_VECTOR;
	flleg->SetLight(lightPosition, lightIntensity);

	sceneGraph.push_back(flleg);


}

void buildGraph()
{
	
	SceneObject* ball = new SceneObject("OBJs/SphereFull.txt", ballRadius, normalProgram);
	SceneObject* ball2 = new SceneObject("OBJs/smoothSphere2.obj", ballRadius, sphereProgram);
	SceneObject* ball3 = new SceneObject("OBJs/smoothSphere.obj", ballRadius, sphereProgram);
	gmtl::Vec3f floorDimensions = gmtl::Vec3f(150.0f, 5.0f, 150.0f);
	SceneObject* floor = new SceneObject("OBJs/cube.obj", floorDimensions, cubeProgram);
	gmtl::Matrix44f initialTranslation;
	gmtl::Quatf initialRotation;

		
	//Ball 1
	ball->type = BALL;
	ball->parent = NULL; 
	ball->children.clear();

	initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, floorDimensions[1] + ballDiameter+1.0f, 100.0f));
	initialTranslation.setState(gmtl::Matrix44f::TRANS);
	ball->AddTranslation(initialTranslation);
	ball->SetTexture(LoadTexture("textures/Berry_Diffuse_square.ppm"));
	ball->SetNormalMap(LoadTexture("textures/Berry_Normal_square.ppm"));
	ball->SetLight(lightPosition, lightIntensity);
	//ball->velocity = ZERO_VECTOR;
	ball->acceleration = ZERO_VECTOR;

	sceneGraph.push_back(ball);

	//Ball 2
	ball2->type = BALL;
	ball2->parent = NULL;
	ball2->children.clear();

	initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(75.0f, floorDimensions[1] + ballDiameter + 1.0f, 100.0f));
	initialTranslation.setState(gmtl::Matrix44f::TRANS);
	ball2->AddTranslation(initialTranslation);
	ball2->SetTexture(LoadTexture("textures/earth_square_flipped.ppm"));
	ball2->SetLight(lightPosition, lightIntensity);
	//ball->velocity = ZERO_VECTOR;
	ball2->acceleration = ZERO_VECTOR;

	sceneGraph.push_back(ball2);
	
	//Floor
	floor->type = FLOOR;
	floor->parent = NULL;
	floor->children.clear();
	initialTranslation = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f,floorDimensions[1],0.0f));
	initialTranslation.setState(gmtl::Matrix44f::TRANS);
	floor->AddTranslation(initialTranslation);
	floor->SetTexture(LoadTexture("textures/carpet.ppm"));
	floor->SetLight(lightPosition, lightIntensity);

	sceneGraph.push_back(floor);

	for (int i = 0; i < 4; ++i)
	{
		sceneGraph.push_back(AddWall(i, floorDimensions));
	}

	buildTable(floorDimensions);

}


bool IsCollided(SceneObject* obj1, SceneObject* obj2)
{
	gmtl::Vec3f posDiff, legPos, ballPos;
	float collisionDiff;

	posDiff = obj2->GetPosition() - obj1->GetPosition();

	if (IsWall(obj1) && obj2->type == BALL)
	{
		switch (obj1->type)
		{
			case FRONT_WALL:
			case BACK_WALL:
				collisionDiff = obj1->depth + obj2->radius;
				if (abs(posDiff[2]) < collisionDiff)
				{
					return true;
				}
				break;

			case LEFT_WALL:
			case RIGHT_WALL:
				collisionDiff = obj1->length + obj2->radius;
				if (abs(posDiff[0]) < collisionDiff)
				{
					return true;
				}
				break;
		}
		
	}
	else if (obj1->type == BALL && IsWall(obj2))
	{
		switch (obj2->type)
		{
		case FRONT_WALL:
		case BACK_WALL:
			collisionDiff = obj2->depth + obj1->radius;
			if (abs(posDiff[2]) < collisionDiff)
			{
				return true;
			}
			break;

		case LEFT_WALL:
		case RIGHT_WALL:
			collisionDiff = obj2->length + obj1->radius;
			if (abs(posDiff[0]) < collisionDiff)
			{
				return true;
			}
			break;
		}

	}
	else if (obj1->type == BALL && obj2->type == BALL)
	{		
		if (gmtl::length(posDiff) < obj1->radius + obj2->radius)
		{
			return true;
		}
	}
	else if (obj1->type == BALL && obj2->type == LEG)
	{

		legPos = obj2->GetPosition();
		ballPos = obj1->GetPosition();

		posDiff = gmtl::Vec3f(legPos[0], ballPos[1], legPos[2]) - ballPos;

		if (legballdist == true)
		{
			cout << gmtl::length(posDiff) << endl;
			legballdist = false;
		}
		if (gmtl::length(posDiff) < obj1->radius + obj2->radius)
		{
			return true;
		}
	}

	return false;
}

// TODO Combine Traverse and Render somehow

void HandleCollisions()
{
	SceneObject* checkObj;
	gmtl::Vec3f collisionNormal, normalRelativeVelocity;
	int cIndex;

	for (std::vector<SceneObject*>::iterator it = sceneGraph.begin(); it < sceneGraph.end(); ++it)
	{
		checkObj = (*it);
		for (std::vector<SceneObject*>::iterator innerIt = sceneGraph.begin(); innerIt < sceneGraph.end(); ++innerIt)
		{
			if (checkObj != (*innerIt) && !IsWall(checkObj))
			{
				cIndex = AlreadyCollided(checkObj, (*innerIt));
				
				
				if (IsCollided(checkObj, (*innerIt)))
				{				
					collsionList.push_back(Collision(checkObj, (*innerIt)));
					
					collisionNormal = checkObj->GetPosition() - (*innerIt)->GetPosition();
					collisionNormal = gmtl::makeNormal(collisionNormal);
					collisionNormal[1] = 0.0f;

					//cout << gmtl::dot((checkObj->velocity - (*innerIt)->velocity), collisionNormal) << endl;
					if (gmtl::dot((checkObj->velocity - (*innerIt)->velocity), collisionNormal) <= 1.0f)
					{
						switch ((*innerIt)->type)
						{
						case FRONT_WALL:
						case BACK_WALL:
							checkObj->velocity = restitutionWall * gmtl::Vec3f(checkObj->velocity[0], checkObj->velocity[1], -checkObj->velocity[2]);
							break;

						case LEFT_WALL:
						case RIGHT_WALL:
							checkObj->velocity = restitutionWall * gmtl::Vec3f(-checkObj->velocity[0], checkObj->velocity[1], checkObj->velocity[2]);
							break;

						case LEG:
							normalRelativeVelocity = gmtl::dot((checkObj->velocity - (*innerIt)->velocity), collisionNormal)*collisionNormal;

							checkObj->velocity = (checkObj->velocity - ((1 + restitutionBall)*normalRelativeVelocity)) / (1 + (checkObj->mass / (*innerIt)->mass));

							if (bounce)
							{
								checkObj->velocity *= -1.2f;
							}
							

							(*innerIt)->velocity = ((*innerIt)->velocity + ((1 + restitutionBall)*normalRelativeVelocity)) / (1 + ((*innerIt)->mass / checkObj->mass));
							break;
						case BALL:
							normalRelativeVelocity = gmtl::dot((checkObj->velocity - (*innerIt)->velocity), collisionNormal)*collisionNormal;

							checkObj->velocity = (checkObj->velocity - ((1 + restitutionBall)*normalRelativeVelocity)) / (1 + (checkObj->mass / (*innerIt)->mass));

							(*innerIt)->velocity = ((*innerIt)->velocity + ((1 + restitutionBall)*normalRelativeVelocity)) / (1 + ((*innerIt)->mass / checkObj->mass));
							break;
						}
					}
										
				}
				else if (!IsCollided(checkObj, (*innerIt)) && cIndex > -1)
				{
					collsionList.erase(collsionList.begin()+cIndex);
				}
			}
		}

	}
}
void ApplyForces()
{
	gmtl::Vec3f dragForce, hitForce, attractVec, attractForce;
	

	for (std::vector<SceneObject*>::iterator it = sceneGraph.begin(); it < sceneGraph.end(); ++it)
	{	
		if ((*it)->type == BALL)
		{
			if (attract)
			{
				attractVec = attractLeg->GetPosition() - (*it)->GetPosition();
				attractForce = gmtl::makeNormal(gmtl::Vec3f(attractVec[0], 0.0f, attractVec[2])) * 2.0f;
			}
			else
			{
				attractForce = ZERO_VECTOR;
			}

			hitForce = hitDirection * hitScale;

			dragForce = -drag * (*it)->velocity;

			if (hit && (*it) == hitBall)
			{
				(*it)->acceleration = (1 / (*it)->mass) * (dragForce + hitForce + attractForce);
			}
			else
			{
				(*it)->acceleration = (1 / (*it)->mass) * (dragForce + attractForce);
			}
			(*it)->velocity *= delta;
			(*it)->acceleration *= delta;

			(*it)->Move();
		}
		
	}
}
void ProcessHit(gmtl::Rayf ray)
{
	gmtl::Spheref ball;
	float t1, t2, smallestT = FLT_MAX;
	int numHits;
	hitBall = NULL;
	for (std::vector<SceneObject*>::iterator it = sceneGraph.begin(); it < sceneGraph.end(); ++it)
	{
		if ((*it)->type == BALL)
		{
			ball = gmtl::Spheref((*it)->GetPosition(), (*it)->radius);
			
			if (gmtl::intersect(ball, ray, numHits, t1, t2))
			{

				if (t1 < smallestT)
				{
					smallestT = t1;
					hitBall = (*it);
				}
				else if (t2 < smallestT)
				{
					smallestT = t2;
					hitBall = (*it);
				}
			}
		}
	}

	if (hitBall != NULL)
	{
		hit = true;
		hitDirection = gmtl::makeNormal(ray.getDir());
		hitDirection = gmtl::Vec3f(hitDirection[0], 0.0f, hitDirection[2]);
	}

}
void renderGraph(std::vector<SceneObject*> graph, gmtl::Matrix44f mv, int eye)
{

	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, screenWidth, screenHeight);
	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(!graph.empty())
	{
		for (int i = 0; i < graph.size(); ++i)
		{
			glUseProgram(graph[i]->VAO.program);

			glBindVertexArray(graph[i]->VAO.vertexArray);		

			if (eye == LEFT)
			{
				graph[i]->Draw(mv * gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(ipd, 0.0f, 0.0f)), leftProjection);
			}
			else
			{
				graph[i]->Draw(mv  * gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(-ipd, 0.0f, 0.0f)), rightProjection);
			}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(renderTextureProgram);
	glBindTexture(GL_TEXTURE2, renderTexture);
	glUniform1i(renderTexture_loc, 2);

	glEnableVertexAttribArray(framebuffer_vertposition_loc);
	glBindBuffer(GL_ARRAY_BUFFER, framebuffer_vertex_buffer);
	glVertexAttribPointer(framebuffer_vertposition_loc,
		3, // Size
		GL_FLOAT, // Type
		GL_FALSE, // Is normalized
		0, ((void*)0));
		
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);

	//glDisableVertexAttribArray(framebuffer_vertposition_loc);
	
	
	return;
}
void UpdateLightPosition(gmtl::Vec3f translation)
{
	lightPosition += translation;
	cout << lightPosition << endl;

	for (std::vector<SceneObject*>::iterator it = sceneGraph.begin(); it < sceneGraph.end(); ++it)
	{
		(*it)->SetLight(lightPosition, lightIntensity);
	}

}
#pragma endregion

#pragma region "Input"

# pragma region "Mouse Input"

void mouse(int button, int state, int x, int y)
{
	gmtl::Point3f leftP, rightP, eyePos;
	gmtl::Vec3f ray;

	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		mouseX = x;
		mouseY = y;

		leftP = gmtl::Point3f(((leftValue + ipd) * frustumScale) + ((mouseX + 0.5f) / screenWidth)*(((rightValue + ipd)*frustumScale) - ((leftValue + ipd) * frustumScale)), (topValue*frustumScale) - ((mouseY + 0.5f) / screenHeight)*((topValue*frustumScale) - (bottomValue*frustumScale)), -nearValue);
		
		originalView = view * gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(ipd, 0.0f, 0.0f));
		gmtl::invert(originalView);
		leftP = originalView * leftP;

		eyePos = gmtl::Point3f(originalView[0][3], originalView[1][3], originalView[2][3]);

		ray = leftP - eyePos;
		ProcessHit(gmtl::Rayf(eyePos, ray));

		if (!hit)
		{

			rightP = gmtl::Point3f(((leftValue - ipd) * frustumScale) + ((mouseX + 0.5f) / screenWidth)*(((rightValue - ipd)*frustumScale) - ((leftValue - ipd) * frustumScale)), (topValue*frustumScale) - ((mouseY + 0.5f) / screenHeight)*((topValue*frustumScale) - (bottomValue*frustumScale)), -nearValue);

			originalView = view * gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(-ipd, 0.0f, 0.0f));
			gmtl::invert(originalView);
			rightP = originalView * rightP;

			eyePos = gmtl::Point3f(originalView[0][3], originalView[1][3], originalView[2][3]);

			ray = rightP - eyePos;
			ProcessHit(gmtl::Rayf(eyePos, ray));
		}
	}

}

void mouseMotion(int x, int y)
{

	mouseDeltaX = x - mouseX;
	mouseDeltaY = y - mouseY;


	elevation += degreesToRadians(arcToDegrees(mouseDeltaY*1.0f)) / (screenHeight/2);
	azimuth += degreesToRadians(arcToDegrees(mouseDeltaX*1.0f)) / (screenWidth / 2);

	cameraRotate();

	mouseX = x;
	mouseY = y;

}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir < 0)
	{
		cameraZFactor += 10.f;
		cameraZ = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, 0.0f, cameraZFactor));
	}
	else
	{
		cameraZFactor -= 10.f;
		cameraZ = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, 0.0f, cameraZFactor));		
	}

	cameraZ.setState(gmtl::Matrix44f::TRANS);
	cameraRotate();
}

# pragma endregion

#pragma region "Keyboard Input"

void keyboard(unsigned char key, int x, int y)
{
	switch (key) 
	{
		case 'f':
			c_cueFollow = true;
			c_cue = c_tableCenter = false;
			break;

		case 'k':
			drag += 0.01f;
			break;

		case 'K':
			drag = max(0.0f,drag - 0.01f);
			break;

		case 'm':
			sceneGraph[0]->mass += 1.0f;
			break;

		case 'M':
			sceneGraph[0]->mass = max(0.0f, sceneGraph[0]->mass - 1.0f);
			break;

		case 'w':
			UpdateLightPosition(gmtl::Vec3f(0.0f, 5.0f, 0.0f));
			break;
		case 's':
			UpdateLightPosition(gmtl::Vec3f(0.0f, -5.0f, 0.0f));
			break;
		case 'a':
			UpdateLightPosition(gmtl::Vec3f(-5.0f, 0.0f, 0.0f));
			break;
		case 'd':
			UpdateLightPosition(gmtl::Vec3f(5.0f, 0.0f, 0.0f));
			break;
		case 'e':
			UpdateLightPosition(gmtl::Vec3f(0.0f, 0.0f, 5.0f));
			break;
		case 'q':
			UpdateLightPosition(gmtl::Vec3f(0.0f, 0.0f, -5.0f));
			break;
			
		/*case 'b':
			bounce = (bounce ? false:true);
			break;

		case 'a':
			attract = (attract ? false : true);
			break;*/

		

		case 033 /* Escape key */:
			exit(EXIT_SUCCESS);
			break;
	}
	
	glutPostRedisplay();
}

#pragma endregion

#pragma endregion

#pragma region "GLUT Functions"


void idle()
{
	ballDelta = gmtl::Vec3f(0, 0, 0);

	HandleCollisions();

	for (int i = 0; i < simStep; ++i)
	{
		ApplyForces();
	}
	
	hit = false;
	
	cameraRotate();

	glutPostRedisplay();
}

void init()
{

	elevation = azimuth = 0;
	ballRadius = 4.0f;
	ballDiameter = ballRadius * 2.0f;
	hit = c_tableCenter = c_cueFollow = c_cue = bounce = attract = false;
	hitScale = 3.0f;
	drag = 0.1f;
	restitutionBall =  1.0f;
	restitutionWall = 1.0f;
	simStep = 1;

	delta = 1.0f;
	// Enable depth test (visible surface determination)
	glEnable(GL_DEPTH_TEST);

	// OpenGL background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Load/compile/link shaders and set to use for rendering
	ShaderInfo cubeShaders[] = { { GL_VERTEX_SHADER, "Cube.vert" },
	{ GL_FRAGMENT_SHADER, "Cube.frag" },
	{ GL_NONE, NULL } };

	cubeProgram = LoadShaders(cubeShaders);

	// Load/compile/link shaders and set to use for rendering
	ShaderInfo normalMappedShaders[] = { { GL_VERTEX_SHADER, "Normal.vert" },
	{ GL_FRAGMENT_SHADER, "Normal.frag" },
	{ GL_NONE, NULL } };

	normalProgram = LoadShaders(normalMappedShaders);

	// Load/compile/link shaders and set to use for rendering
	ShaderInfo sphereShaders[] = { { GL_VERTEX_SHADER, "Sphere.vert" },
	{ GL_FRAGMENT_SHADER, "Sphere.frag" },
	{ GL_NONE, NULL } };

	sphereProgram = LoadShaders(sphereShaders);
	
	//Render to Texture code
	// Load/compile/link shaders and set to use for rendering
	ShaderInfo renderTextureShaders[] = { { GL_VERTEX_SHADER, "RenderTex.vert" },
	{ GL_FRAGMENT_SHADER, "RenderTex.frag" },
	{ GL_NONE, NULL } };

	renderTextureProgram = LoadShaders(renderTextureShaders);

	renderTexture_loc = glGetUniformLocation(renderTextureProgram, "renderTexture");
	framebuffer_vertposition_loc = glGetAttribLocation(renderTextureProgram, "vertexPosition");

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenRenderbuffers(1, &depthbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);

	
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Frame buffer problem" << endl;
		return;
	}

	glGenBuffers(1, &framebuffer_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, framebuffer_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(framebuffer_vertex_buffer_data), framebuffer_vertex_buffer_data, GL_DYNAMIC_DRAW);

	gmtl::identity(view);
	gmtl::identity(viewRotation);
	gmtl::identity(cameraTrans);

	lightPosition.set(-10.0f, 65.0f, 0.0f);
	lightIntensity = 0.75f;

	/*nearValue = 1.0f;
	farValue = 1000.0f;
	topValue = screenHeight / screenWidth;
	bottomValue = topValue * -1.0f;
	rightValue = 1.0f;
	leftValue = -1.0f;*/

	ipd = 0.07f / 2;
	nearValue = 1.0f;
	farValue = 1000.0f;
	topValue = 0.68f;
	bottomValue = -1.6f;
	rightValue = 2.05f;
	leftValue = -2.05f;
	focalDepth = 2.5f;
	frustumScale = nearValue / focalDepth;
	

	leftProjection.set(
		((2.0f * nearValue) / (((rightValue + ipd)*frustumScale) - ((leftValue + ipd) * frustumScale))), 0.0f, ((((rightValue + ipd)*frustumScale) + ((leftValue + ipd) * frustumScale)) / (((rightValue + ipd)*frustumScale) - ((leftValue + ipd) * frustumScale))), 0.0f,
		0.0f, ((2.0f * nearValue) / ((topValue*frustumScale) - (bottomValue*frustumScale))), (((topValue*frustumScale) + (bottomValue*frustumScale)) / ((topValue*frustumScale) - (bottomValue*frustumScale))), 0.0f,
		0.0f, 0.0f, ((-1.0f * (farValue + nearValue)) / (farValue - nearValue)), ((-2.0f*farValue*nearValue)/(farValue-nearValue)),
		0.0f,0.0f,-1.0f,0.0f		
		);

	rightProjection.set(
		((2.0f * nearValue) / (((rightValue - ipd)*frustumScale) - ((leftValue - ipd) * frustumScale))), 0.0f, ((((rightValue - ipd)*frustumScale) + ((leftValue - ipd) * frustumScale)) / (((rightValue - ipd)*frustumScale) - ((leftValue - ipd) * frustumScale))), 0.0f,
		0.0f, ((2.0f * nearValue) / ((topValue*frustumScale) - (bottomValue*frustumScale))), (((topValue*frustumScale) + (bottomValue*frustumScale)) / ((topValue*frustumScale) - (bottomValue*frustumScale))), 0.0f,
		0.0f, 0.0f, ((-1.0f * (farValue + nearValue)) / (farValue - nearValue)), ((-2.0f*farValue*nearValue) / (farValue - nearValue)),
		0.0f, 0.0f, -1.0f, 0.0f
		);

	cameraZFactor = 350.0f;

	cameraZ = gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f,0.0f,cameraZFactor));
	cameraZ.setState(gmtl::Matrix44f::TRANS);
	
	elevation = degreesToRadians(30.0f);
	azimuth = 0.0f;
	
	cameraRotate();

	buildGraph();
	
}
void display()
{

	glDrawBuffer(GL_BACK_LEFT);
	renderGraph(sceneGraph, view, LEFT);

	glDrawBuffer(GL_BACK_RIGHT);
	renderGraph(sceneGraph, view, RIGHT);
	//Ask GL to execute the commands from the buffer
	glutSwapBuffers();	// *** if you are using GLUT_DOUBLE, use glutSwapBuffers() instead 

	//Check for any errors and print the error string.
	//NOTE: The string returned by gluErrorString must not be altered.
	if ((errCode = glGetError()) != GL_NO_ERROR)
	{
		errString = gluErrorString(errCode);
		cout << "OpengGL Error: " << errString << endl;
	}
}
void reshape(int width, int height)
{
	screenWidth = float(width);
	screenHeight = float(height);
	glViewport(0,0,width, height);
}
#pragma endregion

int main(int argc, char** argv)
{
	// For more details about the glut calls, 
	// refer to the OpenGL/freeglut Introduction handout.

	//Initialize the freeglut library
	glutInit(&argc, argv);

	//Specify the display mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO);
	screenWidth = SCREEN_WIDTH;
	screenHeight = SCREEN_HEIGHT;
	//Set the window size/dimensions
	glutInitWindowSize(screenWidth, screenHeight);

	// Specify OpenGL version and core profile.
	// We use 3.3 in thie class, not supported by very old cards
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("415/515 DEMO");
	glutFullScreen();

	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	glewExperimental = GL_TRUE;

	if (glewInit())
		exit(EXIT_FAILURE);

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutMouseWheelFunc(mouseWheel);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	

	//Transfer the control to glut processing loop.
	glutMainLoop();

	return 0;
}