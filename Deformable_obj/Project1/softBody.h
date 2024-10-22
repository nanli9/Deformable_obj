#pragma once
#ifndef SOFT_BODY_H
#define SOFT_BODY_H

#include "Mesh.h"
#include "tetgen.h"
#include <set>

struct Tetrahedra
{
	Vertex vertices[4];
};

class softBody: public Mesh
{
public:
	vec3 pos;
	vec3 velocity;
	vec3 acceleration;
	float compliance;
	tetgenio in, out;
	vector<Tetrahedra> tetrahedras;
	set<pair<int,int>> edges;
	softBody(OBJParser parser, vec3 pos, vec3 velocity, vec3 acceleration, float compliance);
	void solve(float dt);
	void squash();
	void solveVolume(float dt);
	void solveDistance(float dt);
	void solveCollision(float height);
};




#endif // !SOFT_BODY_H
