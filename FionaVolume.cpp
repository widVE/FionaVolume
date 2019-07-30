#include "FionaVolume.h"

#include "fromSPIM\FreeImage.h"
#include "fromSPIM\SpimStack.h"

#include "fromSPIM\Framebuffer.h"

#include "FionaUT.h"

std::string FionaVolume::shaderPath = std::string("../../shaders/");

FionaVolume::FionaVolume() : FionaScene(),
pointShader_(0),
volumeShader_(0),
sliceShader_(0),
tonemapper_(0),
volumeRaycaster_(0),
drawQuad_(0),
volumeDifferenceShader_(0),
drawPosition_(0),
gpuMultiStackSampler_(0),
volumeRenderTarget_(0),
rayStartTarget_(0)

{
	for (int i = 0; i < VOLUME_CHANNELS; i++) {
		currentStacks_[i] = 0;
		frameCurrents_[i] = 0;
		swap_txs_[i] = 0;
		visible[i] = true;
		readers_[i].setPaused(false);
	}
	reloadShaders();


	volumeRenderTarget_ = new Framebuffer(1024, 1024, GL_RGBA32F, GL_FLOAT, 1, GL_LINEAR);
	rayStartTarget_ = new Framebuffer(512, 512, GL_RGBA32F, GL_FLOAT);
}

FionaVolume::~FionaVolume()
{
	for (int i = 0; i < VOLUME_CHANNELS; i++) {
		readers_[i].setPaused(true);
		readers_[i].stop();
	
		if (currentStacks_[i]) {
			delete currentStacks_[i];
			currentStacks_[i] = 0;
		}
	}


	if (pointShader_ != 0)
	{
		delete	pointShader_;
		pointShader_ = 0;
	}

	if (volumeShader_ != 0)
	{
		delete	volumeShader_;
		volumeShader_ = 0;
	}

	if (sliceShader_ != 0)
	{
		delete	sliceShader_;
		sliceShader_ = 0;
	}

	if (tonemapper_ != 0)
	{
		delete	tonemapper_;
		tonemapper_ = 0;
	}

	if (volumeRaycaster_ != 0)
	{
		delete	volumeRaycaster_;
		volumeRaycaster_ = 0;
	}

	if (drawQuad_ != 0)
	{
		delete	drawQuad_;
		drawQuad_ != 0;
	}

	if (volumeDifferenceShader_ != 0)
	{
		delete	volumeDifferenceShader_;
		volumeDifferenceShader_ = 0;
	}

	if (drawPosition_ != 0)
	{
		delete	drawPosition_;
		drawPosition_ = 0;
	}

	if (gpuMultiStackSampler_ != 0)
	{
		delete	gpuMultiStackSampler_;
		gpuMultiStackSampler_ = 0;
	}

	if (rayStartTarget_ != 0)
	{
		delete rayStartTarget_;
		rayStartTarget_ = 0;
	}

	if (volumeRenderTarget_ != 0)
	{
		delete volumeRenderTarget_;
		volumeRenderTarget_ = 0;
	}
}

void FionaVolume::initialize()
{
	//reader_.start();
}

void FionaVolume::drawProxyGeometry(void) {
	//vp->camera->setup();
	//only draw first channel, all should be the same size
	if (currentStacks_[0]) {
		glm::mat4 mvp;
		//vp->camera->getMVP(mvp);
		float mv[16];
		float proj[16];
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
		glGetFloatv(GL_MODELVIEW_MATRIX, mv);
		glm::mat4 glmMV = glm::make_mat4(mv);
		glm::mat4 glmProj = glm::make_mat4(proj);
		mvp = glmProj * glmMV;

		drawPosition_->bind();
		drawPosition_->setMatrix4("mvp", mvp);

		glEnableClientState(GL_VERTEX_ARRAY);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		// draw back faces
		rayStartTarget_->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		currentStacks_[0]->getBBox().drawSolid();

		rayStartTarget_->disable();

		glCullFace(GL_BACK);
		glDisableClientState(GL_VERTEX_ARRAY);

		glDisable(GL_CULL_FACE);
		drawPosition_->disable();
	}
}

void FionaVolume::loadVolume(int index, const std::string & fileName) {
	for (int i = 0; i < VOLUME_CHANNELS; i++) {
		if (currentStacks_[i] != 0) {
			delete currentStacks_[i];
		}

		for (const auto& fn : readers_[i].getAllFileNames()) {
			std::cout << "file = " << fn << "\n";
		}
		currentStacks_[i] = SpimStack::load(fileName);
		currentStacks_[i]->setVoxelDimensions(glm::vec3(1, 1, 1));
		glm::mat4 m(1.0);
		currentStacks_[i]->setTransform(m);

		glm::ivec3 res = currentStacks_[i]->getResolution();
		unsigned long long tSize = res.x*res.y*res.z*currentStacks_[i]->getBytesPerVoxel();
		readers_[i].preallocateMemory(2, tSize);
		//reader_.setPaused(false);
	}
}

void FionaVolume::drawTexturedQuad(GLuint texture)
{

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	drawQuad_->bind();
	drawQuad_->setTexture2D("colormap", texture);

	glBegin(GL_QUADS);
	glVertex2i(0, 1);
	glVertex2i(0, 0);
	glVertex2i(1, 0);
	glVertex2i(1, 1);
	glEnd();

	drawQuad_->disable();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

}

void FionaVolume::render(void) {
	FionaScene::render();

	if (fionaRenderCycleLeft) {
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if (currentStacks_[0]) {
		drawProxyGeometry();

		glm::mat4 mvp(1.f);
		float mv[16];
		float proj[16];
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
		glGetFloatv(GL_MODELVIEW_MATRIX, mv);
		glm::mat4 glmMV = glm::make_mat4(mv);
		glm::mat4 glmProj = glm::make_mat4(proj);
		mvp = glmProj * glmMV;

		/*printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", mv[0], mv[1], mv[2], mv[3], mv[4], mv[5], mv[6], mv[7], mv[8],
			mv[9], mv[10], mv[11], mv[12], mv[13], mv[14], mv[15]);*/

		glm::mat4 imvp = glm::inverse(mvp);

		volumeRenderTarget_->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		volumeRaycaster_->bind();

		glActiveTexture(GL_TEXTURE0);

		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			if (currentStacks_[i]->getTBO() != 0) {
				//GLint maxSize = 0;
				//glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxSize);
				//printf("Max size: %d\n", maxSize);
				glBindBuffer(GL_TEXTURE_BUFFER, currentStacks_[i]->getTBO());
				glBindTexture(GL_TEXTURE_BUFFER, currentStacks_[i]->getTexture());
				glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, currentStacks_[i]->getTBO());
			} else {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_3D, currentStacks_[i]->getTexture());
				char uniName[64];
				sprintf(uniName, "volumeTex[%d]", i);
				volumeRaycaster_->setUniform(uniName, i);
				sprintf(uniName, "visible[%d]", i);
				volumeRaycaster_->setUniform(uniName, visible[i] ? 1 : 0);
			}
		}
		glActiveTexture(GL_TEXTURE0);

		glm::ivec3 dimensions = currentStacks_[0]->getResolution();
		glm::vec3 cellSize = glm::vec3(0.41f, 0.41f, 2.0f);
		glUniform3iv(volumeRaycaster_->getUniform("dimensions"), 1, &dimensions.r);
		volumeRaycaster_->setUniform("invCellSize", 1.0f / cellSize);
		volumeRaycaster_->setMatrix4("inverseMVP", imvp);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		// draw a screen-filling quad, tex coords will be calculated in shader
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(1, 0);
		glVertex2i(1, 1);
		glVertex2i(0, 1);
		glEnd();

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		
		/*if (currentStack_->getTBO() != 0)
		{
			glBindBuffer(GL_TEXTURE_BUFFER, 0);
		}*/

		volumeRaycaster_->disable();
		volumeRenderTarget_->disable();

		//drawTexturedQuad(rayStartTarget_->getColorbuffer());
		drawTexturedQuad(volumeRenderTarget_->getColorbuffer());
		
		// Ensure that swap textures are created
		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			if (swap_txs_[i] == 0) {
				glGenTextures(1, &swap_txs_[i]);
				glBindTexture(GL_TEXTURE_3D, swap_txs_[i]);

				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				glTexImage3D(GL_TEXTURE_3D, 0, GL_R16UI, dimensions.x, dimensions.y, dimensions.z, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, nullptr);
			}
		}

		uint32_t realTargetIncrement = (frameIncrement_ == 0 ? dimensions.z : frameIncrement_);
		//realTargetIncrement = 4;
		int totalSlicesThisFrame = 0;
		int numReadyToSwap = 0;
		float start = FionaUTTime();
		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			bool startUploading = (readers_[i].newImageLoaded() && frameCurrents_[i] == 0);
			if (startUploading) {
				readers_[i].swapOffset();
				readers_[i].setNewImageLoaded(false);
				readers_[i].setPaused(false);
			}

			if (startUploading || frameCurrents_[i] != 0) {
				uint32_t nextFrameCurrent = glm::min(frameCurrents_[i] + (realTargetIncrement == 0 ? dimensions.z : realTargetIncrement), (uint32_t)dimensions.z);
				if (nextFrameCurrent == (uint32_t)dimensions.z) {
					totalSlicesThisFrame += (uint32_t)dimensions.z - frameCurrents_[i];
				}
				else {
					totalSlicesThisFrame += realTargetIncrement;
				}

				if (frameCurrents_[i] != nextFrameCurrent) {
					glBindTexture(GL_TEXTURE_3D, swap_txs_[i]);
					glTexSubImage3D(GL_TEXTURE_3D, 0,
						0, 0, frameCurrents_[i],
						dimensions.x, dimensions.y, nextFrameCurrent - frameCurrents_[i],
						GL_RED_INTEGER, GL_UNSIGNED_SHORT,
						readers_[i].getValidPtr() + (dimensions.x * dimensions.y * sizeof(unsigned short) * frameCurrents_[i])
					);
					frameCurrents_[i] = nextFrameCurrent;
					if (frameCurrents_[i] == dimensions.z) {
						//printf("Updated channel %d, texture %u has data from address %lx\n", i, swap_txs_[i], readers_[i].getValidPtr());
						//printf("Frame current");
					}
				}

				if (frameCurrents_[i] == dimensions.z) {
					numReadyToSwap++;
				}
			}
		}
		float end = FionaUTTime();
		//if(totalSlicesThisFrame != 0) std::cout << "slices per second = " << (double)totalSlicesThisFrame / (end - start) << "\n";
		//frameIncrement_ = int((double)realTargetIncrement / (end - start)) / 60;

		if (numReadyToSwap == VOLUME_CHANNELS) {
			for (int i = 0; i < VOLUME_CHANNELS; i++) {
				frameCurrents_[i] = 0;
				unsigned int oldTx = currentStacks_[i]->getTexture();
				currentStacks_[i]->setTextureObject(swap_txs_[i]);
				swap_txs_[i] = oldTx;
				//printf("Channel %d now has texture %u\n", i, currentStacks_[i]->getTexture());
			}
			std::cout << "swapped textures yo\n";
		}
	}

	if (FionaRenderCycleRight) {
		glFinish();
	}
}

void FionaVolume::preRender(float value)
{
	FionaScene::preRender(value);
}

void FionaVolume::buttons(int button, int state)
{
	FionaScene::buttons(button, state);

	if (button == 1 && state == 1)
	{
		camPos.y += 5.0f;
	}
	else if (button == 2 && state == 1)
	{
		camPos.y -= 5.0f;
	}
	std::cout << camPos.x << ", " << camPos.y << ", " << camPos.z << "\n";
}

void FionaVolume::keyboard(unsigned int key, int x, int y)
{
	FionaScene::keyboard(key, x, y);

	if (key == 'l' || key == 'L') {
		bool anyRunning = false;
		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			if (!readers_[i].getPaused()) {
				anyRunning = true;
				break;
			}
		}
		if (!anyRunning) {
			for (int i = 0; i < VOLUME_CHANNELS; i++) {
				readers_[i].setPaused(false);
			}
		}
	}
	for (char c = '1'; c < ('1' + VOLUME_CHANNELS); c++) {
		if (key == c) {
			visible[c - '1'] = !visible[c - '1'];
		}
	}
}

void FionaVolume::reloadVolumeShader()
{
	//int numberOfVolumes = std::max(1, (int)stacks.size());

	std::vector<std::pair<std::string, std::string> > defines;

	char sVols[64];
	sprintf(sVols, "%u", 1);// numberOfVolumes);
	std::string snumVolumes(sVols);

	char sRays[64];
	sprintf(sRays, "%u", 1000);// config.raytraceSteps);
	std::string snumRays(sRays);

	defines.push_back(std::make_pair("VOLUMES", snumVolumes));
	defines.push_back(std::make_pair("STEPS", snumRays));

	/*if (tonemapper_ != 0)
	{
		delete tonemapper_;
	}
	tonemapper_ = new Shader(shaderPath + "drawQuad.vert", shaderPath + "tonemapper.frag", defines);

	if (volumeShader_ != 0)
	{
		delete volumeShader_;
	}
	volumeShader_ = new Shader(shaderPath + "volume2.vert", shaderPath + "volume2.frag", defines);
	*/
	if (volumeRaycaster_ != 0)
	{
		delete volumeRaycaster_;
	}
	volumeRaycaster_ = new Shader(shaderPath + "volumeRaycast.vert", shaderPath + "volumeRaycast.frag", defines);

	/*if (volumeDifferenceShader_ != 0)
	{
		delete volumeDifferenceShader_;
	}
	volumeDifferenceShader_ = new Shader(shaderPath + "volumeDist.vert", shaderPath + "volumeDist.frag", defines);

	if (gpuMultiStackSampler_ != 0)
	{
		delete gpuMultiStackSampler_;
	}
	gpuMultiStackSampler_ = new Shader(shaderPath + "samplePlane2.vert", shaderPath + "samplePlane2.frag", defines);*/
}

void FionaVolume::reloadShaders()
{
	reloadVolumeShader();

	/*if (pointShader_ != 0)
	{
		delete pointShader_;
	}
	pointShader_ = new Shader(shaderPath + "points2.vert", shaderPath + "points2.frag");

	if (sliceShader_ != 0)
	{
		delete sliceShader_;
	}
	sliceShader_ = new Shader(shaderPath + "slices.vert", shaderPath + "slices.frag");
	*/
	if (drawQuad_)
	{
		delete drawQuad_;
	}
	drawQuad_ = new Shader(shaderPath + "drawQuad.vert", shaderPath + "drawQuad.frag");

	if (drawPosition_)
	{
		delete drawPosition_;
	}
	drawPosition_ = new Shader(shaderPath + "drawPosition.vert", shaderPath + "drawPosition.frag");

	//glEnable(GL_DEPTH_TEST);
}

