#pragma once

#include "FionaScene.h"
#include <GL\gl.h>

class CoordFBO {
	GLuint id;
	GLuint tex;

public:
	inline GLuint getID() {
		return id;
	}

	inline GLuint getTex() {
		return tex;
	}

	inline void clear() {
		bind();
		glm::vec4 val(1.0f, 1.0f, 1.0f, 1.0f);

		glClearBufferfv(GL_COLOR, 0, &val.x);

		glClear(GL_COLOR_BUFFER_BIT);

		disable();
	}

	inline void bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, id);
	}
	inline void disable() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	inline void resize(uint32_t width, uint32_t height) {
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	inline CoordFBO(uint32_t width, uint32_t height) {
		glGenFramebuffers(1, &id);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		disable();
	}
	inline ~CoordFBO() {
		glDeleteTextures(1, &tex);
		glDeleteFramebuffers(1, &id);
	}
};