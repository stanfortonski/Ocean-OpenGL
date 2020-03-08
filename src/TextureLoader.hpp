/* Copyright (c) 2020 by Stan Fortoński */

#ifndef TEXTURE_LOADER_HPP
#define TEXTURE_LOADER_HPP 1
#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <GL/glext.h>

class TextureLoader
{
  static GLenum getFormat(const unsigned & nrChannels);

public:
  static void loadTexture(unsigned & textureId, const std::string & fileName);
};
#endif
