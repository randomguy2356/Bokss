#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void doSomeSceneStuff(Scene *scene);
void init(GLFWwindow **window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void doRendering();
uint32_t getSceneVAO(Scene scene);

// uint32_t VBO;
uint32_t VAO;
uint32_t EBO_length;
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

  shader = newShader("shaders/shader.vert", "shaders/shader.frag");

  scene.objects = malloc(sizeof(Object) * 10);
  doSomeSceneStuff(&scene);

  VAO = getSceneVAO(scene);

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

void printObject(Object object, const char *name) {
  printf("the object %s has the following values:\n", name);
  printf("\t- angular velocity: %f\n", object.angularVelocity);
  printf("\t- speed: %f\n", object.speed);
  printf("\t- weight: %f\n", object.weight);
  printf("\t- mesh: we currently don't have the buget for such things\n");
}

//

// set up the scene :3

void doSomeSceneStuff(Scene *scene) {
  Polygon mesh = newPolygon(7);
  mesh.verticies[0] = newVertex(0.843, 0.492, /**/ 1.0, 0.0, 0.0);
  mesh.verticies[1] = newVertex(0.587, 0.79, /**/ 1.0, 0.0, 0.0);
  mesh.verticies[2] = newVertex(0.024, 0.855, /**/ 1.0, 0.0, 0.0);
  mesh.verticies[3] = newVertex(-0.485, 0.77, /**/ 1.0, 0.0, 0.0);
  mesh.verticies[4] = newVertex(-0.407, 0.236, /**/ 1.0, 0.0, 0.0);
  mesh.verticies[5] = newVertex(-0.59, -0.24, /**/ 1.0, 0.0, 0.0);
  mesh.verticies[6] = newVertex(0.514, -0.384, /**/ 1.0, 0.0, 0.0);

  Material squareMaterial = newMaterial(0.5, 0.2, 0.2);
  Object square = newObject(mesh, 1.0, squareMaterial);

  scene->objects[scene->objectCount] = square;
  scene->objectCount++;
}

uint32_t getSceneVAO(Scene scene) {
  uint32_t vbo;
  uint32_t ebo;
  uint32_t vao;
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);

  // check how many verticies
  size_t vertAmmount = 0;
  for (size_t i = 0; i < scene.objectCount; i++) {
    vertAmmount += scene.objects[i].mesh.vertexCount;
  }

  // allocate space for all the verticies, and then fill them.
  Vertex *verticies = malloc(sizeof(Vertex) * vertAmmount);
  size_t verticies_used = 0;
  for (size_t i = 0; i < scene.objectCount; i++) {
    for (size_t vert = 0; vert < scene.objects[i].mesh.vertexCount; vert++) {
      Vertex vertex = scene.objects[i].mesh.verticies[vert];
      verticies[verticies_used++] = vertex;
    }
  }

  // allocate space for all of the thingies in ebo.
  uint32_t *elements =
      malloc((vertAmmount - (scene.objectCount * 2)) * 3 * sizeof(uint32_t));
  size_t elements_used = 0;

  // fill the ebo.
  for (size_t i = 0; i < scene.objectCount; i++) {
    uint32_t *objectElements = triangulateMesh(scene.objects[i].mesh);
    size_t elementsCreated = (scene.objects[i].mesh.vertexCount - 2) * 3;
    memcpy(elements + elements_used, objectElements,
           elementsCreated * sizeof(uint32_t));
    elements_used += elementsCreated;
    free(objectElements);
  }

  // do the actual vao stuff finally.

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, verticies_used * sizeof(Vertex), verticies,
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements_used * sizeof(uint32_t),
               elements, GL_STATIC_DRAW);

  // tell opengl my layout
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  EBO_length = elements_used;
  return vao;
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
  // get a vao.

  // printObject(scene.objects[0], "square");

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
  glDrawElements(GL_TRIANGLES, (GLsizei)EBO_length, GL_UNSIGNED_INT, 0);
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
