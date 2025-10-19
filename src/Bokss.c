#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bokss.h"

Vec2 newVec2(float x, float y) {
  Vec2 result = {
      x,
      y,
  };
  return result;
}

Vec3 newVec3(float x, float y, float z) {
  Vec3 result = {
      x,
      y,
      z,
  };
  return result;
}

Vertex newVertex(float x, float y, float r, float g, float b) {
  Vertex result = {
      newVec2(x, y),
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
  Vec2 normal = {
      cos(angle),
      sin(angle),
  };
  result.normal = normal;
  result.offset = offset;
  return result;
}

bool inPolygon(Vec2 pos, Polygon polygon) {
  int intersections = 0;
  for (int edgeID = 0; edgeID < polygon.vertexCount; edgeID++) {
    Vec2 vert1 = polygon.verticies[edgeID].coords;
    Vec2 vert2 = polygon.verticies[(edgeID + 1) % polygon.vertexCount].coords;
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
  for (int i = 0; i < polygon.vertexCount; i++) {
    float x1 = polygon.verticies[i].coords.x;
    float y1 = polygon.verticies[i].coords.y;
    float x2 = polygon.verticies[(i + 1) % polygon.vertexCount].coords.x;
    float y2 = polygon.verticies[(i + 1) % polygon.vertexCount].coords.y;
    area += (x1 * y2) + (x2 * y1);
  }
  return area > 0;
}

static inline double crossProduct(Vec2 point1, Vec2 point2, Vec2 testPoint) {
  return ((point2.x - point1.x) * (testPoint.y - point1.y)) -
         ((point2.y - point1.y) * (testPoint.x - point1.x));
}

bool isInTrig(Vec2 point, Vec2 a, Vec2 b, Vec2 c) {
  double eps = 1e-12;
  double c1 = crossProduct(a, b, point);
  double c2 = crossProduct(b, c, point);
  double c3 = crossProduct(c, a, point);

  bool has_neg = (c1 < -eps) || (c2 < -eps) || (c3 < -eps);
  bool has_pos = (c1 > eps) || (c2 > eps) || (c3 > eps);
  return !(has_neg && has_pos);
}

size_t *triangulateMesh(Polygon mesh) {
  size_t *result = malloc(((mesh.vertexCount - 2) * 3) * sizeof(size_t));
  if (!result) {
    return NULL;
  }
  size_t resultUsed = 0;
  while (mesh.vertexCount > 3) {
    for (int verID = 0; verID < mesh.vertexCount; verID++) {
      Polygon tmpPoly = newPolygon(3);
      if (!tmpPoly.verticies) {
        return NULL;
      }
      tmpPoly.verticies[0] = mesh.verticies[verID];
      tmpPoly.verticies[1] = mesh.verticies[(verID + 1) % mesh.vertexCount];
      tmpPoly.verticies[2] = mesh.verticies[(verID + 2) % mesh.vertexCount];
      if (!isCCW(tmpPoly)) {
        continue;
      }
      bool ear = true;
      for (int testVer = 0; testVer < mesh.vertexCount; testVer++) {
        if (testVer == verID || testVer == (verID + 1) % mesh.vertexCount ||
            testVer == (verID + 2) % mesh.vertexCount) {
          continue;
        }
        if (isInTrig(mesh.verticies[testVer].coords,
                     tmpPoly.verticies[0].coords, tmpPoly.verticies[1].coords,
                     tmpPoly.verticies[2].coords)) {
          ear = false;
          break;
        }
      }
      free(tmpPoly.verticies);
      if (ear) {
        result[resultUsed++] = verID;
        result[resultUsed++] = (verID + 1) % mesh.vertexCount;
        result[resultUsed++] = (verID + 2) % mesh.vertexCount;

        memmove(&mesh.verticies[(verID + 1) % mesh.vertexCount],
                &mesh.verticies[(verID + 2) % mesh.vertexCount],
                mesh.vertexCount - (verID + 2) % mesh.vertexCount);
        mesh.vertexCount--;
        break;
      }
    }
  }
  result[resultUsed++] = mesh.verticies[0];
  result[resultUsed++] = mesh.verticies[1];
  result[resultUsed++] = mesh.verticies[2];
}
