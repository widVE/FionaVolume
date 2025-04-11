#pragma once

#include <glm/glm.hpp>
//#include <boost/utility.hpp>
#include <string>

#include "AABB.h"

class InteractionVolume// : boost::noncopyable
{
public:
	bool			enabled;
	bool			locked;

	InteractionVolume();
	virtual ~InteractionVolume() {};

	inline const AABB& getBBox() const { return bbox; };
	virtual AABB getTransformedBBox() const;

	inline void toggle() { enabled = !enabled; }
	inline void toggleLock() { locked = !locked; }

	virtual void drawLocked() const;


	virtual void saveTransform(const std::string& filename) const;
	virtual bool loadTransform(const std::string& filename);
	inline void applyTransform(const glm::mat4& t) { setTransform(t * transform); }

	/// sets the rotation around its center in degrees
	virtual void setRotation(float angle);
	virtual void move(const glm::vec3& delta);
	/// rotate around its center in d degrees
	virtual void rotate(float d);
	virtual void scaleUniform(float s);




	inline const glm::vec3& getForward() const { return glm::vec3(transform[0]); }
	inline const glm::vec3& getUp() const { return glm::vec3(transform[1]); }
	inline const glm::vec3& getRight() const { return glm::vec3(transform[2]); }


	inline const glm::mat4& getTransform() const { return transform; }
	inline const glm::mat4& getInverseTransform() const { return inverseTransform; }

	void setTransform(const glm::mat4& t);
	
	inline bool isInsideVolume(const glm::vec4& worldSpacePt) const { return bbox.isInside(glm::vec3(inverseTransform * worldSpacePt)); }
	inline bool isInsideVolume(const glm::vec3& worldSpacePt) const { return isInsideVolume(glm::vec4(worldSpacePt, 1.f)); }

	
protected:
	AABB			bbox;
	
private:
	glm::mat4		transform, inverseTransform;

};

