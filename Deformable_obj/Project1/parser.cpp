#include "parser.h"

void OBJParser::loadOBJ(const std::string& filePath)
{
	ifstream objFile(filePath);
	if (!objFile.is_open())
		std::cerr << "Error: Could not open the file!" << std::endl;
	string line;
	while (getline(objFile, line))
	{
		istringstream lineStream(line);
		string type;
		lineStream >> type;

		if (type == "v")
		{
			vec3 vertex;
			lineStream >> vertex.x >> vertex.y >> vertex.z;
			vertices.push_back(vertex);
		}
		else if (type == "vn")
		{
			vec3 normal;
			lineStream >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);

		}
		else if (type == "vt")
		{
			vec2 texCoord;
			lineStream >> texCoord.x >> texCoord.y;
			textureCoords.push_back(texCoord);
		}
		else if (type == "f")
		{
			Face face;
			char slash;
			for (int i = 0; i < 3; i++)
			{
				lineStream >> face.vertexIndices[i] >> slash >> face.textureIndices[i] >> slash >> face.normalIndices[i];
				face.vertexIndices[i]--;
				face.textureIndices[i]--;
				face.normalIndices[i]--;
			}
			faces.push_back(face);


		}
	}
	objFile.close();
	
}

