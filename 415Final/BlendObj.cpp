#include "BlendObj.h"
using namespace std;

BlendObj::BlendObj(string filename)
{
	ifstream fp;
	
	string flag;

	fp.open(filename, ios_base::in);

	if (fp)
	{

		for (std::string line; std::getline(fp, line);)
		{
			std::istringstream in(line);
			gmtl::Vec3f newVec3;
			gmtl::Point2f newUV;
			
			string vert;

			in >> flag;

			if (flag == "v")
			{
				in >> newVec3[0] >> newVec3[1] >> newVec3[2];
				this->verts.push_back(newVec3);
			}
			else if (flag == "vt")
			{
				in >> newUV[0] >> newUV[1];
				this->uvs.push_back(newUV);
			}
			else if (flag == "vn")
			{
				in >> newVec3[0] >> newVec3[1] >> newVec3[2];
				this->norms.push_back(newVec3);
			}
			else if (flag == "f")
			{
				in >> vert;
				this->LoadVertex(this->SplitFace(vert));
				in >> vert;
				this->LoadVertex(this->SplitFace(vert));
				in >> vert;
				this->LoadVertex(this->SplitFace(vert));
			}

		}

		fp.close();
	}
}

BlendObj::~BlendObj()
{

}

void BlendObj::LoadVertex(std::vector<int> face)
{
	gmtl::Vec3f position, normal;
	gmtl::Point2f uv;

	position = this->verts[face[0]-1];
	uv = this->uvs[face[1]-1];
	normal = this->norms[face[2]-1];

	this->importedVerticies.push_back(Vertex(position, normal, uv[0], uv[1]));

	this->importedIndexData.push_back(this->importedVerticies.size()-1);
}

std::vector<int> BlendObj::SplitFace(string face)
{
	stringstream faceStream(face);
	string vertPart;
	std::vector<int> faceElems;

	while (getline(faceStream, vertPart, '/')) 
	{
		faceElems.push_back(stoi(vertPart));
	}

	return faceElems;
}