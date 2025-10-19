#include "Shader.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Shader newShader(const char *vertexPath, const char *fragmentPath) {

  //---read from file---

  FILE *vertFile = fopen(vertexPath, "rb");
  if (!vertFile) {
    perror("fopen:");
  ret:
    return shaderErr();
  }
  fseek(vertFile, 0, SEEK_END);
  long vertSize = ftell(vertFile);
  fseek(vertFile, 0, SEEK_SET);

  FILE *fragFile = fopen(fragmentPath, "rb");
  if (!fragFile) {
    perror("fopen:");
  closefirst:
    fclose(vertFile);
    goto ret;
  }
  fseek(fragFile, 0, SEEK_END);
  long fragSize = ftell(fragFile);
  fseek(fragFile, 0, SEEK_SET);

  if ((vertSize < 0) || (fragSize < 0)) {
    perror("ftell:");
  closeall:
    fclose(fragFile);
    goto closefirst;
  }

  char *vertCode = malloc(vertSize + 1);
  char *fragCode = malloc(fragSize + 1);
  if (!(vertCode && fragCode)) {
    perror("malloc:");
    fprintf(stderr,
            "how? malloc failed while allocating space for shader code\n");
  close_and_free_all:
    free(vertCode);
    free(fragCode);
    goto closeall;
  }

  if (fread(vertCode, 1, (size_t)vertSize, vertFile) != (size_t)vertSize) {
    perror("fread:");
    goto close_and_free_all;
  }
  if (fread(fragCode, 1, (size_t)fragSize, fragFile) != (size_t)fragSize) {
    perror("fread:");
    goto close_and_free_all;
  }
  vertCode[vertSize] = '\0';
  fragCode[fragSize] = '\0';

  const char *cVertCode = vertCode;
  const char *cFragCode = fragCode;

  // compile shader
  uint32_t vertexID, fragmentID, programID;
  int success;
  char log[512];

  // vertex shader

  vertexID = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexID, 1, &cVertCode, NULL);
  glCompileShader(vertexID);
  // check for errors and print
  glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexID, 512, NULL, log);
    fprintf(stderr, "failed to compile vertex shader lol: %s\n", log);
  }

  // fragment shader

  fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentID, 1, &cFragCode, NULL);
  glCompileShader(fragmentID);
  // check for errors and print
  glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentID, 512, NULL, log);
    fprintf(stderr, "failed to compile vertex shader lol: %s\n", log);
  }

  // Shader program
  programID = glCreateProgram();
  glAttachShader(programID, vertexID);
  glAttachShader(programID, fragmentID);
  glLinkProgram(programID);
  // check for errors and print
  glGetProgramiv(programID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(programID, 512, NULL, log);
    fprintf(stderr, "failed to link program: %s\n", log);
  }

  // close, free, delete and return
  fclose(vertFile);
  fclose(fragFile);
  free(vertCode);
  free(fragCode);
  glDeleteShader(vertexID);
  glDeleteShader(fragmentID);
  Shader shader = {programID};
  return shader;
}

void use(Shader *self) { glUseProgram(self->programID); }

void setBool(Shader *self, const char *name, bool value) {
  glUniform1i(glGetUniformLocation(self->programID, name), (int)value);
}

void setInt(Shader *self, const char *name, int value) {
  glUniform1i(glGetUniformLocation(self->programID, name), value);
}

void setFloat(Shader *self, const char *name, float value) {
  glUniform1f(glGetUniformLocation(self->programID, name), value);
}
