//
//  main.cpp
//  Base Skeleton framework for running our own C++ framework in the CAVE / Dev Lab / Oculus / etc.
//  Adapted from Hyun Joon Shin and Ross Tredinnick's FionaUT project


#define WITH_FIONA
#ifdef WIN32
#include "gl/glew.h"
#else
#include "GL/glew.h"
#endif

#include "FionaUT.h"

#ifdef WIN32
#include "FionaUtil.h"
#endif
#include <Kit3D/glslUtils.h>
#include <Kit3D/glUtils.h>

#ifdef LINUX_BUILD
#define FALSE 0
#endif

#include "FionaVolume.h"

#include "AntTweakBar.h"

class FionaScene;
FionaScene* scene = NULL;

extern bool cmp(const std::string& a, const std::string& b);
extern std::string getst(char* argv[], int& i, int argc);
/*static bool cmp(const char* a, const char* b)
{
	if( strlen(a)!=strlen(b) ) return 0;
	size_t len = MIN(strlen(a),strlen(b));
	for(size_t i=0; i<len; i++)
		if(toupper(a[i]) != toupper(b[i])) return 0;
	return 1;
}
*/
static bool cmpExt(const std::string& fn, const std::string& ext)
{
	std::string extt = fn.substr(fn.rfind('.')+1,100);
	std::cout<<"The extension: "<<extt<<std::endl;
	return cmp(ext.c_str(),extt.c_str());
}


jvec3 curJoy(0.f,0.f,0.f);
jvec3 secondCurJoy(0.f, 0.f, 0.f);
jvec3 pos(0.f,0.f,0.f);
jvec3 secondPos(0.f, 0.f, 0.f);
quat ori(1.f,0.f,0.f,0.f);
quat secondOri(1.f, 0.f, 0.f, 0.f);

//int		calibMode = 0;

void draw5Teapot(void) {

	static GLuint phongShader=0;
	static GLuint teapotList =0;

	if( FionaIsFirstOfCycle() )
	{
		if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
		{
			pos+=jvec3(0,0,curJoy.z*0.01f);
			ori =exp(YAXIS*curJoy.x*0.01f)*ori;
		}
		else if(_FionaUTIsDualViewMachine())
		{
			secondPos+=jvec3(0,0,secondCurJoy.z*0.01f);
			secondOri =exp(YAXIS*secondCurJoy.x*0.01f)*secondOri;
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		glTranslate(-pos);
		glRotate(ori);
	}
	else if(_FionaUTIsDualViewMachine())
	{
		glTranslate(-secondPos);
		glRotate(secondOri);
	}

	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glLight(vec4(0,0,0,1),GL_LIGHT0,0xFF202020);
	glBindTexture(GL_TEXTURE_2D,0);
	jvec3 posSpheres[5]={jvec3(0,0,-1.5),jvec3(-1.5,0,0),jvec3(1.5,0,0),jvec3(0,-1.5,0), jvec3(0,1.5,0)};
	if( teapotList <=0 )
	{
		teapotList = glGenLists(1);
		glNewList(teapotList,GL_COMPILE);
		//glutSolidTeapot(.65f);
		glEndList();
	}
	if( phongShader<=0 )
	{
		glewInit();
		std::string vshader = std::string(commonVShader());
		std::string fshader = coinFShader(PLASTICY,PHONG,false);
		phongShader = loadProgram(vshader,fshader,true);
	}
	glUseProgram(phongShader);
	glEnable(GL_DEPTH_TEST); glMat(0xFFFF8000,0xFFFFFFFF,0xFF404040);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,10);
	for(int i=0;i<3; i++)
	{ 
		glPushMatrix(); 
		glTranslate(posSpheres[i]);
		//glutSolidTeapot(.65f);
		//glCallList(teapotList);
		glSphere(V3ID,.65);
		glPopMatrix();
	}
	glUseProgram(0);
}

void drawSkyTest(void)
{
	if( FionaIsFirstOfCycle() )
	{
		if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
		{
			pos+=jvec3(0,0,curJoy.z*0.01f);
			ori =exp(YAXIS*curJoy.x*0.01f)*ori;
		}
		else if(_FionaUTIsDualViewMachine())
		{
			secondPos+=jvec3(0,0,secondCurJoy.z*0.01f);
			secondOri =exp(YAXIS*secondCurJoy.x*0.01f)*secondOri;
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		glTranslate(-pos);
		glRotate(ori);
	}
	else if(_FionaUTIsDualViewMachine())
	{
		glTranslate(-secondPos);
		glRotate(secondOri);
	}

	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glLight(vec4(0,0,0,1),GL_LIGHT0,0xFF202020);
	glBindTexture(GL_TEXTURE_2D,0);
	jvec3 posSpheres[5]={jvec3(0,0,-1.5),jvec3(-1.5,0,0),jvec3(1.5,0,0),jvec3(0,-1.5,0), jvec3(0,1.5,0)};

	static GLuint phongShader=0;
	if( phongShader<=0 )
	{
		glewInit();
		std::string vshader = std::string("../../shaders/simple.vert");
		std::string fshader = std::string("../../shaders/red.frag");
		phongShader = loadProgramFiles(vshader,fshader,true);
	}
	
	GLint projLoc = glGetUniformLocation(phongShader, "projection");
	GLint viewLoc = glGetUniformLocation(phongShader, "modelview");
	
	glUseProgram(phongShader);
	
	glEnable(GL_DEPTH_TEST); glMat(0xFFFF8000,0xFFFFFFFF,0xFF404040);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,10);
	for(int i=0;i<3; i++)
	{ 
		glPushMatrix(); 
		glTranslate(posSpheres[i]);
		
		GLfloat view[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, view);
		glUniformMatrix4fv(viewLoc, 1, FALSE, view);
		GLfloat proj[16];
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
		glUniformMatrix4fv(projLoc, 1, FALSE, proj);

		//glutSolidSphere(.25, 12, 12);
		//glutSolidTeapot(.65f);
		//glCallList(teapotList);
		glSphere(V3ID,.65);
		glPopMatrix();
	}
	glUseProgram(0);
}

enum APP_TYPE
{
	APP_TAEPOT = 0
} type = APP_TAEPOT;

void wandBtns(int button, int state, int idx)
{
	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		if(scene)
		{
			scene->buttons(button, state);
		}
	}
	else if(_FionaUTIsDualViewMachine())
	{
		if(idx == 1)
		{
			if(scene)
			{
				scene->buttons(button, state);
			}
		}
	}
}

void keyboard(unsigned int key, int x, int y)
{
	TwEventKeyboardGLUT(key, x, y);
	if(scene)
	{
		scene->keyboard(key, x, y);
	}
}

void joystick(int w, const jvec3& v)
{
	if(!fionaConf.dualView || _FionaUTIsSingleViewMachine())
	{
		curJoy = v;
		if(scene) 
		{
			scene->updateJoystick(v);
		}
	}
	else if(_FionaUTIsDualViewMachine())
	{
		//need this sort of construct because we don't want to 
		//update the first viewer's scene if the second joystick is moved...
		if(w == 1)
		{
			secondCurJoy = v;
			if(scene) 
			{
				scene->updateJoystick(v);
			}
		}
	}
}

void mouseBtns(int button, int state, int x, int y)
{
	TwEventMouseButtonGLUT(button, state, x, y);
	if (scene) {
		scene->mouseCallback(button, state, x, y);
	}
}

void mouseMove(int x, int y) 
{
	TwEventMouseMotionGLUT(x, y);
	if (scene) {
		scene->mouseMoveCallback(x, y);
	}
}

void passiveMouseMove(int x, int y) {
	TwEventMouseMotionGLUT(x, y);
	if (scene) {
		scene->passiveMouseMoveCallback(x, y);
	}
}

void tracker(int s,const jvec3& p, const quat& q)
{ 
	if(s==1)
	{
		fionaConf.wandPos = p;
		fionaConf.wandRot = q;
	}
	else if(s == 3)
	{
		fionaConf.secondWandPos = p;
		fionaConf.secondWandRot = q;
	}

	if((s==1 || s==3) && scene)
	{
		scene->updateWand(p,q); 
	}
}

void preDisplay(float value)
{
	if(scene != 0)
	{
		scene->preRender(value);
	}
}

void postDisplay(void)
{
	if(scene != 0)
	{
		scene->postRender();
	}
}

void render(void)
{
	if(scene!=NULL)
	{
		scene->render();
	}
	else 
	{
		draw5Teapot();
	}
}

void cleanup(int errorCode)
{

}

void reshape(int width, int height) {
	TwWindowSize(width, height);
	if (scene) {
		((FionaVolume*)scene)->reshape(width, height);
	}
}

int main(int argc, char *argv[])
{
	std::string fileName;
	std::string dirs[VOLUME_CHANNELS];
	bool dirsLoaded = false;

	glutInit(&argc, argv);

	for(int i=1; i<argc; i++)
	{
		if (strcmp(argv[i],"-fileName")==0)
		{
			++i;
			fileName.append(argv[i]);
		}

		for (int c = 0; c < VOLUME_CHANNELS; c++) {
			dirsLoaded = true;
			char cn[64];
			sprintf(cn, "-directory%d", c);
			if (strcmp(argv[i], cn) == 0) {
				++i;
				dirs[c] = argv[i];
			}
		}
	}
	
	char reqChan[256];
	sprintf(reqChan, "ALL channels (there are %d) must have a directory", VOLUME_CHANNELS);
	for (int c = 0; c < VOLUME_CHANNELS; c++) {
		if (dirs[c].empty()) throw std::runtime_error(reqChan);
	}

	if (!dirsLoaded) throw std::runtime_error("only directory mode supported");

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	
	glutCreateWindow	("Window");

	if (_FionaUTIsCAVEMachine() || fionaConf.appType == FionaConfig::DEVLAB_WIN8)
	{
		::ShowCursor(FALSE);
		::FreeConsole();
	}

	glewInit();

	scene = new FionaVolume();
	if (!dirsLoaded) {
		//((FionaVolume*)scene)->loadVolume(fileName);
	} else {
		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			((FionaVolume*)scene)->getReaderQueue(i).gatherFileNames(dirs[i]);
			((FionaVolume*)scene)->getReaderQueue(i).start();
		}
		((FionaVolume*)scene)->loadInitialVolume();
	}

	scene->navMode = WAND_WORLD;

	glutDisplayFunc		(render);
	glutJoystickFunc	(joystick);
	glutMouseFunc		(mouseBtns);
	glutMotionFunc		(mouseMove);
	glutPassiveMotionFunc(passiveMouseMove);
	glutWandButtonFunc	(wandBtns);
	glutTrackerFunc		(tracker);
	glutKeyboardFunc	(keyboard);
	glutCleanupFunc		(cleanup);
	glutFrameFunc		(preDisplay);
	glutPostRender		(postDisplay);
	glutReshapeFunc		(reshape);

	//freeglut required for these I believe
	//glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	//TwGLUTModifiersFunc(glutGetModifiers);

	TwWindowSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));

	glutMainLoop();

	delete scene;

	return 0;
}
