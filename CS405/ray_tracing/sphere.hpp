#ifndef SPHERE_H
#define SPHERE_H
#include <iostream>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "hittable.hpp"
#include "ray.hpp"

class sphere : public hittable
{
public:
    glm::vec3 center;
    float radius;

    sphere(glm::vec3 cen, double r) : center(cen), radius(r){};
    bool hit(Ray ray, hitInfo &info) override;
};

bool sphere::hit(Ray ray, hitInfo& info) {
    
    glm::vec3 w = ray.orig - center;
    float a = glm::dot(ray.dir, ray.dir);
    float b = 2.0f * glm::dot(ray.dir, w);
    float c = glm::dot(w,w) - radius*radius;

    float discriminant = b*b - 4*a*c;

    if(discriminant <= 0) {
        return false;
    }

    const float t = (-b - sqrt(discriminant)) / (2.0 * a);
    info.p = ray.at(t);
    info.n = info.p - center;
    info.t = t;

    return t > 0;
}

#endif