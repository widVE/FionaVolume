#version 430

#define STEPS 40000

uniform isampler3D	volumeTex[2];
uniform vec3		invCellSize;
uniform ivec3		dimensions;
uniform sampler2D	rayStart;
uniform mat4		inverseMVP;
uniform int			visible[2];

in vec2 texcoord;
out vec4 fragColor;

//converts a worldspace position to volume cell coordinates;
ivec3 volumeCellCoords(vec3 pos) {
	return ivec3(floor(pos * invCellSize));
}

//whether a worldspace position is inside the volume to be sampled
bool insideVolume(vec3 pos) {
	ivec3 cellCoords = volumeCellCoords(pos);
	return
		cellCoords.x >= 0 &&
		cellCoords.y >= 0 &&
		cellCoords.z >= 0 &&
		cellCoords.x < dimensions.x &&
		cellCoords.y < dimensions.y &&
		cellCoords.z < dimensions.z;
}

//returns the sampled value of a worldspace position, 0 if outside the bounds
int sampleVolume(int channel, vec3 pos) {
	ivec3 cellCoords = volumeCellCoords(pos);
	if( cellCoords.x >= 0 &&
		cellCoords.y >= 0 &&
		cellCoords.z >= 0 &&
		cellCoords.x < dimensions.x &&
		cellCoords.y < dimensions.y &&
		cellCoords.z < dimensions.z)
		return texelFetch(volumeTex[channel], volumeCellCoords(pos), 0).r;
	else
		return 0;
}

/*
//res.xyz = new position
//res.w = distance traversed
vec4 walkCell(vec3 cellSize, vec3 vec3 point, vec3 dir) {
	vec3 inCell = point - floor(point);
	vec3 toGoP = (cellSize - inCell) / dir;
	vec3 toGoN = inCell / dir;
	vec3 tgn = toGo / dir;
	if(tgn.x <= tgn.y && tgn.x <= tgn.z) {
		return vec4( X, X, point.z + cellSize.z, inCell.z);
	} else if(tgn.y <= tgn.x && tgn.y <= tgn.z) {
		return vec4( X, X, point.z + cellSize.z, inCell.z);
	} else { // if(tgn.z <= tgn.x && tgn.z <= tgn.y)
		return vec4( X, X, point.z + cellSize.z, inCell.z);
	}
}*/


void main() {
	vec4 ncp = vec4(texcoord * 2.0 - vec2(1.0), -1.0, 1.0);
	ncp = inverseMVP * ncp;
	ncp /= ncp.w;
	vec4 fcp = vec4(texcoord * 2.0 - vec2(1.0), 1.0, 1.0);
	fcp = inverseMVP * fcp;
	fcp /= fcp.w;
	
	vec3 rayStart = ncp.xyz;
	vec3 rayEnd = fcp.xyz;
	
	double accum = 0;
	double accum1 = 0;
	float invSteps = 1.0 / float(STEPS);
	for(int i = 0; i < STEPS; i++) {
		vec3 samplePos = mix(rayStart, rayEnd, i * invSteps);
		if(insideVolume(samplePos)) {
			//accum = max(accum, sampleVolume(samplePos) / 255.0f);
			int samp = 0;
			int samp1 = 0;
			if(visible[0] != 0) samp = sampleVolume(0, samplePos);
			if(visible[1] != 0) samp1 = sampleVolume(1, samplePos);
			accum += samp;
			accum1 += samp1;
		}
	}

	fragColor = vec4(
	accum / 255.0 / 30.0,
	accum1 / 255.0 / 6000.0, 0, 1);
	//fragColor = vec4(accum1 / 255.0 / 6000.0, (accum1 / 255.0 / 6000.0) - 1, (accum1 / 255.0 / 6000.0) - 2, 1);
}

