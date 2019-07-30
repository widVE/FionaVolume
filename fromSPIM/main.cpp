
#include "SpimRegistrationApp.h"
#include "SimplePointcloud.h"
#include "SpimStack.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <vector>
#include <algorithm>

// goddamnit windows! :(
#undef near
#undef far

enum MenuItem
{
	MENU_TYPE_NONE= 0,
	MENU_TYPE_TRANSLATE,
	MENU_TYPE_ROTATE,
	MENU_TYPE_SCALE,
	MENU_MODE_VIEW,
	MENU_MODE_X,
	MENU_MODE_Y,
	MENU_MODE_Z,

	MENU_SELECT_NONE,
	MENU_SELECT_TOGGLE,
	MENU_SELECT_LOCK,
	MENU_SELECT_UNLOCK_ALL,

	MENU_VIEW_FOCUS,
	MENU_VIEW_ALL,
	MENU_VIEW_AUTOCONTRAST,
	MENU_VIEW_PERSPECTIVE,
	MENU_VIEW_FOURVIEW,
	MENU_VIEW_ORTHOX,
	MENU_VIEW_ORTHOY,
	MENU_VIEW_ORTHOZ,
	MENU_VIEW_TOGGLE_GRID,
	MENU_VIEW_TOGGLE_BBOXES,
	MENU_VIEW_PRESENTATION,

	MENU_SOLVER_DX,
	MENU_SOLVER_DY,
	MENU_SOLVER_DZ,
	MENU_SOLVER_RY,
	MENU_SOLVER_HILLCLIMB,
	MENU_SOLVER_ANNEALING,
	MENU_SOLVER_RANDOM_ROTATION,
	MENU_SOLVER_UNIFORM_SCALE,
	MENU_SOLVER_SHOW_SCORE,
	MENU_SOLVER_CLEAR_HISTORY,

	MENU_POINTCLOUD_BAKE_TRANSFORM,
	MENU_POINTCLOUD_SAVE_CURRENT,

	MENU_RESAMPLE_CREATE_EMPTY,
	MENU_RESAMPLE_SELECTED,



	MENU_MISC_RELOAD_CONFIG,
	MENU_MISC_RELOAD_SHADERS,
	MENU_MISC_SUBSAMPLE_ALL,
	MENU_MISC_SAVE_ALL_TRANSFORMS,
	MENU_MISC_RELOAD_ALL_TRANSFORMS
};


struct Mouse
{
	glm::ivec2		coordinates;
	int				button[3];
}				mouse;

enum SpecialKey
{
	Shift = 0,
	Ctrl,
	Alt,
	COUNT
};

bool specialKey[SpecialKey::COUNT];


SpimRegistrationApp*	regoApp = nullptr;


static void updateSpecialKeys()
{
	int specials = glutGetModifiers();
	
	specialKey[Shift] = specials & GLUT_ACTIVE_SHIFT;
	specialKey[Ctrl] = specials & GLUT_ACTIVE_CTRL;
	specialKey[Alt] = specials & GLUT_ACTIVE_ALT;
	
	//std::cout << "[Debug] Specials: Shift: " << specialKey[Shift] << ", Ctrl: " << specialKey[Ctrl] << ", Alt: " << specialKey[Alt] << std::endl;
}

static void menu(int item)
{

	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	const glm::ivec2 winRes(w, h);


	switch ((MenuItem)item)
	{
	case MENU_TYPE_NONE:
		regoApp->setWidgetType("None");
		break;
	case MENU_TYPE_TRANSLATE:
		regoApp->setWidgetType("Translate");
		break;
	case MENU_TYPE_ROTATE:
		regoApp->setWidgetType("Rotate");
		break;
	case MENU_TYPE_SCALE:
		regoApp->setWidgetType("Scale");
		break;

	case MENU_MODE_VIEW:
		regoApp->setWidgetMode("View");
		break;

	case MENU_MODE_X:
		regoApp->setWidgetMode("X");
		break;
	case MENU_MODE_Y:
		regoApp->setWidgetMode("Y");
		break;
	case MENU_MODE_Z:
		regoApp->setWidgetMode("Z");
		break;
		
	case MENU_SELECT_NONE:
		regoApp->deselectAll();
		break;
	case MENU_SELECT_TOGGLE:
		regoApp->toggleCurrentVolume();
		break;
	case MENU_SELECT_UNLOCK_ALL:
		regoApp->unlockAllVolumes();
		break;

	case MENU_VIEW_FOCUS:
		regoApp->centerCamera();
		break;
	case MENU_VIEW_ALL:
		regoApp->maximizeViews();
		break;
	case MENU_VIEW_AUTOCONTRAST:
		regoApp->autoThreshold();
		break;
	case MENU_VIEW_PERSPECTIVE:
	case MENU_VIEW_FOURVIEW:
		regoApp->setLayout("MainView", winRes, mouse.coordinates);
		break;
	case MENU_VIEW_ORTHOX:
		regoApp->setLayout("OrthoX", winRes, mouse.coordinates);
		break;
	case MENU_VIEW_ORTHOY:
		regoApp->setLayout("OrthoY", winRes, mouse.coordinates);
		break;
	case MENU_VIEW_ORTHOZ:
		regoApp->setLayout("OrthoZ", winRes, mouse.coordinates);
		break;
	case MENU_VIEW_TOGGLE_GRID:
		regoApp->toggleGrid();
		break;
	case MENU_VIEW_TOGGLE_BBOXES:
		regoApp->toggleBboxes();
		break;
	case MENU_VIEW_PRESENTATION:
		regoApp->toggleRotateCamera();


	case MENU_SOLVER_DX:
		regoApp->selectSolver("DX");
		break;
	case MENU_SOLVER_DY:
		regoApp->selectSolver("Uniform DY");
		break;
	case MENU_SOLVER_DZ:
		regoApp->selectSolver("Uniform DZ");
		break;
	case MENU_SOLVER_RY:
		regoApp->selectSolver("Uniform RY");
		break;
	case MENU_SOLVER_ANNEALING:
		regoApp->selectSolver("Simulated Annealing");
		break;
	case MENU_SOLVER_HILLCLIMB:
		regoApp->selectSolver("Hillclimb");
		break;
	case MENU_SOLVER_RANDOM_ROTATION:
		regoApp->selectSolver("Random Rotation");
	case MENU_SOLVER_UNIFORM_SCALE:
		regoApp->selectSolver("Uniform Scale");
	case MENU_SOLVER_CLEAR_HISTORY:
		regoApp->clearHistory();
		break;
	case MENU_SOLVER_SHOW_SCORE:
		regoApp->toggleHistory();
		break;


	case MENU_POINTCLOUD_BAKE_TRANSFORM:
		regoApp->bakeSelectedTransform();
		break;
	case MENU_POINTCLOUD_SAVE_CURRENT:
		regoApp->saveCurrentPointcloud();
		break;
		
	case MENU_RESAMPLE_CREATE_EMPTY:
		regoApp->createEmptyFullSizedStack();
		break;
	case MENU_RESAMPLE_SELECTED:
		regoApp->resampleSelectedStack();
		break;

	case MENU_MISC_RELOAD_CONFIG:
		regoApp->reloadConfig();
		break;
	case MENU_MISC_RELOAD_SHADERS:
		regoApp->reloadShaders();
		break;
	
	case MENU_MISC_SUBSAMPLE_ALL:
		regoApp->subsampleAllStacks();
		break;

	case MENU_MISC_SAVE_ALL_TRANSFORMS:
		regoApp->saveStackTransformations();
		break;

	case MENU_MISC_RELOAD_ALL_TRANSFORMS:
		regoApp->loadStackTransformations();
		break;

	default:

		std::cout << "[Debug] " << (MenuItem)item << " is not a valid menu entry.\n";

		break;
	}


}

static void createRightClickMenu()
{
	
	int selection = glutCreateMenu(menu);
	//glutAddMenuEntry("---[Selection]----", -1);
	glutAddMenuEntry("Select None    [d]", MENU_SELECT_NONE);
	glutAddMenuEntry("Toggle selected[v]", MENU_SELECT_TOGGLE);

	glutAddMenuEntry("Lock selected  [l]", MENU_SELECT_LOCK);
	glutAddMenuEntry("Unlock all     [L]", MENU_SELECT_UNLOCK_ALL);


	int transformation = glutCreateMenu(menu);
	glutAddMenuEntry("---[Transform]---", -1);
	glutAddMenuEntry("None", MENU_TYPE_NONE);
	glutAddMenuEntry("Translate     [t]", MENU_TYPE_TRANSLATE);
	glutAddMenuEntry("Rotate        [r]", MENU_TYPE_ROTATE);
	glutAddMenuEntry("Scale         [s]", MENU_TYPE_SCALE);
	glutAddMenuEntry("-----[Mode]------", -1);
	glutAddMenuEntry("View Relative [v]", MENU_MODE_VIEW);
	glutAddMenuEntry("Lock X        [x]", MENU_MODE_X);
	glutAddMenuEntry("Lock Y        [y]", MENU_MODE_Y);
	glutAddMenuEntry("Lock Z        [z]", MENU_MODE_Z);


	int layout = glutCreateMenu(menu);
	//glutAddMenuEntry("----[Layout]-----", -1);
	glutAddMenuEntry("Perspective       [F1]", MENU_VIEW_PERSPECTIVE);
	glutAddMenuEntry("Four view         [F1]", MENU_VIEW_FOURVIEW);
	glutAddMenuEntry("Along X           [F2]", MENU_VIEW_ORTHOX);
	glutAddMenuEntry("Along Y (top down)[F3]", MENU_VIEW_ORTHOY);
	glutAddMenuEntry("Along Z           [F4]", MENU_VIEW_ORTHOZ);
	
	int view = glutCreateMenu(menu);
	glutAddMenuEntry("Auto Contrast [Shift][c]", MENU_VIEW_AUTOCONTRAST);
	glutAddMenuEntry("Focus on selected    [f]", MENU_VIEW_FOCUS);
	glutAddMenuEntry("Zoom on selected     [m]", MENU_VIEW_ALL);
	glutAddMenuEntry("Toggle bounding boxes[b]", MENU_VIEW_TOGGLE_BBOXES);
	glutAddMenuEntry("Toggle grid          [g]", MENU_VIEW_TOGGLE_GRID);
	glutAddMenuEntry("Toggle auto rotate   [R]", MENU_VIEW_PRESENTATION);

	//int solver = glutCreateMenu(menu);
	glutAddMenuEntry("Uniform DX         [F5]", MENU_SOLVER_DX);
	glutAddMenuEntry("Uniform DY         [F6]", MENU_SOLVER_DY);
	glutAddMenuEntry("Uniform DZ         [F7]", MENU_SOLVER_DZ);
	glutAddMenuEntry("Uniform RY         [F8]", MENU_SOLVER_RY);
	glutAddMenuEntry("Uniform Scale      [F9]", MENU_SOLVER_UNIFORM_SCALE);
	glutAddMenuEntry("Multidim Hillclimb [F10]", MENU_SOLVER_HILLCLIMB);
	glutAddMenuEntry("Random Rotation    [F11]", MENU_SOLVER_RANDOM_ROTATION);
	glutAddMenuEntry("Sim Annealing           ", MENU_SOLVER_ANNEALING);

	glutAddMenuEntry("Show image score   [h]", MENU_SOLVER_SHOW_SCORE);
	glutAddMenuEntry("Clear score history[H]", MENU_SOLVER_CLEAR_HISTORY);


	int resampling = glutCreateMenu(menu);
	glutAddMenuEntry("Create empty stack [e]", MENU_RESAMPLE_CREATE_EMPTY);
	glutAddMenuEntry("Resample selected stack", MENU_RESAMPLE_SELECTED);


	
	int pointclouds = glutCreateMenu(menu);
	glutAddMenuEntry("Bake transform ", MENU_POINTCLOUD_BAKE_TRANSFORM);
	glutAddMenuEntry("Save pointcloud", MENU_POINTCLOUD_SAVE_CURRENT);

	int misc = glutCreateMenu(menu);
	glutAddMenuEntry("Reload config                  [c]", MENU_MISC_RELOAD_CONFIG);
	glutAddMenuEntry("Reload shaders          [Shift][s]", MENU_MISC_RELOAD_SHADERS);
	glutAddMenuEntry("Subsample all stacks           [u]", MENU_MISC_SUBSAMPLE_ALL);
	glutAddMenuEntry("Save all registrations   [Ctrl][S]", MENU_MISC_SAVE_ALL_TRANSFORMS);
	glutAddMenuEntry("Reload all registrations [Ctrl][L]", MENU_MISC_RELOAD_ALL_TRANSFORMS);


	glutCreateMenu(menu);
	glutAddSubMenu("Selection", selection);
	glutAddSubMenu("Transformation", transformation);
	glutAddSubMenu("Layout", layout);
	glutAddSubMenu("View", view);
	glutAddSubMenu("Resample", resampling);
	//glutAddSubMenu("Solver", solver);
	//glutAddSubMenu("Point clouds", pointclouds);
	glutAddSubMenu("Misc", misc);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

}

static int queryVRAM(bool total)
{
	std::string glExtensionsString_(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
	std::string glVendorString_(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
	int availableTexMem = -1;
	bool nVidia = true;

	if (glVendorString_.find("ATI") != std::string::npos)
	{
		nVidia = false;
	}

	if (total)
	{
		if (nVidia) {
			if (glExtensionsString_.find("GL_NVX_gpu_memory_info") != std::string::npos){
				GLint retVal;
				glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &retVal);
				availableTexMem = static_cast<int>(retVal);
			}
			else {
				printf("No GL_NVX_gpu_memory_info support!!!");
			}
		}
		else {
			//ATI
			if (glExtensionsString_.find("GL_ATI_meminfo") != std::string::npos) {
				GLint retVal[4];
				glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, retVal);
				availableTexMem = static_cast<int>(retVal[0]); //0=total 1=availabel
			}
			else {
				printf("No GL_ATI_meminfo support");
			}
		}

		return availableTexMem;
	}

	if (nVidia) {
		if (glExtensionsString_.find("GL_NVX_gpu_memory_info") != std::string::npos){
			GLint retVal;
			int memFlag = 0;
			glGetIntegerv(memFlag, &retVal);
			availableTexMem = static_cast<int>(retVal);
		}
		else {
			printf("No GL_NVX_gpu_memory_info support!!!");
		}
	}
	else {
		if (glExtensionsString_.find("GL_ATI_meminfo") != std::string::npos) {

			GLint retVal[4];
			glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, retVal);
			availableTexMem = static_cast<int>(retVal[1]); //0=total 1=availabel
		}
		else {
			printf("No GL_ATI_meminfo support");
		}
	}

	return availableTexMem;
}




static void display()
{
		
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
	regoApp->draw();
	
	glutSwapBuffers();

}

static void idle()
{

	unsigned int time = glutGet(GLUT_ELAPSED_TIME);
	static unsigned int oldTime = 0;

	float dt = (float)(time - oldTime) / 1000.f;
	oldTime = time;
	
	regoApp->update(dt);
	//regoApp->setCameraMoving(false);

	glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
	updateSpecialKeys();
	//std::cout << "[Debug] " << key << " (" << (int)key << ")\n";
	

	// ctrl-s
	if (key == 19)
		regoApp->saveStackTransformations();

	if (key == 27)
		exit(0);

	if (key == '/')
		regoApp->toggleSlices();

	if (key == 'g')
		regoApp->toggleGrid();
	
	if (key == 'G')
		regoApp->centerGrid();

	if (key == 'b')
		regoApp->toggleBboxes();
	
	if (key == 'p')
		regoApp->togglePhantoms();
	if (key == 'P')
		regoApp->alignPhantoms();

	// backspace
	if (key == '\b')
		regoApp->undoLastTransform();

	if (key == ' ')
		regoApp->beginAutoAlign();
	
	if (key == '\t')
		regoApp->beginMultiAutoAlign();

	if (key == 'a')
		regoApp->runAlignmentOnce();

	if (key == 'R')
		regoApp->toggleRotateCamera();


	if (key == 'l')
	{
		if (specialKey[Ctrl])
			regoApp->loadStackTransformations();
		else
			regoApp->toggleCurrentVolumeLock();

	}
	if (key == 'L')
		regoApp->unlockAllVolumes();



	if (key == 'e')
		//regoApp->createEmptyRandomStack();
		regoApp->createEmptyFullSizedStack();



	if (key == 'h')
		regoApp->toggleHistory();

	if (key == 'H')
		regoApp->clearHistory();
	
	if (key == ',')
		regoApp->decreaseMinThreshold();
	if (key == '.')
		regoApp->increaseMinThreshold();

	if (key == '<')
		regoApp->decreaseMaxThreshold();
	if (key == '>')
		regoApp->increaseMaxThreshold();
	
	if (key == 'C')
		regoApp->autoThreshold();
	
	if (key == 'c')
	{
		regoApp->reloadConfig();



	/*
		if (specialKey[Alt])
			regoApp->contrastEditorResetThresholds();
		else
			regoApp->contrastEditorApplyThresholds();
	*/
	}

	if (key == ';')
		regoApp->decreaseSliceCount();
	if (key == '\'')
		regoApp->increaseSliceCount();
		

	if (key == 'S')
		regoApp->reloadShaders();

	if (key == 'q')
		regoApp->toggleSolutionParameterSpace();
	if (key == 'Q')
		regoApp->clearSolutionParameterSpace();







	if (key == 't')
		regoApp->setWidgetType("Translate");
	if (key == 'r')
		regoApp->setWidgetType("Rotate");
	if (key == 's')
		regoApp->setWidgetType("Scale");

	if (key == 'x')
		regoApp->setWidgetMode("X");
	if (key == 'y')
		regoApp->setWidgetMode("Y");
	if (key == 'z')
		regoApp->setWidgetMode("Z");







	if (key == 'u')
		regoApp->subsampleAllStacks();
		

	if (key == '1')
		regoApp->toggleSelectVolume(0);

	if (key == '2')
		regoApp->toggleSelectVolume(1);

	if (key == '3')
		regoApp->toggleSelectVolume(2);

	if (key == '4')
		regoApp->toggleSelectVolume(3);

	if (key == '5')
		regoApp->toggleSelectVolume(4);

	if (key == '6')
		regoApp->toggleSelectVolume(5);
	
	if (key == '7')
		regoApp->toggleSelectVolume(6);

	if (key == '8')
		regoApp->toggleSelectVolume(7);

	if (key == '9')
		regoApp->toggleSelectVolume(8);

	if (key == '!')
		regoApp->clearSampleStack();

	if (key == '@')
		regoApp->startSampleStack(1);
	if (key == '#')
		regoApp->startSampleStack(2);
	if (key == '$')
		regoApp->startSampleStack(3);
	if (key == '%')
		regoApp->startSampleStack(4);
	if (key == '^')
		regoApp->startSampleStack(5);
	if (key == '&')
		regoApp->startSampleStack(6);
	if (key == '8')
		regoApp->startSampleStack(7);
	if (key == '(')
		regoApp->startSampleStack(8);
	if (key == ')')
		regoApp->startSampleStack(9);



	/*
	if (key == 'T')
		regoApp->loadStackTransformations();

	if (key == 't')
	{

		if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
			regoApp->clearStackTransformations();
		else
			regoApp->saveStackTransformations();
	}

	*/

	if (key == 'm')
		regoApp->maximizeViews();


	if (key == 'v')
		regoApp->toggleCurrentVolume();
	if (key == 'V')
		regoApp->toggleAllVolumes();


	if (key == '-')
		regoApp->zoomCamera(1.4f);
	if (key == '=' || key == '+')
		regoApp->zoomCamera(0.7f);
	if (key == 'f')
		regoApp->centerCamera();

	/*
	if (key == 'w')
		regoApp->panCamera(glm::vec2(0, 10));
	if (key == 's')
		regoApp->panCamera(glm::vec2(0, -10));
	if (key == 'a')
		regoApp->panCamera(glm::vec2(-10, 0));
	if (key == 'd')
		regoApp->panCamera(glm::vec2(10, 0));
	*/
}

static void keyboardUp(unsigned char key, int x, int y)
{
	updateSpecialKeys();

	if (key == ' ')
		regoApp->endAutoAlign();


	if (key == '\t')
		regoApp->endAutoAlign(); 

	if (key == 'x' || key == 'y' || key == 'z')
		regoApp->setWidgetMode("View");
}


static void motion(int x, int y)
{

	updateSpecialKeys();

	float dt = (mouse.coordinates.y - y) * 0.1f;
	float dp = (mouse.coordinates.x - x) * 0.1f;

	//camera.rotate(dt, dp);

	float dx = (x - mouse.coordinates.x) * 2.f;
	float dy = (y - mouse.coordinates.y) * -2.f;

	mouse.coordinates.x = x;
	mouse.coordinates.y = y;
	

	//std::cout << "x " << x << " y " << y << " dx " << dx << " dy " << dy << std::endl;

	int h = glutGet(GLUT_WINDOW_HEIGHT);
	
	if (mouse.button[1])
		regoApp->panCamera(glm::vec2(dx, dy));
		
	if (mouse.button[0])
	{
		// alt -- rotate
		if (specialKey[Alt])
			regoApp->rotateCamera(glm::vec2(dt, dp));

		// shift -- pan
		else if (specialKey[Shift])
			regoApp->panCamera(glm::vec2(dx, dy));


		// interact with data
		else
		{

			if (specialKey[Ctrl])
				regoApp->updateStackMove(mouse.coordinates, 10.f);
			else
				regoApp->updateStackMove(mouse.coordinates);

			regoApp->inspectOutputImage(glm::ivec2(x, h - y));

		}
	

	}

	regoApp->updateMouseMotion(glm::ivec2(x, h - y));
}

static void passiveMotion(int x, int y)
{
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	regoApp->updateMouseMotion(glm::ivec2(x, h - y));
}

static void special(int key, int x, int y)
{
	//std::cout << "[Debug] Special: " << key << std::endl;



	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	const glm::ivec2 winRes(w, h);

	if (key == GLUT_KEY_F1)
		regoApp->setLayout("MainView", winRes, mouse.coordinates);

	if (key == GLUT_KEY_F2)
		regoApp->setLayout("OrthoX", winRes, mouse.coordinates);
	if (key == GLUT_KEY_F3)
		regoApp->setLayout("OrthoY", winRes, mouse.coordinates);
	if (key == GLUT_KEY_F4)
		regoApp->setLayout("OrthoZ", winRes, mouse.coordinates);


	if (key == GLUT_KEY_F5)
		regoApp->selectSolver("Uniform DX");
	if (key == GLUT_KEY_F6)
		regoApp->selectSolver("Uniform DY");
	if (key == GLUT_KEY_F7)
		regoApp->selectSolver("Uniform DZ");
	if (key == GLUT_KEY_F8)
		regoApp->selectSolver("Uniform RY");
	if (key == GLUT_KEY_F9)
		regoApp->selectSolver("Uniform Scale");
	if (key == GLUT_KEY_F10)
		regoApp->selectSolver("Hillclimb");
	if (key == GLUT_KEY_F11)
		regoApp->selectSolver("Random Rotation");

	

	if (key == GLUT_KEY_UP)
	{
		float d = 1.f;

		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			d = 10.f;
		else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			d = 0.1f;

		regoApp->moveCurrentStack(glm::vec2(0, d));
	}

	if (key == GLUT_KEY_DOWN)
	{

		float d = 1.f;

		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			d = 10.f;
		else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			d = 0.1f;

		regoApp->moveCurrentStack(glm::vec2(0, -d));
	}

	if (key == GLUT_KEY_LEFT)
	{

		float d = 1.f;

		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			d = 10.f;
		else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			d = 0.1f;

		regoApp->moveCurrentStack(glm::vec2(-d, 0));

	}

	if (key == GLUT_KEY_RIGHT)
	{
		float d = 1.f;

		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
			d = 10.f;
		else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			d = 0.1f;

		regoApp->moveCurrentStack(glm::vec2(d, 0));

	}
	
}

static void button(int button, int state, int x, int y)
{
	assert(regoApp);

	//std::cout << "Button: " << button << " state: " << state << " x " << x << " y " << y << std::endl;

	mouse.coordinates.x = x;
	mouse.coordinates.y = y;

	mouse.button[button] = (state == GLUT_DOWN);

	if (button == 2 && state == GLUT_UP)
		regoApp->setCameraMoving(false);

	if (button == 0 && state == 0)
		regoApp->startStackMove(glm::ivec2(x,y));
	if (button == 0 && state == 1)
		regoApp->endStackMove(glm::ivec2(x, y));
}

static void reshape(int w, int h)
{
	regoApp->resize(glm::ivec2(w, h));
}

static void cleanup()
{
	delete regoApp;

#ifdef _WIN32
	//system("pause");
#endif


}

static inline std::string extractPath(const std::string& path)
{
	return path.substr(0, path.find_last_of("\\"));
}

int main(int argc, const char** argv)
{
	
	if (argc < 2)
	{
		std::cerr << "[Error] No filename given!\n";
		std::cerr << "[Usage] " << argv[0] << " <spimfile>\n";
		return -1;
	}
	

	
	glutInit(&argc, const_cast<char**>(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("SpimViewer");
	glewInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutMouseFunc(button);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutPassiveMotionFunc(passiveMotion);
	glutSpecialFunc(special);
	

	createRightClickMenu();

	glEnable(GL_DEPTH_TEST);
	glPointSize(2.f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
			

	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);


	atexit(cleanup);

	
	try
	{
		regoApp = new SpimRegistrationApp(glm::ivec2(1024, 768));

		// set the shader path based on the exe's location
		regoApp->setShaderPath(extractPath(argv[0]) + "/shaders/");


		// load global config
		regoApp->loadConfig(extractPath(argv[0]) + "/config.cfg");

		// try to load the local config
		try
		{
			regoApp->loadConfig(extractPath(argv[1]) + "/config.cfg");
		}
		catch (const std::runtime_error& e)
		{
			std::cerr << "[Error] Unable to load local config.\n";
			std::cerr << "[Error] " << e.what() << std::endl;
		}
		
		
		for (int i = 1; i < argc; ++i)
		{
			regoApp->addSpimStack(argv[i]);
		}
		

		/*
		regoApp->addSpimStack("e:/spim/121914 Fish4 no beads_aligned_by_jayne/spim_TL01_Angle0.ome.tiff");
		regoApp->addSpimStack("e:/spim/121914 Fish4 no beads_aligned_by_jayne/spim_TL01_Angle1.ome.tiff");
		regoApp->addSpimStack("e:/spim/121914 Fish4 no beads_aligned_by_jayne/spim_TL01_Angle2.ome.tiff");
		*/


		/*
		SpimStack* s0 = SpimStack::load("e:/spim/Mousetail/5xz2mousetailclearnegtivedegree144.tif");
		SpimStack* s1 = SpimStack::load("e:/spim/Mousetail/5xz2mousetailclearnegtivedegree216.tif");
		s0->reslice(0, s0->getDepth() / 2); 
		s1->reslice(0, s1->getDepth() / 2);

		regoApp->addSpimStack(s0);
		regoApp->addSpimStack(s1);


		regoApp->loadConfig("e:/spim/Mousetail/config.cfg");
		*/


		regoApp->loadStackTransformations();

		
		regoApp->centerCamera();
		regoApp->maximizeViews();

		regoApp->centerGrid();

		regoApp->reloadShaders();

		glutMainLoop();

	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "[Error] " << e.what() << std::endl;
		system("pause");
		return 1;

	}
	catch (const std::string& e)
	{
		std::cerr << "[Error] " << e << std::endl;
		system("pause"); 
		return 2;
	}
	catch (...)
	{
		std::cerr << "[Error] Unknown error.\n";
		system("pause"); 
		return -1;
	}

	return 0;
}

