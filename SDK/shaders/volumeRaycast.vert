#version 120

uniform vec3 camPos;
uniform mat4 inverseMVP;
uniform mat4 inverseMV;
uniform mat4 inverseP;

varying vec2 uv;
varying vec3 fcpWorld;

void main()
{
	gl_Position = vec4(gl_Vertex.xy * 2.0 - vec2(1.0), 0.0, 1.0); //* vec4(0.8, 0.8, 1.0, 1.0);
	uv = gl_Vertex.xy;

	vec4 fcp = inverseMVP * vec4(gl_Vertex.xy * 2.0 - vec2(1.0), 1, 1);
	fcpWorld = camPos * 2 - fcp.xyz / fcp.w;
}
