#ifndef FIONA_VOLUME_READER_H_
#define FIONA_VOLUME_READER_H_

#ifndef LINUX_BUILD
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <atomic>

#include <vector>
#include <mutex>

class FionaVolumeReader
{
public:
	FionaVolumeReader();
	~FionaVolumeReader();

	inline const std::vector<std::string> & getAllFileNames() const { return fileList_; }
	inline const std::string & getFirstFileName(void) const {return fileList_[0];}
	void gatherFileNames(const std::string &dir);
	void preallocateMemory(unsigned int numTextures, unsigned long long singleTextureSize);

	inline void lock(void)
	{
#ifndef LINUX_BUILD
		EnterCriticalSection(&queue_mutex_);
#else
		pthread_mutex_lock(&queue_mutex_);
#endif
		locked_ = true;
	}

	inline void unlock(void)
	{
#ifndef LINUX_BUILD
		LeaveCriticalSection(&queue_mutex_);
#else
		pthread_mutex_unlock(&queue_mutex_);
#endif

		locked_ = false;
	}
#ifndef LINUX_BUILD
	DWORD run(void);
#else
	void* run(void* arg);
#endif
	inline void setPaused(bool p) { paused_ = p; }
	inline bool getPaused() { return paused_; };
	void start(unsigned int tIndex = 0);
	inline void stop() { stopThread_ = true; }
	inline bool newImageLoaded(void) { return newLoad_; }
	inline void setNewImageLoaded(bool newLoad) { newLoad_ = newLoad; }
	inline char * getValidPtr(void) {
		if (loadOffset_ == 0) {
			return &mem_[singleFileSize_];
		} else {
			return &mem_[0];
		}
	}
	inline void swapOffset(int nextFileIndex) {
		if (loadOffset_ == 0) {
			loadOffset_ = singleFileSize_;
		} else {
			loadOffset_ = 0;
		}
		fileIndex_ = nextFileIndex;
		//printf("Now loading into %lx\n", &mem_[loadOffset_]);
		//printf("Now valid in     %lx\n", getValidPtr());
	};

//protected:

//private:

	bool locked_;
	std::atomic_bool paused_;
	bool stopThread_;
	bool newLoad_;

	std::vector<std::string> fileList_;
	unsigned int fileIndex_;
	char *	mem_; //  size = singleFileSize_ * 2
	unsigned long long loadOffset_;
	unsigned long long singleFileSize_;

public:
	static std::mutex outputMut;

private:
#ifndef LINUX_BUILD
	CRITICAL_SECTION				queue_mutex_;	//todo - fix for linux
	HANDLE							threadHandle_;
#else
	pthread_mutex_t 				queue_mutex_;
	pthread_t						thread_id_;
#endif
};

#endif