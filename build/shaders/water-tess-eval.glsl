//Copyright (c) 2020 by Stan Fortoński
#version 410

layout (quads, fractional_even_spacing, ccw) in;

in vec2 tc_out[];
out vec2 texCoords;
out vec3 fragPos;

uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D wavesHeightMap;
uniform float depth;
uniform mat4 mvp;
uniform mat4 model;
uniform float wavesOffset;
uniform float interpolateFactor;

float calcHeight(vec2 tc);

void main()
{
  vec2 tc = vec2(tc_out[0].x + (gl_TessCoord.x) / 64.0, tc_out[0].y + (1.0 - gl_TessCoord.y) / 64.0);
  tc = vec2(tc.x*0.75, tc.y*0.75);
  vec4 tessellatedPoint = vec4(gl_in[0].gl_Position.x + gl_TessCoord.x / 64.0, calcHeight(tc), gl_in[0].gl_Position.z + gl_TessCoord.y / 64.0, 1.0);

  gl_Position = mvp * tessellatedPoint;
  fragPos = vec3(model * tessellatedPoint);
  texCoords = tc;
}

float calcHeight(vec2 tc)
{
  float height1 = texture(heightMap1, vec2(tc.x, tc.y)).r;
  float height2 = texture(heightMap2, vec2(tc.x, tc.y)).r;
  float wavesHeight = texture(wavesHeightMap, vec2(tc.x, tc.y + wavesOffset)).r * 4;

  float height = mix(height1, height2, interpolateFactor);
  height = mix(height, wavesHeight, 0.175);
  height *= depth;
  return height;
}
