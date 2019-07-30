#include "Volume.h"


#include <iostream>
#include "FreeImage.h"

#include <glm/gtx/io.hpp>

using namespace std;
using namespace glm;



static void getStackInfoFromFilename(const std::string& filename, ivec3& res, int& depth)
{
	string s = filename.substr(filename.find_last_of("_") + 1);
	s = s.substr(0, s.find_last_of("."));

	std::cout << "[Debug] " << s << endl;

#ifdef _WIN32
	int result = sscanf_s(s.c_str(), "%dx%dx%d.%dbit", &res.x, &res.y, &res.z, &depth);
	assert(result == 4);
#else
	int result = sscanf_s(s.c_str(), "%dx%dx%d.%dbit", &res.x, &res.y, &res.z, &depth);
	assert(result == 4);
#endif


	std::cout << "[Stack] Read volume info: " << res << ", " << depth << " bits.\n";

}

template<typename T>
void Volume<>::setVoxelDimensions(const glm::vec3& dim)
{
	this->dimensions = dim;

	// update bbox

	cout << "[Stack] Calculating bbox ... ";
	vec3 vol = dimensions * vec3(width, height, depth);
	bbox.min = vec3(0.f); // -vol * 0.5f;
	bbox.max = vol;// *0.5f;
	cout << "done.\n";
}


template<typename T>
Volume<T>* Volume<>::load(const std::string& filename)
{
	const std::string ext = filename.substr(filename.find_last_of(".") + 1);
	std::cout << "[Debug] Extension: " << ext << std::endl;


	Volume<T*> stack = nullptr;

	if (ext == "tiff")
	{
		// find bit depth from image header
		FIMULTIBITMAP* fmb = FreeImage_OpenMultiBitmap(FIF_TIFF, file.c_str(), FALSE, TRUE, 0L, FIF_LOAD_NOPIXELS);

		if (!fmb)
			throw std::runtime_error("Unable to open image \"" + file + "\"!");

		assert(fmb);

		// get the bit depth from the first image
		FIBITMAP* bm = FreeImage_LockPage(fmb, 0);
		int bpp = FreeImage_GetBPP(bm);

		FreeImage_UnlockPage(fmb, 0, FALSE);
		FreeImage_CloseMultiBitmap(fmb);


		std::cout << "[Stack] Loaded found bitmap with depth " << bpp << std::endl;
		if (bpp == 8)
		{
			stack = new Volume < unsigned char > ;
			stack->loadImage(file);
			return stack;
		}
		else if (bpp == 16)
		{
			stack = new SpimStack < unsigned short > ;
			stack->loadImage(file);
			return stack;
		}
	}
	else if (ext == "bin" || ext == "raw")
	{

		ivec3 res(-1);
		int depth = -1;

		getStackInfoFromFilename(file, res, depth);

		if (depth == 8)
			stack = new SpimStackU8;

		else if (depth == 16)
			stack = new SpimStackU16;


		if (!stack)
			throw runtime_error("Invalid bit depth: " + to_string(depth) + "!");

		stack->loadBinary(file, res);

	}
	else
		throw std::runtime_error("Unknown file extension \"" + ext + "\"");

	stack->updateTexture();
	stack->updateStats();

	// don;t forget to set the filename:
	stack->filename = file;

	return stack;
}