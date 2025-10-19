#pragma once

#include <glad/glad.h>
#include <stdint.h>

typedef struct {
  uint32_t programID;
} Shader;

inline static Shader shaderErr() {
  Shader err = {0};
  return err;
}

Shader newShader(const char *vertexPath, const char *fragmentPath);

void use(Shader *self);

void setBool(Shader *self, const char *name, bool value);

void setInt(Shader *self, const char *name, int value);

void setFloat(Shader *self, const char *name, float value);
