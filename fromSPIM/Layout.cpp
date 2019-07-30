#include "Layout.h"
#include "OrbitCamera.h"
#include "AABB.h"

#include <GL/glew.h>
#include <GL/glut.h>

#include <fstream>
#include <cassert>


using namespace std;
using namespace glm;

const float CAMERA_DISTANCE = 4000.f;
const vec3 CAMERA_TARGET(0.f);

#undef near
#undef far


void Viewport::setup() const
{
	glViewport(position.x, position.y, size.x, size.y);

	// set correct matrices
	assert(camera);
	camera->setup();
	
}

void Viewport::drawBorder() const
{

	glDisable(GL_DEPTH_TEST);
	
	// reset any transform
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, size.x, 0, size.y, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw a border
	if (highlighted)
	{
		glColor3f(color.r, color.g, color.b);
		glBegin(GL_LINE_LOOP);
		glVertex2i(1, 1);
		glVertex2i(size.x, 1);
		glVertex2i(size.x, size.y);
		glVertex2i(1, size.y);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);

}


void Viewport::drawTitle() const
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, size.x, 0, size.y, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();


	glRasterPos2i(10, size.y-20);
	glColor4f(1, 1, 1, 1);
	
	//for (int i = 0; i < name.length(); ++i)
	//	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, name[i]);


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void Viewport::maximizeView(const AABB& bbox)
{
	camera->target = bbox.getCentroid();
	camera->maximizeView(bbox);


}

SwitchableLayoutContainer::SwitchableLayoutContainer(ILayout* a, ILayout* b) : currentLayout(0)
{
	layouts[0] = a;
	layouts[1] = b;
}

SwitchableLayoutContainer::~SwitchableLayoutContainer()
{
	delete layouts[0];
	delete layouts[1];
}



SingleViewLayout::SingleViewLayout(const ivec2& resolution)
{
	viewport.position = ivec2(0);
	viewport.size = resolution;
}

SingleViewLayout::~SingleViewLayout()
{
	delete viewport.camera;
}

Viewport* SingleViewLayout::getActiveViewport()
{
	if (viewport.highlighted)
		return &viewport;
	else
		return nullptr;
}

void SingleViewLayout::updateMouseMove(const glm::ivec2& coords)
{
	if (viewport.isInside(coords))
		viewport.highlighted = true;
	else
		viewport.highlighted = false;
}

void SingleViewLayout::resize(const ivec2& size)
{
	viewport.position = ivec2(0);
	viewport.size = size;
	viewport.camera->aspect = (float)size.x / size.y;
}

void SingleViewLayout::panActiveViewport(const vec2& delta)
{
	if (viewport.highlighted)
		viewport.camera->pan(delta.x * viewport.getAspect(), delta.y);
}

void SingleViewLayout::centerCamera(const glm::vec3& tgt)
{
	viewport.camera->target = tgt;
}

TopViewFullLayout::TopViewFullLayout(const ivec2& resolution) : SingleViewLayout(resolution)
{
	viewport.name = "Ortho Y";
	viewport.color = vec3(1.f);
	viewport.camera = new OrthoCamera(vec3(0.f, -1, 0), vec3(1.f, 0.f, 0.f));
	viewport.camera->aspect = (float)resolution.x / resolution.y;
}


OrthoViewFullLayout::OrthoViewFullLayout(const glm::ivec2& resolution, const std::string& axis) : SingleViewLayout(resolution)
{

	if (axis == "Ortho X")
	{
		viewport.camera = new OrthoCamera(glm::vec3(-1, 0, 0), glm::vec3(0.f, 1.f, 0.f));
		viewport.color = vec3(1, 0, 0);

	}
	else if (axis == "Ortho Y")
	{
		viewport.camera = new OrthoCamera(glm::vec3(0, -1, 0), glm::vec3(1.f, 0.f, 0.f));
		viewport.color = vec3(0, 1, 0);

	}
	else if (axis == "Ortho Z")
	{
		viewport.camera = new OrthoCamera(glm::vec3(0, 0, -1), glm::vec3(0.f, 1.f, 0.f));
		viewport.color = vec3(0, 0, 1);

	}
	else
		throw runtime_error("Invalid viewport/axis id for ortho layout!");

	viewport.name = axis;
	viewport.camera->aspect = (float)resolution.x / resolution.y;
}


PerspectiveFullLayout::PerspectiveFullLayout(const ivec2& resolution) : SingleViewLayout(resolution)
{
	viewport.camera = new OrbitCamera;
	viewport.color = vec3(1.f);
	viewport.name = "Perspective";

	OrbitCamera* cam = dynamic_cast<OrbitCamera*>(viewport.camera);
	cam->far = CAMERA_DISTANCE * 4.f;
	cam->near = cam->far / 100.f;
	cam->aspect = (float)resolution.x / resolution.y;

	cam->radius = (CAMERA_DISTANCE * 1.5f);
	cam->target = CAMERA_TARGET;

}

FourViewLayout::FourViewLayout(const ivec2& size)
{
	views[0].name = "Ortho X";
	views[0].camera = new OrthoCamera(glm::vec3(-1, 0, 0), glm::vec3(0.f, 1.f, 0.f));
	views[0].color = vec3(1, 0, 0);

	views[1].name = "Ortho Y";
	views[1].camera = new OrthoCamera(glm::vec3(0.f, -1, 0), glm::vec3(1.f, 0.f, 0.f));
	views[1].color = vec3(0, 1, 0);

	views[2].name = "Ortho Z";
	views[2].camera = new OrthoCamera(glm::vec3(0, 0.f, -1), glm::vec3(0.f, 1.f, 0.f));
	views[2].color = vec3(0, 0, 1);

	views[3].name = "Perspective";
	views[3].camera = new OrbitCamera;
	views[3].color = vec3(1);

	OrbitCamera* cam = dynamic_cast<OrbitCamera*>(views[3].camera);
	cam->far = CAMERA_DISTANCE * 4.0f;
	cam->near = cam->far / 100.0f;
	cam->radius = (CAMERA_DISTANCE * 1.5f);
	cam->target = CAMERA_TARGET;
	
	FourViewLayout::resize(size);

}

void FourViewLayout::centerCamera(const glm::vec3& pos)
{
	for (int i = 0; i < 4; ++i)
		views[i].camera->target = pos;
}

void FourViewLayout::resize(const ivec2& size)
{
	using namespace glm;

	const float ASPECT = (float)size.x / size.y;
	const ivec2 VIEWPORT_SIZE = size / 2;

	// setup the 4 views
	views[0].position = ivec2(0, 0);
	views[1].position = ivec2(size.x / 2, 0);
	views[2].position = ivec2(0, size.y / 2);
	views[3].position = ivec2(size.x / 2, size.y / 2);



	for (int i = 0; i < 4; ++i)
	{
		views[i].size = VIEWPORT_SIZE;
		views[i].camera->aspect = ASPECT;
	}
}

void FourViewLayout::updateMouseMove(const ivec2& m)
{
	for (int i = 0; i < 4; ++i)
		if (views[i].isInside(m))
			views[i].highlighted = true;
		else
			views[i].highlighted = false;

}

void FourViewLayout::panActiveViewport(const vec2& delta)
{
	Viewport* vp = this->getActiveViewport();
	if (vp)
		vp->camera->pan(delta.x * vp->getAspect(), delta.y);
}

Viewport* FourViewLayout::getActiveViewport()
{
	for (int i = 0; i < 4; ++i)
		if (views[i].highlighted)
			return &views[i];

	return nullptr;
}

void FourViewLayout::maximizeView(const AABB& bbox)
{
	for (int i = 0; i < 4; ++i)
		views[i].maximizeView(bbox);
}

ContrastEditLayout::ContrastEditLayout(const glm::ivec2& res)
{

	views[0].name = "Perspective";
	views[0].camera = new OrbitCamera;
	views[0].color = vec3(1);

	views[1].name = "Histogram";
	views[1].camera = new UnitCamera; // OrthoCamera(glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f), 1.1f);
	views[1].color = vec3(1, 1, 0);
		
	OrbitCamera* cam = dynamic_cast<OrbitCamera*>(views[0].camera);
	cam->far = CAMERA_DISTANCE * 4.0f;
	cam->near = cam->far / 100.0f;
	cam->radius = (CAMERA_DISTANCE * 1.5f);
	cam->target = CAMERA_TARGET;

	
	ContrastEditLayout::resize(res);
}

ContrastEditLayout::~ContrastEditLayout()
{
	delete views[0].camera;
	delete views[1].camera;
}


void ContrastEditLayout::updateMouseMove(const ivec2& m)
{
	for (int i = 0; i < 2; ++i)
		if (views[i].isInside(m))
			views[i].highlighted = true;
		else
			views[i].highlighted = false;

}

Viewport* ContrastEditLayout::getActiveViewport()
{
	if (views[0].highlighted)
		return &views[0];
	else if (views[1].highlighted)
		return &views[1];
	else
		return nullptr;
}

void ContrastEditLayout::resize(const glm::ivec2& size)
{
	using namespace glm;

	int top = (int)(size.y * 0.1f);

	// setup the 2 views
	views[0].position = ivec2(0, 0);
	views[0].size = ivec2(size.x, size.y - top);
	views[0].camera->aspect = views[0].getAspect();

	views[1].position = ivec2(0, size.y - top);
	views[1].size = ivec2(size.x, top);
	views[1].camera->aspect = views[1].getAspect();
}


void ContrastEditLayout::panActiveViewport(const vec2& delta)
{
	Viewport* vp = this->getActiveViewport();
	if (vp)// && vp->name != Viewport::CONTRAST_EDITOR)
		vp->camera->pan(delta.x * vp->getAspect(), delta.y);
}

void ContrastEditLayout::maximizeView(const AABB& bbox)
{
	views[0].maximizeView(bbox);
}