#include "AABB.h"

#ifndef NO_GRAPHICS
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <limits>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>

using namespace glm;

bool AABB::isInside(const glm::vec3& pt) const
{
	/*
	bool x = all(greaterThanEqual(this->min, pt));
	bool y = all(lessThanEqual(pt, this->max));
	
	return x && y;
	*/

	return	pt.x >= min.x && pt.x <= max.x &&
			pt.y >= min.y && pt.y <= max.y &&
			pt.z >= min.z && pt.z <= max.z;
}

bool AABB::isInside(const AABB& other) const
{
	return isInside(other.min) && isInside(other.max);
}

std::vector<glm::vec3> AABB::getVertices() const
{
	using namespace std;

	// create the eight vertices of the bounding box
	vec4 vertices[] = { vec4(min.x, min.y, min.z, 1.f),
						vec4(min.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, min.z, 1.f),
						vec4(min.x, max.y, min.z, 1.f),
						vec4(min.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, min.z, 1.f) };
	
	vector<vec3> result(&vertices[0], &vertices[8]);
	return std::move(result);
}

AABB::ClipResult AABB::isVisible(const glm::mat4& mvp) const
{

	// create the eight vertices of the bounding box
	vec4 vertices[] = { vec4(min.x, min.y, min.z, 1.f),
						vec4(min.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, min.z, 1.f),
						vec4(min.x, max.y, min.z, 1.f),
						vec4(min.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, min.z, 1.f) };


	/* simple solution -- does not work for close objects
	for (int i = 0; i < 8; ++i)
	{
		vec4 v = mvp * vertices[i];

		if (v.x >= -v.w && v.x <= v.w &&
			v.y >= -v.w && v.y <= v.w && 
			v.z >= -v.w && v.z <= v.w)
			return INSIDE;

	}

	return OUTSIDE;
	*/





	/* the six planes, based on manually extracting the mvp columns from clip space coordinates, as seen here:
		http://www.lighthouse3d.com/tutorials/view-frustum-culling/clip-space-approach-extracting-the-planes/
		Note that the article's matrix is in row-major format and therefore has to be switched. See also:
		http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
	*/
	vec4 row3 = row(mvp, 3);
	vec4 planes[] = 
	{
		 row(mvp, 0) + row3,
		-row(mvp, 0) + row3,
		 row(mvp, 1) + row3,
		-row(mvp, 1) + row3,
		 row(mvp, 2) + row3,
		-row(mvp, 2) + row3
	};

	
	ClipResult result = INSIDE;
	for (int i = 0; i < 6; ++i)
	{
		int out = 0;
		int in = 0;

		for (int j = 0; j < 8; ++j)
		{
			float d = dot(planes[i], vertices[j]);
			
			if (d < 0.f)
				++out;
			else
				++in;
		}

		if (!in)
			return OUTSIDE;
		else if (out > 0)
			result = INTERSECT;
	}

	return result;

}

#ifndef NO_GRAPHICS
void AABB::draw() const
{
	vec4 vertices[] = {	vec4(min.x, min.y, min.z, 1.f),
						vec4(min.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, min.z, 1.f),
						vec4(min.x, max.y, min.z, 1.f),
						vec4(min.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, min.z, 1.f)};
	
	static const unsigned char indices[] = 
	{
		0,1,1,2,2,3,3,0, 
		4,5,5,6,6,7,7,4,
		0,4,1,5,2,6,3,7
	};


	//glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, 0, value_ptr(vertices[0]));

	glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, indices);

	//glDisableClientState(GL_VERTEX_ARRAY);

}


void AABB::drawSolid() const
{
	vec4 vertices[] = { vec4(min.x, min.y, min.z, 1.f),
						vec4(min.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, min.z, 1.f),
						vec4(min.x, max.y, min.z, 1.f),
						vec4(min.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, min.z, 1.f) };

	static const unsigned char indices[] =
	{
		0,3,2,1,		// bottom
		4,5,6,7,		// top
		1,2,6,5,		// front
		6,2,3,7,		// right
		7,3,0,4,		// back
		0,1,5,4			// left
	};

	glVertexPointer(4, GL_FLOAT, 0, value_ptr(vertices[0]));
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);

}

#endif // NO_GRAPHICS


void AABB::extend(const glm::vec3& pt)
{
	this->min = glm::min(this->min, pt);
	this->max = glm::max(this->max, pt);
}

void AABB::reset()
{
	min = glm::vec3(std::numeric_limits<float>::max());
	max = glm::vec3(std::numeric_limits<float>::lowest());
	//max = glm::vec3(-(FLT_MAX-2.f));	

}

void AABB::reset(const glm::vec3& v)
{
	this->min = v;
	this->max = v;
}

bool AABB::isIntersectedByRay(const glm::vec3& o, const glm::vec3& v) const
{
	// see 'Essential Math ..., 12.3, pg. 567f

	float maxS = 0.f;
	float minT = std::numeric_limits<float>::max();

	// test yz planes
	float s, t;

	vec3 r = glm::vec3(1.f) / v;
	if (r.x >= 0.f)
	{
		s = (min.x - o.x) * r.x;
		t = (max.x - o.x) * r.x;
	}
	else
	{
		s = (max.x - o.x) * r.x;
		t = (min.x - o.x) * r.x;
	}

	maxS = glm::max(s, maxS);
	minT = glm::min(t, minT);

	if (maxS > minT)
		return false;


	// test xz planes
	if (r.y >= 0.f)
	{
		s = (min.y - o.y) * r.y;
		t = (max.y - o.y) * r.y;
	}
	else
	{
		s = (max.y - o.y) * r.y;
		t = (min.y - o.y) * r.y;
	}

	maxS = glm::max(s, maxS);
	minT = glm::min(t, minT);

	if (maxS > minT)
		return false;


	// test xy planes
	if (r.z >= 0.f)
	{
		s = (min.z - o.z) * r.z;
		t = (max.z - o.z) * r.z;
	}
	else
	{
		s = (max.z - o.z) * r.z;
		t = (min.z - o.z) * r.z;
	}

	maxS = glm::max(s, maxS);
	minT = glm::min(t, minT);

	if (maxS > minT)
		return false;


	return true;
}

void AABB::calculate(const std::vector<glm::vec3>& points)
{
	this->reset(points[0]);
	for (size_t i = 1; i < points.size(); ++i)
		this->extend(points[i]);
}

AABB AABB::calculateScreenSpaceBBox(const glm::mat4& mvp) const
{
	vec4 vertices[] = { vec4(min.x, min.y, min.z, 1.f),
						vec4(min.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, max.z, 1.f),
						vec4(max.x, min.y, min.z, 1.f),
						vec4(min.x, max.y, min.z, 1.f),
						vec4(min.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, max.z, 1.f),
						vec4(max.x, max.y, min.z, 1.f) };

	vec3 minBox(std::numeric_limits<float>::max());
	vec3 maxBox(std::numeric_limits<float>::lowest());

	for (int i = 0; i < 8; ++i)
	{
		vec4& v = vertices[i];

		v = mvp * v;
		v /= v.w;
	
		vec3 v2(v);

		minBox = glm::min(minBox, v2);
		maxBox = glm::max(maxBox, v2);
				
		/*
		minBox.x = glm::min(minBox.x, v.x);
		minBox.y = glm::min(minBox.y, v.y);
		

		maxBox.x = glm::max(maxBox.x, v.x);
		maxBox.y = glm::max(maxBox.y, v.y);
		*/
	
	}
	

	AABB result;
	result.min = minBox;
	result.max = maxBox;

	return std::move(result);
}

float AABB::getBoundingSphereRadius() const
{
	float maxRad = 0.f;
	vec3 c = getCentroid();

	vec3 d0 = max - c;
	vec3 d1 = min - c;

	// compare the square lengths
	maxRad = glm::max(dot(d0, d0), dot(d1, d1));

	maxRad = sqrtf(maxRad);
	return maxRad;
}

void AABB::scale(const glm::vec3& s)
{
	vec3 d = getSpan() * s;
	d /= 2;

	min = getCentroid() - d;
	max = getCentroid() + d;

}
