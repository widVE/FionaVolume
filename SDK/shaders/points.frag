#version 330

in float intensity;

out vec4 fragColor;

void main()
{
	vec3 red = vec3(1.0, 0.0, 0.0);


	fragColor = vec4(red, 1.0);

}