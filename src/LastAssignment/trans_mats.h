#ifndef TRANS_MATS_H
#define TRANS_MATS_H
#include <glm/glm.hpp>

struct TransMats 
{
    glm::mat4 viewMat;
    glm::mat4 projMat;
    glm::mat4 acRotMat;
    glm::mat4 acTransMat;
    glm::mat4 acScaleMat;
    float acScale = 1.0f;
};

#endif 