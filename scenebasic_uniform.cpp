#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

using glm::vec3;

float randFloat() {
    return ((float)rand() / RAND_MAX);
}

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f) {

}

void SceneBasic_Uniform::initScene()
{
    compile();
    startTime = clock();
    std::cout << std::endl;

    prog.printActiveUniforms();
    plane = new Plane(10, 10, 10, 10, 1, 1);
    /////////////////// Create the VBO ////////////////////
    float positionData[] = {
        -1, -1, 0.0f,
     1, -1, 0.0f,
    -1,  1, 0.0f,
     1,  1, 0.0f };
    float colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f };

    // Create and populate the buffer objects
    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint colorBufferHandle = vboHandles[1];

    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), positionData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), colorData, GL_STATIC_DRAW);

    // Create and set-up the vertex array object
    glGenVertexArrays( 1, &vaoHandle );
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);  // Vertex position
    glEnableVertexAttribArray(1);  // Vertex color

    #ifdef __APPLE__
        glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );

        glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL );
    #else
    		glBindVertexBuffer(0, positionBufferHandle, 0, sizeof(GLfloat)*3);
    		glBindVertexBuffer(1, colorBufferHandle, 0, sizeof(GLfloat)*3);

    		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    		glVertexAttribBinding(0, 0);
    		glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
    	  glVertexAttribBinding(1, 1);
    #endif
    glBindVertexArray(0);


    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set the point size
    glPointSize(10.0f);

    angle = glm::half_pi<float>();

    const char* texName = "./media/bluewater.png";
    glActiveTexture(GL_TEXTURE0);
    Texture::loadTexture(texName);

    const char* texName1 = "./media/fire.png";
    glActiveTexture(GL_TEXTURE1);
    Texture::loadTexture(texName1);


    nParticles = 8000;

    // Generate the buffers
    glGenBuffers(1, &initVel);   // Initial velocity buffer
    glGenBuffers(1, &startTimeBuffer); // Start time buffer

    // Allocate space for all buffers
    int size = nParticles * 3 * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, startTimeBuffer);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), NULL, GL_STATIC_DRAW);

    // Fill the first velocity buffer with random velocities
    vec3 v(0.0f);
    float velocity, theta, phi;
    GLfloat* data = new GLfloat[nParticles * 3];
    for (unsigned int i = 0; i < nParticles; i++) {

        theta = glm::mix(0.0f, glm::pi<float>() / 6.0f, randFloat());
        phi = glm::mix(0.0f, glm::two_pi<float>(), randFloat());

        v.x = sinf(theta) * cosf(phi);
        v.y = cosf(theta);
        v.z = sinf(theta) * sinf(phi);

        velocity = glm::mix(1.25f, 1.5f, randFloat());
        v = glm::normalize(v) * velocity;

        data[3 * i] = v.x;
        data[3 * i + 1] = v.y;
        data[3 * i + 2] = v.z;
    }
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

    // Fill the start time buffer
    delete[] data;
    data = new GLfloat[nParticles];
    float time = 0.0f;
    float rate = 0.00075f;
    for (unsigned int i = 0; i < nParticles; i++) {
        data[i] = time;
        time += rate;
    }
    glBindBuffer(GL_ARRAY_BUFFER, startTimeBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    delete[] data;

    glGenVertexArrays(1, &particles);
    glBindVertexArray(particles);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTimeBuffer);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Array for quad
    GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);

    // Set up the vertex array object
    glGenVertexArrays(1, &quad);
    glBindVertexArray(quad);

    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte*)NULL + (0)));
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte*)NULL + (0)));
    glEnableVertexAttribArray(2);  // Texture coordinates

    glBindVertexArray(0);
    progWood.use();
    //progWood.setUniform("Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    progWood.setUniform("NoiseTex", 2);

    glm::mat4 slice;
    slice = glm::rotate(slice, glm::radians(1.0f), vec3(1.0, 0.0, 0.0));
    slice = glm::rotate(slice, glm::radians(-2.0f), vec3(0.0, 0.0, 1.0));
    slice = glm::scale(slice, vec3(0.0, 4.0, 1.0));
    slice = glm::translate(slice, vec3(-0.35, -0.5, 2.0));

    //progWood.setUniform("Slice", slice);

    noiseTex = NoiseTex::generate2DTex();
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);


}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
        progStar.compileShader("shader/stars.vert");
        progStar.compileShader("shader/stars.frag");
        progStar.link();
        progStar.use();

        progParticles.compileShader("shader/particles.vert");
        progParticles.compileShader("shader/particles.frag");
        progParticles.link();
        progParticles.use();

        progWood.compileShader("shader/wood.vert");
        progWood.compileShader("shader/wood.frag");
        progWood.link();
        progWood.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
	//update your angle here
    iTime = t;

}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    //create the rotation matrix here and update the uniform in the shader 
    progStar.use();
    progStar.setUniform("projection", projection);
    progStar.setUniform("view", view);

    glm::vec2 iResolution = glm::vec2(1280, 960);
    progStar.setUniform("iResolution", iResolution);
    //float iTime = GLfloat(clock() - startTime) / CLOCKS_PER_SEC;
    progStar.setUniform("iTime", iTime);

    plane->render();
    glBindVertexArray(vaoHandle);
    //glDrawArrays(GL_TRIANGLES, 0, 3 );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);


    progParticles.use();
    view = glm::lookAt(vec3(3.0f * cos(angle), 1.5f, 3.0f * sin(angle)), vec3(0.0f, 1.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::mat4(1.0f);
    glm::mat4 mv = view * model;
    progParticles.setUniform("MVP", projection * mv);
    progParticles.setUniform("ParticleTex", 0);
    progParticles.setUniform("ParticleLifetime", 3.5f);
    progParticles.setUniform("Gravity", vec3(0.0f, -0.5f, 0.0f));
    progParticles.setUniform("Time", iTime);
    glBindVertexArray(particles);
    glDrawArrays(GL_POINTS, 0, nParticles);

    progWood.use();
    model = glm::mat4(1.0f);
    model = glm::scale(model, vec3(0.25, 0.25, 1.0));
    mv = view * model;
    progWood.setUniform("MVP", projection * mv);
    glBindVertexArray(quad);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    
}

void SceneBasic_Uniform::resize(int w, int h)
{
  
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.3f, 100.0f);
}
