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
	inverseMass.resize(out.numberofpoints);
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
		for (int j = 0; j < out.numberofcorners; j++)
			inverseMass[v[j]] += t.restVolume > 0 ? t.restVolume / 4 : 0.0f;
	}
	std::cout << "Number of edges in Set: " << edges.size() << std::endl;
	std::cout << "Number of tetrahedras: " << tetrahedras.size() << std::endl;

	for (int i = 0; i < out.numberofpoints; i++)
	{
		tetrahedra_vertex v;
		double x = out.pointlist[3 * i];
		double y = out.pointlist[3 * i + 1];
		double z = out.pointlist[3 * i + 2];
		v.pos = vec3((float)x, (float)y, (float)z);
		v.pre_pos = v.pos;
		v.velocity = vec3(0, 0, 0);
		tetrahedra_vertices.push_back(v);
	}
	getSurfaceMesh();
	GenSphere();
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
		v.velocity.y -= dt * 9.8 * 0.2;
		v.pos += dt * v.velocity;
		/*if (length(v.velocity) > 0)
			continue;*/
		if (v.pos.y < height)
		{
			v.pos = v.pre_pos;
			v.pos.y = height;
		}
	}
}
void softBody::postSolve(float dt)
{
	for (auto& v : tetrahedra_vertices)
	{
		v.velocity = (v.pos - v.pre_pos) / (dt);
		if (length(v.velocity) > 10)
			continue;
	}
}
void softBody::solve(float dt)
{
	solveDistance(0.0000000001f,dt); //0.00000001f
	solveVolume(0.0f,dt);
}

void softBody::squash()
{
	for (auto& v : tetrahedra_vertices)
		v.pos.x = s.center.x;

}
void softBody::solveDistance(float edgeCompliance, float dt)
{
	float alpha = edgeCompliance / (dt * dt);
	for (auto& e : edges)
	{
		vec3 gradient = tetrahedra_vertices[e.indices[0]].pos - tetrahedra_vertices[e.indices[1]].pos;
		float w0 = inverseMass[e.indices[0]];
		float w1 = inverseMass[e.indices[1]];
		float w = w0 + w1;
		float len = length(gradient);
		if (len == 0.0f || w==0.0f)
			continue;
		gradient = normalize(gradient);
		float C = len - e.restLen;
		float s =  C / (w+alpha);
		tetrahedra_vertices[e.indices[0]].pos -= gradient * s * w0;
		tetrahedra_vertices[e.indices[1]].pos += gradient * s * w1;
	}
}

void softBody::solveVolume(float volumeCompliance, float dt)
{
	float alpha = volumeCompliance / (dt * dt);
	for (int i = 0; i < tetrahedras.size(); i++)
	{
		//out.tetrahedronlist
		vec3 A = tetrahedra_vertices[tetrahedras[i].indices[0]].pos;
		vec3 B = tetrahedra_vertices[tetrahedras[i].indices[1]].pos;
		vec3 C = tetrahedra_vertices[tetrahedras[i].indices[2]].pos;
		vec3 D = tetrahedra_vertices[tetrahedras[i].indices[3]].pos;
		float volume = (1.0f / 6) * fabs(dot((B - A), cross(C - A, D - A)));
		float c = volume - tetrahedras[i].restVolume;
		vec3* Jacobian = getTetrahedrasGradient(A, B, C, D);
		float w = 0.0f;
		for (int j = 0; j < 4; j++)
		{
			w += inverseMass[tetrahedras[i].indices[j]] * length(Jacobian[j]) * length(Jacobian[j]);
		}
		if (c > 1)
			continue;
		if (w == 0.0f)
			continue;
		
		float s = -c / (alpha + w);
		
		for (int j = 0; j < 4; j++)
		{
			vec3 a = s * Jacobian[j] * inverseMass[tetrahedras[i].indices[j]];
			tetrahedra_vertices[tetrahedras[i].indices[j]].pos += s * Jacobian[j] * inverseMass[tetrahedras[i].indices[j]];
		}
	}
}
void softBody::update(float dt)
{
	int numSubstps = 5;
	float sub_dt = dt / numSubstps;
	for (int step = 0; step < numSubstps; step++)
	{
		preSolve(sub_dt, -2.5);
		solve(sub_dt);
		postSolve(sub_dt);
		getSurfaceMesh();
		setupMesh();
		GenSphere();
	}
	
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
	vec3 p0 = tetrahedra_vertices[0].pos;
	vec3 p1 = p0;
	float maxDistance = 0.0f;
	for (auto& v : tetrahedra_vertices)
	{
		float d = length(p0 - v.pos);
		if ( d > maxDistance)
		{
			maxDistance = d;
			p1 = v.pos;
		}
	}
	vec3 p2 = p1;
	maxDistance = 0.0f;
	for (auto& v : tetrahedra_vertices)
	{
		float d = length(p1 - v.pos);
		if (d > maxDistance)
		{
			maxDistance = d;
			p2 = v.pos;
		}
	}
	s.center = (p1 + p2) / 2.0f;
	s.radius = length(p1 - p2) / 2.0f;

	for (auto& v : tetrahedra_vertices)
	{
		float d = length(s.center - v.pos);
		if (d > s.radius)
		{
			float new_radius = (s.radius + d) / 2.0f;
			vec3 dir = normalize(v.pos - s.center);
			s.center += dir * (d-s.radius) / 2.0f;
			s.radius = new_radius;
		}
	}

}
vec3* softBody::getTetrahedrasGradient(vec3 p1, vec3 p2, vec3 p3, vec3 p4)
{
	vec3* Jacobian = new vec3[4];
	Jacobian[0] = (1.0f / 6) * cross(p4 - p2, p3 - p2);
	Jacobian[1] = (1.0f / 6) * cross(p3 - p1, p4 - p1);
	Jacobian[2] = (1.0f / 6) * cross(p4 - p1, p2 - p1);
	Jacobian[3] = (1.0f / 6) * cross(p2 - p1, p3 - p1);
	return Jacobian;
}
void softBody::drag(vec3 p)
{
	float minDistance = INT_MAX;
	int index = -1;
	int i = -1;
	for (auto& v : tetrahedra_vertices)
	{
		float d = length(p - v.pos);
		if (d < minDistance)
		{
			minDistance = d;
			index = i;
		}
		i++;
	}
	tetrahedra_vertices[i].pos = p;
	inverseMass[i] = 0.0f;
}