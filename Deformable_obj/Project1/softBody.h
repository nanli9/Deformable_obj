#pragma once
#ifndef SOFT_BODY_H
#define SOFT_BODY_H

#include "Mesh.h"
#include "tetgen.h"
#include <set>

struct Tetrahedra
{
	int indices[4];
	double restVolume = 0;
};
struct Edge
{
	int indices[2];
	float restLen;

	bool operator<(const Edge& other) const {
		if (indices[0] == other.indices[0]) {
			return indices[1] < other.indices[1];
		}
		return indices[0] < other.indices[0];
	}
};
struct tetrahedra_vertex
{
	vec3 pos;
	vec3 pre_pos;
	vec3 velocity;
};
struct Sphere
{
	vec3 center;
	float radius;
};
class softBody: public Mesh
{
public:
	vec3 pos;
	vec3 velocity;
	vec3 acceleration;
	float compliance;
	Sphere s;
	tetgenio in, out;
	vector<Tetrahedra> tetrahedras;
	vector<tetrahedra_vertex> tetrahedra_vertices;
	set<Edge> edges;
	softBody(OBJParser parser, vec3 pos, vec3 velocity, vec3 acceleration, float compliance);
	void preSolve(float dt, float height);
	void solve(float dt);
	void postSolve(float dt);
	void squash();
	void solveVolume(float dt);
	void solveDistance(float dt);
	void update(float dt);
	void addEdge(int v1,int v2);
	void getSurfaceMesh();
	void GenSphere();
};




#endif // !SOFT_BODY_H
