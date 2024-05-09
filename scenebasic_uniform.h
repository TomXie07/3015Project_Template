#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "plane.h"
#include "texture.h"
#include "noisetex.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLSLProgram progParticles;
    GLSLProgram prog;
    GLSLProgram progStar;
    GLSLProgram progWood;

    GLuint quad;
    float angle;
    float startTime;
    float iTime;
    Plane *plane;

    GLuint noiseTex;
    GLuint initVel, startTimeBuffer, particles;
    GLuint nParticles;
    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
