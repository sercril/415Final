#ifndef __BLENDOBJ_H__
#define __BLENDOBJ_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <gmtl\gmtl.h>
#include <gmtl\Matrix.h>

#include "Vertex.h"

using namespace std;

class BlendObj
{
public:
	BlendObj(string filename);
	~BlendObj();

	std::vector<Vertex> importedVerticies;
	std::vector<GLushort> importedIndexData;

private:
	void LoadVertex(std::vector<int> face);
	std::vector<int> SplitFace(string face);

	std::vector<gmtl::Vec3f> verts, norms;
	std::vector<gmtl::Point2f> uvs;
};

#endif __BLENDOBJ_H__