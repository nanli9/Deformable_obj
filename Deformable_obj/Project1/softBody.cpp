#include "softBody.h"


softBody::softBody(OBJParser parser,vec3 pos, vec3 velocity,vec3 acceleration,float compliance) : Mesh(parser)
{
	this->pos = pos;
	this->velocity = velocity;
	this->acceleration = acceleration;
	this->compliance = compliance;

	in.firstnumber = 0;

	in.numberofpoints = parser.vertices.size();
	in.pointlist = new double[in.numberofpoints*3];
	for (int i = 0; i < parser.vertices.size(); i++)
	{
		in.pointlist[3 * i] = (double)parser.vertices[i].x;
		in.pointlist[3 * i + 1] = (double)parser.vertices[i].y;
		in.pointlist[3 * i + 2] = (double)parser.vertices[i].z;
	}
	in.numberoffacets = parser.faces.size();
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];
	
	//printf("asddassda%d\n", in.numberoffacets);

	tetgenio::facet *f;
	tetgenio::polygon *p;

	for (int i = 0; i < in.numberoffacets; i++)
	{
		f = &in.facetlist[i];
		f->numberofpolygons = 1;
		f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
		f->numberofholes = 0;
		f->holelist = NULL;
		p = &f->polygonlist[0];
		p->numberofvertices = 3;
		p->vertexlist = new int[p->numberofvertices];
		p->vertexlist[0] = parser.faces[i].vertexIndices[0];
		p->vertexlist[1] = parser.faces[i].vertexIndices[1];
		p->vertexlist[2] = parser.faces[i].vertexIndices[2];
	}
	tetgenbehavior behavior;
	behavior.quality = 1;
	behavior.plc = 1;
	behavior.nobisect = 1;
	tetrahedralize(&behavior, &in, &out);
	std::cout << "Number of tetrahedra: " << out.numberoftetrahedra << std::endl;
	std::cout << "Number of cornor: " << out.numberofcorners << std::endl;
	std::cout << "Number of edge: " << out.numberofedges << std::endl;
	std::cout << "Number of surface Mesh: " << out.numberoftrifaces << std::endl;
	std::cout << "Number of faces: " << out.numberofvfacets << std::endl;
	std::cout << "Number of points : " << out.numberofpoints << std::endl;
	//out.save_nodes("bunny");
	//out.save_elements("bunny");
	//out.save_faces("bunny");
	for (int i = 0; i < out.numberoftetrahedra; i++)
	{
		int v[4];
		for (int j = 0; j < out.numberofcorners; j++)
			v[j] = out.tetrahedronlist[i * out.numberofcorners + j];

		//add edges
		addEdge(v[0], v[1]);
		addEdge(v[0], v[2]);
		addEdge(v[0], v[3]);
		addEdge(v[1], v[2]);
		addEdge(v[1], v[3]);
		addEdge(v[2], v[3]);

		Tetrahedra t;
		t.indices[0] = v[0];
		t.indices[1] = v[1];
		t.indices[2] = v[2];
		t.indices[3] = v[3];
		vec3 A = vec3(out.pointlist[3 * v[0]], out.pointlist[3 * v[0] + 1], out.pointlist[3 * v[0] + 2]);
		vec3 B = vec3(out.pointlist[3 * v[1]], out.pointlist[3 * v[1] + 1], out.pointlist[3 * v[1] + 2]);
		vec3 C = vec3(out.pointlist[3 * v[2]], out.pointlist[3 * v[2] + 1], out.pointlist[3 * v[2] + 2]);
		vec3 D = vec3(out.pointlist[3 * v[3]], out.pointlist[3 * v[3] + 1], out.pointlist[3 * v[3] + 2]);
		t.restVolume = (1.0f / 6) * fabs(dot((B - A), cross(C - A, D - A)));
		tetrahedras.push_back(t);
	}
	std::cout << "Number of edges in Set: " << edges.size() << std::endl;
	std::cout << "Number of tetrahedras: " << tetrahedras.size() << std::endl;

	for (int i = 0; i < out.numberofpoints; i++)
	{
		tetrahedra_vertex v;
		double x = out.pointlist[3 * i];
		double y = out.pointlist[3 * i + 1];
		double z = out.pointlist[3 * i + 2];
		v.pos = vec3(x, y, z);
		v.pre_pos = v.pos;
		v.velocity = vec3(0, 0, 0);
		tetrahedra_vertices.push_back(v);
	}
	getSurfaceMesh();
	GenSphere();
	/*for (auto& v : vertices)
	{
		v.Pos.y = 0;
	}*/
}
void softBody::addEdge(int v1, int v2)
{
	Edge e;
	e.indices[0] = v1;
	e.indices[1] = v2;
	vec3 p1 = vec3(out.pointlist[3 * v1], out.pointlist[3 * v1 + 1], out.pointlist[3 * v1 + 2]);
	vec3 p2 = vec3(out.pointlist[3 * v2], out.pointlist[3 * v2 + 1], out.pointlist[3 * v2 + 2]);
	e.restLen = length(p1 - p2);
	edges.insert(e);
}
void softBody::preSolve(float dt, float height)
{
	for (auto& v : tetrahedra_vertices)
	{
		v.pre_pos = v.pos;
		//v.velocity.y -= dt * 9.8 * 0.05;
		v.pos += dt * v.velocity;
		if (v.pos.y < height)
			v.pos.y = height;
	}
}
void softBody::postSolve(float dt)
{
	for (auto& v : tetrahedra_vertices)
	{
		v.velocity = (v.pos - v.pre_pos) / dt;

	}
}
void softBody::solve(float dt)
{
	solveDistance(dt);
	solveVolume(dt);
}

void softBody::squash()
{
	for (auto& x : vertices)
	{
		x.Pos.y = pos.y;
	}

}
void softBody::solveDistance(float dt)
{
	float alpha = compliance / (dt * dt);
	for (auto& e : edges)
	{
		vec3 gradient = tetrahedra_vertices[e.indices[0]].pos - tetrahedra_vertices[e.indices[1]].pos;
		float len = length(gradient);
		float C = len - e.restLen;
		float s = C / alpha;
		tetrahedra_vertices[e.indices[0]].pos -= gradient * s;
		tetrahedra_vertices[e.indices[1]].pos += gradient * s;
	}
}

void softBody::solveVolume(float dt)
{
	float alpha = compliance / (dt * dt);
	for (int i = 0; i < tetrahedras.size(); i++)
	{
		//out.tetrahedronlist
		vec3 A = tetrahedra_vertices[tetrahedras[i].indices[0]].pos;
		vec3 B = tetrahedra_vertices[tetrahedras[i].indices[1]].pos;
		vec3 C = tetrahedra_vertices[tetrahedras[i].indices[2]].pos;
		vec3 D = tetrahedra_vertices[tetrahedras[i].indices[3]].pos;
		float volume = (1.0f / 6) * fabs(dot((B - A), cross(C - A, D - A)));
		float c = volume - tetrahedras[i].restVolume;
		float s = c / alpha;

		for (int j = 0; j < 4; j++)
		{

		}

	}
}
void softBody::update(float dt)
{
	preSolve(dt,-2.5);
	solve(dt);
	postSolve(dt);
	getSurfaceMesh();
	setupMesh();
}
void softBody::getSurfaceMesh()
{
	vertices.resize(3 * out.numberoftrifaces);
	for (int i = 0; i < out.numberoftrifaces; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int index = out.trifacelist[3 * i + j];
			vertices[3 * i + j].Pos = tetrahedra_vertices[index].pos;
		}
		vec3 v1 = vertices[3 * i].Pos;
		vec3 v2 = vertices[3 * i + 1].Pos;
		vec3 v3 = vertices[3 * i + 2].Pos;

		vec3 n = cross(v1 - v3, v1 - v2);
		n = normalize(n);
		vertices[3 * i].normal = n;
		vertices[3 * i + 1].normal = n;
		vertices[3 * i + 2].normal = n;
	}

}
//generate bounding sphere for object picking
void softBody::GenSphere()
{


}
