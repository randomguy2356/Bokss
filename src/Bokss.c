#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bokss.h"

Vec2 newVec2(float x, float y) {
  Vec2 result = {{
      x,
      y,
  }};
  return result;
}

Vec3 newVec3(float x, float y, float z) {
  Vec3 result = {{
      x,
      y,
      z,
  }};
  return result;
}

Vertex newVertex(float x, float y, float r, float g, float b) {
  Vertex result = {
      newVec3(x, y, 0.0),
      newVec3(r, g, b),
  };
  return result;
}

Polygon newPolygon(size_t vertCount) {
  Vertex *verticies = malloc(sizeof(Vertex) * vertCount);
  Polygon result = {
      verticies,
      vertCount,
  };
  return result;
}

Material newMaterial(double staticFriction, double kinematicFriction,
                     double restitution) {
  Material result = {
      staticFriction,
      kinematicFriction,
      restitution,
  };
  return result;
}

Object newObject(Polygon mesh, double weight, Material material) {
  Object result = {
      mesh, weight, material, 0, 0,
  };
  return result;
}

Line newLine(double angle, double offset) {
  angle = ((angle / 180) * PI);
  Line result;
  Vec2 normal = {{
      cos(angle),
      sin(angle),
  }};
  result.normal = normal;
  result.offset = offset;
  return result;
}

bool inPolygon(Vec3 pos, Polygon polygon) {
  int intersections = 0;
  for (size_t edgeID = 0; edgeID < polygon.vertexCount; edgeID++) {
    Vec3 vert1 = polygon.verticies[edgeID].coords;
    Vec3 vert2 = polygon.verticies[(edgeID + 1) % polygon.vertexCount].coords;
    if (vert1.x == vert2.x) {
      continue;
    }
    if ((vert1.x > pos.x) != (vert2.x > pos.x)) {
      double y_at_posx = vert1.y + (pos.x - vert1.x) * (vert2.y - vert1.y) /
                                       (vert2.x - vert1.x);
      if (y_at_posx > pos.y) {
        intersections++;
      }
    }
  }
  return intersections & 0x1;
}

Collisions checkCollisions(size_t objectID, Scene scene) {
  Collisions err = {NULL, 0};
  size_t *tmp_collisions = malloc((scene.objectCount - 1) * sizeof(size_t));
  if (!tmp_collisions) {
    return err;
  }
  size_t collisionCount = 0;

  if (objectID >= scene.objectCount) {
    return err;
  }
  Object object = scene.objects[objectID];
  for (size_t objID = 0; objID < scene.objectCount; objID++) {
    if (objID == objectID) {
      continue;
    }
    for (size_t vertID = 0; vertID < object.mesh.vertexCount; vertID++) {
      if (inPolygon(object.mesh.verticies[vertID].coords,
                    scene.objects[objID].mesh)) {
        tmp_collisions[collisionCount] = objID;
        break;
      }
    }
  }

  size_t *collisions = malloc(collisionCount * sizeof(size_t));
  if (!collisions) {
    return err;
  }
  memccpy(collisions, tmp_collisions, sizeof(size_t), collisionCount);
  free(tmp_collisions);

  Collisions result = {collisions, collisionCount};
  return result;
}

void update(Scene scene) {
  Collisions *map = malloc(sizeof(Collisions *) * scene.objectCount);
  if (!map) {
    return;
  }
  for (size_t i = 0; i < scene.objectCount; i++) {
    map[i] = checkCollisions(i, scene);
  }
  for (size_t i = 0; i < scene.objectCount; i++) {
    printf("the object with id %zu is colliging with:\n", i);
    for (size_t j = 0; j < map[i].colisionCount; j++) {
      printf("- %zu\n", j);
    }
  }
}

bool isCCW(Polygon polygon) {
  double area = 0;
  for (size_t i = 0; i < polygon.vertexCount; i++) {
    float x1 = polygon.verticies[i].coords.x;
    float y1 = polygon.verticies[i].coords.y;
    float x2 = polygon.verticies[(i + 1) % polygon.vertexCount].coords.x;
    float y2 = polygon.verticies[(i + 1) % polygon.vertexCount].coords.y;
    area += (x1 * y2) + (x2 * y1);
  }
  return area > 0;
}

static inline double crossProduct(Vec3 point1, Vec3 point2, Vec3 testPoint) {
  return ((point2.x - point1.x) * (testPoint.y - point1.y)) -
         ((point2.y - point1.y) * (testPoint.x - point1.x));
}

bool isInTrig(Vec3 point, Vec3 a, Vec3 b, Vec3 c) {
  double eps = 1e-12;
  double c1 = crossProduct(a, b, point);
  double c2 = crossProduct(b, c, point);
  double c3 = crossProduct(c, a, point);

  bool has_neg = (c1 < -eps) || (c2 < -eps) || (c3 < -eps);
  bool has_pos = (c1 > eps) || (c2 > eps) || (c3 > eps);
  return !(has_neg && has_pos);
}

static inline Vertex vertAtID(Polygon mesh, uint32_t ID, uint32_t *IDList) {
  return mesh.verticies[IDList[ID]];
}

uint32_t *triangulateMesh(Polygon mesh) {
  uint32_t *indecies = malloc(mesh.vertexCount * (sizeof(uint32_t)));
  uint32_t indecies_used = 0;
  uint32_t *EBO = malloc(((mesh.vertexCount) - 2) * 3 * (sizeof(uint32_t)));
  uint32_t EBO_used = 0;
  // fill indecies
  for (uint32_t i = 0; i < mesh.vertexCount; i++) {
    indecies[indecies_used++] = i;
  }

  // repeat untill only one trig left
  while (indecies_used > 3) {
    // iterate through indecies
    for (uint32_t vertID = 0; vertID < indecies_used; vertID++) {
      uint32_t vert1 = vertID;
      uint32_t vert2 = (vertID + 1) % indecies_used;
      uint32_t vert3 = (vertID + 2) % indecies_used;
      // check if its counter clockwise
      Polygon tmpMesh = newPolygon(3);
      tmpMesh.verticies[0] = vertAtID(mesh, vert1, indecies);
      tmpMesh.verticies[1] = vertAtID(mesh, vert2, indecies);
      tmpMesh.verticies[2] = vertAtID(mesh, vert3, indecies);
      if (!isCCW(tmpMesh)) {
        free(tmpMesh.verticies);
        continue;
      }
      free(tmpMesh.verticies);

      // check if no vertex is inside it
      bool isEar = true;
      for (uint32_t testVert = 0; testVert < indecies_used; testVert++) {
        if (testVert == vert1 || testVert == vert2 || testVert == vert3) {
          continue;
        }
        if (isInTrig(vertAtID(mesh, testVert, indecies).coords,
                     vertAtID(mesh, vert1, indecies).coords,
                     vertAtID(mesh, vert2, indecies).coords,
                     vertAtID(mesh, vert3, indecies).coords)) {
          isEar = false;
          break;
        }
      }
      if (isEar) {

        // add to EBO
        EBO[EBO_used++] = indecies[vert1];
        EBO[EBO_used++] = indecies[vert2];
        EBO[EBO_used++] = indecies[vert3];

        // remove middle vertex from indecies
        if (vert2 + 1 < indecies_used) {
          memmove(&indecies[vert2], &indecies[vert2 + 1],
                  (indecies_used - (vert2 + 1)) * sizeof(uint32_t));
        }

        indecies_used--;
        break;
      }
    }
  }
  EBO[EBO_used++] = indecies[0];
  EBO[EBO_used++] = indecies[1];
  EBO[EBO_used++] = indecies[2];
  free(indecies);
  return EBO;
}
