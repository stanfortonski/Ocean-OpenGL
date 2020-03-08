/* Copyright (c) 2020 by Stan Fortoński */

#include "TextureLoader.hpp"
#include <stb_image.h>

void TextureLoader::loadTexture(unsigned & textureId, const std::string & fileName)
{
  int width, height, nrChannels;
  unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrChannels, 0);
  if (!data)
  {
    stbi_image_free(data);
    throw std::runtime_error(std::string("Failed to load texture from file: "+fileName+"."));
  }

  GLenum format = getFormat(nrChannels);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);

  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
}

GLenum TextureLoader::getFormat(const unsigned & nrChannels)
{
  if (nrChannels == 1)
    return GL_RED;
  else if (nrChannels == 3)
    return GL_RGB;
  else if (nrChannels == 4)
    return GL_RGBA;
  else throw std::runtime_error("Can't detected the format of the image from the number of channels");
}
