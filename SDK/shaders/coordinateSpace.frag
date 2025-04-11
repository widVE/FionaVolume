#version 430

in vec3 transCoord;
out vec3 fragColor;

uniform ivec3 dimensions;

void main() {
	fragColor = vec3(dimensions.x * transCoord.x, dimensions.y * transCoord.y, dimensions.z * transCoord.z);
}
