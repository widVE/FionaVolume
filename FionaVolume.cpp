#include "FionaVolume.h"

#include "fromSPIM\FreeImage.h"
#include "fromSPIM\SpimStack.h"

#include "fromSPIM\Framebuffer.h"

#include "FionaUT.h"

#include "AntTweakBar.h"

std::string FionaVolume::shaderPath = std::string("../../shaders/");

void uicb(void* data) {
	FionaVolume::renderMode& mode = *(FionaVolume::renderMode*)data;
	mode = (FionaVolume::renderMode)((((uint32_t)mode) + 1) % (uint32_t)FionaVolume::renderMode::renderModeCount);
}

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
volumeRenderTarget_(0)

{
	data.sampleRate = 1.0;
	data.drawMode = 0;
	data.scale[0] = 1.0496f;
	data.scale[1] = 0.5248f;
	data.scale[2] = 0.445;
	//data.scale[0] = 0.001f;
	//data.scale[1] = 0.001f;
	//data.scale[2] = 0.004878f;

	for (int i = 0; i < VOLUME_CHANNELS; i++) {
		currentStacks_[i] = 0;
		frameCurrents_[i] = 0;
		swap_txs_[i] = 0;
		readers_[i].setPaused(true);
		data.color[i][0] = 0.0f;
		data.color[i][1] = 0.0f;
		data.color[i][2] = 0.0f;
		data.color[i][3] = 1.0f;
		data.visible[i] = true;
		data.scaleFactor[i] = 0.1;
	}
	data.color[0][0] = 1.0f;
	if(VOLUME_CHANNELS > 1) data.color[1][1] = 1.0f;
	data.scaleFactor[0] = 20;

	data.playing = 0;
	data.currentFrame = 0;

	reloadShaders();

	volumeRenderTarget_ = new Framebuffer(1024, 1024, GL_RGBA32F, GL_FLOAT, 1, GL_LINEAR);
	frontCoord = new CoordFBO(fionaConf.FBOWidth, fionaConf.FBOHeight);
	backCoord = new CoordFBO(fionaConf.FBOWidth, fionaConf.FBOHeight);

	TwInit(TW_OPENGL, NULL);

	if (!fionaConf.slave) {
		bar = TwNewBar("UIBar");
		TwDefine(" GLOBAL help='' ");
		TwDefine(" UIBar size='200 400' color='96 216 224' ");

		TwAddVarRW(bar, "X Position", TW_TYPE_FLOAT, &data.position[0], "group=Manipulation step=.01");
		TwAddVarRW(bar, "Y Position", TW_TYPE_FLOAT, &data.position[1], "group=Manipulation step=.01");
		TwAddVarRW(bar, "Z Position", TW_TYPE_FLOAT, &data.position[2], "group=Manipulation step=.01");
		//TwAddVarRW(bar, "Orientation", TW_TYPE_QUAT4F, &volumeOri.x, "group=Manipulation");
		TwAddVarRW(bar, "X Scale", TW_TYPE_FLOAT, &data.scale[0], "group=Manipulation min=0.1 max=5 step=.01");
		TwAddVarRW(bar, "Y Scale", TW_TYPE_FLOAT, &data.scale[1], "group=Manipulation min=0.1 max=5 step=.01");
		TwAddVarRW(bar, "Z Scale", TW_TYPE_FLOAT, &data.scale[2], "group=Manipulation min=0.1 max=5 step=.01");
		TwAddVarRW(bar, "Angle X", TW_TYPE_FLOAT, &data.eulerAngles[0], "group=Manipulation min=0 max=360 step=1");
		TwAddVarRW(bar, "Angle Y", TW_TYPE_FLOAT, &data.eulerAngles[1], "group=Manipulation min=0 max=360 step=1");
		TwAddVarRW(bar, "Angle Z", TW_TYPE_FLOAT, &data.eulerAngles[2], "group=Manipulation min=0 max=360 step=1");


		TwAddVarRW(bar, "Sample Rate", TW_TYPE_DOUBLE, &data.sampleRate, "group=View min=0.5 max=4 step=0.5");
		for (int i = 0; i < VOLUME_CHANNELS; i++) TwAddVarRW(bar, ("Channel " + std::to_string(i) + " Visible").c_str(), TW_TYPE_BOOL32, &data.visible[i], "group=View");
		for (int i = 0; i < VOLUME_CHANNELS; i++) TwAddVarRW(bar, ("Channel " + std::to_string(i) + " Color").c_str(), TW_TYPE_COLOR4F, &data.color[i], "group=View");
		for (int i = 0; i < VOLUME_CHANNELS; i++) TwAddVarRW(bar, ("Channel " + std::to_string(i) + " Intensity").c_str(), TW_TYPE_DOUBLE, &data.scaleFactor[i], "group=View");
		TwAddButton(bar, "Cycle Mode", uicb, &data.drawMode, "group=View");
		TwAddVarRO(bar, "Mode: ", TW_TYPE_STDSTRING, &drawModeStr, "group=View");

		TwAddVarRW(bar, "Playing", TW_TYPE_BOOL32, &data.playing, "group=Playback");
		TwAddVarRO(bar, "Frame Index", TW_TYPE_UINT32, &readers_[0].fileIndex_, "group=Playback");
	}

	for (int i = 0; i < VOLUME_CHANNELS; i++) {
		readers_[i].setPaused(false);
	}
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

	if (volumeRenderTarget_ != 0)
	{
		delete volumeRenderTarget_;
		volumeRenderTarget_ = 0;
	}

	if (frontCoord) {
		delete frontCoord;
	}
	if (backCoord) {
		delete backCoord;
	}
}

void FionaVolume::initialize()
{
	//reader_.start();
}

void FionaVolume::drawProxyGeometry(glm::mat4 mvp, glm::mat4 volumeModelMat, glm::ivec3 dimensions) {
	coordinateSpace_->bind();
	coordinateSpace_->setMatrix4("modelMVP", mvp * volumeModelMat);
	glUniform3iv(coordinateSpace_->getUniform("dimensions"), 1, &dimensions.x);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	CoordFBO* fbos[2] = { frontCoord, backCoord };
	for (int i = 0; i < 2; i++) {
		if (i == 0)
			glCullFace(GL_BACK);
		else
			glCullFace(GL_FRONT);

		//fbos[i]->clear();
		fbos[i]->bind();
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//-z face
		glBegin(GL_TRIANGLES);
		glVertex3f(0, 0, 0); glTexCoord3f(0, 0, 0);
		glVertex3f(1, 0, 0); glTexCoord3f(1, 0, 0);
		glVertex3f(1, 1, 0); glTexCoord3f(1, 1, 0);
		glVertex3f(0, 0, 0); glTexCoord3f(0, 0, 0);
		glVertex3f(1, 1, 0); glTexCoord3f(1, 1, 0);
		glVertex3f(0, 1, 0); glTexCoord3f(0, 1, 0);
		glEnd();

		//+z face
		glBegin(GL_TRIANGLES);
		glVertex3f(1, 0, 1); glTexCoord3f(1, 0, 1);
		glVertex3f(0, 0, 1); glTexCoord3f(0, 0, 1);
		glVertex3f(1, 1, 1); glTexCoord3f(1, 1, 1);
		glVertex3f(0, 0, 1); glTexCoord3f(0, 0, 1);
		glVertex3f(0, 1, 1); glTexCoord3f(0, 1, 1);
		glVertex3f(1, 1, 1); glTexCoord3f(1, 1, 1);
		glEnd();

		//-x face
		glBegin(GL_TRIANGLES);
		glVertex3f(0, 0, 0); glTexCoord3f(0, 0, 0);
		glVertex3f(0, 1, 0); glTexCoord3f(0, 1, 0);
		glVertex3f(0, 1, 1); glTexCoord3f(0, 1, 1);
		glVertex3f(0, 0, 0); glTexCoord3f(0, 0, 0);
		glVertex3f(0, 0, 1); glTexCoord3f(0, 0, 1);
		glVertex3f(0, 1, 1); glTexCoord3f(0, 1, 1);
		glEnd();

		//+x face
		glBegin(GL_TRIANGLES);
		glVertex3f(1, 0, 0); glTexCoord3f(1, 0, 0);
		glVertex3f(1, 1, 0); glTexCoord3f(1, 1, 0);
		glVertex3f(1, 1, 1); glTexCoord3f(1, 1, 1);
		glVertex3f(1, 0, 0); glTexCoord3f(1, 0, 0);
		glVertex3f(1, 0, 1); glTexCoord3f(1, 0, 1);
		glVertex3f(1, 1, 1); glTexCoord3f(1, 1, 1);
		glEnd();

		//-y face
		glBegin(GL_TRIANGLES);
		glVertex3f(0, 0, 0); glTexCoord3f(0, 0, 0);
		glVertex3f(1, 0, 0); glTexCoord3f(1, 0, 0);
		glVertex3f(1, 0, 1); glTexCoord3f(1, 0, 1);
		glVertex3f(0, 0, 0); glTexCoord3f(0, 0, 0);
		glVertex3f(0, 0, 1); glTexCoord3f(0, 0, 1);
		glVertex3f(1, 0, 1); glTexCoord3f(1, 0, 1);
		glEnd();

		//+y face
		glBegin(GL_TRIANGLES);
		glVertex3f(0, 1, 0); glTexCoord3f(0, 1, 0);
		glVertex3f(1, 1, 0); glTexCoord3f(1, 1, 0);
		glVertex3f(1, 1, 1); glTexCoord3f(1, 1, 1);
		glVertex3f(0, 1, 0); glTexCoord3f(0, 1, 0);
		glVertex3f(0, 1, 1); glTexCoord3f(0, 1, 1);
		glVertex3f(1, 1, 1); glTexCoord3f(1, 1, 1);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void FionaVolume::loadVolume(int index, const std::string & fileName) {
	for (int i = 0; i < VOLUME_CHANNELS; i++) {
		if (currentStacks_[i] != 0) {
			delete currentStacks_[i];
		}

		for (const auto& fn : readers_[i].getAllFileNames()) {
			//std::cout << "file = " << fn << "\n";
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

void FionaVolume::drawBoundsImmediate(glm::mat4 mvp, glm::mat4 volumeModelMat) {
	volumeSpace_->bind();
	volumeSpace_->setMatrix4("modelMVP", mvp * volumeModelMat);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
	//-z face
	glVertex3f(0, 0, 0); glVertex3f(1, 0, 0);
	glVertex3f(1, 0, 0); glVertex3f(1, 1, 0);
	glVertex3f(1, 1, 0); glVertex3f(0, 1, 0);
	glVertex3f(0, 1, 0); glVertex3f(0, 0, 0);

	//+z face
	glVertex3f(0, 0, 1); glVertex3f(1, 0, 1);
	glVertex3f(1, 0, 1); glVertex3f(1, 1, 1);
	glVertex3f(1, 1, 1); glVertex3f(0, 1, 1);
	glVertex3f(0, 1, 1); glVertex3f(0, 0, 1);

	//edges between z faces
	glVertex3f(0, 0, 0); glVertex3f(0, 0, 1);
	glVertex3f(1, 0, 0); glVertex3f(1, 0, 1);
	glVertex3f(1, 1, 0); glVertex3f(1, 1, 1);
	glVertex3f(0, 1, 0); glVertex3f(0, 1, 1);
	glEnd();
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void FionaVolume::render(void) {
	FionaScene::render();

	//fionaConf.tvFileIndex;
	//_FionaUTSyncSendFileIndex(0, 0);

	if (fionaConf.master) {
		_FionaUTSyncVolume(data);
	}

	drawModeStr = getRenderModeString((renderMode)data.drawMode);

	if (fionaRenderCycleLeft) {
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if (currentStacks_[0]) {
		float mv[16];
		float proj[16];
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
		glGetFloatv(GL_MODELVIEW_MATRIX, mv);
		glm::ivec3 dimensions = currentStacks_[0]->getResolution();
		glm::mat4 glmMV = glm::make_mat4(mv);
		glm::mat4 glmProj = glm::make_mat4(proj);
		glm::mat4 mvp = glmProj * glmMV;
		glm::mat4 imvp = glm::inverse(mvp);
		glm::mat4 ip = glm::inverse(glmProj);
		glm::mat4 volumeRotMat = glm::rotate(data.eulerAngles[2], glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(data.eulerAngles[1], glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(data.eulerAngles[0], glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 volumeModelMat = glm::translate(glm::vec3(data.position[0], data.position[1], data.position[2])) * volumeRotMat * glm::scale(glm::vec3(data.scale[0], data.scale[1], data.scale[2])) * glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f));

		//drawProxyGeometry(mvp, volumeModelMat, dimensions);

		//volumeRenderTarget_->bind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		volumeRaycaster_->bind();
		volumeRaycaster_->setTexture2D("frontCoords", frontCoord->getTex(), 10);
		volumeRaycaster_->setTexture2D("backCoords", backCoord->getTex(), 11);

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
				volumeRaycaster_->setUniform(uniName, data.visible[i] ? 1 : 0);
				sprintf(uniName, "channelColors[%d]", i);
				glUniform4f(volumeRaycaster_->getUniform(uniName), data.color[i][0], data.color[i][1], data.color[i][2], data.color[i][3]);
				sprintf(uniName, "scaleFactor[%d]", i);
				glUniform1d(volumeRaycaster_->getUniform(uniName), data.scaleFactor[i]);
			}
		}
		glActiveTexture(GL_TEXTURE0);


		//glm::vec3 cellSize = glm::vec3(0.41f, 0.41f, 2.0f);
		glUniform3iv(volumeRaycaster_->getUniform("dimensions"), 1, &dimensions.x);
		//volumeRaycaster_->setUniform("invCellSize", 1.0f / cellSize);
		//volumeRaycaster_->setUniform("cellSize", cellSize);
		volumeRaycaster_->setMatrix4("MVP", mvp);
		volumeRaycaster_->setMatrix4("inverseMVP", imvp);
		volumeRaycaster_->setMatrix4("inverseP", ip);
		volumeRaycaster_->setMatrix4("inverseMV", glm::inverse(glmMV));
		volumeRaycaster_->setMatrix4("invVolumeModelMat", glm::inverse(volumeModelMat));
		glUniform1d(volumeRaycaster_->getUniform("sampleRate"), data.sampleRate);
		jvec3 vCavePos = camPos + camOri.rot(fionaConf.headPos);
		volumeRaycaster_->setUniform("camPos", vCavePos.x, vCavePos.y, vCavePos.z);
		volumeRaycaster_->setUniform("renderMode", (int)data.drawMode);

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
		//volumeRenderTarget_->disable();

		//drawTexturedQuad(rayStartTarget_->getColorbuffer());
		//drawTexturedQuad(volumeRenderTarget_->getColorbuffer());
		
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

		// if frameIncrement would be 0 just try to upload the whole frame instead
		uint32_t realTargetIncrement = (frameIncrement_ == 0 ? dimensions.z : frameIncrement_);

		// for benchmarking and dynamic upload limiting
		int totalSlicesThisFrame = 0;

		// keep track of how many channels are ready to swap, if all are ready then perform the swap
		int numReadyToSwap = 0;
		float start = FionaUTTime();
		for (int i = 0; i < VOLUME_CHANNELS; i++) {
			// if the current frame is not being uploaded and a new one is ready, start uploading
			bool startUploading = (readers_[i].newImageLoaded() && frameCurrents_[i] == 0);
			if (startUploading) {
				// unpause, swap buffers to load
				readers_[i].swapOffset(data.currentFrame % readers_[i].fileList_.size());
				readers_[i].setNewImageLoaded(false);
				readers_[i].setPaused(false);
			}

			// if we are starting uploading or currently uploading, load slices
			if (startUploading || frameCurrents_[i] != 0) {

				// next z level to upload to, 
				uint32_t nextFrameCurrent = glm::min(frameCurrents_[i] + realTargetIncrement, (uint32_t)dimensions.z);
				if (nextFrameCurrent == (uint32_t)dimensions.z) {
					totalSlicesThisFrame += (uint32_t)dimensions.z - frameCurrents_[i];
				} else {
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
				}

				if (frameCurrents_[i] == dimensions.z) {
					numReadyToSwap++;
				}
			}
		}
		float end = FionaUTTime();
		//if(totalSlicesThisFrame != 0) std::cout << "slices per second = " << (double)totalSlicesThisFrame / (end - start) << "\n";
		//frameIncrement_ = int((double)realTargetIncrement / (end - start)) / 60;

		if (numReadyToSwap == VOLUME_CHANNELS && data.playing) {
			for (int i = 0; i < VOLUME_CHANNELS; i++) {
				frameCurrents_[i] = 0;
				unsigned int oldTx = currentStacks_[i]->getTexture();
				currentStacks_[i]->setTextureObject(swap_txs_[i]);
				swap_txs_[i] = oldTx;
			}
			if (!fionaConf.slave) {
				data.currentFrame++;
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//drawBoundsImmediate(mvp, volumeModelMat);

		if (!fionaConf.slave) {
			TwRefreshBar(bar);
			TwDraw();
		}
	}

	/*if (FionaRenderCycleRight) {
		glFinish();
	}*/
}

//void FionaVolume::preRender(float value) {
//	printf("preRender(...)\n");
//}

void FionaVolume::postRender(void)
{
	FionaScene::postRender();

	//if (fionaConf.appType == FionaConfig::HEADNODE) {
	//	fionaConf.tvFileIndex = (readers_[0].fileIndex_ + 1) % readers_[0].fileList_.size();
	//	_FionaUTSyncSendFileIndex(fionaConf.tvFileIndex, 0);
	//}
}

void FionaVolume::buttons(int button, int state)
{
	FionaScene::buttons(button, state);

	printf("buttons\n");

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

	/*if (key == 'l' || key == 'L') {
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
	}*/
}

void FionaVolume::mouseCallback(int button, int state, int x, int y) {
	
}

void FionaVolume::mouseMoveCallback(int x, int y) {
	
}

void FionaVolume::passiveMouseMoveCallback(int x, int y) {
	//printf("Mouse move\n");
}

void FionaVolume::updateVolumeState(VolumeData const & data)
{
	this->data = data;
}

void FionaVolume::reshape(int width, int height) {
	frontCoord->resize(width, height);
	backCoord->resize(width, height);
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
	volumeSpace_ = new Shader(shaderPath + "volumeSpace.vert", shaderPath + "volumeSpace.frag", defines);
	coordinateSpace_ = new Shader(shaderPath + "coordinateSpace.vert", shaderPath + "coordinateSpace.frag", defines);

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