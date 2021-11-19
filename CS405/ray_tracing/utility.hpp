#ifndef UTILITY_H
#define UTILITY_H
#include <glm/vec3.hpp>
#include <fstream>
#include <vector>
#include "ray.hpp"
#include <limits>

using namespace std;

void writeImage(glm::vec3 color, std::ofstream& output)
{
    int r = min(color.r * 255.0f, 255.0f);
    int g = min(color.g * 255.0f, 255.0f);
    int b = min(color.b * 255.0f, 255.0f);

    output << r << " " << g << " " << b << endl;
}

float randFloat() {
    return 2.0 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 1;
}

#endif