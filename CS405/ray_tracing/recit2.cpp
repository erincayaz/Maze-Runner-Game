#include <iostream>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
using namespace std;

struct Ray
{
    glm::vec3 orig;
    glm::vec3 dir;

    Ray(glm::vec3 origin, glm::vec3 direction) : orig(origin), dir(direction) {}

    glm::vec3 at(float t)
    {
        return orig + t * dir;
    };
};

bool hitsSphere(Ray &ray, glm::vec3 center, float radius)
{
    glm::vec3 w = ray.orig - center;
    float a = glm::dot(ray.dir, ray.dir);
    float b = 2.0f * glm::dot(ray.dir, w);
    float c = glm::dot(w, w) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    return discriminant > 0;
}

glm::vec3 rayColor(Ray &ray)
{
    if (hitsSphere(ray, glm::vec3(0, 0, -1), 0.5))
    {
        return glm::vec3(1, 0, 1);
    }

    float y = glm::normalize(ray.dir).y;
    float t = (y + 1) * 0.5;

    return (1.0f - t) * glm::vec3(1) + t * glm::vec3(0.5, 0.7, 1.0);
}

void writeImage(ofstream & file, glm::vec3 color)
{
    int r = std::min(color.r * 255.0f, 255.0f);
    int g = std::min(color.g * 255.0f, 255.0f);
    int b = std::min(color.b * 255.0f, 255.0f);

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

            writeImage(imageFile, rayColor(ray));
        }
    }
    return 0;
}