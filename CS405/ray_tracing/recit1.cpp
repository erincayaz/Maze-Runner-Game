#include <iostream>
#include <fstream>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
using namespace std;

bool isCircleHit(glm::vec3 coord, unsigned r, glm::vec3 center)
{
    const float distance = glm::distance(coord, center);
    return distance <= r;
}

int main()
{



    // image
    const int W = 800, H = 600;

    // circle
    unsigned r = 100;
    glm::vec3 center(400, 300, 0);

    // render
    ofstream output("output.ppm");
    output << "P3" << endl
        << W << " " << H << endl
        << 255 << endl;

    for (int y = H - 1; y >= 0; y--)
    {
        for (int x = 0; x < W; x++)
        {
            glm::vec3 coord(x, y, 0);

            glm::vec3 color =
                isCircleHit(coord, r, center)
                ? glm::vec3(255)
                : glm::vec3(50, 50, 50);

            output << color.r << " " << color.g << " " << color.b << endl;
        }
    }

    output.close();
    return 0;
}