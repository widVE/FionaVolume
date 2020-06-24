#version 430

#define CHANNELS 2

uniform double		sampleRate;
uniform isampler3D	volumeTex[CHANNELS];
uniform ivec3		dimensions;
uniform mat4		MVP;
uniform mat4		inverseMVP;
uniform mat4		invVolumeModelMat;
uniform int			visible[CHANNELS];
uniform vec4		channelColors[CHANNELS];
uniform double		scaleFactor[CHANNELS];
uniform vec3		camPos;
uniform uint		lowerLims[3];
uniform uint		upperLims[3];

uniform int			renderMode;

uniform isampler2D	frontCoords;
uniform isampler2D	backCoords;

in vec2 uv;
in vec3 fcpWorld;
out vec4 fragColor;

//converts a worldspace position to volume cell coordinates;
ivec3 volumeCellCoords(vec3 pos) {
	return ivec3(floor(pos));
}

//whether a volumespace position is inside the volume to be sampled
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

bool insideLimits(ivec3 coord) {
	return
		coord.x >= lowerLims[0] &&
		coord.y >= lowerLims[1] &&
		coord.z >= lowerLims[2] &&
		coord.x <= upperLims[0] &&
		coord.y <= upperLims[1] &&
		coord.z <= upperLims[2];
}

bool insideVolumeWorld(vec3 pos) {
	return
		pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
		pos.x < 1 && pos.y < 1 && pos.z < 1;
}

bool insideVolumeCell(ivec3 pos) {
	return pos.x >= 0 &&
		pos.y >= 0 &&
		pos.z >= 0 &&
		pos.x < dimensions.x &&
		pos.y < dimensions.y &&
		pos.z < dimensions.z;
}

//returns the sampled value of a worldspace position, 0 if outside the bounds
int sampleVolume(int channel, vec3 pos) {
	ivec3 cellCoords = volumeCellCoords(pos);
	if(!insideLimits(cellCoords)) return 0;
	/*if( cellCoords.x >= 0 &&
		cellCoords.y >= 0 &&
		cellCoords.z >= 0 &&
		cellCoords.x < dimensions.x &&
		cellCoords.y < dimensions.y &&
		cellCoords.z < dimensions.z)
		return texelFetch(volumeTex[channel], cellCoords, 0).r;
	else
		return 0;*/
	return texelFetch(volumeTex[channel], cellCoords, 0).r;
}

//returns the sampled value of a worldspace position, 0 if outside the bounds
int sampleVolumeCell(int channel, ivec3 pos) {
	if( pos.x >= 0 &&
		pos.y >= 0 &&
		pos.z >= 0 &&
		pos.x < dimensions.x &&
		pos.y < dimensions.y &&
		pos.z < dimensions.z)
		return texelFetch(volumeTex[channel], pos, 0).r;
	else
		return 0;
}

/*
//accumulate values of traversed cells
double traverse(int channel, vec3 point, vec3 dir) {
	vec3 cellSize = vec3(1,1,1);
	dir = normalize(dir);
	ivec3 cellCoords = volumeCellCoords(point);
	vec3 inCell = point - cellSize * cellCoords;
	
	//preserve cell increment direction
	ivec3 inc = ivec3(
		dir.x >= 0 ? 1 : -1,
		dir.y >= 0 ? 1 : -1,
		dir.z >= 0 ? 1 : -1
	);
	
	//remaining amount to go in the ray direction
	vec3 rem = vec3(
		dir.x >= 0 ? cellSize.x - inCell.x : inCell.x,
		dir.y >= 0 ? cellSize.y - inCell.y : inCell.y,
		dir.z >= 0 ? cellSize.z - inCell.z : inCell.z
	);
	
	//work with only positive values from now on
	dir = abs(dir);
	vec3 invDir = vec3(1, 1, 1) / dir;
	
	vec3 nrem = rem * invDir;
	
	double accum = 0;
	for(int i = 0; i < numSteps; i++) {
		int samp = sampleVolumeCell(channel, cellCoords);
		if(nrem.x <= nrem.y && nrem.x <= nrem.z) {
			cellCoords.x += inc.x;
			rem.y -= nrem.x * dir.y;
			rem.z -= nrem.x * dir.z;
			accum += nrem.x * samp;
			rem.x = cellSize.x;
		} else if(nrem.y <= nrem.x && nrem.y <= nrem.z) {
			cellCoords.y += inc.y;
			rem.x -= nrem.y * dir.x;
			
			rem.z -= nrem.y * dir.z;
			accum += nrem.y * samp;
			rem.y = cellSize.y;
		} else {
			cellCoords.z += inc.z;
			rem.x -= nrem.z * dir.x;
			rem.y -= nrem.z * dir.y;
			accum += nrem.z * samp;
			rem.z = cellSize.z;
		}
		nrem = rem * invDir;
	}
	
	return accum;
}*/

// takes worldspace ray and returns volumespace ray start
vec3 getRelativeStartPos(vec3 rayStart, vec3 rayEnd) {
	vec3 a = (invVolumeModelMat * vec4(rayStart, 1)).xyz;
	if(insideVolumeWorld(a)) return a;
	vec3 b = (invVolumeModelMat * vec4(rayEnd, 1)).xyz;
	vec3 d = normalize(b - a);
	float toX = (d.x > 0) ? (-a.x / d.x) : ((1.0 - a.x) / d.x);
	float toY = (d.y > 0) ? (-a.y / d.y) : ((1.0 - a.y) / d.y);
	float toZ = (d.z > 0) ? (-a.z / d.z) : ((1.0 - a.z) / d.z);
	if(toX > 0) {
		vec3 r = a + d * toX;
		if(r.y > 0 && r.y < 1.0 && r.z > 0 && r.z < 1.0) {
			return r;
		}
	}
	if(toY > 0) {
		vec3 r = a + d * toY;
		if(r.x > 0 && r.x < 1.0 && r.z > 0 && r.z < 1.0) {
			return r;
		}
	}
	if(toZ > 0) {
		vec3 r = a + d * toZ;
		if(r.x > 0 && r.x < 1.0 && r.y > 0 && r.y < 1.0) {
			return r;
		}
	}
	return vec3(0,0,0);
}
// takes worldspace ray and returns volumespace ray end
vec3 getRelativeEndPos(vec3 rayStart, vec3 rayEnd) {
	vec3 b = (invVolumeModelMat * vec4(rayEnd, 1)).xyz;
	if(insideVolumeWorld(b)) return b;
	vec3 a = (invVolumeModelMat * vec4(rayStart, 1)).xyz;
	vec3 d = normalize(b - a);
	float toX = (d.x < 0) ? (-a.x / d.x) : ((1.0 - a.x) / d.x);
	float toY = (d.y < 0) ? (-a.y / d.y) : ((1.0 - a.y) / d.y);
	float toZ = (d.z < 0) ? (-a.z / d.z) : ((1.0 - a.z) / d.z);
	if(toX > 0) {
		vec3 r = a + d * toX;
		if(r.y > 0 && r.y < 1.0 && r.z > 0 && r.z < 1.0) {
			return r;
		}
	}
	if(toY > 0) {
		vec3 r = a + d * toY;
		if(r.x > 0 && r.x < 1.0 && r.z > 0 && r.z < 1.0) {
			return r;
		}
	}
	if(toZ > 0) {
		vec3 r = a + d * toZ;
		if(r.x > 0 && r.x < 1.0 && r.y > 0 && r.y < 1.0) {
			return r;
		}
	}
	return vec3(0,0,0);
}


void main() {
	vec3 tStart =	getRelativeStartPos(camPos, fcpWorld) * dimensions;
	vec3 tEnd =		getRelativeEndPos(camPos, fcpWorld) * dimensions;
	float sampLen = distance(tStart, tEnd);
	int sampCount = max(int(sampLen * sampleRate), 0);
	
	if(sampCount == 0) {
		discard;
	}
	double accum[CHANNELS];
	if(renderMode == 0 || renderMode == 1) {
		float invSteps = 1.0 / float(sampCount);
		for(int c = 0; c < CHANNELS; c++) {
			accum[c] = 0;
			if(visible[c] == 0) continue;
			for(int i = 0; i < sampCount; i++) {
				vec3 samplePos = mix(tStart, tEnd, i * invSteps);
				accum[c] += sampleVolume(c, samplePos);
			}
		}
	} else {
		float invSteps = 1.0 / float(sampCount);
		for(int c = 0; c < CHANNELS; c++) {
			accum[c] = 0;
			if(visible[c] == 0) continue;
			for(int i = 0; i < sampCount; i++) {
				vec3 samplePos = mix(tStart, tEnd, i * invSteps);
				accum[c] = max(accum[c], sampleVolume(c, samplePos));
			}
		}
	}
	
	vec3 res = vec3(0,0,0);
	for(int c = 0; c < CHANNELS; c++) {
		res += channelColors[c].rgb * channelColors[c].a * float(scaleFactor[c] * accum[c] * 0.0001f);
	}
	
	if(renderMode == 1) res.rgb = res.rgb / float(sampCount);
	fragColor = vec4(res.rgb * 0.01, 1);
	
	//if(val == -1) {
	//	fragColor = vec4(fcpWorld, 1);
	//}
	//fragColor = vec4(accum1 / 255.0 / 6000.0, (accum1 / 255.0 / 6000.0) - 1, (accum1 / 255.0 / 6000.0) - 2, 1);
}
