//Copyright (c) 2020 by Stan Fortoński
#version 410 core

layout (vertices = 4) out;

in vec2 tc[];
out vec2 tc_out[];

uniform int tessLevel;
uniform mat4 mvp;

void main()
{
  if (gl_InvocationID == 0)
  {
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelOuter[3] = tessLevel;
    gl_TessLevelInner[0] = tessLevel;
    gl_TessLevelInner[1] = tessLevel;
  }
  tc_out[gl_InvocationID] = tc[gl_InvocationID];
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
