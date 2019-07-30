#pragma once

#include <vector>
#include <string>
//#include <boost/utility.hpp>
#include <glm/glm.hpp>

#include "AABB.h"
#include "InteractionVolume.h"
#include "TinyStats.h"

template <typename T>
class Volume : public std::vector<T>, public InteractionVolume//, boost::noncopyable
{
public:

	static Volume* load(const std::string& filename);


	inline size_t getIndex(unsigned int x, unsigned int y, unsigned int z) const { return x + y*resolution.x + z*resolution.x*resolution.y; }
	inline size_t getIndex(const glm::ivec3& coords) const { return getIndex(coords.x, coords.y, coords.z); }

	inline unsigned int getWidth() const { return resolution.x; }
	inline unsigned int getHeight() const { return resolution.y; }
	inline unsigned int getDepth() const { return resolution.z; }
	inline size_t getVoxelCount() const { return resolution.x*resolution.y*resolution.z; }
	
	// returns the world position of the given coordinate
	glm::vec3 getWorldPosition(const glm::ivec3& coordinate) const;

	// returns the world position of the given index
	glm::vec3 getWorldPosition(const unsigned int index) const;

	// returns the internal stack voxel coordinates for a given world coordinate
	glm::ivec3 getStackVoxelCoords(const glm::vec4& worldCoords) const;
	inline glm::ivec3 getStackVoxelCoords(const glm::vec3& worldCoords) const { return getStackVoxelCoords(glm::vec4(worldCoords, 1.f)); }

	inline size_t getPlanePixelCount() const { return resolution.x*resolution.y; }



	inline const unsigned int getTexture() const { return volumeTextureId; }

	TinyStats<double> getLimits() const;
	std::vector<size_t> calculateHistogram(const TinyStats<double>& threshold) const;


	glm::ivec3 getStackCoords(size_t index) const;

	inline glm::ivec3 getResolution() const { return glm::ivec3(width, height, depth); }
	inline const glm::vec3& getVoxelDimensions() const { return dimensions; }
	void setVoxelDimensions(const glm::vec3& dimensions);

	inline const std::string& getFilename() const { return filename; }



protected:
	// the physical dimension of each voxel
	glm::vec3				dimension;

	// the volume's resolution
	glm::ivec3				resolution;


	std::string				filename;
	unsigned int			volumeTextureId;



	virtual void updateStats();
	virtual void updateTexture();

	virtual double getValue(size_t index) const = 0;
	virtual double getRelativeValue(size_t index) const = 0;

	virtual void loadImage(const std::string& filename) = 0;
	virtual void loadBinary(const std::string& filename, const glm::ivec3& resolution) = 0;

	virtual void saveBinary(const std::string& filename) = 0;
	virtual void saveImage(const std::string& filename) = 0;


};

typedef Volume<unsigned char> VolumeU8;
typedef Volume<unsigned short> VolumeU16;


