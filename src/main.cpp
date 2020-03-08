#include <iostream>
#include <stdexcept>
#include <cmath>
#include <climits>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Shader.hpp"
#include "Program.hpp"
#include "TextureLoader.hpp"

const unsigned GL_BASE_VERSION = 4;
const unsigned GL_SUB_VERSION = 1;
const unsigned WINDOW_WIDTH = 1280;
const unsigned WINDOW_HEIGHT = 720;
const char * WINDOW_NAME = "Ocean OpenGL by Stan Fortoński";
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 500.0f;
const float CAM_FOV = 45.0f;
const unsigned SAMPLES = 8;
const unsigned TEXTURES_AMOUNT = 13;
const unsigned TESS_LEVEL = 1;
const float DEPTH = 0.11f;

GLFWwindow * window = nullptr;
Program program;
glm::mat4 projection;
unsigned VAO = 0;
float interpolateFactor = 0.0f;
double deltaTime = 0.0;
unsigned heightMap[TEXTURES_AMOUNT];
unsigned normalMap[TEXTURES_AMOUNT];
unsigned waterTex;
unsigned wavesNormalMap;
unsigned wavesHeightMap;
unsigned firstIndex = 0;
unsigned lastIndex = 1;
bool rotate = false;

void initGL();
void initProgram();
void resizeCallback(GLFWwindow* window, int width, int height);
void setWindowSize(int width, int height);
void processInput(GLFWwindow * window);
void renderWater();

int main()
{
  try
  {
    initGL();
    initProgram();
    glGenVertexArrays(1, &VAO);
    glGenTextures(TEXTURES_AMOUNT, heightMap);
    glGenTextures(TEXTURES_AMOUNT, normalMap);
    glGenTextures(1, &waterTex);

    for (unsigned i = 0; i < TEXTURES_AMOUNT; ++i)
    {
      std::string num = std::to_string(i+1);

      TextureLoader::loadTexture(heightMap[i], "textures/heights/"+num+".png");
      TextureLoader::loadTexture(normalMap[i], "textures/normals/"+num+".png");
    }

    TextureLoader::loadTexture(waterTex, "textures/water.jpg");
    TextureLoader::loadTexture(wavesNormalMap, "textures/wavesNormal.jpg");
    TextureLoader::loadTexture(wavesHeightMap, "textures/wavesHeight.jpg");

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f));
    glm::vec3 viewPos = glm::vec3(1.0f);
    glm::mat4 view = glm::lookAt(
      viewPos,
      glm::vec3(0.0f, 0.0f,  0.0f),
      glm::vec3(0.0f, 1.0f,  0.0f)
    );

    program.use();
    program.setVec3("light.direction", glm::vec3(0.0, -1.0, 0.0));
    program.setVec3("light.ambient", glm::vec3(0.15, 0.15, 0.15));
    program.setVec3("light.diffuse", glm::vec3(0.75, 0.75, 0.75));
    program.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));
    program.setFloat("interpolateFactor", interpolateFactor);
    program.setFloat("depth", DEPTH);
    program.setInt("tessLevel", TESS_LEVEL);

    double lastFrame = 0;
    double actualFrame = 0;
    float radius = 75.0f;

    float camX = sin(glfwGetTime()*0.5) * radius,
          camZ = cos(glfwGetTime()*0.5) * radius;
    viewPos = glm::vec3(camX, 50.0f, camZ);

    while (!glfwWindowShouldClose(window))
    {
      processInput(window);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glClearColor(0, 0, 0, 1);

      actualFrame = glfwGetTime();
      deltaTime = actualFrame - lastFrame;
      lastFrame = actualFrame;

      if (rotate)
      {
        radius = 60.0f;
        float camX = sin(glfwGetTime()*0.5) * radius,
              camZ = cos(glfwGetTime()*0.5) * radius;
        viewPos = glm::vec3(camX, 30.0f, camZ);
      }

      view = glm::lookAt(
        viewPos,
        glm::vec3(0.0f, 0.0f,  0.0f),
        glm::vec3(0.0f, 1.0f,  0.0f)
      );

      program.use();
      program.setMat4("model", model);
      program.setMat4("mvp", projection * view * model);
      program.setVec3("viewPos", viewPos);
      renderWater();

      glfwPollEvents();
      glfwSwapBuffers(window);
    }
  }
  catch(std::exception & e)
  {
    std::cerr<<e.what()<<std::endl;
    std::cin.get();
  }
  glDeleteVertexArrays(1, &VAO);
  glDeleteTextures(TEXTURES_AMOUNT, heightMap);
  glDeleteTextures(TEXTURES_AMOUNT, normalMap);
  glDeleteTextures(1, &waterTex);
  glfwTerminate();
  return 0;
}

void renderWater()
{
  program.use();
  program.setInt("heightMap1", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, heightMap[firstIndex]);

  program.setInt("heightMap2", 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, heightMap[lastIndex]);

  program.setInt("normalMap1", 2);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, normalMap[firstIndex]);

  program.setInt("normalMap2", 3);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, normalMap[lastIndex]);

  program.setInt("water", 4);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, waterTex);

  program.setInt("wavesHeightMap", 5);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, wavesHeightMap);

  program.setInt("wavesNormalMap", 6);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, wavesNormalMap);

  if (interpolateFactor >= 1)
  {
    interpolateFactor = 0.0f;
    if (lastIndex == TEXTURES_AMOUNT-1)
    {
      firstIndex = 0;
      lastIndex = 1;
    }
    else
    {
      ++firstIndex;
      ++lastIndex;
    }
  }
  else
  {
    interpolateFactor += 0.4 * deltaTime;
    program.setFloat("interpolateFactor", interpolateFactor);
  }

  static float offset = 0.0f;
  if (offset >= INT_MAX-2)
    offset = 0;
  offset += 0.2 * deltaTime;
  program.setFloat("wavesOffset", offset);

  glBindVertexArray(VAO);
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  glDrawArraysInstanced(GL_PATCHES, 0, 4, 64*64);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void initGL()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_BASE_VERSION);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_SUB_VERSION);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, SAMPLES);

  if (!(window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr)))
    throw std::runtime_error("Can't create window.\n");
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resizeCallback);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
    throw std::runtime_error("Can't init OpenGL.\n");
  setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
}

void initProgram()
{
  program.create();
  program.attachShader(Shader::createVertexShader("shaders/water-vert.vs"));
  program.attachShader(Shader::createTessalationControlShader("shaders/water-tess-control.glsl"));
  program.attachShader(Shader::createTessalationEvaluationShader("shaders/water-tess-eval.glsl"));
  program.attachShader(Shader::createFragmentShader("shaders/water-frag.fs"));
  program.link();
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void setWindowSize(int width, int height)
{
  projection = glm::perspective(glm::radians(CAM_FOV), (float)width/(float)height, NEAR_PLANE, FAR_PLANE);
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow * window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    rotate = true;
  else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    rotate = false;
}
