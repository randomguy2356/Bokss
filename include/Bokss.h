#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846

typedef union {
  struct {
    float x, y;
  };
  float arr[2];
} Vec2;

Vec2 newVec2(float x, float y);

typedef union {
  struct {
    float x, y, z;
  };
  float arr[3];
} Vec3;

Vec3 newVec3(float x, float y, float z);

typedef struct {
  Vec3 coords;
  Vec3 colors;
} Vertex;

Vertex newVertex(float x, float y, float r, float g, float b);

typedef struct {
  Vertex *verticies;
  size_t vertexCount;
} Polygon;

Polygon newPolygon(size_t vertCount);

typedef struct {
  double staticFriction;
  double kinematicFriction;
  double restitution;
} Material;

Material newMaterial(double staticFriction, double kinematicFriction,
                     double restitution);

typedef struct {
  Polygon mesh;
  double weight;
  Material material;
  double speed;
  double angularVelocity;
} Object;

Object newObject(Polygon mesh, double weight, Material material);

typedef struct {
  Vec2 normal;
  double offset;
} Line;

Line newLine(double angle, double offset);

typedef struct {
  Object *objects;
  size_t objectCount;
  Line *bounds;
  size_t boundCount;
} Scene;

typedef struct {
  size_t *collisionobjects;
  size_t colisionCount;
} Collisions;

bool inPolygon(Vec3 pos, Polygon polygon);

Collisions checkCollisions(size_t objectID, Scene scene);

void update(Scene scene);

uint32_t *triangulateMesh(Polygon mesh);
