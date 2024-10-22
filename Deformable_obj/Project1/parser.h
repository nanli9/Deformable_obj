#pragma once
#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

struct Face
{
	int vertexIndices[3];
	int normalIndices[3];
	int textureIndices[3];
};

class OBJParser
{
public:
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> textureCoords;
	vector<Face> faces;
	void loadOBJ(const std::string& filePath);
};


#endif // FUNCTIONS_H