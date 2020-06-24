#ifndef FIONA_VOLUME_H_
#define FIONA_VOLUME_H_

#define VOLUME_CHANNELS 2

#include "FionaScene.h"
#include "FionaVolumeReader.h"
#include "fromSPIM/Shader.h"

#include "fromSPIM\AABB.h"

#include "AntTweakBar.h"

#include "CoordFBO.h"

class SpimStack;
class Framebuffer;

class FionaVolume : public FionaScene
{
public:

	FionaVolume();
	virtual ~FionaVolume();

	void			initialize();
	void			loadVolume(int stackIndex, const std::string & fileName);
	void			loadInitialVolume(void) {
		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			loadVolume(i, readers_[i].getFirstFileName());
		}
	}



	void			drawBoundsImmediate(glm::mat4 mvp, glm::mat4 volumeModelMat);
	virtual void	render(void);
	//virtual void	preRender(float value);	//called via frame function callback
	virtual void	postRender();
	virtual void	buttons(int button, int state);
	virtual void	keyboard(unsigned int key, int x, int y);
	virtual void	mouseCallback(int button, int state, int x, int y);
	virtual void	mouseMoveCallback(int x, int y);
	virtual void	passiveMouseMoveCallback(int x, int y);
	void			updateVolumeState(VolumeData const& data);
	void			reshape(int width, int height);

	void			drawProxyGeometry(glm::mat4 mvp, glm::mat4 volumeModelMat, glm::ivec3 dimensions);
	void			reloadShaders();
	FionaVolumeReader &		getReaderQueue(int index) { return readers_[index]; }

	enum class renderMode : uint32_t {
		renderAccum = 0,
		renderAvg = 1,
		renderMax = 2,

		renderModeCount = 3
	};
	static inline std::string getRenderModeString(renderMode mode) {
		std::string names[(uint32_t)renderMode::renderModeCount] = {
			"Accumulate",
			"Average",
			"Max"
		};
		return names[(uint32_t)mode];
	}

protected:

private:

	void drawTexturedQuad(GLuint texture);

	static std::string shaderPath;

	void reloadVolumeShader();

	FionaVolumeReader	readers_[VOLUME_CHANNELS];

	Shader*					pointShader_;
	Shader*					volumeShader_;
	Shader*					sliceShader_;
	Shader*					tonemapper_;
	Shader*					volumeRaycaster_;
	Shader*					volumeSpace_;
	Shader*					coordinateSpace_;
	Shader*					drawQuad_;
	Shader*					volumeDifferenceShader_;
	Shader*					drawPosition_;
	Shader*					gpuMultiStackSampler_;

	uint32_t				frameIncrement_ = 0;
	uint32_t				frameCurrents_[VOLUME_CHANNELS];
	SpimStack *				currentStacks_[VOLUME_CHANNELS];
	unsigned int			swap_txs_[VOLUME_CHANNELS];

	struct renderParams {
		double intensityFactor = 1.0 / 65536.0;
		int lowerClamp = 0;
		int upperClamp = 65535;
		uint32_t enabled = false;
		vec4 color;
	};
	
	struct slicePlane {
		glm::vec3 pos; //point on plane
		glm::vec3 normal; //vector normal to plane
		glm::vec3 viewerPos;
		glm::vec3 viewerAngles;
		bool transformWithVolume = true; //whether to move with the volume or not
		bool visible = true;
		bool selected = false;
		bool viewerVisible = false;
		renderParams params;
	};

	VolumeData data;
	std::string				drawModeStr;

	Framebuffer*			volumeRenderTarget_;
	CoordFBO*				frontCoord = nullptr;
	CoordFBO*				backCoord = nullptr;

	AABB					globalBBox;

	TwBar* bar;
};

#endif