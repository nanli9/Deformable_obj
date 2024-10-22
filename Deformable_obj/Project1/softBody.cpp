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
	behavior.coarsen = 1;
	tetrahedralize(&behavior, &in, &out);
	std::cout << "Number of tetrahedra: " << out.numberoftetrahedra << std::endl;
	std::cout << "Number of cornor: " << out.numberofcorners << std::endl;
	std::cout << "Number of edge: " << out.numberofedges << std::endl;
	std::cout << "Number of surface Mesh: " << out.numberoftrifaces << std::endl;
	std::cout << "Number of faces: " << out.numberofvfacets << std::endl;
	//out.save_nodes("bunny");
	//out.save_elements("bunny");
	//out.save_faces("bunny");
	for (int i = 0; i < out.numberoftetrahedra; i++)
	{
		int v[4];
		for (int j = 0; j < out.numberofcorners; j++)
			v[j] = out.tetrahedronlist[i * out.numberofcorners + j];
		//add edges
		edges.insert(pair<int, int>(v[0], v[1]));
		edges.insert(pair<int, int>(v[0], v[2]));
		edges.insert(pair<int, int>(v[0], v[3]));
		edges.insert(pair<int, int>(v[1], v[2]));
		edges.insert(pair<int, int>(v[1], v[3]));
		edges.insert(pair<int, int>(v[2], v[3]));
	}
	std::cout << "Number of edges in Set: " << edges.size() << std::endl;
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

}

void softBody::solveVolume(float dt)
{
	for (int i = 0; i < out.numberoftetrahedra; i++)
	{
		//out.tetrahedronlist
	}
}
void softBody::solveCollision(float height)
{
	for (auto& x : vertices)
	{
		if (x.Pos.y < height)
			x.Pos.y = height;
	}
}

