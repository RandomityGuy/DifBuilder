//-----------------------------------------------------------------------------
// Copyright (c) 2016, HiGuy Smith
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the project nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include <DIFBuilder/DIFBuilder.hpp>
#include "DIFBuilder\BSPLib.h"
#include <iomanip>
#include <math.h>
#include <algorithm>

DIF_NAMESPACE

struct ObjectHash
{
	int hash;
	int obj;
};

struct HullPoly
{
	std::vector<int> points;
	int planeIndex;
};

Interior::TexGenEq getTexGenFromPoints(const glm::vec3 &point0, const glm::vec3 &point1, const glm::vec3 &point2, glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2);

DIFBuilder::DIFBuilder(const DIF &dif) : mScale(1.0f) {
	//TODO: Extract triangles
}

void DIFBuilder::addTriangle(const Triangle &triangle) {
	mTriangles.push_back(triangle);
}

void DIFBuilder::addTriangle(const Triangle &triangle, const std::string &material) {
	Triangle tri(triangle);

	//Find the material
	auto find = std::find(mMaterials.begin(), mMaterials.end(), material);
	if (find == mMaterials.end()) {
		tri.material = mMaterials.size();
		mMaterials.push_back(material);
	} else {
		//Distance from the start
		tri.material = (find - mMaterials.begin());
	}

	addTriangle(tri);
}


//Here come the dif writing functions, algorithm is nearly the same as the ones used in map2dif
short ExportPlane(Interior *interior, POLYGON poly,std::vector<ObjectHash>* planehashes)
{
	Plane testplane = Plane(poly.VertexList[poly.Indices[0]].p, poly.VertexList[poly.Indices[1]].p, poly.VertexList[poly.Indices[2]].p);
	std::size_t xhash = std::hash<float>()(testplane.normal.x);
	std::size_t yhash = std::hash<float>()(testplane.normal.y);
	std::size_t zhash = std::hash<float>()(testplane.normal.z);
	std::size_t dhash = std::hash<float>()(testplane.d);

	int hash = xhash ^ yhash ^ zhash ^ dhash;
	for (int i = 0; i < planehashes->size(); i++)
		if (planehashes->at(i).hash == hash)
			return planehashes->at(i).obj;

	int index = interior->plane.size();

	Interior::Plane p = Interior::Plane();
	
	int normindex = interior->normal.size();

	interior->normal.push_back(testplane.normal);
	interior->normal2.push_back(testplane.normal);

	p.normalIndex = normindex;
	p.planeDistance = testplane.d;

	interior->plane.push_back(p);

	ObjectHash o = ObjectHash();
	o.hash = hash;
	o.obj = index;

	planehashes->push_back(o);

	return index;

}

short ExportPlane(Interior *interior, Plane pl, std::vector<ObjectHash>* planehashes)
{
	std::size_t xhash = std::hash<float>{}(pl.normal.x);
	std::size_t yhash = std::hash<float>{}(pl.normal.y);
	std::size_t zhash = std::hash<float>{}(pl.normal.z);
	std::size_t dhash = std::hash<float>{}(pl.d);

	int hash = xhash ^ yhash ^ zhash ^ dhash;
	for (int i = 0; i < planehashes->size(); i++)
		if (planehashes->at(i).hash == hash)
			return planehashes->at(i).obj;

	int index = interior->plane.size();

	Interior::Plane p = Interior::Plane();

	int normindex = interior->normal.size();

	interior->normal.push_back(pl.normal);
	interior->normal2.push_back(pl.normal);

	p.normalIndex = normindex;
	p.planeDistance = pl.d;

	interior->plane.push_back(p);

	ObjectHash o = ObjectHash();
	o.hash = hash;
	o.obj = index;

	planehashes->push_back(o);

	return index;

}

short ExportTexture(Interior *interior,std::string tex)
{
	for (int i = 0; i < interior->materialName.size(); i++)
		if (interior->materialName[i] == tex)
			return i;

	int index = interior->materialName.size();
	interior->materialName.push_back(tex);
	return index;
}

short ExportTexGen(Interior *interior, POLYGON poly)
{
	glm::vec3 v1 = poly.VertexList[poly.Indices[0]].p;
	glm::vec3 v2 = poly.VertexList[poly.Indices[1]].p;
	glm::vec3 v3 = poly.VertexList[poly.Indices[2]].p;

	glm::vec2 uv1 = poly.VertexList[poly.Indices[0]].uv;
	glm::vec2 uv2 = poly.VertexList[poly.Indices[1]].uv;
	glm::vec2 uv3 = poly.VertexList[poly.Indices[2]].uv;

	Interior::TexGenEq texgen = getTexGenFromPoints(v1, v2, v3, uv1, uv2, uv3);

	int index = interior->texGenEq.size();
	interior->texGenEq.push_back(texgen);
	return index;
}

int ExportPoint(Interior *interior, glm::vec3 p, std::vector<ObjectHash>* pointhashes)
{
	std::size_t xhash = std::hash<float>{}(p.x);
	std::size_t yhash = std::hash<float>{}(p.y);
	std::size_t zhash = std::hash<float>{}(p.z);

	int hash = xhash ^ yhash ^ zhash;
	
	for (int i = 0; i < pointhashes->size(); i++)
		if (pointhashes->at(i).hash == hash)
			return pointhashes->at(i).obj;

	int index = interior->point.size();

	interior->point.push_back(p);
	interior->pointVisibility.push_back(-1);
	ObjectHash o = ObjectHash();
	o.hash = hash;
	o.obj = index;
	pointhashes->push_back(o);
	return index;
}

void ExportWinding(Interior *interior, POLYGON poly, std::vector<ObjectHash>* pointhashes)
{
	std::vector<int> finalWinding = std::vector<int>();

	for (int i = 0; i < poly.NumberOfIndices; i++)
	{
		glm::vec3 p = poly.VertexList[poly.Indices[i]].p;
		finalWinding.push_back(ExportPoint(interior, p, pointhashes));
	}
	Interior::WindingIndex a = Interior::WindingIndex();
	a.windingStart = interior->index.size();
	a.windingCount = finalWinding.size();


	interior->windingIndex.push_back(a);

	for (int i = 0; i < finalWinding.size(); i++)
		interior->index.push_back(finalWinding[i]);
}

void ExportSurfaces(Interior *interior, std::vector<POLYGON> polys,std::vector<ObjectHash>* planehashes,std::vector<std::string> materialList, std::vector<ObjectHash>* pointhashes)
{
	for (int i = 0; i < polys.size(); i++)
	{
		POLYGON poly = polys[i];

		Interior::Surface rSurface = Interior::Surface();
		rSurface.planeIndex = ExportPlane(interior, poly, planehashes);
		rSurface.textureIndex = ExportTexture(interior, materialList[poly.TextureIndex]);
		rSurface.texGenIndex = ExportTexGen(interior, poly);
		rSurface.surfaceFlags = 16;
		rSurface.fanMask = 15;
		ExportWinding(interior, poly, pointhashes);
		Interior::WindingIndex last = interior->windingIndex.back();
		rSurface.windingStart = last.windingStart;
		rSurface.windingCount = last.windingCount;
		interior->windingIndex.pop_back();
		rSurface.lightCount = 0;
		rSurface.lightStateInfoStart = 0;
		rSurface.mapSizeX = 0;
		rSurface.mapSizeY = 0;
		rSurface.mapOffsetX = 0;
		rSurface.mapOffsetY = 0;
		interior->surface.push_back(rSurface);
		interior->normalLMapIndex.push_back(0);
		interior->alarmLMapIndex.push_back(0);
	}
}

int CreateLeafIndex(int baseIndex, bool isSolid)
{
	int baseRet;
	if (isSolid)
	{
		baseRet = 0xC000;
	}
	else
	{
		baseRet = 0x8000;
	}
	return baseRet | baseIndex;
}

int ExportBSP(Interior *interior, NODE n, std::vector<POLYGON>* polys, std::vector<ObjectHash>* planehashes)
{
	if (n.IsLeaf)
	{
		int leafindex = interior->bspSolidLeaf.size();


		Interior::BSPSolidLeaf leaf = Interior::BSPSolidLeaf();
		leaf.surfaceIndex = interior->solidLeafSurface.size();
		leaf.surfaceCount = 0;

		std::vector<int> leafPolyIndices = std::vector<int>();

		for (int i = 0; i < polys->size(); i++)
			if (polys->at(i).leaf == n.FrontLeaf)
			{
				leafPolyIndices.push_back(i);
			}
		for (int i = 0; i < polys->size(); i++)
			if (polys->at(i).leaf == n.BackLeaf)
			{
				leafPolyIndices.push_back(i);
			}

		for (int i = 0; i < leafPolyIndices.size(); i++)
		{
			interior->solidLeafSurface.push_back(leafPolyIndices[i]);
			leaf.surfaceCount++;
		}

		interior->bspSolidLeaf.push_back(leaf);
		return CreateLeafIndex(leafindex, true);
	}
	else
	{
		Interior::BSPNode bspnode = Interior::BSPNode();
		int nodeindex = interior->bspNode.size();
		interior->bspNode.push_back(bspnode);
		interior->bspNode[nodeindex].planeIndex = ExportPlane(interior, n.Plane, planehashes);
		if (n.Front != NULL)
			interior->bspNode[nodeindex].frontIndex = ExportBSP(interior, *n.Front, polys, planehashes);
		else
			interior->bspNode[nodeindex].frontIndex = CreateLeafIndex(0, false);
		if (n.Back != NULL)
			interior->bspNode[nodeindex].backIndex = ExportBSP(interior, *n.Back, polys, planehashes);
		else		 
			interior->bspNode[nodeindex].backIndex = CreateLeafIndex(0, false);
		return nodeindex;
	}
}

int ExportEmitString(Interior* interior,std::vector<U8> emitstring, std::vector<ObjectHash>* emitstrHashes)
{
	// Search for already included strings.

	std::size_t strhash;
	for (int i = 0; i < emitstring.size(); i++)
	{
		std::size_t charhash = std::hash<char>{}(emitstring[i]);
		strhash ^= charhash;
		strhash ^= std::hash<int>{}(i);
	}

	for (int i = 0; i < emitstrHashes->size(); i++)
		if (emitstrHashes->at(i).hash == strhash)
			return emitstrHashes->at(i).obj;

	int index = interior->convexHullEmitStringCharacter.size();

	auto hashobj = ObjectHash();
	hashobj.hash = strhash;
	hashobj.obj = index;

	emitstrHashes->push_back(hashobj);

	for (int i = 0; i < emitstring.size(); i++)
		interior->convexHullEmitStringCharacter.push_back(emitstring.at(i));

	return index;

	//auto res = std::search(interior->convexHullEmitStringCharacter.begin(), interior->convexHullEmitStringCharacter.end(), emitstring.begin(), emitstring.end());

	//if (res == interior->convexHullEmitStringCharacter.end())
	//{
	//	int insertpt = interior->convexHullEmitStringCharacter.size();
	//	for (int i = 0; i < emitstring.size(); i++)
	//		interior->convexHullEmitStringCharacter.push_back(emitstring[i]);

	//	return insertpt;
	//}
	//else
	//{
	//	return std::distance(interior->convexHullEmitStringCharacter.begin(), res);
	//}
}

void ExportConvexHulls(Interior* interior, std::vector<std::vector<POLYGON>> polys, std::vector<ObjectHash>* pointHashes, std::vector<ObjectHash>* planeHashes,std::vector<ObjectHash>* emitstrHashes)
{
	for (int polyIndex = 0; polyIndex < polys.size(); polyIndex++)
	{
		Interior::ConvexHull hull = Interior::ConvexHull();
		hull.surfaceStart = interior->hullSurfaceIndex.size();
		hull.surfaceCount = polys[polyIndex].size();

		for (int i = 0; i < hull.surfaceCount; i++)
			interior->hullSurfaceIndex.push_back(hull.surfaceStart + i);

		hull.hullStart = interior->hullIndex.size();
		hull.hullCount = 0;

		for (int i = 0; i < polys[polyIndex].size(); i++)
			hull.hullCount += polys[polyIndex][i].NumberOfVertices;

		std::vector<int> hullPoints = std::vector<int>();
		std::vector<HullPoly> hullpolys = std::vector<HullPoly>();

		hull.polyListPointStart = interior->polyListPointIndex.size();

		for (int i = 0; i < polys[polyIndex].size(); i++)
		{
			HullPoly hp = HullPoly();
			hp.points = std::vector<int>();
			for (int j = 0; j < polys[polyIndex][i].NumberOfIndices; j++)
			{
				int pt = ExportPoint(interior, polys[polyIndex][i].VertexList[polys[polyIndex][i].Indices[j]].p, pointHashes);
				interior->hullIndex.push_back(pt);
				interior->polyListPointIndex.push_back(pt);
				hp.points.push_back(pt);
				hullPoints.push_back(pt);
			}
			hp.planeIndex = ExportPlane(interior, polys[polyIndex][i], planeHashes);
			hullpolys.push_back(hp);
		}

		hull.polyListPlaneStart = interior->polyListPlaneIndex.size();
		hull.planeStart = interior->hullPlaneIndex.size();

		for (int i = 0; i < polys[polyIndex].size(); i++)
		{
			int planeindex = ExportPlane(interior, polys[polyIndex][i], planeHashes);
			interior->polyListPlaneIndex.push_back(planeindex);
			interior->hullPlaneIndex.push_back(planeindex);
		}

		float minx = 1E+08f;
		float miny = 1E+08f;
		float minz = 1E+08f;
		float maxx = -1E+08f;
		float maxy = -1E+08f;
		float maxz = -1E+08f;

		for (int i = 0; i < polys[polyIndex].size(); i++)
		{
			for (int j = 0; j < polys[polyIndex][i].NumberOfVertices; j++)
			{
				glm::vec3 v = polys[polyIndex][i].VertexList[j].p;
				if (v.x < minx)
					minx = v.x;
				if (v.y < miny)
					miny = v.y;
				if (v.z < minz)
					minz = v.z;
				if (v.x > maxx)
					maxx = v.x;
				if (v.y > maxy)
					maxy = v.y;
				if (v.z > maxz)
					maxz = v.z;
			}
		}

		hull.minX = minx;
		hull.minY = miny;
		hull.minZ = minz;
		hull.maxX = maxx;
		hull.maxY = maxy;
		hull.maxZ = maxz;

		//This is straight up copied from map2dif
		for (int i = 0; i < hullPoints.size(); i++)
		{
			static std::vector<U32> emitPoints(128);
			static std::vector<U16> emitEdges(128);
			static std::vector<U32> emitPolys;

			emitPoints.clear();
			emitEdges.clear();
			emitPolys.clear();

			int point = hullPoints[i];

			for (int j = 0; j < hullpolys.size(); j++)
			{
				bool found = false;
				for (int k = 0; k < hullpolys[j].points.size(); k++)
					if (hullpolys[j].points[k] == point)
						found = true;

				if (found)
				{
					for (int k = 0; k < hullpolys[j].points.size(); k++)
						emitPoints.push_back(hullpolys[j].points[k]);

					for (int k = 0; k < hullpolys[j].points.size(); k++)
					{
						int first = hullpolys[j].points[k];
						int next = hullpolys[j].points[(k + 1) % hullpolys[j].points.size()];

						int edge = (((int)fmin(first, next) << 8) | ((int)fmax(first, next)) << 0);
						emitEdges.push_back(edge);
					}

					emitPolys.push_back(j);
				}
			}

			for (int j = 0; j < hullpolys.size(); j++)
			{
				for (int k = 0; k < emitPolys.size(); k++)
				{
					if (emitPolys[k] == j) continue;

					if (hullpolys[emitPolys[k]].planeIndex == hullpolys[j].planeIndex)
					{
						bool found = false;

						for (int l = 0; l < emitPolys.size(); l++)
							if (emitPolys[l] == j)
								found = true;

						if (!found)
						{
							for (int l = 0; l < hullpolys[j].points.size(); l++)
							{
								emitPoints.push_back(hullpolys[j].points[l]);

								int first = hullpolys[j].points[l];
								int next = hullpolys[j].points[(l + 1) % hullpolys[j].points.size()];

								int edge = (((int)fmin(first, next) << 8) | ((int)fmax(first, next)) << 0);
								emitEdges.push_back(edge);
							}
							emitPolys.push_back
(j);
						}
					}
				}
			}

			std::sort(emitPoints.begin(), emitPoints.end());
			auto last = std::unique(emitPoints.begin(), emitPoints.end());
			emitPoints.erase(last, emitPoints.end());
			std::sort(emitPoints.begin(), emitPoints.end());


			std::sort(emitEdges.begin(), emitEdges.end());
			auto last2 = std::unique(emitEdges.begin(), emitEdges.end());
			emitEdges.erase(last2, emitEdges.end());
			std::sort(emitEdges.begin(), emitEdges.end());

			for (int j = 0; j < emitEdges.size(); j++)
			{
				int firstIndex = emitEdges[j] >> 8;
				int nextIndex = emitEdges[j] & 0xFF;

				int newFirst = 0xFFFF;
				int newNext = 0xFFFF;
				for (int k = 0; k < emitPoints.size(); k++)
				{
					if (emitPoints[k] == firstIndex)
						newFirst = k;
					if (emitPoints[k] == nextIndex)
						newNext = k;
				}

				int newSignature = (((int)fmin(newFirst, newNext) << 8) |
					((int)fmax(newFirst, newNext) << 0));
				emitEdges[j] = newSignature;
			}

			std::vector<U8> emitString = std::vector<U8>();

			emitString.push_back(emitPoints.size());
			for (auto &p : emitPoints)
				emitString.push_back(i);

			emitString.push_back(emitEdges.size());
			for (auto &e : emitEdges)
			{
				emitString.push_back(e >> 8);
				emitString.push_back(e & 0xFF);
			}

			emitString.push_back(emitPolys.size());

			for (int j = 0; j < emitPolys.size(); j++)
			{
				emitString.push_back(hullpolys[emitPolys[j]].points.size());
				emitString.push_back(emitPolys[j]);
				for (int k = 0; k < hullpolys[emitPolys[j]].points.size(); k++)
				{
					bool found = false;
					for (int l = 0; l < emitPoints.size(); l++)
					{
						if (emitPoints[l] == hullpolys[emitPolys[j]].points[k])
						{
							emitString.push_back(l);
							found = true;
							break;
						}
					}
				}
			}

			interior->hullEmitStringIndex.push_back(ExportEmitString(interior, emitString,emitstrHashes));

			//interior->hullEmitStringIndex.push_back(0);
		}
		interior->convexHull.push_back(hull);			
	}	
	//interior->convexHullEmitStringCharacter.push_back(0);
	interior->polyListStringCharacter.push_back(0);
}

void ExportCoordBins(Interior* interior)
{
	for (int i = 0; i < 256; i++)
		interior->coordBin.push_back(Interior::CoordBin());

	for (int i = 0; i < 16; i++)
	{
		float minX = interior->boundingBox.minX;
		float maxX = interior->boundingBox.minX;
		minX += i * ((interior->boundingBox.maxX - interior->boundingBox.minX) / 16);
		maxX += (i+1) * ((interior->boundingBox.maxX - interior->boundingBox.minX) / 16);
		for (int j = 0; j < 16; j++)
		{
			float minY = interior->boundingBox.minY;
			float maxY = interior->boundingBox.minY;
			minY += j * ((interior->boundingBox.maxY - interior->boundingBox.minY) / 16);
			maxY += (j + 1) * ((interior->boundingBox.maxY - interior->boundingBox.minY) / 16);

			int binIndex = (i * 16) + j;
			interior->coordBin[binIndex].binStart = interior->coordBinIndex.size();

			for (int k = 0; k < interior->convexHull.size(); k++)
			{
				auto hull = interior->convexHull[k];

				if (!(minX > hull.maxX || maxX < hull.minX || maxY < hull.minY || minY > hull.maxY))
				{
					interior->coordBinIndex.push_back(k);
				}
			}
			interior->coordBin[binIndex].binCount = interior->coordBinIndex.size() - interior->coordBin[binIndex].binStart;
		}
	}
}

void FixPlanes(std::vector<Interior::Plane>& planes,std::vector<glm::vec3>& normals)
{
	for (int i = 0; i < planes.size(); i++)
	{
		if (planes[i].planeDistance < 0.00001 && planes[i].planeDistance > -0.00001 || isnan(planes[i].planeDistance))
			planes[i].planeDistance = 0;

	}
	for (int i = 0; i < normals.size(); i++)
	{
		if (normals[i].x < 0.00001 && normals[i].x > -0.00001 || isnan(normals[i].x))
			normals[i].x = 0;
		if (normals[i].y < 0.00001 && normals[i].y > -0.00001 || isnan(normals[i].y))
			normals[i].y = 0;
		if (normals[i].z < 0.00001 && normals[i].y > -0.00001 || isnan(normals[i].z))
			normals[i].z = 0;
	}
}

void DIFBuilder::build(DIF &dif,bool flipNormals,bool fastBSP) 
{
	POLYGON polyList;
	POLYGON *poly = &polyList;

	for (int i = 0; i < mTriangles.size(); i++)
	{
		poly->VertexList = (Vertex*)calloc(3, sizeof(Vertex));
		if (!flipNormals)
		{
			for (int j = 0; j < 3; j++)
				poly->VertexList[j].p = glm::vec3(mTriangles[i].points[j].vertex.x, mTriangles[i].points[j].vertex.y, mTriangles[i].points[j].vertex.z);
		}
		else
		{
			for (int j = 0; j < 3; j++)
				poly->VertexList[2-j].p = glm::vec3(mTriangles[i].points[j].vertex.x, mTriangles[i].points[j].vertex.y, mTriangles[i].points[j].vertex.z);
		}

		poly->NumberOfVertices = 3;
		
		poly->Indices = (int*)calloc(3, sizeof(int));
		for (int j = 0; j < 3; j++)
			poly->Indices[j] = j;

		poly->NumberOfIndices = 3;

		poly->TextureIndex = mTriangles[i].material;

		if (!flipNormals)
		{
			for (int j = 0; j < 3; j++)
				poly->VertexList[j].uv = mTriangles[i].points[j].uv;
		}
		else
		{
			for (int j = 0; j < 3; j++)
				poly->VertexList[2-j].uv = mTriangles[i].points[j].uv;
		}
		if (!flipNormals)
		{
			poly->plane = Plane(mTriangles[i].points[0].vertex, mTriangles[i].points->normal);
			poly->Normal = poly->plane.normal;
		}
		else
		{
			poly->plane = Plane(mTriangles[i].points[2].vertex, (float)(-1)*mTriangles[i].points->normal);
			poly->Normal = poly->plane.normal;
		}

		poly->BeenUsedAsSplitter = false;

		if (i != mTriangles.size() - 1)
		{
			poly->Next = new POLYGON();
			poly = poly->Next;
		}
		else
		{
			poly->Next = NULL;
		}
	}

	printf("Generating BSP\n");
	InitPolygons(polyList);
	NODE root;
	int polycount = GetCount(&polyList);
	BuildBspTree(root, &polyList,fastBSP);
	polycount = GetCount(&polyList);
	printf("Gathering primitives\n");
	std::vector<POLYGON> polys = std::vector<POLYGON>();
	GatherBrushes(root, &polys);

	Interior interior = Interior();
	
	std::vector<ObjectHash> planehashes, pointhashes;

	printf("Exporting Surfaces\n");
	ExportSurfaces(&interior, polys, &planehashes, mMaterials, &pointhashes);
	printf("Exporting BSP\n");
	ExportBSP(&interior, root, &polys, &planehashes);

	std::vector<std::vector<POLYGON>> groupedPolys = std::vector<std::vector<POLYGON>>();

	int fullpolycount = polys.size() / 8;
	int rem = polys.size() % 8;

	for (int i = 0; i < polys.size() - rem; i += 8)
	{
		std::vector<POLYGON> polysList = std::vector<POLYGON>();
		for (int j = 0; j < 8; j++)
			polysList.push_back(polys[i + j]);

		groupedPolys.push_back(polysList);
	}
	std::vector<POLYGON> lastPolys = std::vector<POLYGON>();
	for (int i = polys.size() - rem; i < polys.size(); i++)
	{
		lastPolys.push_back(polys[i]);
	}
	if (lastPolys.size() != 0)
		groupedPolys.push_back(lastPolys);

	printf("Exporting Convex Hulls\n");
	std::vector<ObjectHash> emitStrHashes;
	ExportConvexHulls(&interior, groupedPolys, &pointhashes, &planehashes,&emitStrHashes);

	printf("Exporting Zones\n");
	Interior::Zone z = Interior::Zone();
	z.portalStart = 0;
	z.portalCount = 0;
	z.surfaceStart = 0;
	z.surfaceCount = interior.surface.size();
	z.flags = 0;
	z.staticMeshStart = 0;
	z.staticMeshCount = 0;

	interior.zone.push_back(z);

	for (int i = 0; i < interior.surface.size(); i++)
		interior.zoneSurface.push_back(i);

	interior.coordBinMode = 0;
	interior.baseAmbientColor = glm::cvec4(1, 1, 1, 1);
	interior.alarmAmbientColor = glm::cvec4(1, 1, 1, 1);
	interior.detailLevel = 0;
	interior.lightMapBorderSize = 0;
	interior.minPixels = 250;
	interior.hasAlarmState = 0;
	interior.numLightStateEntries = 0;
	interior.boundingBox = getBoundingBox();
	interior.boundingSphere = getBoundingSphere();

	printf("Exporting CoordBins\n");

	ExportCoordBins(&interior);

	printf("Finalizing\n");
	//FixPlanes(interior.plane, interior.normal);
	dif.interior.push_back(interior);
}

BoxF DIFBuilder::getBoundingBox() {
	BoxF box(1e8, 1e8, 1e8, -1e8, -1e8, -1e8);
	for (const auto &triangle : mTriangles) {
		box.unionPoint(triangle.points[0].vertex);
		box.unionPoint(triangle.points[1].vertex);
		box.unionPoint(triangle.points[2].vertex);
	}
	return box;
}

SphereF DIFBuilder::getBoundingSphere() {
	BoxF box = getBoundingBox();

	SphereF sphere;
	glm::vec3 center = box.getCenter();
	sphere.x = center.x;
	sphere.y = center.y;
	sphere.z = center.z;
	glm::vec3 rvec = (box.getMax() - box.getCenter());
	sphere.radius = sqrt(rvec.x * rvec.x + rvec.y * rvec.y + rvec.z * rvec.z);
	return sphere;
}

glm::vec3 DIFBuilder::getAverageNormal(const Triangle &triangle) {
	const glm::vec3 &point0 = triangle.points[0].normal;
	const glm::vec3 &point1 = triangle.points[1].normal;
	const glm::vec3 &point2 = triangle.points[2].normal;

	//average
	return (point0 + point1 + point2) / 3.0f;

	//n = (v2 - v1) x (v2 - v0)
//	glm::vec3 crossNorm = glm::normalize(glm::cross(point2 - point1, point2 - point0));

//	return crossNorm;
}

F32 DIFBuilder::getPlaneDistance(const Triangle &triangle, const glm::vec3 &center) {
	//Use the center of the plane, probably correct
	glm::vec3 averagePoint(0);
	const glm::vec3 &point0 = triangle.points[0].vertex;
	const glm::vec3 &point1 = triangle.points[1].vertex;
	const glm::vec3 &point2 = triangle.points[2].vertex;
	averagePoint += point0;
	averagePoint += point1;
	averagePoint += point2;
	averagePoint /= 3.0f;

	//Get the normal
	const glm::vec3 &normal = getAverageNormal(triangle);

	//http://mathworld.wolfram.com/Point-PlaneDistance.html
	//D = n • (x_0 - x_i)
	// Where x_0 = (0, 0, 0)
	return glm::dot(-averagePoint, normal);
}

//Because floating point math sucks
template<typename T>
bool closeEnough(const T &p1, const T &p2, const F32 distance = 0.0001f) {
	return glm::distance(p1, p2) < distance;
}

std::ostream &operator<<(std::ostream &stream, const glm::mat3x4 &input) {
	return stream << std::setw(10) << input[0][0] << " " << std::setw(10) << input[0][1] << " " << std::setw(10) << input[0][2] << " " << std::setw(10) << input[0][3] << std::endl
	              << std::setw(10) << input[1][0] << " " << std::setw(10) << input[1][1] << " " << std::setw(10) << input[1][2] << " " << std::setw(10) << input[1][3] << std::endl
	              << std::setw(10) << input[2][0] << " " << std::setw(10) << input[2][1] << " " << std::setw(10) << input[2][2] << " " << std::setw(10) << input[2][3] << std::endl;
}

//The 3 elementary matrix row operations that you can apply without changing the result
void swapRows(glm::mat3x4 &mat, int rowA, int rowB) {
	std::swap(mat[rowA], mat[rowB]);
}
void scaleRow(glm::mat3x4 &mat, int row, float scale) {
	mat[row] *= scale;
}
void addRow(glm::mat3x4 &mat, int destRow, int srcRow, float factor) {
	mat[destRow] += mat[srcRow] * factor;
}

glm::vec3 solveMatrix(glm::mat3x4 pointMatrix, bool print = false) {
#define MaybePrint(a) if (print) { std::cout << a << std::endl; }

	//Clean up stuff that is almost zero
	MaybePrint(pointMatrix);
	if (closeEnough(pointMatrix[0][0], 0.f)) pointMatrix[0][0] = 0.f;
	if (closeEnough(pointMatrix[1][0], 0.f)) pointMatrix[1][0] = 0.f;
	if (closeEnough(pointMatrix[2][0], 0.f)) pointMatrix[2][0] = 0.f;
	if (closeEnough(pointMatrix[0][1], 0.f)) pointMatrix[0][1] = 0.f;
	if (closeEnough(pointMatrix[1][1], 0.f)) pointMatrix[1][1] = 0.f;
	if (closeEnough(pointMatrix[2][1], 0.f)) pointMatrix[2][1] = 0.f;
	if (closeEnough(pointMatrix[0][2], 0.f)) pointMatrix[0][2] = 0.f;
	if (closeEnough(pointMatrix[1][2], 0.f)) pointMatrix[1][2] = 0.f;
	if (closeEnough(pointMatrix[2][2], 0.f)) pointMatrix[2][2] = 0.f;
	if (closeEnough(pointMatrix[0][3], 0.f)) pointMatrix[0][3] = 0.f;
	if (closeEnough(pointMatrix[1][3], 0.f)) pointMatrix[1][3] = 0.f;
	if (closeEnough(pointMatrix[2][3], 0.f)) pointMatrix[2][3] = 0.f;
	MaybePrint(pointMatrix);

	//For checking at the end
	glm::mat3x4 test = pointMatrix;

	/*
	 We have three simultaneous equations:
	 ax + by + cz = uv0
	 dx + ey + fz = uv1
	 gx + hy + iz = uv2

	 We can arrange them in a matrix like this:
	 [ a b c ]   ( x )   ( uv0 )
	 [ d e f ] x ( y ) = ( uv1 )
	 [ g h i ]   ( z )   ( uv2 )
	 
	 And then if we can get the matrix into reduced echelon form we can solve
	 for (x y z) with no trouble.
	 */

	//Swap around rows so that we can get something non-zero for [0][0]
	if (closeEnough(pointMatrix[0][0], 0.f)) {
		if (closeEnough(pointMatrix[1][0], 0.f)) {
			swapRows(pointMatrix, 0, 2);
			MaybePrint(pointMatrix);
		} else {
			swapRows(pointMatrix, 0, 1);
			MaybePrint(pointMatrix);
		}
	}
	//If all three have zero for [0][0] then we're fine here
	if (!closeEnough(pointMatrix[0][0], 0.f)) {
		/*
		 Reduce second and third rows so the first column is zero
		 To get
		 [ a b c ]
		 [ 0 d e ]
		 [ 0 f g ]
		 */
		if (!closeEnough(pointMatrix[1][0], 0.f)) {
			addRow(pointMatrix, 1, 0, -pointMatrix[1][0] / pointMatrix[0][0]);
			MaybePrint(pointMatrix);
		}
		if (!closeEnough(pointMatrix[2][0], 0.f)) {
			addRow(pointMatrix, 2, 0, -pointMatrix[2][0] / pointMatrix[0][0]);
			MaybePrint(pointMatrix);
		}
	}

	//If mat[1][1] is zero we should swap rows 1 and 2 so we can reduce the other row
	if (closeEnough(pointMatrix[1][1], 0.f)) {
		swapRows(pointMatrix, 1, 2);
		MaybePrint(pointMatrix);
	}
	//If mat[1][1] is zero then both [1][1] and [2][1] are zero and we can continue
	if (!closeEnough(pointMatrix[1][1], 0.f)) {
		/*
		 Reverse third row so the second column is zero
		 To get
		 [ a b c ]
		 [ 0 d e ]
		 [ 0 0 f ]
		 */
		if (!closeEnough(pointMatrix[2][1], 0.f)) {
			addRow(pointMatrix, 2, 1, -pointMatrix[2][1] / pointMatrix[1][1]);
			MaybePrint(pointMatrix);
		}
	}

	/*
	 Scale each of the rows so the first component is one
	 To get
	 [ 1 a b ]
	 [ 0 1 c ]
	 [ 0 0 1 ]
	 */
	if (!closeEnough(pointMatrix[0][0], 0.f, 0.00001f)) {
		scaleRow(pointMatrix, 0, 1.0f / pointMatrix[0][0]);
		MaybePrint(pointMatrix);
	}
	if (!closeEnough(pointMatrix[1][1], 0.f, 0.00001f)) {
		scaleRow(pointMatrix, 1, 1.0f / pointMatrix[1][1]);
		MaybePrint(pointMatrix);
	}
	if (!closeEnough(pointMatrix[2][2], 0.f, 0.00001f)) {
		scaleRow(pointMatrix, 2, 1.0f / pointMatrix[2][2]);
		MaybePrint(pointMatrix);
	}

	/*
	 At this point the matrix is

	 [ 1 a b ]   ( x )   ( uv0' )
	 [ 0 1 c ] x ( y ) = ( uv1' )
	 [ 0 0 1 ]   ( z )   ( uv2' )

	 where
	 x + ay + bz = uv0
	      y + cz = uv1
	           z = uv2

	 therefore
	 z = uv2'
	 y = uv1' - cz
	 x = uv0' - ay - bz

	 */

	//Convenience
	const glm::vec4 &xvec = pointMatrix[0];
	const glm::vec4 &yvec = pointMatrix[1];
	const glm::vec4 &zvec = pointMatrix[2];

	//These are easy now
	float z = zvec[3];
	float y = yvec[3] - z * yvec[2];
	float x = xvec[3] - y * xvec[1] - z * xvec[2];

	//Check our work
//	assert(closeEnough(x * test[0][0] + y * test[0][1] + z * test[0][2], test[0][3]));
//	assert(closeEnough(x * test[1][0] + y * test[1][1] + z * test[1][2], test[1][3]));
//	assert(closeEnough(x * test[2][0] + y * test[2][1] + z * test[2][2], test[2][3]));

	//And there we go
	return glm::vec3(x, y, z);
}

Interior::TexGenEq getTexGenFromPoints(const glm::vec3 &point0, const glm::vec3 &point1, const glm::vec3 &point2,
								  glm::vec2 uv0, glm::vec2 uv1, glm::vec2 uv2) {
	Interior::TexGenEq texGen;

	//Construct these matrices for the solver to figure out
	glm::mat3x4	xTexMat = glm::mat3x4(glm::vec4(point0, uv0.x), glm::vec4(point1, uv1.x), glm::vec4(point2, uv2.x));
	glm::mat3x4 yTexMat = glm::mat3x4(glm::vec4(point0, uv0.y), glm::vec4(point1, uv1.y), glm::vec4(point2, uv2.y));

	//Solving is rather simple
	glm::vec3 xsolve = solveMatrix(xTexMat);
	glm::vec3 ysolve = solveMatrix(yTexMat);

	//Rigorous checking because I don't like being wrong
	if (!closeEnough(xsolve.x * point0.x + xsolve.y * point0.y + xsolve.z * point0.z, uv0.x, 0.001f)) {
		solveMatrix(xTexMat, true);
	}
	if (!closeEnough(xsolve.x * point1.x + xsolve.y * point1.y + xsolve.z * point1.z, uv1.x, 0.001f)) {
		solveMatrix(xTexMat, true);
	}
	if (!closeEnough(xsolve.x * point2.x + xsolve.y * point2.y + xsolve.z * point2.z, uv2.x, 0.001f)) {
		solveMatrix(xTexMat, true);
	}
	if (!closeEnough(ysolve.x * point0.x + ysolve.y * point0.y + ysolve.z * point0.z, uv0.y, 0.001f)) {
		solveMatrix(yTexMat, true);
	}
	if (!closeEnough(ysolve.x * point1.x + ysolve.y * point1.y + ysolve.z * point1.z, uv1.y, 0.001f)) {
		solveMatrix(yTexMat, true);
	}
	if (!closeEnough(ysolve.x * point2.x + ysolve.y * point2.y + ysolve.z * point2.z, uv2.y, 0.001f)) {
		solveMatrix(yTexMat, true);
	}

	//And there we go
	texGen.planeX.x = xsolve.x;
	texGen.planeX.y = xsolve.y;
	texGen.planeX.z = xsolve.z;
	texGen.planeY.x = ysolve.x;
	texGen.planeY.y = ysolve.y;
	texGen.planeY.z = ysolve.z;

	return texGen;
}

Interior::TexGenEq DIFBuilder::getTexGen(const Triangle &triangle) {
	return getTexGenFromPoints(triangle.points[0].vertex,
							   triangle.points[1].vertex,
							   triangle.points[2].vertex,
							   triangle.points[0].uv,
							   triangle.points[1].uv,
							   triangle.points[2].uv);
}

DIF_NAMESPACE_END
