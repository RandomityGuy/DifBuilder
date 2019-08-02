#pragma once
#include <stdlib.h>
#include <glm/glm.hpp>
#include <vector>

class Plane
{
public:
	Plane()
	{

	}
	~Plane()
	{

	}

	Plane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		glm::vec3 v1 = a - b;
		glm::vec3 v2 = c - b;
		glm::vec3 res = glm::cross(v1, v2);

		this->normal = res;
		float w = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		this->normal.x /= w;
		this->normal.y /= w;
		this->normal.z /= w;

		//normal = glm::normalize(normal);

		d = -(glm::dot(b, normal));
		pt = b;
	}

	Plane(glm::vec3 a, float d)
	{
		this->normal = a;
		this->d = d;
	}

	Plane(glm::vec3 point, glm::vec3 normal)
	{
		this->normal = normal;
		float w = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		this->normal.x /= w;
		this->normal.y /= w;
		this->normal.z /= w;
		//normal = glm::normalize(normal);

		d = -(glm::dot(point, normal));
		pt = point;
	}

	float DistanceToPoint(glm::vec3 point)
	{
		return (glm::dot(this->normal, point)) + this->d;
	}

	glm::vec3 pt;
	glm::vec3 normal;
	float d;
};

struct POLYGON;

struct LEAF
{
	std::vector<POLYGON> polygons;
};

struct NODE
{
	bool IsLeaf = false;
	Plane Plane;
	NODE* Front = NULL;
	NODE* Back = NULL;
	LEAF* FrontLeaf = NULL;
	LEAF* BackLeaf = NULL;
};


struct Vertex
{
	glm::vec3 p;
	glm::vec2 uv;
};

struct POLYGON
{
	Vertex* VertexList;
	glm::vec3 Normal;
	int NumberOfVertices;
	int NumberOfIndices;
	int* Indices;
	POLYGON* Next;
	Plane plane;
	bool BeenUsedAsSplitter;
	long TextureIndex;
	LEAF* leaf = NULL;
	bool IsUsed = false;
};

inline POLYGON* GetNext(POLYGON* p)
{
	return p->Next;
	if (p->Next == NULL)
		return NULL;
	else
	{
		if (p->Next->IsUsed)
			return GetNext(p->Next);
		else
			return p->Next;
	}

}

inline int GetCount(POLYGON* p)
{
	int count = 0;;
	POLYGON* temp;
	temp = p;
	while (temp != NULL)
	{
		count++;
		temp = temp->Next;
	}
	return count;
}

void InitPolygons(POLYGON polyList);
void BuildBspTree(NODE& node, POLYGON* PolyList, bool fastSplit);
Plane* SelectBestSplitter(POLYGON *PolyList);
Plane* SelectBestSplitter_Fast(POLYGON *PolyList);
int ClassifyPoly(Plane *Plane, POLYGON * Poly);
int ClassifyPoint(glm::vec3 *pos, Plane Plane);
//void SplitPolygon(POLYGON *Poly, Plane *Plane, POLYGON *FrontSplit, POLYGON *BackSplit);
enum
{
	CP_FRONT,
	CP_BACK,
	CP_ONPLANE,
	CP_SPANNING
};
//bool Get_Intersect(glm::vec3 *linestart, glm::vec3 *lineend, glm::vec3 *vertex, glm::vec3 *normal, glm::vec3 & intersection, float &percentage);
//void DeletePolygon(POLYGON *Poly);
void GatherBrushes(NODE node, std::vector<POLYGON>* list);