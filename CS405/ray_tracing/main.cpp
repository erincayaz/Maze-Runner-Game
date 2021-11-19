#include <iostream>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include "utility.hpp"
#include "sphere.hpp"
using namespace std;

glm::vec3 rayColor(Ray & ray, HittableList& world, int depth) {
    if(depth <= 0) {
        return glm::vec3(0);
    }

    hitInfo info;
    if(world.hit(ray, info)) {
        const glm::vec3 normal = (glm::normalize(info.n) + glm::vec3(1)) * 0.5f;
        const glm::vec3 target = info.p + normal + glm::vec3(randFloat(), randFloat(), randFloat());
        Ray newRay(info.p, target - info.p);
        
        return 0.5f * rayColor(newRay, world, depth - 1);
    } 

    float y = glm::normalize(ray.dir).y;
    float t = (y + 1) * 0.5;
    
    return (1.0f-t)*glm::vec3(1) + t*glm::vec3(0.5, 0.7, 1.0);
}

int main()
{

    // image
    const float aspectRatio = 16.0 / 9.0;
    const int imageWidth = 800, imageHeight = imageWidth / aspectRatio;
    const int depth = 100;
    const int sample = 10;

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
    scene.add(new sphere(glm::vec3(-1.5,0,-1), 0.5));
    scene.add(new sphere(glm::vec3(-1,-100.5,-1), 100));

    // output
    ofstream output("output.ppm");
    output << "P3" << endl;
    output << imageWidth << " " << imageHeight << endl;
    output << 255 << endl;

    // render
    for (int y = imageHeight-1; y >= 0; y--)
    {
        for (int x = 0; x < imageWidth; x++)
        {
            glm::vec3 color(0);
            for(int s = 0; s < sample; s++)
            {
                float u = ((x + randFloat()) / (imageWidth - 1.0)) * wpWidth;
                float v = ((y + randFloat()) / (imageHeight - 1.0)) * wpHeight;
                glm::vec3 point = leftCorner + v*vertical + u*horizontal;
                glm::vec3 dir = point - origin;
                Ray ray(origin, dir);

                color += rayColor(ray, scene, depth);
            }

            writeImage(sqrt(color * (1.0f/sample)), output);
        }
    }
    cout << "Rendered" << endl;

    output.close();
    return 0;
}
