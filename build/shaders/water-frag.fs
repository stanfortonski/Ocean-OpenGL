//Copyright (c) 2020 by Stan Fortoński
#version 410 core
out vec4 fragColor;

in vec2 texCoords;
in vec3 fragPos;

const int SHINENESS = 512;
const float NORMAL_HEIGHT = 2.0;
const vec3 WATER_COLOR = vec3(0.04, 0.45, 0.92);

struct Light
{
  vec3 direction;
  vec3 diffuse;
  vec3 ambient;
  vec3 specular;
};

uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D wavesNormalMap;
uniform sampler2D wavesHeightMap;
uniform sampler2D water;
uniform float depth;
uniform Light light;
uniform float interpolateFactor;
uniform float wavesOffset;
uniform vec3 viewPos;

vec3 calcLight(Light light, vec3 material, vec3 viewDir, vec3 normal);
vec3 calcNormal();
vec3 calcHeight();

void main()
{
  vec3 viewDir = normalize(viewPos - fragPos);

  vec3 normal = calcNormal();
  vec3 height = calcHeight();
  height.r = max(14 * height.r, 1.0);

  vec3 water1 = texture(water, vec2(texCoords.x, texCoords.y + wavesOffset/5)).rgb;
  vec3 water2 = texture(water, vec2(texCoords.x/5, -texCoords.y/5 - wavesOffset/10)).rgb;
  vec3 water = mix(water1, water2, 0.5);
  vec3 material = mix(water*WATER_COLOR, water, 0.35);

  material = material * height.rrr;
  material = mix(vec3(1.0, 1.0, 1.0), material, pow(normal.y, 2));

  fragColor = vec4(calcLight(light, material, viewDir, normal), 1.0);
}

vec3 calcNormal()
{
  vec3 normalColor1 = texture(normalMap1, vec2(texCoords.x, texCoords.y)).rgb;
  vec3 normalColor2 = texture(normalMap2, vec2(texCoords.x, texCoords.y)).rgb;
  vec3 wavesColor = texture(wavesNormalMap, vec2(texCoords.x, texCoords.y + wavesOffset)).rgb * 4;

  vec3 normalColor = mix(normalColor1, normalColor2, interpolateFactor).rgb;
  normalColor = mix(normalColor, wavesColor, 0.175).rgb;

  vec3 normal = vec3(normalColor.r * 2.0 - 1.0, normalColor.b * NORMAL_HEIGHT, normalColor.g * 2.0 - 1.0);
  return normalize(normal);
}

vec3 calcHeight()
{
  float height1 = texture(heightMap1, vec2(texCoords.x, texCoords.y)).r;
  float height2 = texture(heightMap2, vec2(texCoords.x, texCoords.y)).r;
  float wavesHeight = texture(wavesHeightMap, vec2(texCoords.x, texCoords.y + wavesOffset)).r * 4;

  float height = mix(height1, height2, interpolateFactor);
  height = mix(height, wavesHeight, 0.175);
  height *= depth;
  return vec3(height, height, height);
}

vec3 calcLight(Light light, vec3 material, vec3 viewDir, vec3 normal)
{
  vec3 fragToLightDir = normalize(-light.direction);

  float diff = max(dot(fragToLightDir, normal), 0.0);
  vec3 halfwayDir = normalize(fragToLightDir + viewDir);
  float specAngle = max(dot(halfwayDir, normal), 0.0);
  float spec = pow(specAngle, SHINENESS);

  vec3 ambient = light.ambient * material;
  vec3 diffuse = light.diffuse * material * diff;
  vec3 specular = light.specular * spec;
  return ambient + diffuse + specular;
}
