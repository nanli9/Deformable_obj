#include "Mesh.h"

Mesh::Mesh(OBJParser p)
{
	for (auto& f : p.faces)
	{
		for (int i = 0; i < 3; i++)
		{
			Vertex v;
			v.Pos = p.vertices[f.vertexIndices[i]];
			v.normal = p.normals[f.normalIndices[i]];
			vertices.push_back(v);
		}
	}
	setupMesh();
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),&vertices[0],GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}


void Mesh::draw(Shader &shader)
{
	shader.use();

	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glBindVertexArray(0);

}
