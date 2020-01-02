#include "DIFBuilder/BSPLib.h"
#define GLM_FORCE_INTRINSICS
#include <glm\glm.hpp>

/* http://www.cs.utah.edu/~jsnider/SeniorProj/BSP/default.htm
 * is a very good tutorial on how to BSP tree
 * half the code here is taken from there
*/


POLYGON PolygonList;

void InitPolygons(POLYGON polyList)
{
	PolygonList = polyList;
}

Plane* SelectBestSplitter(POLYGON *PolyList)
{
	POLYGON* Splitter = PolyList;
	POLYGON* CurrentPoly = NULL;
	unsigned long BestScore = 1000000;
	POLYGON * SelectedPoly = NULL;

	while (Splitter != NULL)
	{
		if (Splitter->BeenUsedAsSplitter != true)
		{
			Plane SplittersPlane = Plane(Splitter->VertexList[0].p, Splitter->Normal);
			CurrentPoly = PolyList;
			unsigned long score, splits, backfaces, frontfaces;
			score = splits = backfaces = frontfaces = 0;

			while (CurrentPoly != NULL)
			{
				int result = ClassifyPoly(&SplittersPlane, CurrentPoly);
				switch (result)
				{
				case CP_ONPLANE:
					break;
				case CP_FRONT:
					frontfaces++;
					break;
				case CP_BACK:
					backfaces++;
					break;
				case CP_SPANNING:
					splits++;
					break;
				default:
					break;
				}// switch

				CurrentPoly = CurrentPoly->Next;
			}// end while current poly

			score = abs((long)(frontfaces - backfaces)) + (splits * 3);
			if (score < BestScore)
			{
				BestScore = score;
				SelectedPoly = Splitter;
			}

		}// end if this splitter has not been used yet
		Splitter = Splitter->Next;
	}// end while splitter != null 

	if (SelectedPoly == NULL) return NULL;
	SelectedPoly->BeenUsedAsSplitter = true; 
	return new Plane(SelectedPoly->VertexList[0].p, SelectedPoly->Normal);
} // End Function

Plane* SelectBestSplitter_Fast(POLYGON *PolyList)
{
	float minx = -100000, miny = -100000, minz = -100000, maxx = 100000, maxy = 100000, maxz = 100000;

	POLYGON* curpoly;
	curpoly = PolyList;
	while (curpoly != NULL)
	{
		for (int i = 0; i < curpoly->NumberOfVertices; i++)
		{
			auto v = curpoly->VertexList[i];
			if (v.p.x < minx) minx = v.p.x;
			if (v.p.y < miny) miny = v.p.y;
			if (v.p.z < minz) minz = v.p.z;
			if (v.p.x > maxx) maxx = v.p.x;
			if (v.p.y > maxy) maxy = v.p.y;
			if (v.p.z > maxz) maxz = v.p.z;
		}		

		curpoly = curpoly->Next;
	}

	glm::vec3 min = glm::vec3(minx, miny, minz);
	glm::vec3 max = glm::vec3(maxx, maxy, maxz);

	glm::vec3 norm = glm::vec3(-1, 0, 0);

	return new Plane(glm::vec3((minx+maxx)/2,(miny+maxy)/2,(minz+maxz)/2), norm);

}

void BuildBspTree(NODE& node, POLYGON* PolyList,bool fastSplit)
{
	std::vector<POLYGON*> FrontList;
	std::vector<POLYGON*> BackList;
	Plane* Splitter;
	if (!fastSplit)
		Splitter = SelectBestSplitter(PolyList);
	else
		Splitter = SelectBestSplitter_Fast(PolyList);

	if (Splitter != NULL)
		node.Plane = *Splitter;
	POLYGON* CurPoly;
	CurPoly = PolyList;
	if (Splitter != NULL)
		while (CurPoly != NULL)
	{
		switch (ClassifyPoly(&node.Plane,CurPoly))
		{
		case CP_FRONT:
			FrontList.push_back(CurPoly);
			break;

		case CP_BACK:
			BackList.push_back(CurPoly);
			break;

		case CP_SPANNING:
			FrontList.push_back(CurPoly);
			//BackList.push_back(CurPoly);

		case CP_ONPLANE:
			FrontList.push_back(CurPoly);
		}

		CurPoly = CurPoly->Next;
	}
	else
	{
		while (CurPoly != NULL)
		{
			FrontList.push_back(CurPoly);
			CurPoly = CurPoly->Next;
		}
	}

	if (FrontList.size() == 0 && BackList.size() != 0) //All the leaves should be in the frontlist, if frontlist is empty and backlist isnt empty, then we simply swap em and flip the plane
	{
		node.Plane = Plane(node.Plane.pt, (float)-1 * node.Plane.normal);

		std::vector<POLYGON*> temp;
		temp = FrontList;
		FrontList = BackList;
		BackList = temp;
	}


	if (FrontList.size() != 0)
	{
		if (BackList.size() == 0)
		{
			LEAF* leaf = new LEAF();
			for (int i = 0; i < FrontList.size(); i++)
			{
				leaf->polygons.push_back(*FrontList[i]);
			}
			for (int i = 0; i < leaf->polygons.size(); i++)
				leaf->polygons[i].leaf = leaf;


			node.IsLeaf = true;
			node.FrontLeaf = leaf;
		}
		else
		{
			POLYGON* FrontPolys = FrontList[0];
			POLYGON* PolyPtr;
			PolyPtr = FrontPolys;

			for (int i = 1; i < FrontList.size(); i++)
			{
				PolyPtr->Next = FrontList[i];
				PolyPtr = PolyPtr->Next;
			}

			PolyPtr->Next = NULL;

			node.Front = new NODE();
			BuildBspTree(*node.Front, FrontPolys,fastSplit);
		}
	}

	if (BackList.size() != 0)
	{
		POLYGON* BackPolys = BackList[0];
		POLYGON* PolyPtr;
		PolyPtr = BackPolys;

		for (int i = 1; i < BackList.size(); i++)
		{
			PolyPtr->Next = BackList[i];
			PolyPtr = PolyPtr->Next;
		}

		PolyPtr->Next = NULL;

		node.Back = new NODE();
		BuildBspTree(*node.Back, BackPolys,fastSplit);
	}
	else
	{
		node.Back = NULL;
	}
}

void GatherBrushes(NODE node, std::vector<POLYGON>* list)
{
	if (node.IsLeaf)
	{
		if (node.FrontLeaf != NULL)
		{
			for (int i = 0; i < node.FrontLeaf->polygons.size(); i++)
				list->push_back(node.FrontLeaf->polygons[i]);
		}
		if (node.BackLeaf != NULL)
		{
			for (int i = 0; i < node.BackLeaf->polygons.size(); i++)
				list->push_back(node.BackLeaf->polygons[i]);
		}
	}
	else
	{
		if (node.Front != NULL)
		{
			GatherBrushes(*node.Front, list);
		}
		if (node.Back != NULL)
		{
			GatherBrushes(*node.Back, list);
		}
	}
}

int ClassifyPoly(Plane *Pl, POLYGON * Poly)
{
	int Infront = 0;
	int Behind = 0;
	int OnPlane = 0;
	float result;
	for (int a = 0; a < Poly->NumberOfVertices; a++)
	{

		result = ClassifyPoint(&Poly->VertexList[a].p, *Pl);

		if (result == CP_FRONT)
			Infront++;

		if (result == CP_BACK)
			Behind++;

		if (result == CP_ONPLANE)
		{
			OnPlane++;
		}

	}

	if (OnPlane == Poly->NumberOfVertices) return CP_ONPLANE;
	if (Behind == Poly->NumberOfVertices) return CP_BACK;
	if (Infront == Poly->NumberOfVertices) return CP_FRONT;
	return CP_SPANNING;
}

int ClassifyPoint(glm::vec3 *pos, Plane Plane)
{
	//float result;
	//result = glm::dot(*pos, Plane.normal) + Plane.d;
	auto dir = (Plane.pt - *pos);
	float result = glm::dot(dir, Plane.normal);
	if (result < -0.0001) return CP_FRONT;
	if (result > 0.0001) return CP_BACK;
	return CP_ONPLANE;
}