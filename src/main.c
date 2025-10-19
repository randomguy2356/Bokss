#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Bokss.h"
#include "Shader.h"

#define key(x) glfwGetKey(window, x)

// float verticies[] = {
// positions         // colors
// -0.5, -0.5, 0.0, /**/ 1.0, 0.0, 0.0, // bottom left
// 0.5,  -0.5, 0.0, /**/ 0.0, 1.0, 0.0, // bottom right
// -0.5, 0.5,  0.0, /**/ 0.0, 0.0, 1.0, // top left
// 0.5,  0.5,  0.0, /**/ 1.0, 0.0, 0.0, // top right
// };
// uint32_t indecies[] = {
// 0, 1, 2, //
// 1, 3, 2, //
// };

void doSomeSceneStuff(Scene scene);
void init(GLFWwindow **window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void doRendering();

// uint32_t VBO;
// uint32_t VAO;
// uint32_t EBO;
uint32_t vertexShader;
uint32_t fragmentShader;
uint32_t shaderProgram;

bool wireframe = false;
int FLast = false;

Shader shader;

Scene scene;

int main() {
  GLFWwindow *window = NULL;
  init(&window);

  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &VAO);

  shader = newShader("shaders/shader.vert", "shaders/shader.frag");

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  doSomeSceneStuff(scene);

  //    setting up the VAO

  // bind the VAO
  glBindVertexArray(VAO);
  // copy the verticies into buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);
  // copy the indecies into other buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecies), indecies,
               GL_STATIC_DRAW);
  // tell glsl what our data looks like
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    // rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    doRendering();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glfwTerminate();
  return 0;
}

//
//

// set up the scene :3

void doSomeSceneStuff(Scene scene) {
  Polygon squareMesh = newPolygon(4);
  squareMesh.verticies[0] = newVertex(0.5, 0.5, /**/ 1.0, 0.0, 0.0);  // top
                                                                      // right
  squareMesh.verticies[1] = newVertex(-0.5, 0.5, /**/ 1.0, 0.0, 0.0); // top
                                                                      // left
  squareMesh.verticies[2] =
      newVertex(-0.5, -0.5, /**/ 1.0, 0.0, 0.0); // bottom left
  squareMesh.verticies[3] =
      newVertex(0.5, -0.5, /**/ 1.0, 0.0, 0.0); // bottom right
  Material squareMaterial = newMaterial(0.5, 0.2, 0.2);
  Object square = newObject(squareMesh, 1.0, squareMaterial);

  scene.objects[scene.objectCount] = square;
  scene.objectCount++;
}

uint32_t getSceneVAO(Scene scene) {
  uint32_t vbo;
  uint32_t ebo;
  uint32_t vao;
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1 & vao);
}

//
//  functions and stuff idk ¯\_(ツ)_/¯

void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (key(GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
  int F = key(GLFW_KEY_F);
  if (F == GLFW_PRESS) {
    if (!(FLast == GLFW_PRESS)) {
      wireframe = !wireframe;
    }
  }
  FLast = F;
}

void doRendering() {
  // set wireframe mode lol
  if (wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // use the program
  use(&shader);
  // bind the VAO
  glBindVertexArray(VAO);
  // draw the trig :3
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  // idk why dis here :(
  // i think it's here so we can go to draw another object later... ¯\_(ツ)_/¯
  glBindVertexArray(0);
}

void init(GLFWwindow **window) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  *window = glfwCreateWindow(800, 600, "bokss", NULL, NULL);
  if (*window == NULL) {
    fprintf(stderr, "WIF(Window Is Fucked)");
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(*window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "GIF(GLAD IS FUCKED)");
    exit(-1);
  }

  glViewport(0, 0, 800, 600);

  glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
}
