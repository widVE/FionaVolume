#include "FionaVolumeReader.h"
#include "FionaUT.h"
#include "FionaUtil.h"
#include "fromSPIM/SpimStack.h"
#include <fstream>

std::mutex FionaVolumeReader::outputMut;

FionaVolumeReader::FionaVolumeReader() : locked_(false), stopThread_(false), newLoad_(false), fileIndex_(1), mem_(0), loadOffset_(0), singleFileSize_(0)
{
	::InitializeCriticalSection(&queue_mutex_);
	paused_.store(false);
}

FionaVolumeReader::~FionaVolumeReader()
{

	printf("cleaning up reading thread\n");
	stopThread_ = true;

	FionaUTSleep(1.f);

#ifndef LINUX_BUILD

	if (threadHandle_ != 0)
	{
		::TerminateThread(threadHandle_, 0);
	}

	::DeleteCriticalSection(&queue_mutex_);

#else
	if (single_file_handle != -1)
	{
		close(single_file_handle);
	}

	pthread_join(thread_id, NULL);
#endif
}

#ifndef LINUX_BUILD
unsigned long WINAPI runStub(void *lpParam)
{
	if (!lpParam) return -1;
	return ((FionaVolumeReader*)lpParam)->run();
}
#else
void * runStub(void *lpParam)
{
	return ((FionaVolumeReader*)lpParam)->run(lpParam);
}
#endif

#ifdef LINUX_BUILD
void * FionaVolumeReader::run(void* arg)
#else
DWORD FionaVolumeReader::run()
#endif
{
	while (!stopThread_)
	{
		if (!paused_)
		{
			FILE * f = fopen(fileList_[fileIndex_].c_str(), "r");
			if (f)
			{
				float tStart = FionaUTTime();
				//printf("Opening index %d\n",fileIndex_);
				_fseeki64(f, 0L, SEEK_END);
				uint64_t fileSize = _ftelli64(f);
				_fseeki64(f, 0L, SEEK_SET);
				fread(&mem_[loadOffset_], 1, glm::min(fileSize, singleFileSize_), f);
				//loadOffset_ += fileSize;
				fclose(f);

				//swap endianness
				for (uint64_t i = 0; i < singleFileSize_ / 2; i++) {
					uint16_t &old = ((uint16_t*)&mem_[loadOffset_])[i];
					old = (uint16_t)((old & 0xff00) >> 8) | ((old & 0x00ff) << 8);
				}

				/*if (mem_) {
					unsigned long long dist[65536];
					memset(dist, 0, sizeof(dist));
					for (int i = 0; i < (1024 * 512 * 89); i++) {
						int index = ((uint16_t*)&mem_[loadOffset_])[i];
						if (dist[index] != std::numeric_limits<unsigned long long>::max()) dist[index]++;
					}
					std::ofstream out("hist.csv");
					for (int i = 0; i < 65536; i++) {
						out << (std::to_string(i) + ", " + std::to_string(dist[i]) + "\n");
					}
				}*/
				
				paused_ = true;
				newLoad_ = true;

				//fileIndex_ is now set with swapOffset(...)
				//fileIndex_ = (fileIndex_ + 1) % fileList_.size();

				//std::ifstream file(fileList_[fileIndex_].c_str(), ios::binary);
				//assert(file.is_open());
				//file.read(mem_[loadOffset_], singleFileSize_);

				//cout << "[Stack] Loaded binary volume: " << width << "x" << height << "x" << depth << endl;*/
			}
		}

		FionaUTSleep(0.001f);
	}

	return 0;
}

void FionaVolumeReader::gatherFileNames(const std::string &dir)
{
	if (dir.length() > 0)
	{
		FionaUtil::ListFilesInDirectory(dir.c_str(), fileList_);
		printf("Found %d image files\n", fileList_.size());
	}
}

void FionaVolumeReader::preallocateMemory(unsigned int numTextures, unsigned long long singleTextureSize)
{
	printf("Pre allocated %llu bytes of memory\n", numTextures * singleTextureSize);
	mem_ = new char[numTextures * singleTextureSize];
	memset(mem_, 0, numTextures * singleTextureSize);
	singleFileSize_ = singleTextureSize;
}

void FionaVolumeReader::start(unsigned int tIndex)
{
	//
#ifdef LINUX_BUILD
	pthread_create(&thread_id, 0, &runStub, this);
#else
	DWORD threadId;
	threadHandle_ = CreateThread(NULL, 0, &runStub, this, 0, &threadId);
#endif
}