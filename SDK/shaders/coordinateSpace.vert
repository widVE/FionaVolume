#version 120

varying vec3 transCoord;
uniform mat4 modelMVP;

void main() {
	gl_Position = modelMVP * vec4(gl_Vertex.xyz, 1.0);
	transCoord = gl_TexCoord[0].xyz;
}
