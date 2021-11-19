#ifndef RAY_H
#define RAY_H
#include <glm/vec3.hpp>

struct Ray
{
    glm::vec3 orig;
    glm::vec3 dir;

    Ray(glm::vec3 origin, glm::vec3 direction):
    orig(origin), dir(direction){}

    glm::vec3 at(float t)
    {
        return orig + t * dir;
    };
};

struct hitInfo {
    glm::vec3 n;
    glm::vec3 p;
    float t;
};

#endif