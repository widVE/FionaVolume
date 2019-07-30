#version 120

uniform sampler2D 	gaussmap;

void main()
{

	float intensity = texture2D(gaussmap, gl_PointCoord).x;
	vec3 color = vec3(intensity);

	gl_FragColor = vec4(color, 1.0);

}