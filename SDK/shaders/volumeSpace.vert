#version 120

varying vec3 vertColor;

uniform mat4 modelMVP;

void main() {
	gl_Position = modelMVP * vec4(gl_Vertex.xyz, 1.0);
	vertColor = vec3(0.7);
}
