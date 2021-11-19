#ifndef HITTABLE_H
#define HITTABLE_H
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "ray.hpp"
#include <vector>
class hittable
{
public:
    virtual bool hit(Ray r, hitInfo &info) { return false; };

    friend class HittableList;
};

class HittableList
{
public:
    std::vector<hittable* > objects;

    HittableList() {}
    void add(hittable *object);
    bool hit(Ray r, hitInfo &rec);
};

void HittableList::add(hittable *object)
{
    objects.push_back(object);
}

bool HittableList::hit(Ray r, hitInfo& rec) {
    hitInfo tempRec;
    float minDistance = std::numeric_limits<float>::max();
    bool hit_anything = false;

    for (const auto& object : objects) {
        if (object -> hit(r, tempRec)) {
            hit_anything = true;
            const float distance = glm::distance(tempRec.p, r.orig);
            if(distance < minDistance) {
                minDistance = distance;
                rec = tempRec;
            }
        }
    }

    return hit_anything;
}

#endif