#pragma once
#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "parser.h"
#include "shader.h"

struct Vertex
{
	vec3 Pos;
	vec3 normal;
};

class Mesh
{
public:
	unsigned VBO, VAO;
	vector<Vertex> vertices;
	Mesh(OBJParser p);
	void setupMesh();
	void draw(Shader& shader);
};



#endif // !MESH_H




