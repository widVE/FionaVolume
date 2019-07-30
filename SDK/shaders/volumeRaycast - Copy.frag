// renders volume slices

#version 430

//#extension GL_EXT_gpu_shader4 : enable

struct Volume
{
	isampler3D		texture;
	//usamplerBuffer	texture;
	mat4			inverseTransform;
	mat4			transform;
	vec3			bboxMin, bboxMax, cellSize;
	float			minThreshold;
	float			maxThreshold;

	bool 			enabled;

};

#define VOLUMES 1
#define STEPS 4000

uniform Volume volume[VOLUMES];

uniform sampler2D	rayStart;

uniform mat4		inverseMVP;

uniform float		stepLength = 2.0;

uniform int  		activeVolume = -1;

in vec2 texcoord;
out vec4 fragColor;




void main()
{
	vec4 rayInput = texture(rayStart, texcoord);
	if(rayInput.a > 0) {
		//fragColor = vec4(rayInput.xyz, 1);
	} else {
		fragColor = vec4(1,0,0,1);
		return;
	}
	
	vec4 finalValue = vec4(0.0);
	
	// define 7 colors that will color the differetn volumes
	vec3 color_table[7];
	color_table[0] = vec3(0.0, 1.0, 0.0);
	color_table[1] = vec3(1.0, 0.0, 0.0);
	color_table[2] = vec3(0.0, 0.0, 1.0);
	color_table[3] = vec3(1.0, 1.0, 0.0);
	color_table[4] = vec3(0.0, 1.0, 1.0);
	color_table[5] = vec3(1.0, 0.0, 1.0);
		//default
	color_table[6] = vec3(1.0, 1.0, 1.0);

	// create ray
	if (texture(rayStart, texcoord).a > 0)
	{
		vec3 rayOrigin = texture(rayStart, texcoord).xyz;
		vec4 nearPlane = vec4(texcoord * 2.0 - vec2(1.0), -1.0, 1.0);
		nearPlane = inverseMVP * nearPlane;
		nearPlane /= nearPlane.w;
		vec3 rayDestination = nearPlane.xyz;

		float maxDistance = length(rayDestination - rayOrigin);
		vec3 rayDirection = (rayDestination - rayOrigin) /maxDistance; // = farPlane.xyz;
		rayDirection *= stepLength;

		
		float maxValue = 0.0;
		float meanValue = 0.0;
		float distanceTravelled = 0.0;
		
		bool hitActiveVolume = false;
		bool hitAnyVolume = false;


		float maxValues[VOLUMES];
		for (int i = 0; i < VOLUMES; ++i)
			maxValues[i] = 0.0;

		vec3 worldPosition = rayOrigin;
		for (int i = 0; i < STEPS; ++i)
		{
			if (distanceTravelled >= maxDistance)
				break;


			float value[VOLUMES];
			

			for (int v = 0; v < VOLUMES; ++v)
			{
				if (!volume[v].enabled)
				{
					value[v] = 0.0;
					continue;
				}

				// check all volumes
				vec3 volPosition = vec3(volume[v].inverseTransform * vec4(worldPosition, 1.0));

				if (volPosition.x > volume[v].bboxMin.x && volPosition.x < volume[v].bboxMax.x &&
					volPosition.y > volume[v].bboxMin.y && volPosition.y < volume[v].bboxMax.y &&
					volPosition.z > volume[v].bboxMin.z && volPosition.z < volume[v].bboxMax.z) 
				{

					vec3 volCoord = volPosition - volume[v].bboxMin; 
					volCoord /= (volume[v].bboxMax - volume[v].bboxMin);

					float val = texture(volume[v].texture, volCoord).r;
						
					//float val = float(texelFetch(volume[v].texture, int((1776.0 * 1760.0 * (volCoord.z)) + ((volCoord.y * 1776.0) + (volCoord.x)))).r);
					
					//float val = texelFetch(volume[v].texture, int(((volCoord.y * 1760.0 * 1776.0) + (volCoord.x * 1776.0)))).r;
		
					value[v] = val;

					if (v == activeVolume && val > volume[v].minThreshold && val < volume[v].maxThreshold)
						hitActiveVolume = true;

					hitAnyVolume = true;

				}
				else
					value[v] = 0.0;

			}


			float mean = 0.0;
			for (int v = 0; v < VOLUMES; ++v)
			{
				// calcualte the max
				maxValue = max(maxValue, value[v]);
				mean += value[v];

				maxValues[v] = max(maxValues[v], value[v]);
			}

			mean /= float(VOLUMES);
			meanValue = max(meanValue, mean);

				
			worldPosition += rayDirection;
			distanceTravelled += stepLength;

		}

				
		vec3 baseColor = vec3(1.0);

		// this just highlights the active vvolume
		/*
		if (hitActiveVolume)
			baseColor = vec3(1.0, 1.0, 0.0);
	
		finalValue = vec4(val * baseColor, 1.0);
		*/


		
		vec3 aggregateColor = vec3(0.0);
		for (int v = 0; v < VOLUMES; ++v) 
		{
			float val = (maxValues[v] - volume[v].minThreshold) / (volume[v].maxThreshold - volume[v].minThreshold);
			aggregateColor += (color_table[min(v, 6)] * val);	
		}

		//finalValue = vec4(maxValues[0]/255.0);//vec4(aggregateColor, meanValue);
		
		finalValue = vec4(aggregateColor, meanValue);
		


		if (!hitAnyVolume)
			finalValue = vec4(0.0); //1.0, 0.0, 1.0, 0.0);

	}

	fragColor = finalValue;

	//fragColor = texture(rayEnd, texcoord);
	//fragColor = vec4(texcoord, 0.0, 1.0);
}

