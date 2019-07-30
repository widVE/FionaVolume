#ifndef FIONA_VOLUME_H_
#define FIONA_VOLUME_H_

#define VOLUME_CHANNELS 2

#include "FionaScene.h"
#include "FionaVolumeReader.h"
#include "fromSPIM/Shader.h"

#include "fromSPIM\AABB.h"

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
	virtual void	render(void);
	virtual void	preRender(float value);	//called via frame function callback
	virtual void	buttons(int button, int state);
	virtual void	keyboard(unsigned int key, int x, int y);

	void			drawProxyGeometry(void);
	void			reloadShaders();
	FionaVolumeReader &		getReaderQueue(int index) { return readers_[index]; }

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
	Shader*					drawQuad_;
	Shader*					volumeDifferenceShader_;
	Shader*					drawPosition_;
	Shader*					gpuMultiStackSampler_;

	uint32_t				frameIncrement_ = 0;
	uint32_t				frameCurrents_[VOLUME_CHANNELS];
	SpimStack *				currentStacks_[VOLUME_CHANNELS];
	unsigned int			swap_txs_[VOLUME_CHANNELS];
	bool					visible[VOLUME_CHANNELS];


	Framebuffer*			volumeRenderTarget_;
	Framebuffer*			rayStartTarget_;

	AABB					globalBBox;
};

#endif