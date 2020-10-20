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

#ifndef DIFBuilder_hpp
#define DIFBuilder_hpp

#include <dif/objects/dif.h>
#include <glm/glm.hpp>

DIF_NAMESPACE

struct Marker
{
	glm::vec3 position;
	int msToNext;
	int smoothing;
	int initialPathPosition = -1;
	int initialTargetPosition = -1;
};

class DIFBuilder {
public:
	struct Point {
		glm::vec3 vertex;
		glm::vec3 normal;
		glm::vec2 uv;

		Point() : vertex(), uv(), normal() {

		}
		Point(const glm::vec3 &vertex, const glm::vec2 &uv) : vertex(vertex), uv(uv) {
			
		}
	};
	struct Triangle {
		Point points[3];
		U32 material;

		Triangle() {
			points[0] = Point();
			points[1] = Point();
			points[2] = Point();
			material = -1;
		}
		Triangle(Point point0, Point point1, Point point2) {
			points[0] = point0;
			points[1] = point1;
			points[2] = point2;
			material = -1;
		}
		Triangle(Point point0, Point point1, Point point2, U32 material) : material(material) {
			points[0] = point0;
			points[1] = point1;
			points[2] = point2;
		}
	};

protected:
	std::vector<std::string> mMaterials;
	std::vector<Triangle> mTriangles;
	std::vector<std::pair<Interior, std::vector<Marker>>> mPathedInteriors;
	std::vector<GameEntity> mGameEntities;
	F32 mScale;
	

public:
	DIFBuilder() : mScale(1.0f) {

	}
	DIFBuilder(const DIF &dif);

	bool exportEmitStrings = false;

	void addTriangle(const Triangle &triangle);
	void addTriangle(const Triangle &triangle, const std::string &material);
	void addPathedInterior(const Interior& interior, std::vector<Marker> path);
	void addEntity(const GameEntity& entity);
	void build(DIF &dif,bool flipNormals = false);

	inline F32 getScale() const {
		return mScale;
	}
	inline void setScale(const F32 &scale) {
		mScale = scale;
	}
protected:
	Interior::TexGenEq getTexGen(const Triangle &triangle);
	BoxF getBoundingBox();
	SphereF getBoundingSphere();
	glm::vec3 getAverageNormal(const Triangle &triangle);
	F32 getPlaneDistance(const Triangle &triangle, const glm::vec3 &center);
};

DIF_NAMESPACE_END

#endif /* DIFBuilder_hpp */
