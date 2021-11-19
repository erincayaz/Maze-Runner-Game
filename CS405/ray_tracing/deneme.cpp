#include <iostream>
#include <fstream>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "ray.hpp"
#include "sphere.hpp"1

using namespace std;

glm::vec3 rayColor(Ray& ray, HittableList& world) {
    hitInfo info;

    if (world.hit(ray, info)) {
        const glm::vec3 normal = (glm::normalize(info.n) + glm::vec3(1)) * 0.5f;

        return normal;
    }

    float y = glm::normalize(ray.dir).y;
    float t = (y + 1) * 0.5;

    return (1.0f - t) * glm::vec3(1) + t * glm::vec3(0.5, 0.7, 1.0);
}

void writeImage(ofstream& file, glm::vec3 color)
{
    int r = min(color.r * 255.0f, 255.0f);
    int g = min(color.g * 255.0f, 255.0f);
    int b = min(color.b * 255.0f, 255.0f);

    file << r << " " << g << " " << b << std::endl;
}

int main()
{
    // image
    const float aspectRatio = 16.0 / 9.0;
    const int imageWidth = 800, imageHeight = imageWidth / aspectRatio;
    ofstream imageFile("output.ppm");

    // camera
    glm::vec3 origin(0, 0, 0);
    glm::vec3 horizontal(1, 0, 0);
    glm::vec3 vertical(0, 1, 0);
    const float focalLength = 1;
    const float wpHeight = 2.0;
    const float wpWidth = wpHeight * aspectRatio;
    const glm::vec3 leftCorner = origin - glm::vec3(0, 0, focalLength) - glm::vec3(0, wpHeight * 0.5, 0) - glm::vec3(wpWidth * 0.5, 0, 0);

    // scene
    HittableList scene;
    scene.add(new sphere(glm::vec3(0, 0, -1), 0.5));
    scene.add(new sphere(glm::vec3(0, -100.5, -1), 100));

    imageFile << "P3" << endl << imageWidth << endl << imageHeight << endl << 255 << endl;
    // render
    for (int y = imageHeight - 1; y >= 0; y--)
    {
        for (int x = 0; x < imageWidth; x++)
        {
            float u = (x / (imageWidth - 1.0)) * wpWidth;
            float v = (y / (imageHeight - 1.0)) * wpHeight;
            glm::vec3 point = leftCorner + v * vertical + u * horizontal;
            glm::vec3 dir = point - origin;

            Ray ray(origin, dir);

            glm::vec3 color = rayColor(ray, scene);
            writeImage(imageFile, color);
        }
    }
    return 0;
}