
#version 330

in vec3 vertex;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;


void main()
{
	gl_Position = mvpMatrix * modelMatrix * vec4(vertex.xyz, 1.0);
}

